# Raid Portal (design, in progress 2026-11-02)

A Raid Portal NPC in The Void sends a raid into a private instance to fight one existing boss, scaled by raid
tier and raid size. Voidforged only. The portal offers single boss fights, not whole events. Nothing here is
built yet, and every boss in the roster still needs an in-game test.

## Access

- The raid leader needs a deepest Greater Rift clear of at least N x 100 to open raid tier N. Only the leader
  needs it; every raid member can enter, fight and earn rewards.
- The player must be in a raid. Minimum and maximum raid size are settings, default 6 and 72.
- Difficulty and membership lock when the instance is created. Nobody can be added afterwards.
- The boss is picked at random from the tier's pool.
- No lockouts for now. Prism prices are the throttle.

| Raid tier | Leader's deepest clear | Boss pool | Prisms per player |
|---|---|---|---|
| 1 | T100 | Classic and Kunark | 2 |
| 2 | T200 | Velious | 4 |
| 3 | T300 | Luclin | 8 |
| 4 | T400 | Planes of Power | 16 |
| 5 | T500 | Plane of Time and Gates of Discord | 32 |

## The fight

- The instance uses its own zone version, proposed 50. Version 0 holds the open-world spawns, version 1 the rift
  spawns, and 255 can grant PoP flags and spawn planar projections. Some zones already use other numbers, such as
  2, 3, 10 and 20, so 50 must be checked against every roster zone.
- The portal spawns the boss at its spawn point. There is no trash, clear path or trigger chain.
- Only the portal boss pays out. Each member assigned to the instance earns Prisms when it dies.
- Portal bosses drop no original loot.
- An instance lasts 2 hours, as a setting. After a wipe the boss resets and the raid can try again until the timer
  runs out.
- The zone eligibility check in `global_player.pl` must allow raid instances, as it already does for rifts.

## Mechanics

| Boss type | Test | What it gets |
|---|---|---|
| Scripted | Its script changes the fight: adds, hate wipes, phases, scripted spells | Keeps all of its own mechanics, no shared ones |
| Tank and spank | Its script does nothing in combat, or only leashes it | Shared mechanics |

- Bosses of both types keep their native spells and special abilities.
- A mechanic must not be lost to fit the single boss format. If a mechanic depends on a helper NPC, the helper
  comes with the boss, as Saryrn's script already does by spawning Sorrowsong.
- If a mechanic only makes sense with a whole event, the script is rewritten to keep the mechanic working with the
  boss alone. If that can't be done, the boss leaves the pool.
- Portal changes go in `quests/<zone>/v50/`, which the quest loader checks by zone version, so open-world fights
  stay as they are. Encounter scripts load differently and need a test.
- Shared mechanics are the Rift Guardian's: AoE pulses, adds at 50% HP, and a hate wipe.

A keyword scan can't make this call. It marked Lord Inquisitor Seru as having mechanics, but that script is only a
leash, and Muzlakh the Chosen's short script is two hate wipe timers. Each boss's script has to be read.

## Scaling

Native boss stats are too uneven to multiply (Lady Vox has 32,000 HP, Quarm 1,702,760), so stats are set to the solo
Rift Guardian's at tier N x 100, then raised by raid size. The Guardian's base is level 70, 34,500 HP, 92-320 melee
and 344 AC.

| Raid tier | Solo tier | Boss HP | Melee hit | AC |
|---|---|---:|---:|---:|
| 1 | T100 | 888,375 | 1,002-3,488 | 1,365 |
| 2 | T200 | 1,750,875 | 1,922-6,688 | 2,397 |
| 3 | T300 | 2,613,375 | 2,842-9,888 | 3,429 |
| 4 | T400 | 3,475,875 | 3,762-13,088 | 4,461 |
| 5 | T500 | 4,338,375 | 4,682-16,288 | 5,493 |

Per added raid member, as settings:

- HP: +100%, so 6 players fight 6x HP and 72 players fight 72x HP.
- Damage: +5%, capped at +50%.

Still to tune:

- Boss spell damage. Rift scaling multiplies it by 10.9 at T100, which could one-shot players with a late-game boss's
  big nuke. Each boss needs its own spell damage value.
- Adds and helper NPCs, which must be scaled when they spawn. The starting values are 2% of boss HP and 50% of boss
  melee.
- Bosses are set to level 70 like the Guardian, unless testing shows a reason not to.

## Rewards

Chronal Prisms are raid-only and the step up from Chronal Shards. Shards and Prisms can't be converted either way.
Five kills at a tier buy one weapon at that tier, and about three buy an augment.

| Raid tier | Prisms per kill | Weapon | Augment |
|---|---:|---:|---:|
| 1 | 2 | 10 | 6 |
| 2 | 4 | 20 | 12 |
| 3 | 8 | 40 | 24 |
| 4 | 16 | 80 | 48 |
| 5 | 32 | 160 | 96 |

Changed 2026-11-03: weapons moved to the rift tier vendors (docs/dynamic_items/TIERED_GEAR.md, Weapons), so the
Weapon column above no longer applies. The raid vendor sells augments only.

The Prism_Augmenter (npc 1200072, beside the Raid Portal) sells the Legendary Prismatic Scales (Kerafyrm) and Splinters
of Time (Zebuxoruk), 28 augments, copied for each raid tier with every stat times the multiplier of gear tier
raid tier + 1: Tempered x1.5, Exalted x2.0, Eternal x2.5, Timeless x3.0 (Exalted Crashing Splinter of Time and so
on). Aug types and names as the source, No Trade. Each raid tier is hidden until the character-scoped bucket
`raid_cleared` reaches it; `plugins/raid_portal.pl` raises it when the portal boss dies (on next zone-in for members
who weren't there). `test` raids count as clears too. Built by gen_tier_gear.py from tier_gear.json `raid_augments`.

## Starting roster

Picks use the box's data from 2026-11-02. "Own" and "Shared" are the mechanics type. Every boss's type comes from its
script; in-game testing is still to come.

Built so far: Lady Vox, Saryrn and Rallos Zek the Warlord, then all 23 Shared bosses on 2026-11-03. A Shared boss gets
`quests/<zone>/v50/<npc id>.lua`, which replaces its open-world script with the shared mechanics and a 400-unit leash.
Entry points are trash spawn points 150 to 550 units from the boss, unverified in game. In version 50,
`script_init_v50.lua` keeps sleeper, sebilis, codecay and ssratemple from loading their encounters, and
`soldungb/v50/player.lua` replaces the zone's banish of players over level 80.

On 2026-11-08 the other 17 Own bosses got scripts, which are in the repo but not deployed or tested. See "Own bosses,
2026-11-08" below. Muzlakh and Solusek Ro need no v50 file, because their open-world scripts work in an instance as
they are.

| Tier | Boss | NPC ID | Zone | Mechanics | Work needed |
|---|---|---:|---|---|---|
| 1 | Lady Vox | 73057 | permafrost | Shared | Remove banish, which can't fire below level 81 anyway. Keeps her cleric spells (parent list 1: roots, self-buffs, Complete Heal) as on Live |
| 1 | Lord Nagafen | 32040 | soldungb | Shared | Remove banish, which can't fire below level 81 anyway |
| 1 | Trakanon | 89154 | sebilis | Shared | Added by hand; not flagged as a raid target |
| 1 | Innoruuk | 2000552 | hateplane | Shared | None found |
| 1 | Phara Dar | 108510 | veeshan | Shared | None found |
| 1 | Xygoz | 108053 | veeshan | Shared | None found |
| 1 | Nexona | 108047 | veeshan | Shared | None found |
| 1 | Silverwing | 108509 | veeshan | Shared | None found |
| 2 | Lord Yelinak | 114106 | skyshrine | Shared | None found |
| 2 | Lord Koi`Doken | 124103 | templeveeshan | Shared | None found |
| 2 | Dozekar the Cursed | 124037 | templeveeshan | Shared | None found |
| 2 | Velketor the Sorcerer | 112025 | velketor | Shared | None found |
| 2 | The Final Arbiter | 128045 | sleeper | Shared | Check against the Sleeper encounter |
| 2 | Sontalak | 120005 | westwastes | Shared | None found |
| 2 | Vulak`Aerr | 124155 | templeveeshan | Shared | Its call for a guard does nothing, since no script or spawn creates that guard; spawn at its Thylex point |
| 2 | Tunare | 127098 | growthplane | Own | Script written: calls 2 protectors on pull and every 60s; encounter blocked |
| 2 | King Tormax | 113215 | kael | Own | Script written: 4 guards spawn with him and are called on pull and every 5 min |
| 2 | Bristlebane | 126160 | mischiefplane | Own | Script written: puppet god waves at 80/50/20; encounter blocked |
| 3 | Lord Inquisitor Seru | 159691 | sseru | Shared | None found; keep its leash |
| 3 | Thought Horror Overfiend | 164078 | thedeep | Shared | Check its death plugin |
| 3 | High Priest of Ssraeshza | 162076 | ssratemple | Shared | None found |
| 3 | Xerkizh the Creator | 162190 | ssratemple | Shared | Check its death plugin |
| 3 | Kaas Thox Xi Aten Ha Ra | 158007 | vexthal | Shared | None found |
| 3 | Emperor Ssraeshza | 162227 | ssratemple | Shared | Reclassified: his script has no combat mechanics, so he uses the shared template |
| 3 | Aten Ha Ra | 158096 | vexthal | Shared | Spawn at its trigger's point; this is the NPC the trigger spawns, not 158006 |
| 4 | Saryrn | 207001 | potorment | Own | Check shared state; flag and projection code already skips other versions |
| 4 | Xegony | 215056 | poair | Own | Script written: lieutenant and guard waves at 98/82/66/50/34/18 run to her |
| 4 | Fennin Ro | 217054 | pofire | Shared | Spawn at its controller's point |
| 4 | Solusek Ro | 212025 | solrotower | Own | No v50 file needed: the open-world script works in an instance |
| 4 | Agnarr the Storm Lord | 209026 | bothunder | Own | Script written: lords at 98/75/50/25, storms every 120s; no trigger signal |
| 4 | Rallos Zek the Warlord | 214113 | potactics | Own | Find a spawn point; remove its reliance on the trigger |
| 4 | Mithaniel Marr | 220006 | hohonorb | Shared | Spawn at its trigger's point |
| 4 | Bertoxxulous | 200055 | codecay | Shared | Spawn at its trigger's point |
| 4 | Muzlakh the Chosen | 215053 | poair | Own | No v50 file needed: the open-world script works in an instance |
| 5 | Quarm | 223201 | potimeb | Own | Script written: vortexes every 50s, AE sets per head, heads at 76/51/26; no phase path |
| 5 | Rallos Zek | 223168 | potimeb | Own | Script written: minions and boars at 90/75/50/25, rampage and flurry steps |
| 5 | Terris Thule | 223075 | potimeb | Own | Script written: nightmare pairs walk in at 91/51/11 |
| 5 | Vallon Zek | 223078 | potimeb | Own | Script written: arena tether, two copies at 51%; Tallon Zek (223077) joins at 50% with Vallon's raid HP and damage, and the raid is paid when both are dead |
| 5 | Innoruuk | 223167 | potimeb | Own | Script written: guardians and hatebringers at 75 and 20, idle adds pulled in |
| 5 | Noqufiel | 296065 | inktuta | Own | Script written: True and Mirror swap every 10%, reveal at each 5%, state in zone variables |
| 5 | Tunat`Muram Cuu Vauax | 298055 | tacvi | Own | Script written: second form only, eight boss phases 90 to 20 |
| 5 | Ancient Cragbeast Matriarch | 297056 | txevu | Own | Encounter copy `acm_portal` written: bearer buffs set against raid stats |
| 5 | Zun`Muram Votal | 284080 | ferubi | Own | Script written: two Pixtt spawn with him and join on pull; room leash |

## Own bosses, 2026-11-08

Each boss has `quests/<zone>/v50/<npc id>.lua`, written from its open-world script and encounter. None is deployed or
tested yet. `utils/scripts/rift/sql/2026_11_08_01_raid_portal_own_bosses.sql` adds the version 50 rows for the 11 new
zones. It has not been applied.

Shared pieces in `lua_modules/raid_portal.lua`:

- `portal.RESET_SIGNAL` (50). After a wipe reset, the Raid_Controller sends it to the boss, and each phased boss
  depops its adds and re-arms its HP events.
- `portal.set_damage(npc, min_ratio, max_ratio)` and `portal.set_resists(npc, delta)`. Open-world phase changes use
  fixed numbers. Here they become a ratio of the boss's own base damage, or a resist delta, applied to its raid stats.
  The raid base is saved the first time they are used, after the global scaling.
- `portal.spawn_add` and `portal.depop_all`.

Local `event_spawn` runs before the global scaling in `global_npc.pl`, so HP or stats that must be set after the
scaling (Noqufiel's HP carry-over, the Matriarch's bearer buffs) are set from a short timer.

### Decisions for review (2026-11-08)

| # | Boss | Decision | Why |
|---|---|---|---|
| 1 | Muzlakh, Solusek Ro | No v50 file; the open-world script runs as it is | Nothing in it depends on flags, the zone controller or version 0. Solusek's death unlocks doors 38/39, which the SQL copies |
| 2 | Emperor Ssraeshza | Shared template, not Own | His script has no combat mechanics |
| 3 | Tunare | Her open-world "zone assist" is replaced by calling 2 protectors on pull and every 60s. Kept (user, 2026-11-12) | A fixed call is easier to tune per tier |
| 4 | King Tormax | His 4 guards spawn with him and are also called on pull and every 5 min | His call for help does nothing without guards nearby |
| 5 | All phased bosses | Fixed phase damage becomes a ratio of the boss's base damage | Fixed numbers would undo the tier scaling |
| 6 | All phased bosses | A wipe reset sends signal 50 so phases re-arm | The controller only heals and moves the boss |
| 7 | Quarm | Starts at his spawn point; no phase 6 path, no zone status or Zebuxoruk signals. Approved (user, 2026-11-12) | The portal spawns him directly |
| 8 | Rallos Zek, Innoruuk (PoTime) | Spawn ready to fight, without their army of twelve | The army is part of the open-world phase controller |
| 9 | Vallon Zek | Tallon Zek spawns at his open-world spot when Vallon reaches 50%, copies Vallon's raid HP and damage, and is depopped on a wipe reset. One payout, once both are dead (`RAID_PARTNERS` in `plugins/raid_portal.pl`). Decided by the user 2026-11-12 | Tallon's spot is also the raid's entry point, so he spawns among the raid |
| 10 | Noqufiel | The True Image is the portal boss. The Mirror copies the True's HP and damage, because the global scaling would make it a 2% add | The pair respawns many times, so state is kept in zone variables |
| 11 | Tunat | Only the second form, with its eight phases. The first form and phylacteries are skipped. A wipe respawns him fresh | The first form is a separate NPC that hands off on death |
| 12 | Ancient Cragbeast Matriarch | A portal copy of the encounter (`acm_portal`), with two open-world bugs fixed: the hatchling and AE timers had mismatched names, so they never fired | It adds real difficulty. Kept (user, 2026-11-12) |
| 13 | Ancient Cragbeast Matriarch | The regeneration bearer's 6000 HP per tick is scaled to the same share of her raid HP (0.285% of max HP per tick). Decided by the user 2026-11-12 | A fixed 6000 does nothing against raid HP |
| 14 | Zun`Muram Votal | His Pixtt come back 2s after a wipe reset, not 5 min | So the raid can try again at once |
| 15 | Entry points | Some are guesses from spawn rows: Quarm (645 units away), Vallon (Tallon's spot), Tormax (500 units), Noqufiel (inside the room) | Check them in game |
| 16 | Emperor Ssraeshza, Lord Inquisitor Seru | Both keep their bane requirement (special ability 22: melee immune except bane). Kept on purpose (user, 2026-11-08) | The portal scaling doesn't change special ability 22 |

### Deploy steps (when testing)

1. Back up peq with `~/voidforged/src/docker/backup.sh`, then apply the SQL.
2. Copy `lua_modules/raid_portal.lua`, `quests/global/Raid_Controller.lua`, `quests/thevoida/1200071.lua` and the new
   `quests/<zone>/v50/`, `script_init_v50.lua` and `txevu/encounters/acm_portal.lua` files to the box runtime.
3. Zone rows are read at boot, so this needs a server restart, done when players are offline.
4. Test each boss with `test <key> <size>` at the portal NPC.

## To test first

Prove these on Lady Vox (shared), Saryrn (own, with adds) and Rallos Zek the Warlord (own, needs its trigger removed):

1. An empty version 50 instance opens, and whether each zone needs a new `zone` row.
2. The raid can enter, including zones players haven't flagged for.
3. Spawn scaling covers the boss, adds and helpers.
4. Payout fires once, per member, even when the boss has its own death script.
5. Original loot is removed, including items a script adds.
6. Data bucket and global keys don't collide with the open world or another raid instance. They are not scoped to an
   instance unless a script does it, as the Sleeper script does.
7. `plugin::handle_death` and `plugin::handle_killed_merit` do nothing unwanted inside a raid instance. `handle_death`
   passes its instance check in any instance, so it would spawn a flag mob for a boss with a subflag. Portal scripts must
   skip it, along with title rolls such as `AddTitleFlag`.