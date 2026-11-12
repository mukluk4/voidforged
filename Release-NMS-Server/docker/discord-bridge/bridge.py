"""Two-way bridge between Voidforged's server-wide OOC chat and one Discord channel.

Game -> Discord: logs into world's telnet console, turns on `acceptmessages`, and posts every OOC line it sees.
Discord -> game: sends `ooc <name>: <text>` through the same console, so it shows in game as
    *<console account> says out of character, '<name>: <text>'

Settings come from the environment (see docker/.env.example):
    DISCORD_TOKEN, DISCORD_CHANNEL_ID, CONSOLE_HOST, CONSOLE_PORT, CONSOLE_USER, CONSOLE_PASSWORD
"""
import asyncio
import logging
import os
import re
import time

import discord

log = logging.getLogger("bridge")

# RoF2 item links: 0x12, a 77-character hex body, the item name, 0x12
ITEM_LINK = re.compile(r"\x12[0-9A-Fa-f]{77}([^\x12]*)\x12")
OOC_LINE = re.compile(r"^(\S+) says ooc, '(.*)'$")
CONTROL = re.compile(r"[\x00-\x1f\x7f]")

MAX_GAME_TEXT = 200  # the client cuts long chat lines; keep Discord messages short in game
USER_COOLDOWN = 2.0  # seconds between relayed messages from one Discord user


def game_to_discord(text: str) -> str:
    text = ITEM_LINK.sub(lambda m: f"[{m.group(1)}]", text)
    text = CONTROL.sub("", text)
    return discord.utils.escape_markdown(discord.utils.escape_mentions(text))


def discord_to_game(name: str, text: str) -> str:
    """Builds the console line. Newlines and control characters are removed: a newline would start another
    console command, and the console account can run admin commands."""
    name = re.sub(r"[^A-Za-z0-9_-]", "", name)[:20] or "Someone"
    text = CONTROL.sub(" ", text)
    # The EQ client is cp1252; drop what it can't show (emoji etc.)
    text = text.encode("cp1252", "ignore").decode("cp1252")
    text = " ".join(text.split())
    if len(text) > MAX_GAME_TEXT:
        text = text[: MAX_GAME_TEXT - 3] + "..."
    return f"ooc {name}: {text}" if text else ""


class Console:
    """World's telnet console. Reconnects forever; lines go to on_line."""

    def __init__(self, host, port, user, password, on_line):
        self.host, self.port, self.user, self.password = host, port, user, password
        self.on_line = on_line
        self.writer = None
        self.prompt = None

    async def run(self):
        delay = 5
        while True:
            try:
                await self._session()
                delay = 5
            except (OSError, asyncio.IncompleteReadError, ConnectionError) as e:
                log.warning("console connection lost: %s", e)
            except PermissionError as e:
                log.error("%s", e)
                delay = 300  # bad credentials; don't hammer world
            self.writer = None
            log.info("reconnecting to console in %ss", delay)
            await asyncio.sleep(delay)
            delay = min(delay * 2, 300)

    async def _session(self):
        reader, writer = await asyncio.open_connection(self.host, self.port)
        await self._expect(reader, b"Username: ")
        writer.write(self.user.encode() + b"\n")
        await self._expect(reader, b"Password: ")
        writer.write(self.password.encode() + b"\n")
        await writer.drain()
        # After login the console prints the prompt "<account name>> "
        try:
            reply = await asyncio.wait_for(reader.readuntil(b"> "), 15)
        except asyncio.IncompleteReadError as e:
            reply = e.partial  # world disconnects right after "Access denied"
        if b"Access denied" in reply or not reply.endswith(b"> "):
            raise PermissionError("console login refused: check CONSOLE_USER/CONSOLE_PASSWORD and account status >= 50")
        self.prompt = reply.decode("latin-1").strip().split()[-1]  # e.g. "discordbridge>"
        writer.write(b"acceptmessages on\n")
        await writer.drain()
        self.writer = writer
        log.info("logged into world console as %s", self.prompt[:-1])

        buf = b""
        while True:
            chunk = await reader.read(4096)
            if not chunk:
                raise ConnectionError("console closed the connection")
            buf += chunk
            *lines, buf = buf.split(b"\n")
            for raw in lines:
                line = raw.decode("cp1252", "replace").replace("\x08", "").strip("\r ")
                while line.startswith(self.prompt):
                    line = line[len(self.prompt):].lstrip()
                if line:
                    await self.on_line(line)

    @staticmethod
    async def _expect(reader, token):
        data = await asyncio.wait_for(reader.readuntil(token), 15)
        return data

    async def send(self, line: str) -> bool:
        if not self.writer:
            return False
        self.writer.write(line.encode("cp1252", "ignore") + b"\n")
        await self.writer.drain()
        return True


class Bridge(discord.Client):
    def __init__(self, channel_id, console_args):
        intents = discord.Intents.default()
        intents.message_content = True
        super().__init__(intents=intents, allowed_mentions=discord.AllowedMentions.none())
        self.channel_id = channel_id
        self.console = Console(*console_args, on_line=self.game_line)
        self.last_sent = {}

    async def setup_hook(self):
        self.loop.create_task(self.console.run())

    async def on_ready(self):
        log.info("logged into Discord as %s", self.user)

    async def game_line(self, line):
        m = OOC_LINE.match(line)
        if not m:
            return
        sender, text = m.groups()
        if sender.startswith("*"):
            return  # sent from a console (ours or a GM's); ours would echo forever
        channel = self.get_channel(self.channel_id)
        if channel is None:
            return
        try:
            await channel.send(f"**{sender}**: {game_to_discord(text)}")
        except discord.HTTPException as e:
            log.warning("couldn't post to Discord: %s", e)

    async def on_message(self, message: discord.Message):
        if message.channel.id != self.channel_id or message.author.bot or message.webhook_id:
            return
        now = time.monotonic()
        if now - self.last_sent.get(message.author.id, 0) < USER_COOLDOWN:
            return
        text = message.clean_content
        if not text and message.attachments:
            text = "[attachment]"
        line = discord_to_game(message.author.display_name, text)
        if not line:
            return
        if await self.console.send(line):
            self.last_sent[message.author.id] = now
        else:
            log.warning("console not connected; dropped a Discord message")


def main():
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(name)s: %(message)s")
    token = os.environ.get("DISCORD_TOKEN", "")
    channel = os.environ.get("DISCORD_CHANNEL_ID", "")
    if not token or not channel:
        log.info("DISCORD_TOKEN or DISCORD_CHANNEL_ID not set; bridge idle")
        while True:
            time.sleep(3600)
    console_args = (
        os.environ.get("CONSOLE_HOST", "server"),
        int(os.environ.get("CONSOLE_PORT", "9001")),
        os.environ["CONSOLE_USER"],
        os.environ["CONSOLE_PASSWORD"],
    )
    Bridge(int(channel), console_args).run(token, log_handler=None)


if __name__ == "__main__":
    main()
