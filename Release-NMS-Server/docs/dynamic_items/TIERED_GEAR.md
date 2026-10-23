# Tiered Rift Gear (design, agreed 2026-10-23)

Five gear tiers sold for Chronal Shards, unlocked by each character's own deepest Greater Rift clear.
Everything sold is No Trade. Dynamic (rolled) pieces use the existing class pools; no new roll code.

## Unlocks

| Gear tier | Armor | Non-visible | Augments | Stat multiplier | Name prefix |
|---|---|---|---|---|---|
| 1 | T1 | T25 | T75 | 1.0x | (current names) |
| 2 | T100 | T125 | T175 | 1.5x | Tempered |
| 3 | T200 | T225 | T275 | 2.0x | Exalted |
| 4 | T300 | T325 | T375 | 2.5x | Eternal |
| 5 | T400 | T425 | T475 | 3.0x | Timeless |

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
| Beastlord | +4% hand to hand, +3% cold spells | Double Attack V -> IX* | Strike of Venom IX -> XIII |
| Cleric | +4% heal amount, +3% heal crit | Divine Insight I -> V* | Sympathetic Healing Light I -> V* (Healing Burst IX -> XIII on self) |
| Druid | +4% fire and cold spells | Divine Insight I -> V* | Sympathetic Healing Light I -> V* |
| Shaman | +4% cold, poison and disease spells | Divine Insight I -> V* | Sympathetic Healing Light I -> V* |
| Necromancer | +4% poison and disease spells | Arcane Insight I -> V* | Sympathetic Force of Corruption X -> XIV |
| Wizard | +4% fire and cold spells | Arcane Insight I -> V* | Sympathetic Burst of Ice IX -> XIII |
| Magician | +4% fire and magic spells | Arcane Insight I -> V* | Sympathetic Force of Magic IX -> XIII |
| Enchanter | +4% magic spells, +3% spell crit | Arcane Insight I -> V* | Sympathetic Force of Magic IX -> XIII |

\* New server-side spell rows (not in the client spell file): extra worn ranks continue each family's
step; Divine Insight (heal crit + mana regen) and Arcane Insight (spell crit damage + mana regen) are
new worn families; Sympathetic Healing Light I-V are new triggers (SPA 383, detrimental damage spells)
pointing at existing Healing Burst IX-XIII. Test in game that the client ignores unknown worn spells
before building on it. Procs themselves stay existing spells (the client must know them).

Sympathetic procs fire on casting detrimental damage spells; a beneficial trigger lands on the caster
(zone/mob.cpp TrySympatheticProc).

## Augments

Fixed stats, no class restriction, No Trade, aug type 1 or 2 (the set pieces' general slots). Six per
tier, stats x tier multiplier, focus from existing item focus spells:

| Augment | Stats | Focus |
|---|---|---|
| Bulwark | HP, AC, heroic STA, resists | none |
| Ferocity | Attack, STR/DEX, heroic STR/DEX | none, or a melee worn effect |
| Arcana | Spell Damage, INT, mana | Improved Damage |
| Mending | Heal Amount, WIS, mana | Improved Healing |
| Alacrity | Mana, INT/WIS | spell haste |
| Valor | HP, Attack, Heal Amount | Mana Preservation |

## Vendors

Three per tier in thevoida (15), one line from (-89.84, -30.07) to (-107.79, 49.33) in tier order, each
titled "Tier N Armor / Jewelry / Augments". Tier 1 armor is the existing Chronal Quartermaster; the others
are <prefix>_Armorer (Clockwork Golem), <prefix>_Jeweler (Clockwork Gnome) and <prefix>_Augmenter (Junk
Golem), prefix Chronal for tier 1.

## Prices (Chronal Shards, sell-back 20%)

| | T1 | T2 | T3 | T4 | T5 |
|---|---|---|---|---|---|
| Armor | 25 | 50 | 100 | 200 | 400 |
| Non-visible | 15 | 30 | 60 | 120 | 240 |
| Augments | 40 | 80 | 160 | 320 | 640 |

## Build

- Data: `utils/scripts/dynamic_items/gen_tier_gear.py <eqemu_config.json> class_sets.json tier_gear.json <out.sql>`
  (output committed as `2026_10_23_tier_gear.sql`). It rebuilds its own id ranges on every run:
  items 940000-964999 (item = 940000 + (tier - 1) * 5000 + block + class; augments block 1800),
  armor sets tier * 100 + class, non-visible sets 1000 + tier * 100 + class, spells 50001-50024,
  vendor npcs 1200030-1200043 (spawngroups named `rift_tier_vendor_<npc id>`).
- New items and spells load into shared memory at server start, so they appear after a restart.
- Unlock bucket: `rift_core.lua` writes the character bucket `rift_deepest` on each new deepest clear;
  `utils/scripts/rift/sql/2026_10_23_03_rift_deepest_bucket.sql` copied existing values once.

## Code

The only C++: set bonuses that grant a spell. Worn spells apply like an item worn effect
(CalcItemBonuses), procs are added and removed as the piece count changes, and sympathetic triggers
count as a focus source. Everything else is data generated by extending gen_class_sets.py.
