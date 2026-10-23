# Greater Rifts: scripts, generators and SQL

Snapshot of the Greater Rift content. The live copies run from the server's runtime folder
(`Build/bin/Release`), which git ignores; keep these in sync when you change them.

| Here | Live location |
|---|---|
| `quests/global/Rift_*.lua` | `quests/global/` |
| `quests/thevoida/459000.lua` (Zebuxoruk: story, tier menu, re-enter, close) | `quests/thevoida/` |
| `lua_modules/*.lua` | `lua_modules/` (the server only searches this folder, **not** `quests/lua_modules`) |
| `start_server.bat` | `Build/bin/Release/` (sets Perl 5.26 paths so plugins like `plugin::LoadMysql` work) |

## Settings

All rift settings (hub location, time limit, tier cap, rewards and currency, rift NPC ids, champion and
Guardian spells and timers) live in `lua_modules/rift_config.lua`; `rift_core.lua`,
`champion_affixes.lua` and `Rift_Guardian.lua` read it. The Void gate's settings (zone, access bucket,
where unflagged players are sent) are named at the top of `quests/global/global_player.pl`; set
`$VOID_ACCESS_BUCKET = ''` for no gate. Server rules: `Custom:KillFactionHits`,
`Custom:ArmorSetTierVisuals`, `Custom:BlockGlamourResale`, `Custom:DynamicItemDisplayIds`,
`Merchant:DynamicItemSellRefundPct`.

## Generators

Both read the database settings from `eqemu_config.json` (password included; nothing is stored here).

* `gen_rift_zones.py <eqemu_config.json> <out.sql> <out rift_zones.lua> [--zones a,b,c] [--entry zone:x,y,z]`: per zone in its `ZONES`
  list, a rift spawn at every normal spawn location (180s +-30s respawn), the controller, doors /
  zone points / ground spawns copied to instance version 1, and a version-1 zone row with no
  flag / level / status requirement. Add a zone to the list, rerun, apply the SQL, deploy the Lua.
* `gen_rift_appearances.py <eqemu_config.json> <out rift_appearances.lua>`: per zone, the looks
  (race, texture, size, body type) of the creatures that normally spawn there, plus models the
  client loads in every zone (`GlobalLoad.txt`). The client only draws models loaded for the
  current zone, so other races show as the default model. Also per-zone boss looks for the Guardian.
* `luacheck.py <lua51.dll> <files...>`: compiles (doesn't run) Lua files with the server's own
  Lua, to catch syntax errors before deploying.

## SQL

`sql/` holds what was applied to the live database on 2026-10-22, in order, for reference or for
setting up another server: vendor and Zebuxoruk, archetype roles and the kill-faction rule,
respawn, The Void waypoint and the server-buff rule fix, mob levels, plus the class test sets and
their armor set bonuses.

Class armor sets: `../dynamic_items/gen_class_sets.py` builds the per-class pieces, their dynamic_items
rows, armor sets with bonuses and the token vendor list from `../dynamic_items/class_sets.json`
(`export` writes that file from a live database; this server's is checked in).
