# Greater Rifts: scripts, generators and SQL

Snapshot of the Greater Rift content. The live copies run from the server's runtime folder
(`Build/bin/Release`), which git ignores; keep these in sync when you change them.

| Here | Live location |
|---|---|
| `quests/global/Rift_*.lua` | `quests/global/` |
| `quests/thevoida/459000.lua` (Zebuxoruk: story, tier menu, re-enter, close) | `quests/thevoida/` |
| `quests/thevoida/1200050.lua` (Chronal Artificer: points players to `#reforge`) | `quests/thevoida/` |
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

## Platinum and AA sinks

`gen_sinks.py` writes `sql/2026_10_27_01_plat_aa_sinks.sql` and `client/dbstr_paragon.txt` (no arguments, no database access).
Every number below is at the top of the script. The SQL deletes its own rows first, so it can be rerun.

* **Chronal Tributes** (tributes 60-63, benefit items 980001-980035): Might, Arcana, Bulwark and Fortune, 5 tiers each.
  - Tiers stack: the tier selected turns on its band and every band below it. Tier III works in rift tiers 1-300 and
    gives the benefit of the rift's band (I's in a tier 50 rift, III's in a tier 250 rift).
    Above its top band, and outside rifts, it does nothing and costs nothing.
  - Cost: 100 favor x the rift tier, once per run, taken on entering the rift (or on turning the tribute on inside it).
    That is 10% of the run's platinum per tribute. The cost the tribute window shows is the top of the band.
  - Fortune adds 5% Chronal Shards per tier to the clear reward. It never adds platinum.
  - Code: `zone/nms_chronal_tribute.cpp`; rules `Custom:ChronalTribute*`. Fortune's numbers are in `rift_config.lua` (`reward.fortune_*`).
* **Paragon AAs** (abilities 17800-17811, ranks 19000-19559), General tab, level 51:
  - 50 ranks of +25 HP, mana or endurance, +1 AC or +1 attack: 5 AA for rank 1, +1 per rank (1,475 AA each).
  - 10 ranks of +1% melee damage: 25 AA, +10 per rank (700 AA).
  - 50 ranks of +1 of each heroic stat: 15 AA, +3 per rank (4,425 AA each).
  - Starting values are deliberately small; every number is at the top of `gen_sinks.py`.
  - Heroic stats use server-only AA effects 9001-9006 (`zone/nms_paragon.cpp`), counted like item heroics. They aren't sent to the client.
  - **Client file:** the names and descriptions are new `dbstr_us.txt` lines. Run
    `python client/add_dbstr.py "<EverQuest>/Resources/dbstr_us.txt"` on each player's client; it keeps a `.bak`.
* **Reforging** (`#reforge`, any player): pay platinum and Chronal Shards to replace one random line of a dynamic item.
  - Each paid reforge offers two new lines; take one or keep the current one.
  - The first line reforged is the only one on that item that can be reforged again.
  - Cost: 25,000 platinum and 50 shards x (2 x item tier - 1), +25% per reforge of the same item, up to 5x.
  - Paying and choosing need the Chronal Artificer (npc 1200050, The Void) targeted within 30 units; looking works anywhere.
  - Stored on the item as `dyn_ench` (and `dyn_offer` while a choice is pending); the seed is unchanged.
  - Rules `Custom:Reforge*`.

Apply: back up, run the SQL with the server down, and clear The Void's `zone_state_spawns` rows (zone state saving would
bring back the old spawns without the Artificer). Then run `shared_memory`, copy `1200050.lua` and the two `lua_modules` files, and add the client strings.
