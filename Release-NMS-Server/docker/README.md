# Voidforged in Docker

Runs the game server (world, zone launcher, shared memory) and MariaDB on Linux. The login server is separate:
world registers with whichever one `LOGIN_HOST` names.

## Setup

1. `cp .env.example .env` and fill it in.
2. Put the runtime folder at `RUNTIME_DIR`: `quests`, `lua_modules`, `maps`, `plugins` and `opcodes.conf`, laid out
   like a Windows `Build/bin/Release`. Lua modules are read from `lua_modules`, not `quests/lua_modules`.
3. Build: `docker compose build`. The first build takes a long time with `BUILD_JOBS=1`.
4. Start the database and load your dump:
   ```
   docker compose up -d db
   docker compose exec -T db sh -c 'mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" --max-allowed-packet=256M "$MARIADB_DATABASE"' < dump.sql
   ```
5. `docker compose up -d`

Open UDP 9000 and 7001-7030 to players. Keep the database closed: this compose file doesn't publish its port.

## Notes

- **Logs:** in the `data` volume, under `/data/logs` in the server container.
- **Client spell files:** export them with `export_client_files` from this image, not with Spire.
- **Zone count:** `eqlaunch zone` starts what the `launcher` table says. Each idle zone uses about 100 MB, more with
  players in it.
- **Backups aren't set up here yet.** Dump the database on a schedule before real players use it.
