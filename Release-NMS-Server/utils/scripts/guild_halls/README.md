# Guild halls: scripts and SQL

Phase 1 of the design in `docs/guild_halls/GUILD_HALLS.md`: Infamy, the guild treasury and the T1 hall (the
existing guild hall zone). No server code changes; Phase 2 (map entry, guild bank outside the guild hall zone,
halls that don't expire) needs a rebuild.

## The switch

Guild halls are **off** until a GM turns them on. Hail the Guild Steward in the Bazaar as a GM and click
**turn on** or **turn off** (or say `guild halls on` / `guild halls off`). It's the data bucket
`guild_halls_enabled` (`1` = on).

While off:

- The Steward and the porter answer GMs only (account status 80 or higher).
- Only GM characters earn Infamy, and only GMs see the guild messages.
- Nobody else's guild hall access changes.

Test Infamy and purchases with GM characters in a test guild. A GM in a real guild adds to that guild's real
totals and uses up its first-kill bonuses.

## Files

| Here | Live location |
|---|---|
| `plugins/guild_halls.pl` | `plugins/` |
| `quests/bazaar/Guild_Steward.pl` (npc 1200124) | `quests/bazaar/` |
| `quests/bazaar/Guild_Hall_Porter.pl` (npc 1200118) | `quests/bazaar/` |
| `quests/highkeep/Guild_Hall_Mystic.pl` (npc 1200119) | `quests/highkeep/` |
| `quests/highkeep/Hall_Hearthkeeper.lua` (npc 1200122, regen pool) | `quests/highkeep/` |
| `quests/highkeep/Hall_Doorman.pl` (npc 1200123, back to the Bazaar) | `quests/highkeep/` |
| `sql/2026_11_13_01_guild_halls_phase1.sql` | Tables and the raid boss list |
| `sql/2026_11_13_02_guild_halls_npcs.sql` | The Steward and the Porter. Run `#zoneshutdown zone bazaar` first |
| `sql/2026_11_13_03_highkeep_hall.sql` | The T1 hall: Highkeep version 60 and its NPCs. Apply while Highkeep is down |

Hooks in the rift snapshot (`utils/scripts/rift`):

- `quests/global/global_npc.pl` EVENT_DEATH_COMPLETE calls `plugin::GuildInfamyNPCDeath`.
- `quests/global/global_player.pl` EVENT_LEVEL_UP calls `plugin::GuildInfamyMaxLevel`.
- `plugins/raid_portal.pl` RaidPortalDeath calls `plugin::GuildInfamyRaid` once per paid raid.
- `lua_modules/rift_core.lua` `rift.complete` sets `gr_cleared_<instance>`, which the Guardian's death reads.

Deploy the plugin before the hooks: a hook that calls a missing plugin sub errors on every NPC death.

## Infamy

Settings are at the top of `plugins/guild_halls.pl`. Each award is per guild member credited, grouped by guild.

| Source | Infamy | Who is credited |
|---|---|---|
| Rift clear (Guardian killed in time) | tier / 10, at least 1 | Everyone in the rift |
| Raid portal boss | 25 x raid tier | Everyone assigned to the raid |
| Open-world raid boss (`nms_guild_infamy_bosses`) | 10 / 15 / 20 / 25 / 30 by era | The killer (or pet owner) and their raid or group in the zone |
| Guild's first kill of a boss | 5x, once per boss | |
| Member reaches `Character:MaxLevel` | 100, once per character | |

Open-world bosses in raid portal or rift instances don't pay through the boss list. A boss with several phases
pays once per zone copy per 30 minutes.

## The T1 hall: Highkeep

T1 is Highkeep's third floor (the noble quarters), zone version 60, one private instance per guild. The Porter
creates the guild's instance on first entry (data bucket `gh_hall_instance_<guild>_highkeep_60`), assigns the
member and renews it to 90 days on every entry. Version 60 has none of Highkeep's NPCs and no zone exits; its doors
are copied unlocked. In the hall: the Guild Hall Mystic, a banker (personal bank), a guild tribute master, the
Hearthkeeper (regen pool, like the guild hall's Aporia) and the Doorman (back to the Bazaar). The guild bank only
works in the old guild hall zone until Phase 2. NPC positions are first guesses until the #loc walk-through.

## Guild Hall Mystic (hall buffs)

Stands in the guild hall. A member who hails it gets every spell of the guild's blessings tier, and so does their
pet, at normal duration and with normal stacking. No cooldown, and no levitation spells. The leader buys tiers in order at the Steward;
the guild must own a hall. Spells and prices are `%GH_BUFF_TIERS` in the plugin.

| Tier | Infamy | Platinum | Spells |
|---|---:|---:|---|
| 1 | 2,500 | 100,000 | Temperance, Talisman of Altuna, Clarity, Swift Like the Wind, Shield of Thorns, Spirit of Wolf |
| 2 | 7,500 | 250,000 | Aegolism, Talisman of Kragg, Clarity II, Wondrous Rapidity, Shield of Blades, Spirit of Wolf |
| 3 | 15,000 | 500,000 | Virtue, Focus of Spirit, Gift of Brilliance, Visions of Grandeur, Flameshield of Ro, Spirit of Wolf |
| 4 | 30,000 | 1,000,000 | Virtue, Focus of Soul, Voice of Quellious, Speed of Vallon, Maelstrom of Ro, Protection of the Nine, Spirit of Wolf |
| 5 | 50,000 | 2,000,000 | Conviction, Focus of Soul, Clairvoyance, Speed of Salik, Nettle Shield, Ferocity, Spirit of the Predator, Spirit of Wolf |

## Tables

- `nms_guild_halls`: Infamy, treasury platinum and hall tier per guild.
- `nms_guild_infamy_log`, `nms_guild_treasury_log`: every award, donation and purchase.
- `nms_guild_unlocks`: amenities bought and their level (`buffs` = Mystic tier).
- `nms_guild_first_kills`, `nms_guild_max_level`: one-time awards already paid.
- `nms_guild_infamy_bosses`: the open-world bosses that pay, by era.

To clear test data before going live:

```sql
TRUNCATE nms_guild_halls; TRUNCATE nms_guild_infamy_log; TRUNCATE nms_guild_treasury_log;
TRUNCATE nms_guild_first_kills; TRUNCATE nms_guild_max_level; TRUNCATE nms_guild_unlocks;
```
