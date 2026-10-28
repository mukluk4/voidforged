# Dynamic Items — Admin & GM Guide

Dynamic items roll their own random stats, Diablo style, when a player buys one from an event
token vendor. Every copy is different, rolls are weighted for the item's class, and once an item is
bought its stats are set.

This guide covers creating dynamic items, tuning the rolls, setting up a vendor, and the commands.
For how it works under the hood, see [README.md](README.md).

---

## Contents
1. [Quick start](#quick-start)
2. [Creating a dynamic item](#creating-a-dynamic-item)
3. [Selling it on a token vendor](#selling-it-on-a-token-vendor)
4. [Tuning the rolls](#tuning-the-rolls)
5. [Affix reference](#affix-reference)
   - [Armor set bonuses with affixes](#armor-set-bonuses-with-affixes)
6. [Commands](#commands)
7. [What players see](#what-players-see)
8. [Rules and good practice](#rules-and-good-practice)
9. [Test checklist](#test-checklist)
10. [Troubleshooting](#troubleshooting)

---

## Quick start

1. Boot `world` once after updating. It creates the `dynamic_items` and `dynamic_affix_pool`
   tables and fills the pool with starting data for all 16 classes.
2. Create a **stat-less** base item in `items`. Make it No Trade and restricted to one class.
3. Add it to `dynamic_items`.
4. Restart the zone, or run `#dynitem reload`.
5. Test with `#dynitem give <item_id>`. No vendor is needed.

---

## Creating a dynamic item

### 1. The base item (`items` table)

The base item is a shell: name, icon, model, slot and restrictions. Its rolled stats come from the
pool, so leave the stats at zero. Any stats you do put on the base item are added on top of the
rolls.

| Field | Set to |
|---|---|
| Name, icon, idfile, slots, item type, size, weight | as normal |
| `classes` | the single class this item is for (e.g. Rogue only) |
| `races` | as desired |
| `nodrop` | **0** (No Trade) |
| `hp`, `mana`, `endur`, `ac`, `astr` … `acha`, `heroic_*`, resists | **0** |
| `reqlevel` / `reclevel` | optional. Below `reqlevel` the item gives nothing; below `reclevel` rolls are scaled down like normal item stats. |

> Make one base item **per class**. Vendors sell e.g. "Wizard Boots" and "Rogue Boots" as separate
> items.

### 2. Register it (`dynamic_items` table)

```sql
INSERT INTO dynamic_items (item_id, class_id, tier, min_affixes, max_affixes, stat_multiplier, notes)
VALUES (900101, 9, 1, 2, 3, 1.0, 'Rogue Boots - event set 1');
```

| Column | What it does |
|---|---|
| `item_id` | the base item's ID |
| `class_id` | which class's pool it rolls from (see table below). The buyer must have this class. |
| `tier` | shown to players as "Tier N". Labelling only. |
| `min_affixes`, `max_affixes` | how many **random** affixes it gets on top of the guaranteed stats |
| `stat_multiplier` | scales **every** rolled value on this item. Use it to make better items: `1.0` normal, `1.5` = 50% stronger. |
| `notes` | anything you like |

**Class IDs**

| ID | Class | ID | Class |
|---|---|---|---|
| 1 | Warrior | 9 | Rogue |
| 2 | Cleric | 10 | Shaman |
| 3 | Paladin | 11 | Necromancer |
| 4 | Ranger | 12 | Wizard |
| 5 | Shadowknight | 13 | Magician |
| 6 | Druid | 14 | Enchanter |
| 7 | Monk | 15 | Beastlord |
| 8 | Bard | 16 | Berserker |

**Suggested affix counts by tier**: tier 1: 1–2, tier 2: 2–3, tier 3: 3–5.

### 3. Load it
Zones read the tables at boot. In a running zone, `#dynitem reload` picks up changes immediately
for that zone; other zones need a restart or their own reload.

---

## Selling it on a token vendor

Dynamic items roll when bought from an **alternate currency merchant**. A normal coin merchant or
`#summonitem` gives an un-rolled item with no stats.

1. **Merchant NPC** (`npc_types`): `class = 70` (Alternate Currency Merchant), `alt_currency_id` =
   your event token currency ID, `merchant_id` = a merchant list ID.
2. **Merchant list** (`merchantlist`): add a row per item with that `merchantid`, the `item`, and
   `alt_currency_cost` = price in tokens.

```sql
INSERT INTO merchantlist (merchantid, slot, item, alt_currency_cost)
VALUES (900001, 1, 900101, 50),   -- Rogue Boots, 50 tokens
       (900001, 2, 900112, 50);   -- Wizard Boots, 50 tokens
```

**When a player buys:**
* If they don't have the item's class, they get "*Rogue Boots can only be purchased by a Rogue.*"
  and are not charged. On the multiclass server, any of their three classes counts.
* Otherwise the item rolls, goes into their inventory, and they get a chat summary of the rolls.

**Selling back:** players get **20%** of the token price back by default (rule
`Merchant:DynamicItemSellRefundPct`), so selling and rebuying to reroll costs tokens. Change it in
`rule_values`; 0 means no refund. The server-wide rule `Merchant:EnableAltCurrencySell` still
controls whether token items can be sold at all.

---

## Tuning the rolls

All tuning is in `dynamic_affix_pool`, one row per class per stat. Edit, then `#dynitem reload`.

| Column | Meaning |
|---|---|
| `class_id` | the class |
| `affix` | the stat code (see [Affix reference](#affix-reference)) |
| `guaranteed` | `1` = every item of this class rolls it; `0` = it's in the random affix pool |
| `weight` | for random affixes: how often it's picked. 200 is twice as likely as 100. Ignored when `guaranteed = 1`. |
| `min_value`, `max_value` | roll range (before `stat_multiplier`) |
| `slot_mask` | `0` = any item. Otherwise the affix only rolls on items worn in those slots (see below). |

**Rules of thumb**
* **Low isn't zero.** If a class shouldn't care about a stat, give it a small range like 1–6
  rather than removing it. Remove the row entirely only when the class should never get it (e.g.
  no Mana for Rogues).
* **Ranges are safe to change at any time.** Existing items update and keep their relative roll:
  a roll near the top of the old range lands near the top of the new one.
* **Pool changes reshuffle random affixes.** Adding or removing a `guaranteed = 0` row, or
  changing a weight, can change *which* random affixes existing items of that class have.
  Guaranteed stats never move. Settle the pools before launch; after launch, prefer range changes.
* **Small pools repeat.** If a class has only 3 random affixes and items roll up to 3, every item
  gets all three. Add entries to make casters' items more varied.

**Examples**
```sql
-- Rogues: stronger Backstab damage rolls
UPDATE dynamic_affix_pool SET min_value = 8, max_value = 25
WHERE class_id = 9 AND affix = 'backstab_dmg_pct';

-- Wizards: roll Cold Spell Damage more often
UPDATE dynamic_affix_pool SET weight = 250 WHERE class_id = 12 AND affix = 'spell_cold_pct';

-- Monks: Flying Kick damage only on hands/feet items (Hands 4096 + Feet 524288)
UPDATE dynamic_affix_pool SET slot_mask = 528384
WHERE class_id = 7 AND affix = 'flying_kick_dmg_pct';
```

**Slot mask values** (add together for several slots)

| Slot | Value | Slot | Value |
|---|---|---|---|
| Charm | 1 | Range | 2048 |
| Ear | 2 + 16 = 18 | Hands | 4096 |
| Head | 4 | Primary | 8192 |
| Face | 8 | Secondary | 16384 |
| Neck | 32 | Finger | 32768 + 65536 = 98304 |
| Shoulders | 64 | Chest | 131072 |
| Arms | 128 | Legs | 262144 |
| Back | 256 | Feet | 524288 |
| Wrist | 512 + 1024 = 1536 | Waist | 1048576 |

### Starting data

The ranges were tuned in three passes:

1. **Plane of Time** (2026-10-21): a little above PoTime drops (about 180 HP, 175 Mana, 30 AC and
   12 per stat, no heroics).
2. **Melee best-in-slot** (2026-10-23, `rebalance_to_bis.py`): a full set of the 8 visible dynamic
   slots lands at about **122%** of best-in-slot gear in those slots, measured on a
   Warrior/Paladin/Monk with `#dyninfo worn`.
3. **Caster best-in-slot** (2026-10-24, `rebalance_casters.py`): the same 122% target against a
   Necromancer/Enchanter/Magician in best-in-slot gear. INT casters reach about 480 INT and 480
   Spell Damage per set. Priests mirror that on WIS and Heal Amount, and keep 350 Spell Damage so
   they can solo. Caster STR, DEX, AGI, Attack and AC came down, and caster STA went up.

Every item rolls these guaranteed lines (per tier 1 item before `stat_multiplier`; a full set of
the 8 visible pieces is about 7.8x these):

| Class | HP | Mana | End | AC | STR | STA | DEX | AGI | INT | WIS | CHA | hSTR | hSTA | hDEX | hAGI | hINT | hWIS | Spell Dmg | Heal Amt | ATK |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 Warrior | 424-518 | - | 290-350 | 124-161 | 49-61 | 25-37 | 33-40 | 38-50 | 21-26 | 23-28 | 18-23 | 19-25 | 19-25 | 11-15 | 12-15 | 5-10 | 5-10 | 23-27 | 24-28 | 26-44 |
| 2 Cleric | 353-436 | 360-440 | - | 80-103 | 11-14 | 44-59 | 24-28 | 24-28 | 21-26 | 52-71 | 25-32 | 4-6 | 11-17 | 5-10 | 5-10 | 5-10 | 19-25 | 37-53 | 47-76 | 11-14 |
| 3 Paladin | 406-497 | 265-320 | 145-175 | 111-145 | 48-59 | 24-35 | 32-38 | 35-46 | 21-26 | 26-33 | 18-23 | 17-23 | 17-23 | 10-14 | 10-14 | 5-10 | 8-14 | 24-30 | 26-34 | 23-38 |
| 4 Ranger | 406-497 | 265-320 | 145-175 | 94-125 | 43-49 | 20-27 | 37-47 | 35-46 | 21-26 | 26-33 | 18-23 | 16-21 | 17-23 | 10-14 | 10-14 | 5-10 | 8-14 | 24-30 | 26-34 | 23-38 |
| 5 Shadowknight | 406-497 | 265-320 | 145-175 | 99-131 | 46-57 | 24-35 | 32-38 | 35-46 | 24-31 | 23-28 | 18-23 | 16-21 | 17-23 | 10-14 | 10-14 | 8-14 | 5-10 | 25-33 | 24-28 | 23-38 |
| 6 Druid | 353-436 | 360-440 | - | 62-83 | 10-12 | 44-59 | 24-28 | 24-28 | 21-26 | 52-71 | 25-32 | 2-3 | 11-17 | 5-10 | 5-10 | 5-10 | 19-25 | 37-53 | 47-76 | 11-14 |
| 7 Monk | 424-518 | - | 290-350 | 96-130 | 49-61 | 20-27 | 40-52 | 38-50 | 21-26 | 23-28 | 18-23 | 19-25 | 19-25 | 12-15 | 12-15 | 5-10 | 5-10 | 23-27 | 24-28 | 26-44 |
| 8 Bard | 406-497 | 265-320 | 145-175 | 88-117 | 43-49 | 20-27 | 37-47 | 35-46 | 24-31 | 23-28 | 29-41 | 16-21 | 17-23 | 10-14 | 10-14 | 8-14 | 5-10 | 25-33 | 24-28 | 23-38 |
| 9 Rogue | 424-518 | - | 290-350 | 108-142 | 44-51 | 20-27 | 40-52 | 38-50 | 21-26 | 23-28 | 18-23 | 19-25 | 19-25 | 12-15 | 12-15 | 5-10 | 5-10 | 23-27 | 24-28 | 26-44 |
| 10 Shaman | 353-436 | 360-440 | - | 69-91 | 11-14 | 44-59 | 24-28 | 24-28 | 21-26 | 52-71 | 25-32 | 4-6 | 11-17 | 5-10 | 5-10 | 5-10 | 19-25 | 37-53 | 47-76 | 11-14 |
| 11 Necromancer | 353-436 | 360-440 | - | 44-63 | 10-12 | 44-59 | 24-28 | 24-28 | 52-71 | 23-28 | 25-32 | 2-3 | 11-17 | 5-10 | 5-10 | 19-25 | 5-10 | 47-76 | 24-28 | 11-14 |
| 12 Wizard | 353-436 | 360-440 | - | 44-63 | 10-12 | 44-59 | 24-28 | 24-28 | 52-71 | 23-28 | 25-32 | 2-3 | 11-17 | 5-10 | 5-10 | 19-25 | 5-10 | 47-76 | 24-28 | 11-14 |
| 13 Magician | 353-436 | 360-440 | - | 44-63 | 10-12 | 44-59 | 24-28 | 24-28 | 52-71 | 23-28 | 25-32 | 2-3 | 11-17 | 5-10 | 5-10 | 19-25 | 5-10 | 47-76 | 24-28 | 11-14 |
| 14 Enchanter | 353-436 | 360-440 | - | 44-63 | 10-12 | 44-59 | 24-28 | 24-28 | 52-71 | 23-28 | 40-56 | 2-3 | 11-17 | 5-10 | 5-10 | 19-25 | 5-10 | 47-76 | 24-28 | 11-14 |
| 15 Beastlord | 406-497 | 265-320 | 145-175 | 97-128 | 44-51 | 20-27 | 37-47 | 35-46 | 21-26 | 26-33 | 18-23 | 17-23 | 17-23 | 10-14 | 10-14 | 5-10 | 8-14 | 24-30 | 26-34 | 23-38 |
| 16 Berserker | 424-518 | - | 290-350 | 108-142 | 49-61 | 25-37 | 40-52 | 33-40 | 21-26 | 23-28 | 18-23 | 19-25 | 19-25 | 12-15 | 11-15 | 5-10 | 5-10 | 23-27 | 24-28 | 26-44 |

**Melee DPS** (Rogue, Monk, Berserker) get a standout DEX band: regular 40-52, heroic 12-15.

**Hybrids** are blended from two templates, line by line: 75% of a melee class plus 25% of a
caster class. Paladin = Warrior + Cleric, Shadowknight = Warrior + Necromancer, Ranger = Rogue +
Druid, Beastlord = Rogue + Shaman, Bard = Rogue + Enchanter. Their AC is then cut to 90%, so the
hybrid tanks always have less HP and AC than a Warrior. Signature lines keep full value (Bard CHA and
instrument mods, Ranger archery). In their random affixes, melee affixes are scaled to 75% and spell
or healing % to 25%. The scripts that produce these numbers are in `utils/scripts/dynamic_items/`.

**Every item** also has all six regular resists, never 0: Magic, Fire, Cold, Disease and Poison
share one range for every class (**28-42**), and Corruption rolls **15-25**. The five matching
heroic resists are guaranteed too, at **6-12** each.

Random affix pools (weight in brackets): weapon skills roll 3–12%, defensive skills 2–8%, ability
skills 3–10%, ability damage 5–18%, spell/heal/bard 3–15%.

**Advanced stats** are guaranteed lines too (migration v28):

* **On every item, never 0:** Spell Damage, Heal Amount and Attack (see the table above for each
  class).
* **Can roll 0** (then the item doesn't have it and it isn't shown): HP regen, Mana regen (not pure
  melee), Endurance regen (melee and hybrids), Combat Effects, Spell Shield, DoT Shielding,
  Avoidance and Stun Resist (everyone), Shielding, DS Mitigation, Strike Through and Accuracy (melee
  and hybrids), Damage Shield (WAR/PAL/SHD). Most are **0-3**; the bigger ones are Avoidance 0-13,
  Accuracy 0-23 (hybrids 0-15) and HP regen 0-11 (casters 0-3).
* **Crits** can roll 0 too and every class has all of them, weighted by use. Crit rates max 6 / 4 / 2,
  crit damage max 10 / 6 / 3 (strong / medium / weak). Strong: DD crit WIZ/MAG, DoT crit NEC/DRU,
  heal crits CLR/DRU/SHM, physical crit WAR/MNK/ROG/BER, archery crit RNG.

**Per-slot strength:** every slot rolls the same ranges; use the item's `stat_multiplier` to make
some slots stronger (e.g. 1.1 for chest and legs, 0.9 for ears and rings). The multiplier scales
every line, resists included, so a 0.8 item rolls resists of 12-20.

To make a 0-able stat appear less often, lower its `max_value`; to make it never appear for a
class, delete that class's row.

**Heroic Corruption** is the one heroic resist left in the random pools, at **2-10** (weight 20),
low enough not to crowd out class affixes.

| Class | Random affixes |
|---|---|
| 1 Warrior | 1HS, 2HS, 1HB, 2HB, Pierce, Defense, Parry, Riposte, Bash, Bash Dmg (all 100) |
| 2 Cleric | 1HB, 2HB, Defense (100), Healing (250), Magic (100) |
| 3 Paladin | 1HS, 2HS, 1HB, 2HB, Defense, Parry, Riposte, Bash, Bash Dmg, Healing (all 100) |
| 4 Ranger | 1HS, 2HS, Pierce (100), Archery (250), Defense, Parry, Riposte, Dodge, Fire, Cold (100) |
| 5 Shadowknight | 1HS, 2HS, 1HB, 2HB, Defense, Parry, Riposte, Bash, Bash Dmg, Disease, Poison, Magic (all 100) |
| 6 Druid | 1HB, Defense (100), Fire, Cold (150), Magic (100), Healing (150) |
| 7 Monk | H2H (250), 1HB, 2HB, Defense, Block, Dodge, Riposte (100), Flying Kick (150), Flying Kick Dmg (200) |
| 8 Bard | 1HS, Pierce, 1HB, Defense, Dodge, Parry, Riposte (100), Stringed, Wind, Brass, Percussion, Singing (150) |
| 9 Rogue | Pierce (250), 1HS, 1HB, Defense, Dodge, Parry, Riposte (100), Backstab (150), Backstab Dmg (250) |
| 10 Shaman | 1HB, 2HB, Defense, Cold, Poison, Disease (100), Healing (150) |
| 11 Necromancer | Fire (100), Poison, Disease (150), Magic (100) |
| 12 Wizard | Fire, Cold, Magic (150) |
| 13 Magician | Fire (200), Magic (150), Cold (100) |
| 14 Enchanter | Magic (200), Poison, Cold (100) |
| 15 Beastlord | H2H (200), 1HB, 2HB, Pierce, Block, Dodge, Parry, Riposte, Cold, Poison, Healing (100) |
| 16 Berserker | 2HS, 2HB, 1HS, 1HB, Defense, Parry, Riposte, Dodge (100), Frenzy (150), Frenzy Dmg (250) |

---

## Affix reference

Use these codes in `dynamic_affix_pool.affix`. All bonuses **stack**: across dynamic items, with
normal gear, armor set bonuses, spells, AAs and focus effects.

**Flat stats** (shown in the item window)

| Code | Effect |
|---|---|
| `hp`, `mana`, `endurance`, `ac` | + that much HP / Mana / Endurance / AC |
| `str`, `sta`, `dex`, `agi`, `int`, `wis`, `cha` | + regular ("white") stat, subject to the server's stat caps |
| `heroic_str`, `heroic_sta`, `heroic_dex`, `heroic_agi`, `heroic_int`, `heroic_wis` | + heroic stat (counts toward the base stat and raises its cap, like heroic stats on normal items) |
| `mr`, `fr`, `cr`, `dr`, `pr`, `corruption` | + Magic / Fire / Cold / Disease / Poison / Corruption resist |
| `heroic_mr`, `heroic_fr`, `heroic_cr`, `heroic_dr`, `heroic_pr`, `heroic_corruption` | + heroic resist |

| `spelldmg`, `healamt`, `attack` | + Spell Damage / Heal Amount / Attack (Worn ATK) |
| `hp_regen`, `mana_regen`, `end_regen` | + HP / Mana / Endurance regen |
| `combat_effects`, `spell_shield`, `shielding`, `damage_shield`, `dot_shielding`, `ds_mitigation`, `avoidance`, `accuracy`, `stun_resist`, `strike_through` | + that advanced item stat (shown in the item window) |

Regular stats, regular resists and the advanced item stats Combat Effects, Spell Shield, Shielding,
Avoidance, Accuracy, Stun Resist and Strike Through are stored in small item fields, so each item's
total for one of them is capped at 127 (only reachable with a large `stat_multiplier`).

**Critical hits %**: these are character bonuses, not item fields. They don't show on the item; they
show in `#dyninfo` and in the **Advanced Character Stats** window.

| Code | Effect |
|---|---|
| `spell_crit_chance_pct`, `spell_crit_dmg_pct` | + direct damage spell crit rate / crit damage |
| `dot_crit_chance_pct`, `dot_crit_dmg_pct` | + DoT crit rate / crit damage (the stats window only shows DoT crit damage above 100) |
| `heal_crit_chance_pct`, `hot_crit_chance_pct` | + heal / heal-over-time crit rate |
| `melee_crit_chance_pct`, `melee_crit_dmg_pct` | + physical crit rate / crit damage (all skills: melee, archery, throwing and special attacks; shown as "Physical Crit") |
| `archery_crit_chance_pct` | + archery crit rate |

**Skill %**: raises the skill's value by that percent. On this server weapon skill drives both
to-hit and damage; defensive skills drive their avoidance chance.

| Code | Skill |
|---|---|
| `skill_1hs_pct`, `skill_2hs_pct` | 1H / 2H Slashing |
| `skill_1hb_pct`, `skill_2hb_pct` | 1H / 2H Blunt |
| `skill_pierce_pct` | Piercing (1H and 2H) |
| `skill_h2h_pct` | Hand to Hand |
| `skill_archery_pct` | Archery |
| `skill_defense_pct`, `skill_parry_pct`, `skill_riposte_pct`, `skill_dodge_pct`, `skill_block_pct` | Defense / Parry / Riposte / Dodge / Block |
| `backstab_skill_pct`, `flying_kick_skill_pct`, `frenzy_skill_pct`, `bash_skill_pct` | the melee ability's skill |

**Damage and spell %**

| Code | Effect |
|---|---|
| `backstab_dmg_pct`, `flying_kick_dmg_pct`, `frenzy_dmg_pct`, `bash_dmg_pct` | +% damage with that ability |
| `spell_fire_pct`, `spell_cold_pct`, `spell_magic_pct`, `spell_poison_pct`, `spell_disease_pct` | +% damage from your spells of that resist type, nukes and DoTs |
| `heal_amount_pct` | +% to heals you cast, direct and over time |
| `bard_stringed_pct`, `bard_wind_pct`, `bard_brass_pct`, `bard_percussion_pct`, `bard_singing_pct` | song effect +% for that instrument type, applied after the song cap |

Adding a brand-new kind of affix needs a code change; these are all the codes the server knows.

---

## Armor set bonuses with affixes

A dynamic item can also be an armor set piece (`armor_set_items`). Set bonuses in
`armor_set_bonuses` can use any affix code above: put it in the `affix` column and the amount in
`stat_value`. `stat_id` is then ignored (leave it 0). With `affix` empty, the row uses the old
numeric `stat_id` as before; use `stat_id = 7` for haste, which has no affix code.

Bonuses kick in at `min_pieces` worn (2, 4 and 8 give tiers 1, 2 and 3). Both wrists count, so a
full visible set (Head, Chest, Arms, 2x Wrist, Hands, Legs, Feet) is 8 pieces.

```sql
-- Wizard set (set 112): +5% spell crit rate at 8 pieces
INSERT INTO armor_set_bonuses (set_id, min_pieces, class_mask, stat_id, affix, stat_value)
VALUES (112, 8, 0, 0, 'spell_crit_chance_pct', 5);
```

**Class sets (test data).** Each class has a 7-item set, `set_id = 100 + class id`, sold by
Chronal_Quartermaster in thevoida for 25 tokens a piece. Item IDs are `9000xx + class id` with the
piece block: Boots 9002xx, Helm 9003xx, Chestguard 9004xx, Sleeves 9006xx, Bracer 9007xx,
Gloves 9008xx, Leggings 9009xx. Names are "<theme> <piece>", e.g. *Archmage Helm*, plus the usual
roll suffix.

| Set | Class | Theme |
|---|---|---|
| 101 | Warrior | Juggernaut |
| 102 | Cleric | Divine Conduit |
| 103 | Paladin | Bulwark |
| 104 | Ranger | Wildstalker |
| 105 | Shadowknight | Dread Harbinger |
| 106 | Druid | Verdant Storm |
| 107 | Monk | Iron Fist |
| 108 | Bard | Maestro |
| 109 | Rogue | Shadowblade |
| 110 | Shaman | Spiritcaller |
| 111 | Necromancer | Lich Lord |
| 112 | Wizard | Archmage |
| 113 | Magician | Elementalist |
| 114 | Enchanter | Mesmerist |
| 115 | Beastlord | Feral Bond |
| 116 | Berserker | Bloodrage |

Set bonuses are applied straight to the character; they don't show on any item.

---

## Commands

### `#dyninfo` (all players)
Compares your dynamic items, including ones in the bank and shared bank. Every item gets a number.

| Command | What it shows |
|---|---|
| `#dyninfo` | Overview by slot: what you're wearing in each slot, and each spare as "*X better, Y worse*" plus its standout affix. Clickable item numbers (`#2 #3`) appear in chat, per slot. |
| `#dyninfo <#>` | A table comparing that item with what you're wearing in its slot: **Stat / Worn / This item / Change**, gains in green, losses in red, `-` where an item lacks the stat. Clicking a number in chat does the same. |
| `#dyninfo <slot>` | Full side-by-side for one slot (e.g. `feet`, `finger`, `weapon`, `ear`, `wrist`): your equipped item, then every spare with each line's difference in green or red, and "(new)" / "(lost)" for affixes |
| `#dyninfo compare <#> <#>` | The same table for any two items, differences relative to the first |
| `#dyninfo worn` | Totals of everything your worn gear gives, as **Stat / Dynamic / Items / Sets / Total**: Dynamic = rolls on worn dynamic items, Items = other worn items plus every augment (including augments in dynamic items), Sets = active armor set bonuses, with a line per set (pieces worn, bonus thresholds, which are active). Only items you can use and meet the required level for count, and ammo gives nothing, the same as in game. Item haste shows the best one, as it works in game. Buffs and AAs aren't included, and stat caps still apply in game. |

A table too long for one window (the client's popup holds about 4,000 characters, roughly 35-40 stat
lines in a comparison) is split into pages: the title shows "(1/2)", and chat gets clickable
**[Page 2]** links. Add the page number to jump straight there: `#dyninfo 9 2`,
`#dyninfo compare 6 9 2`. The better/worse totals always count the whole comparison.

Paired slots (Ear, Wrist, Finger, and Primary/Secondary as "Weapon") are one group, compared
against the first one you're wearing. Random affixes are marked with `*`.

### `#reforge` (all players)

Replaces one random (`*`) line of a carried dynamic item, for platinum and Chronal Shards.
`#reforge` lists the items, `#reforge [slot]` shows the lines and the price.
Paying (`#reforge [slot] [line]`) and choosing (`take 1`, `take 2` or `keep`) need the Chronal Artificer in The Void targeted.
Only one line per item can ever be reforged, and each reforge of the same item costs more.
The choice is saved on the item (`dyn_offer`), so zoning doesn't lose it. See `utils/scripts/rift/README.md` for costs and rules.

### `#dynitem` (GMs, status 200+)
Acts on your target if you have a player targeted, otherwise on you.

| Command | What it does |
|---|---|
| `#dynitem give <item_id> [seed]` | Puts a rolled copy of the item on the cursor. Omit the seed for a random roll; give a seed to recreate an exact item. |
| `#dynitem info` | Lists the target's dynamic items with their **slot IDs** and **seeds** |
| `#dynitem reroll <slot_id>` | New random rolls for the item in that slot |
| `#dynitem setseed <slot_id> <seed>` | Sets an exact seed, e.g. to restore an item a player lost or to copy one for testing |
| `#dynitem reload` | Re-reads the dynamic item tables and the armor set tables, and updates every player **in this zone** |

Use `#dynitem info` to find slot IDs and seeds. Any seed can recreate the same item later with
`#dynitem give <item_id> <seed>`, as long as the tables haven't changed.

---

## What players see

* **Item name:** each copy is named after its best random roll, e.g. *Rogue Boots of the Assassin*.
* **When buying:** a chat summary, plus a comparison with what they're wearing in that slot, e.g.
  *You receive Rogue Boots of the Assassin (Tier 1 Rogue): +503 HP, +506 Endurance, +78 AC, …,
  \* +10% Backstab Damage*, then *vs equipped Rogue Boots of Parrying: HP +81, Endurance -285, …,
  Backstab Damage +10% (new), Parry lost*
* **Item window:** shows the rolled HP, Mana, Endurance, AC, regular and heroic stats and resists as
  normal stats, and the biggest skill % as its **Skill Mod** line.
* **`#dyninfo`:** everything, including the % affixes the item window can't show, and comparisons.
* **Linking the item:** clicking a link shows that copy's actual rolls if the clicker is in the same
  zone the item was loaded in; otherwise they get a short message instead.

### Name suffixes

Suffixes come from `dynamic_affix_suffixes` (`affix`, `suffix`): the item takes the suffix of its
random affix that rolled highest within its range. Edit freely; an affix with no row gives no suffix.
Names are cut to 63 characters.

```sql
UPDATE dynamic_affix_suffixes SET suffix = 'of the Night Blade' WHERE affix = 'backstab_dmg_pct';
```

---

## Rules and good practice

* Always make base items **No Trade** and **single-class**.
* Keep base item stats at **zero**.
* Only alternate-currency vendors and `#dynitem give` roll items. `#summonitem`, quest rewards and
  normal vendors give an un-rolled item with no stats.
* Settle class pools (which affixes, weights) before launch; tune ranges whenever you like.
* After changing tables, `#dynitem reload` in each running zone, or restart zones.
* `Merchant:DynamicItemSellRefundPct` (default 20) controls the sell-back refund.
* Works the same on single-class servers: `class_id` is checked against the character's class.

---

## Test checklist

**Setup**
1. Boot `world`; check its log for custom migrations up to v28.
2. Create a stat-less Rogue-only Feet item and add it to `dynamic_items` with `class_id = 9`.
3. **Run `shared_memory.exe`** (or restart the whole server) so the new item exists in game.
4. Start a zone. Its log should show `Loaded [1] dynamic items, [918] affix lines across [16] classes and [43] name suffixes`.

**As a GM on a Rogue**
1. `#dynitem give <item_id>`: item on cursor plus chat summary.
2. Inspect it: the item window shows the rolled stats.
3. Equip it: your stats rise by those amounts. `#dyninfo` lists every line.
4. Zone, then camp and log back in: rolls identical.
5. Put it in a bag, then the bank, then take it out: identical.
6. Die and loot your corpse: identical.
7. Change a range in `dynamic_affix_pool`, then `#dynitem reload`: only that line changes.
8. `#dynitem info`, then `#dynitem reroll <slot>`: new rolls.
9. `#dynitem give` to a character that isn't a Rogue: the item arrives but can't be equipped.

**With a token vendor**
1. Buy as a Rogue: rolled item plus chat summary.
2. Try to buy on a character that isn't a Rogue: refused with a message and not charged.
3. Sell it back: the price shown and the tokens paid are both 20% of the cost.

---

## Troubleshooting

| Symptom | Check |
|---|---|
| Item has no stats | Is it in `dynamic_items`? Was it given by `#summonitem` or a normal vendor (not rolled)? Does `#dynitem info` show a seed? |
| "Item X is not in dynamic_items" | Add the row, then `#dynitem reload` |
| Zone log: "unknown affix … skipping" | Typo in `dynamic_affix_pool.affix`; use a code from the reference |
| Zone log: "invalid class_id … skipping" | `dynamic_items.class_id` must be 1–16 |
| Stats higher than the rolls | The base item has stats of its own; set them to 0 |
| Changes not showing | Run `#dynitem reload` in that zone, or restart it. Bank items update on next zone. |
| Every caster item has the same affixes | That class's random pool is small; add entries or lower `max_affixes` |
| Skills window doesn't show skill % bonuses | Expected: it shows trained skill. `#show skills` lists **Current** (with bonuses) next to **Raw** |
| `#dynitem` / `#dyninfo` seem to do nothing | Replies go to the **Other Chat** window by default; scroll it down |
| "Item X does not exist" for a new item | Shared memory is out of date: stop the server, run `shared_memory.exe`, start again |
| Two copies show the same stats in the item window | The client caches items by ID. Check `Custom:DynamicItemDisplayIds` is `true`; if a window still shows old numbers, restart the game client once |
| Items misbehave after an update | Set `Custom:DynamicItemDisplayIds` to `false` and restart to rule out display IDs, then report it |
