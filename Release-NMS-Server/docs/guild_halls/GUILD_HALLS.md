# Guild Halls (design, in progress 2026-11-07)

Guilds earn Infamy from guild achievements, fill a treasury with platinum, and spend it on a private hall that grows
with the guild: a better zone each tier, amenities, decorations, and boss trophies that upgrade as the guild keeps
killing. Voidforged only. Every number marked *draft* is a placeholder to tune.

## Build phases (decided 2026-11-13)

| Phase | Contents | Server rebuild |
|---|---|---|
| 1. Foundation (built 2026-11-13) | Infamy, the treasury, buying the T1 hall, the Steward and porter in the Bazaar | No |
| 2. Hall features | Waypoints map entry, guild bank outside the guild hall zone, halls that don't expire, amenities | Yes |
| 3. The rest | T2+ zones, decoration placement, trophies, rank-gated perks and guild tributes | Yes |

Phase 1 decisions (2026-11-13):

- Players donate and check totals at an NPC only: the Guild Steward in the Bazaar. The Guild Hall Porter stands
  next to it.
- The draft numbers below are used as they are.
- A guild's members can't reach the guild hall until the guild buys T1. New characters still land in their own
  shared copy.
- Everything is behind an on/off switch, off until the owner turns it on. While off only GMs are served or earn
  Infamy. Scripts, SQL and the switch: `utils/scripts/guild_halls/README.md`.

## Currencies

| Currency | What it is | Spent? |
|---|---|---|
| Infamy | The guild's running total of achievements | Never. It only unlocks things in the store |
| Treasury platinum | Platinum members donate to the guild | Yes. It pays for everything in the store |

- Any member can donate platinum to the treasury. Donations can't be withdrawn.
- Only the guild leader buys from the store.
- Guild chat announces Infamy earned and store purchases. Members can check the Infamy total and treasury balance.

## Earning Infamy

Boss kill Infamy scales with how many guild members were on the kill: each value below is per guild member credited.
Gear and items never earn Infamy.

| Source | Infamy *(draft)* |
|---|---|
| Rift Guardian kill | Rift tier / 10, per guild member |
| Raid portal boss | 25 x raid tier, per guild member |
| Open-world raid bosses | Per guild member: 10 Classic, 15 Kunark, 20 Velious, 25 Luclin, 30 Planes of Power |
| The guild's first kill of a boss | 5x the normal amount, once per boss |
| A member reaches max level | 100, once per character |

At these rates a six-member guild group clearing a tier 100 rift earns 60 Infamy, and a 12-member guild raid on
Lady Nagafen earns 120, or 600 on the guild's first kill.

Infamy goes to the guild the character is in at the time of the kill or level.

## Halls

- A guild owns one hall at a time. There are no free halls: the T1 hall also needs Infamy and platinum.
- Halls are bought in order. A guild can't skip a tier or save for T5 without owning T1 to T4 first.
- Upgrading replaces the old hall with no refund. Everything the guild bought comes along: amenities are set up in the
  new hall, and placed decorations and trophies go to the guild's stored list to be placed again.
- Each tier offers a choice of zones. The guild picks one.

| Tier | Infamy *(draft)* | Platinum *(draft)* | Zones |
|---|---:|---:|---|
| T1 | 1,000 | 250,000 | Highkeep (decided 2026-11-13) |
| T2 | 5,000 | 1,000,000 | Open: city zones |
| T3 | 15,000 | 3,000,000 | Open: for example temples |
| T4 | 40,000 | 8,000,000 | Open: for example raid zones |
| T5 | 100,000 | 20,000,000 | Open |

Which zones go in which tier is still to be discussed.

Decided 2026-11-13 (owner's notes), all five tiers "for now". Where a row names two sections, the choice is made at
the in-game walk-through. Each hall is one section of the zone, sized for about a dozen
NPCs, a guild banker, a tribute master and a full set of tradeskill stations:

| Tier | Zone | Hall section | Amenities |
|---|---|---|---|
| T1 | Highkeep (Classic) | Third-floor noble quarters and banquet room | Tradeskill stations along the banquet table, banker in the vault side room, tribute master by the grand staircase |
| T2 | Sanctus Seru (Luclin) | The Praesertum rotunda or the upper Arx Seru atrium | NPCs around the circular edge, tradeskill containers in the side alcoves |
| T3 | Skyshrine (Velious) | The grand entry rotunda or Lord Yelinak's throne room | Banker and tribute master by the central fountain or dais, crafting stations along the pillared walkways |
| T4 | Plane of Knowledge (`poknowledge`) | The Great Library interior: the main circular floor under the floating astrolabe | Tradeskill stations in the book-lined wings, banker, tribute master and utility NPCs in the central atrium |
| T5 | Temple of Marr (`hohonorb`, decided 2026-11-13) | The main altar terrace | Tribute master and banker on the raised central dais, utility NPCs and tradeskill stations flanking the grand staircase |


Decided 2026-11-13: Highkeep replaces the existing guild hall as T1. The existing guild hall stays only as the
new-character landing zone. Until the Highkeep hall is set up, Phase 1's T1 still sends owners to the existing guild
hall, and they move to Highkeep when it is ready.

### Layout

A hall is a small part of its zone, in the guild's own private copy of the zone. It has two areas:

- **Public showcase**, where everyone arrives. Trophies and decorations are on display, and any player can visit.
- **Private wing**, for members only. The guild bank, tribute master, banker and other amenities are here. A
  non-member who walks in is moved back to the showcase, and the amenity NPCs don't serve non-members.

The rest of the zone is empty and outside the hall. Players who leave the hall's area are moved back into it. The
zone's own NPCs don't spawn in guild copies.

### Entry

- The Waypoints map gets a Guild Hall entry. Members go to their own hall.
- Next to it, "Visit a Guild Hall" lists the guilds with a hall and sends the player to that guild's showcase.
- The guild leader can close the hall to visitors.

### Every hall includes

- Guild bank
- Guild tribute master
- Banker
- Regen bath (health and mana)

## Store

Everything costs Infamy to unlock and treasury platinum to buy. Amenities also need a minimum hall tier.

### Amenities *(draft)*

| Amenity | Min tier | Infamy | Platinum |
|---|---|---:|---:|
| Porter to the Bazaar and The Void | T1 | 1,500 | 100,000 |
| General merchant (food, drink, reagents) | T1 | 2,000 | 100,000 |
| Tradeskill stations (each bought separately) | T2 | 5,000 | 150,000 each |
| Corpse summoner | T2 | 7,500 | 500,000 |
| Guild Hall Mystic buffs, tiers 1-5 (see below) | T1 | 2,500 to 50,000 | 100,000 to 2,000,000 |
| Better regen bath, level 1 / level 2 | T3 | 15,000 / 30,000 | 1,000,000 / 3,000,000 |
| Rift entrance | T3 | 20,000 | 2,000,000 |
| Portal pool to popular zones | T4 | 40,000 | 5,000,000 |
| Raid portal entrance | T4 | 50,000 | 5,000,000 |

### Guild Hall Mystic (decided 2026-11-13, built in Phase 1)

A buff NPC in the hall that casts player class spells (Temperance, damage shields, mana regen, haste and more) on
a member and their pet when hailed. No cooldown. Five tiers, each bought in order with Infamy and treasury
platinum, each with better spells: from Temperance, Clarity and Shield of Thorns at tier 1 to Conviction,
Clairvoyance, Speed of Salik and Nettle Shield at tier 5. The full list and prices are in
`utils/scripts/guild_halls/README.md`.

### Decorations *(draft)*

| Type | Unlocked by | Platinum |
|---|---|---:|
| Furniture (items 61800-61821), plants, lights, banners | Hall tier | 5,000-25,000 |
| Statues and large pieces | Owning a T3 hall | 100,000 |
| Boss trophies | Killing that boss in the raid portal | 50,000 |

## Boss trophies

- Trophies come only from raid portal bosses, one per boss in the roster. Open-world kills earn Infamy but no trophy.
- A guild buys a boss's trophy once. After that it upgrades by itself from the guild's record for that boss.

| Record | Trophy change |
|---|---|
| Highest raid tier killed | Name plate and tint: Bronze, Silver, Gold, Platinum, Void (tier 1-5) |
| Total kills at any tier | Bigger at 10 and 25 kills, a glow at 50, a stronger effect at 100 |

- The name plate shows the tier and kill count, for example "Lady Vox - Gold, 50 kills".
- Some boss models ignore tints. Those trophies show their tier through the name plate only.

## Placement

- Free placement: the leader or an officer stands where an item should go and places it with a command. Commands
  also rotate, move, remove and list items.
- Removed items go back to the guild's stored list.
- Items can only be placed inside the hall's areas.
- Placed item limit per hall *(draft)*: about 25 at T1, rising to about 150 at T5, so halls load quickly.

## Zones

The zone list is open. Each hall zone is one entry in a config file. The shared code doesn't change when zones are
added or removed.

A zone entry holds:

- the arrival point
- the public and private areas, as boxes with height limits, so multi-floor buildings work
- where items can be placed
- which doors and zone exits stay in the guild copy
- any of the zone's own scripts that must skip guild copies

Effort per zone is about an hour of setup plus 15-30 minutes in game marking corners with `#loc`, then a walk-through.
Zones with odd geometry, a lot of height, or zone-wide scripts take longer.

**Changing the list**

- Adding a zone: a config entry and generated database rows. The zone shows up in its tier.
- Removing or replacing a zone: guilds in it move to a replacement zone in the same tier for free. Their placed items
  go to the stored list.

## New characters

The guild hall zone is also where new characters land today (`quests/crescent/player.pl` sends them there through
`SendToGuildHall`, and `quests/guildhall/player.pl` gives starter AAs). New characters keep a separate shared copy
of the zone, so guild halls stay private.

## Rank access *(idea, added 2026-11-09)*

Guild ranks decide how much of the guild's lifetime achievements a member can use. A new recruit doesn't get
instant access to what older members earned; access opens up as the guild promotes them, once the guild feels they
have earned it. This covers items, spells, buffs, clickies and the other perks below.

- Each perk the guild unlocks has a minimum guild rank to use it.
- The leader sets the minimum rank per perk, or per group of perks.
- Recruits start with the basics and gain the rest by promotion. Nothing is lost when a member is demoted again, except
  access.
- The guild ranks are the game's own (RoF2 guilds have eight ranks, with names the guild can change).

## Guild power from rifts *(idea, added 2026-11-09)*

As members progress through the rifts, the guild gains power toward unlocking guild perks: new or higher AAs, spells
and clickies that help members push into the next rift tiers.

- The guild's power is its Infamy. There is no separate rift currency: Infamy unlocks the perk tiers, the same way
  it unlocks the store (decided 2026-11-09).
- Progress is shown as Infamy toward the next perk tier.
- Members use the perks according to [Rank access](#rank-access-idea-added-2026-11-09).

Perks come in two kinds (decided 2026-11-09):

| Kind | Who pays | Examples |
|---|---|---|
| Guild perks | The treasury, bought by the leader | Guild-wide buffs, guild tribute, access to special things that benefit the whole guild |
| Member unlocks | The member, with their own platinum | Special items, such as a Call of the Hero clicky or a resurrection clicky usable by all races and classes |

- Infamy opens both kinds. A member unlock is then bought by each member who wants it, if their rank allows.
- Member items are lore and no-drop, and stop working when the member leaves the guild (decided 2026-11-09).
- Guild buffs work like guild tribute (decided 2026-11-09): new guild tribute entries that Infamy unlocks, paid from
  the treasury, active for every member who opts in. The server already has guild tribute (`zone/tribute.cpp`,
  `Guild:Tribute*` rules): a guild runs up to two tributes at a time, they cost guild favor over time, and each
  member opts in or out. Rank access adds a minimum rank check where the tribute is applied
  (`Client::DoGuildTributeUpdate`).

To decide:

- The perk list for each Infamy tier, and the prices.
- Recast per member item, depending on what it does. Member items have no charges (decided 2026-11-09).
- Whether the treasury pays guild favor directly.

Two active guild tributes is enough (decided 2026-11-09), because each one can stack many effects. A guild tribute
tier is an item, and its bonuses apply through `AddItemBonuses` (`zone/bonuses.cpp`): item stats (HP, mana, AC,
regen, resists, haste and the rest), a worn effect spell with up to 12 effect slots, and a focus. That covers what
the Echo server buffs do, without using buff slots. If a guild buff ever has to show in the buff window, the Echo
approach (`Zone::HandlePermanentServerBuffs`) can be copied for guild members instead.

## Technical notes

What exists today:

- `Client::SendToGuildHall` (`zone/client.cpp`) keeps one guildhall instance per guild in the data bucket
  `guild-hall-instance-<guild id>`, using instance version 1, for 90 days (`Instances:GuildHallExpirationDays`).
- The guild bank only works in the guildhall zone. `Zones::GUILDHALL` checks are in `zone/client_packet.cpp`,
  `zone/guild.cpp` and `zone/zone.cpp` (`GuildBankManager` is only created there).
- `Client::TransportToWaypoint` (`zone/nms_waypoints.cpp`) sends players only to non-instanced zones or to their
  expedition.
- The Guild Lobby is GM-only (`min_status` 255) and its doors lead nowhere.
- The guild hall's regen bath is `quests/guildhall/Aporia.lua`.

Changes needed:

- Server code (rebuild and restart):
  - allow the guild bank in guild hall copies of other zones
  - make the map's Guild Hall entry find the guild's hall instance
  - possibly let a guild hall instance persist without expiring while the guild owns it
- Guild hall copies use their own zone version (proposed 60), with no normal spawns and only the doors and zone exits
  the zone entry keeps. The number must be checked against every hall zone, as the raid portal did with 50.
- New tables, for example:
  - the guild's Infamy total and log
  - treasury balance and donations
  - the guild's hall (tier, zone, instance)
  - unlocks and purchases
  - placed and stored items
  - trophy records (boss, highest tier, kills)
- Scripts:
  - Infamy awards on boss death and level up
  - the store and treasury NPC
  - placement commands
  - spawning placed items and trophies when the hall loads
  - the public and private area checks
- Back up the database before the migration.

## Tabled

- Guild hopping and alt farming protection. For now the players are friends on the tailnet.

## Open questions

- Hall zones for each tier.
- Store contents and every *draft* number above.
- ~~How players donate and check totals~~: an NPC only, the Guild Steward (decided 2026-11-13).
- What a visitor sees when a guild has closed its hall.
