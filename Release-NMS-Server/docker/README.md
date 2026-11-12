# Voidforged in Docker

Runs the game server (world, zone launcher, shared memory), the login server and MariaDB on Linux.

## Setup

1. `cp .env.example .env` and fill it in.
2. Put the runtime folder at `RUNTIME_DIR`: `quests`, `lua_modules`, `maps`, `plugins` and `opcodes.conf`, laid out
   like a Windows `Build/bin/Release`. Lua modules are read from `lua_modules`, not `quests/lua_modules`.
3. Build: `docker compose build`. The first build takes a long time with `BUILD_JOBS=1`.
4. Start the database and load your dumps. `login.sql` needs the login server's tables: `login_accounts`,
   `login_api_tokens`, `login_server_admins`, `login_server_list_types` and `login_world_servers`
   (`loginserver/login_util/login_schema.sql` creates them empty).
   ```
   docker compose up -d db
   docker compose exec -T db sh -c 'mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" --max-allowed-packet=256M "$MARIADB_DATABASE"' < dump.sql
   docker compose exec -T db sh -c 'mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" -e "CREATE DATABASE IF NOT EXISTS voidforged_login"'
   docker compose exec -T db sh -c 'mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" voidforged_login' < login.sql
   ```
5. `docker compose up -d`

Open UDP 5998, 5999, 9000 and 7001-7030 to players, and TCP `PATCH_PORT` (8088) for VoidPatcher downloads
(`utils/patcher`). Keep the database closed: this compose file doesn't publish its port.

## Notes

- **Logs:** in the `data` volume, under `/data/logs` in the server container.
- **Client spell files:** export them with `export_client_files` from this image, not with Spire. It leaves out spell
  IDs the client can't handle (see `common/repositories/spells_new_repository.h`).
- **Zone count:** `eqlaunch zone` starts what the `launcher` table says. Each idle zone uses about 75 MB, more with
  players in it.
- **Backups:** `backup.sh` dumps both databases to `~/voidforged/backups` and keeps 14 days (`BACKUP_DIR` and `KEEP_DAYS`
  change that). Run it from cron and copy the dumps to another machine; they hold player data and password hashes.

## Discord bridge

The `discord` service mirrors server-wide OOC to one Discord channel and back. Discord messages show in game as
`*discordbridge says out of character, 'Name: text'`.

1. In the Discord developer portal, create an application, add a bot, turn on **Message Content Intent**, and invite
   it to your server with View Channel, Send Messages and Read Message History on the OOC channel.
2. Put the bot token and the channel's ID (Developer Mode, right-click the channel, Copy Channel ID) in `.env` as
   `DISCORD_TOKEN` and `DISCORD_CHANNEL_ID`, and set a random `CONSOLE_PASSWORD`.
3. Create the console account in peq (status 50 is the console minimum; the account can't log into the game):
   ```
   docker compose exec -T db sh -c 'mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" "$MARIADB_DATABASE"' <<SQL
   INSERT INTO account (name, ls_id, password, status) VALUES ('discordbridge', 'eqemu', MD5('<CONSOLE_PASSWORD>'), 50);
   SQL
   ```
4. `docker compose up -d --build discord`, then `docker compose logs discord` should show both logins.

World's console (TCP 9001) is enabled by `start.sh` but not published. Set `CONSOLE_ENABLED=false` on the server
service to turn it off.
