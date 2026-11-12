# Tiered Rift Gear (design, agreed 2026-10-23)

Five gear tiers sold for Chronal Shards, unlocked by each character's own deepest Greater Rift clear.
Everything sold is No Trade. Dynamic (rolled) pieces use the existing class pools; no new roll code.

## Unlocks

| Gear tier | Armor | Non-visible | Augments | Weapons | Stat multiplier | Name prefix |
|---|---|---|---|---|---|---|
| 1 | T1 | T25 | T50 | T75 | 1.0x | (current names) |
| 2 | T100 | T125 | T150 | T175 | 1.5x | Tempered |
| 3 | T200 | T225 | T250 | T275 | 2.0x | Exalted |
| 4 | T300 | T325 | T350 | T375 | 2.5x | Eternal |
| 5 | T400 | T425 | T450 | T475 | 3.0x | Timeless |

- Gate: merchantlist `bucket_name` / `bucket_value` / `bucket_comparison = 2` (>=) on a character-scoped
  data bucket holding the deepest clear. Locked items are hidden. The rift scripts write the bucket on a
  new deepest clear; existing characters get it copied once from `gr_deepest_<char_id>`.
- Tier strength is baked into each item's `dynamic_items.stat_multiplier` (slot x tier).

## Armor (7 items, 8 worn)

Copies of the current class sets per tier: class-specific, No Trade, own 2/4/8 set bonuses = tier 1
bonuses x tier multiplier.

## Non-visible (8 items, 10 worn)

Class-specific, No Trade, class pools. Slot multipliers (total 5.0, ~64% of armor's 7.8):

| Charm | Earring x2 | Face | Neck | Shoulders | Back | Waist | Ring x2 |
|---|---|---|---|---|---|---|---|
| 0.5 | 0.5 | 0.5 | 0.6 | 0.6 | 0.6 | 0.4 | 0.4 |

Own set per class per tier, bonuses at 3/6/9 worn. Sets of different tiers stack (by design: mixing
trades the 6/9 bonuses for extra 3-piece bonuses).

| Pieces | Bonus | Built from |
|---|---|---|
| 3 | % lines (focus-like, stack with real focus effects), T1 value x tier multiplier | existing set bonus affixes |
| 6 | worn effect, one rank per tier, starting one rank above the best rank that drops | spell (new set bonus type) |
| 9 | proc, standard ladder rank IX-XIII = 900/1050/1200/1350/1500 | spell (new set bonus type) |

| Class | 3 pieces (T1) | 6 pieces: worn (T1 -> T5) | 9 pieces: proc (T1 -> T5) |
|---|---|---|---|
| Warrior | +4% defense, +3% parry | Improved Parry/Block IV -> VIII* | Blessed Rune VI -> X (absorb 1000-1600) |
| Paladin | +4% heal amount, +3% heal crit | Improved Parry/Block IV -> VIII* | Remote Healing Surge IX -> XIII |
| Shadowknight | +4% disease and poison spells | Ferocity VII -> XI | Vampiric Strike IX -> XIII (540-900 lifetap) |
| Bard | +4% all instruments | Accuracy I -> V | Prismatic Strike IX -> XIII |
| Monk | +4% Flying Kick damage, +3% hand to hand | Improved Dodge V -> IX* | Force of Magic IX -> XIII |
| Rogue | +4% Backstab damage, +3% piercing | Cleave VII -> XI* | Strike of Venom IX -> XIII |
| Berserker | +4% Frenzy damage, +3% 2H | Ferocity VII -> XI | Burst of Ice IX -> XIII |
| Ranger | +4% archery, +3% archery crit | Sharpshooting VII -> XI* | Burst of Ice IX -> XIII |
| Beastlord | +4% hand to hand, +3% cold spells | Void Conduit I -> V (pet focus)* | Strike of Venom IX -> XIII |
| Cleric | +4% heal amount, +3% heal crit | Divine Insight I -> V* | Sympathetic Healing Light I -> V* (Healing Burst IX -> XIII on self) |
| Druid | +4% fire and cold spells | Divine Insight I -> V* | Sympathetic Healing Light I -> V* |
| Shaman | +4% cold, poison and disease spells | Divine Insight I -> V* | Sympathetic Healing Light I -> V* |
| Necromancer | +4% poison and disease spells | Void Conduit I -> V (pet focus)* | Sympathetic Force of Corruption X -> XIV |
| Wizard | +4% fire and cold spells | Arcane Insight I -> V* | Sympathetic Burst of Ice IX -> XIII |
| Magician | +4% fire and magic spells | Void Conduit I -> V (pet focus)* | Sympathetic Force of Magic IX -> XIII |
| Enchanter | +4% magic spells, +3% spell crit | Void Conduit I -> V (pet focus)* | Sympathetic Force of Magic IX -> XIII |

\* New server-side spell rows (not in the client spell file): extra worn ranks continue each family's
step; Divine Insight (heal crit + mana regen) and Arcane Insight (spell crit damage + mana regen) are
new worn families; Void Conduit I-V are pet focus spells (pet power 30/40/50/60/70, pet spells up to level 85, best-wins
with item pet focus; power past a pet's strongest pets row scales it, rule Pets:ScaleExcessPetPower); Sympathetic Healing Light I-V are new triggers (SPA 383, detrimental damage spells)
pointing at existing Healing Burst IX-XIII. Test in game that the client ignores unknown worn spells
before building on it. Procs themselves stay existing spells (the client must know them).

Sympathetic procs fire on casting detrimental damage spells; a beneficial trigger lands on the caster
(zone/mob.cpp TrySympatheticProc).

## Augments

One all-stat Facet per tier, so any class can wear it: fixed stats, no class restriction, No Trade, aug types 1, 2
and 4, every slot. Each tier sells it as Facet I and Facet II, with identical stats under two item ids, because one
item can't hold the same augment twice. Names: Chronal, Tempered, Exalted, Eternal and Timeless Facet I and II.
No focus effects. Stats are the tier 1 values below times the tier multiplier (changed 2026-11-03 from six themed
augments per tier; nobody owned the old ones).

| Stat | Tier 1 |
|---|---|
| AC | 15 |
| HP, Mana, Endurance | 150 each |
| STR, STA, AGI, DEX, WIS, INT, CHA | 10 each |
| Magic, Fire, Cold, Disease, Poison resist | 10 each |
| Heroic STR, STA, DEX | 4 each |
| Attack, Spell Damage, Heal Amount | 15 each |

## Weapons (added 2026-11-03)

The 30 Legendary Plane of Time weapons and shields (Darkblade of the Warlord, Greatstaff of Power, Bow of the
Tempest, Shield of Strife and the rest), copied per tier. Damage and every stat are the Legendary's times the
weapon multiplier, 1.5 / 2.0 / 2.5 / 3.0 / 3.5, half a step ahead of armor and jewelry (Darkblade: 114 damage and 675
HP at tier 1, 266 and 1,575 at tier 5, against the Legendary's 76 and 450). Until 2026-11-12 weapons used the gear
multiplier, so tier 1 had the Legendary's own stats. The random-roll strength moved by the same ratio. Delay, focus
and click effects and aug slots stay as the source; procs are the new ones in WEAPON_PROCS.md. Names: tier prefix + source name
(Chronal Darkblade of the Warlord ... Timeless Darkblade of the Warlord). No Trade. A tier's weapons unlock 75 past
its armor, the slot that was left open for a new vendor type. Two stats-only range items come with them: Stone of
Flowing Time (melee) and Spool of Woven Time (casters).

Each weapon also rolls affixes from the class pools, on top of its fixed stats. The pools are per class, so each
weapon is copied once per class the source allows (item = block 2000 + index * 20 + class), and the vendor shows a
player only their own class's copies: 6 to 14 weapons per class per tier, 143 per tier in all. Roll strength
(`dynamic_items.stat_multiplier`) is two-handers 1.2 (double a one-hander, as they fill both hands), other weapons and
shields 0.6, range items 0.5, times the
tier multiplier.

Tabled (2026-11-03): procs don't scale, so every tier procs the Legendary's spell at its rate. Scaling them needs
new proc spells per tier (in the client spell file too); revisit when players reach the deeper weapon tiers.

## Vendors

Four per tier in thevoida (20), hand placed in a short line per tier, each titled "Tier N Armor / Jewelry /
Augments / Weapons". Tier 1 armor is the existing Chronal Quartermaster; the others are <prefix>_Armorer
(Clockwork Golem), <prefix>_Jeweler (Clockwork Gnome), <prefix>_Augmenter (Junk Golem) and
<prefix>_Weaponsmith (Clockwork Golem), prefix Chronal for tier 1. The Weaponsmiths stand one step past each
tier's Augmenter, a guessed spot to move in game.

## Prices (Chronal Shards, sell-back 20%)

| | T1 | T2 | T3 | T4 | T5 |
|---|---|---|---|---|---|
| Armor | 25 | 50 | 100 | 200 | 400 |
| Non-visible | 15 | 30 | 60 | 120 | 240 |
| Augments | 40 | 80 | 160 | 320 | 640 |
| Weapons | 100 | 200 | 400 | 800 | 1600 |

Weapons (shields and range items too) went from 30 / 60 / 120 / 240 / 480 to these on 2026-11-12, once they carried
the new procs and sympathetics (WEAPON_PROCS.md). The tier arrows at the Weaponsmiths keep 15 / 30 / 60 / 120 / 240.

## Build

- Data: `utils/scripts/dynamic_items/gen_tier_gear.py <eqemu_config.json | box:user@host[,key]> class_sets.json
  tier_gear.json <out.sql>` (output committed as `2026_10_23_tier_gear.sql`; `box:` reads the schema from the
  Docker server over SSH). It rebuilds its own id ranges on every run:
  items 940000-964999 (item = 940000 + (tier - 1) * 5000 + block + class; augments block 1800, weapons block
  2000), raid augments 965000-965399, armor sets tier * 100 + class, non-visible sets 1000 + tier * 100 + class,
  spells 44001-44029, vendor npcs 1200030-1200043, Weaponsmiths 1200044-1200048 and the Prism_Augmenter 1200072
  (spawngroups named `rift_tier_vendor_<npc id>`).
- The weapons and the raid augment vendor went live as `utils/scripts/rift/sql/2026_11_03_03_tier_weapons_raid_augments.sql`,
  only the new rows cut from the generator's output, so the rest of the live tier gear was left alone.
- New items and spells load into shared memory at server start, so they appear after a restart.
- Unlock bucket: `rift_core.lua` writes the character bucket `rift_deepest` on each new deepest clear;
  `utils/scripts/rift/sql/2026_10_23_03_rift_deepest_bucket.sql` copied existing values once.

## Code

The only C++: set bonuses that grant a spell. Worn spells apply like an item worn effect
(CalcItemBonuses), procs are added and removed as the piece count changes, and sympathetic triggers
count as a focus source. Everything else is data generated by extending gen_class_sets.py.
