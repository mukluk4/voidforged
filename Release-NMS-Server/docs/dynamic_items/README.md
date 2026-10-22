# Dynamic Items — Technical Overview

Diablo-style seeded items: each copy of a dynamic item rolls its own stats once, when it is bought
from an alternate-currency (event token) merchant, and keeps them. Rolls are class-weighted and
drawn from database tables, so they can be retuned without recompiling.

For setting items up and the GM commands, see [USER_GUIDE.md](USER_GUIDE.md).

Branch: `ArmorSets` (built alongside the armor set bonus feature).

---

## Design at a glance

| Decision | Behavior |
|---|---|
| What rolls | Only items listed in `dynamic_items`. Each is **single-class** (`class_id`), even on multiclass servers. |
| When | Once, at alternate-currency purchase (or via `#dynitem give`). Never re-rolls on its own. |
| Stored per item | Only a 64-bit **seed**, in the item's `custom_data` (`dyn_seed=<n>`). No new item rows, no schema change to inventory. |
| Derived | Stats are recomputed from *seed + current tables* whenever the item is loaded. Editing a range in the tables retunes every existing item. |
| Base item | A stat-less shell in `items`. Rolled stats are added to its (zero) stats. |
| Stacking | Dynamic stats add with normal items, armor set bonuses, spells, AAs and focus effects. No caps. |
| Trading | Items are expected to be No Trade (set on the base item). |
| Sell-back | Refunds `Merchant:DynamicItemSellRefundPct` (default 20%) of the token cost, so selling and rebuying isn't a free reroll. |
| Multiclass | Purchase check uses `Mob::HasClass()` → `Client::GetClassesBits()`, which is all classes when `Custom:MulticlassingEnabled` is on and the single class otherwise. Works unchanged on single-class servers. |

---

## Data model

Created by custom migration **v26** (`2026_10_21_dynamic_items_tables`,
`common/database/database_update_manifest_custom.cpp`, content schema). `CUSTOM_BINARY_DATABASE_VERSION`
is now **26** (`common/version.h`). The migration runs on the next `world` boot.

### `dynamic_items`

| Column | Type | Meaning |
|---|---|---|
| `item_id` | INT UNSIGNED, PK | Base item that rolls |
| `class_id` | TINYINT UNSIGNED | Class pool to roll from (1–16) |
| `tier` | TINYINT UNSIGNED | Display only ("Tier N") |
| `min_affixes` / `max_affixes` | TINYINT UNSIGNED | How many random affixes roll (inclusive range) |
| `stat_multiplier` | FLOAT | Scales every rolled value on this item |
| `notes` | VARCHAR(255) | Free text |

### `dynamic_affix_pool`

| Column | Type | Meaning |
|---|---|---|
| `class_id` | TINYINT UNSIGNED, PK part | Class |
| `affix` | VARCHAR(32), PK part | Affix code (see list below) |
| `guaranteed` | TINYINT | 1 = always rolls; 0 = random affix candidate |
| `weight` | INT UNSIGNED | Pick weight for random affixes (ignored for guaranteed) |
| `min_value` / `max_value` | INT | Roll range |
| `slot_mask` | INT UNSIGNED | 0 = any item; otherwise only items whose `slots` overlap this mask |

A class with no row for an affix never gets it. v26 seeds **273 rows** covering all 16 classes
(pure melee roll Endurance, casters roll Mana, hybrids PAL/RNG/SHD/BRD/BST roll both at reduced
ranges; heroics weighted per class with a non-zero minimum). **v28** (`2026_10_21_dynamic_items_stats_and_resists`)
adds **645 rows**: guaranteed regular STR/STA/DEX/AGI/INT/WIS/CHA weighted from each class's heroic
bands; guaranteed advanced stats (Spell Damage, Heal Amount, Attack never 0; regen, Combat Effects,
shielding, Avoidance, Accuracy, Stun Resist, Strike Through and nine crit stats with `min_value = 0`);
and all 12 regular/heroic resists (including Corruption) as low-weight random affixes. Both use
`INSERT IGNORE`, so hand-tuned rows survive. `RollDynamicItem` drops guaranteed lines that roll 0.
Crit affixes are totalled in `DynamicItemBonuses` and added to `itembonuses` (`CriticalSpellChance`,
`SpellCritDmgIncrease`, `CriticalDoTChance`, `DotCritDmgIncrease`, `CriticalHealChance`,
`CriticalHealOverTime`, `CriticalHitChance[all]`, `CritDmgMod[all]`, `CriticalHitChance[Archery]`) in
`ApplySkillBonuses`, so the server's crit code and the stats window both see them.

### `dynamic_affix_suffixes` (v27)

| Column | Type | Meaning |
|---|---|---|
| `affix` | VARCHAR(32), PK | Affix code |
| `suffix` | VARCHAR(48) | Appended to the item name, e.g. "of the Assassin" |

Seeded with 31 names (v27) plus 12 resist names (v28). `CUSTOM_BINARY_DATABASE_VERSION` is now **28**.

---

## Roll algorithm (`zone/dynamic_seed_parser.cpp`)

`RollDynamicItem(seed, def, pool, item_slots)` is a pure function (no DB, logging or zone state),
so it can be tested in isolation.

* Every random number is derived by hashing `(seed, affix code, purpose)` with SplitMix64 / FNV-1a,
  not drawn sequentially from a shared RNG. Lines are therefore independent:
  * changing one line's `min_value`/`max_value` rescales **only that line**, and the value keeps its
    relative position in the range (a roll near the top stays near the top);
  * adding/removing/reweighting pool entries can change **which random affixes** an existing item
    has, but never the guaranteed lines.
* Guaranteed lines: all rows with `guaranteed = 1` that fit the item's slots.
* Random affixes: count = uniform in `[min_affixes, max_affixes]` (capped at the candidate count),
  picked without repeats by weighted reservoir sampling (key = `ln(u) / weight`).
* Value: `min + floor(u * (max - min + 1))`, times `stat_multiplier`, rounded; never below 1 when
  `min_value > 0`.
* Output is sorted by affix enum order.
* `GenerateNewItemSeed()` uses `std::random_device` (64 bits; 0 is reserved for "not rolled").

---

## How stats are applied

### Flat stats (HP, Mana, Endurance, AC, regular and heroic stats, regular and heroic resists)

Regular stats and regular resists are `int8` in `ItemData`, so `ApplyToInstance` clamps them to
±127; the others are `int32`.

`EQ::ItemInstance` gained a zone-installed hook:

* `ItemInstance::SetCustomData("dyn_seed", …)` calls `ItemInstance::dynamic_stats_hook`, which zone
  sets at boot to `DynamicItemsManager::ApplyToInstance` (`zone/main.cpp`). Every path that gives an
  item its seed goes through this setter: purchase, DB inventory load (`SetCustomDataString`),
  corpse item recreation, `#dynitem`.
* `ApplyToInstance` rolls the item and stores a copy of the base `ItemData` with the rolled flat
  stats added, via `ItemInstance::SetDynamicItemData()` (reuses `m_scaledItem`; new flag
  `m_dynamic_stats`). Copies/clones carry it over.
* `ItemInstance::GetItem()` already returns `m_scaledItem` when present, so the **normal item stat
  code** (`Mob::AddItemBonuses`, required/recommended level, equip checks, `STR`+`HeroicSTR`+cap
  mods) counts the rolls exactly like regular item stats. Nothing is double counted.
* Client item serializers (`common/patches/{rof2,rof,uf,sod,sof,titanium}.cpp`) normally send the
  *unscaled* item, because EQ scaling items are scaled client-side. They now send the rolled copy
  when `HasDynamicStats()` is true, so the owner's item window shows the real numbers. Scaling
  items are unaffected.

### Item window: display IDs, name suffix, Skill Mod line

* **Display IDs.** The RoF2 client caches item data by item ID, so every copy of the same item showed
  the first copy's stats. Each copy is now sent under its own ID, `DISPLAY_ID_BASE (0x1000000) +
  hash(seed) % (0xFFFFFFF - base)`, which is above every real item ID (this server's max is ~9.0M) and
  stable across zones and logins. `ItemInstance::m_display_id` carries it; the client serializers send
  `GetDisplayID()` when set. Nothing is stored: each zone keeps an in-memory map *display ID → (item
  ID, seed)* for copies it has built, used by `Handle_OP_ItemLinkClick` (rebuilds the exact copy, so
  links show real rolls in that zone), `OP_ItemLinkResponse`, `OP_ItemName`, `OP_ItemPreview` and
  `OP_ItemPreviewRequest`. Everything that moves or uses items works by slot and is unaffected.
  Rule `Custom:DynamicItemDisplayIds` (default true) turns it off.
* **Name suffix.** `ApplyToInstance` appends the `dynamic_affix_suffixes` entry of the random affix
  with the highest roll *position* (`RolledAffix::position`, 0-1 within its range). Truncated to the
  63-character item name limit.
* **Skill Mod line.** The copy's biggest skill % is written to `SkillModType/SkillModValue` so the
  item window shows it. `Mob::AddItemBonuses` skips the item Skill Mod for `HasDynamicStats()` items,
  since `ApplySkillBonuses` already counts every skill affix.

### Percentage affixes

Totalled per character in `Client::dynamic_item_bonuses` (`EQ::DynamicItemBonuses`,
`common/dynamic_item_structs.h`), reset at the start of `Client::CalcBonuses()` and filled from
`Mob::AddItemBonuses` (equipped, non-augment, non-tribute items with `HasDynamicStats()`; recommended
level scaling applies). They're then applied as follows:

| Affix group | Where | Mechanism |
|---|---|---|
| Weapon skill % (1HS, 2HS, 1HB, 2HB, Piercing [1H and 2H], H2H, Archery) | `DynamicItemsManager::ApplySkillBonuses`, called right after `CalcItemBonuses` | Added to `itembonuses.skillmod[skill]`, which `Client::GetSkill()` applies as a % increase of the skill value. On this server skill drives both to-hit and damage. |
| Defensive skill % (Defense, Parry, Riposte, Dodge, Block) | same | `skillmod[skill]`. Skill value drives the avoidance chances. |
| Ability skill % (Backstab, Flying Kick, Frenzy, Bash) | same | `skillmod[skill]` |
| Ability damage % (same four) | same | `itembonuses.DamageModifier[skill]` (read by `Mob::GetMeleeDamageMod_SE`) |
| Spell damage % by element (Fire/Cold/Magic/Poison/Disease) | `Mob::GetActSpellDamage`, `Mob::GetActDoTDamage` (`zone/effects.cpp`) | Base value × (1 + pct) at function entry, by `spells[].resist_type`; focuses and crits build on it |
| Healing % | `Mob::GetActSpellHealing` | Base value × (1 + pct) at function entry |
| Instrument / Singing % | `Mob::GetInstrumentMod` (`zone/client_mods.cpp`) | Multiplier on the final mod **after** the song cap. The mod is in tenths, so very small bonuses on a 1.0× mod round away. |

`ApplySkillBonuses` runs after all items because normal item skill mods are "highest wins"
(overwrite); adding afterwards keeps the result order-independent: *best normal skill mod + sum of
dynamic skill %*.

There are no weapon-damage affixes; skill % covers damage on this server.

### Armor set bonuses

`armor_set_bonuses.affix` (migration v29, which also creates the armor set tables on servers without
them) lets a set bonus use any affix code instead of the numeric `stat_id`. `LoadArmorSetBonuses`
resolves it with `AffixFromCode` (unknown codes are logged and skipped) and `CheckArmorSetBonuses`
applies it with `DynamicItemsManager::ApplyCharacterAffix`: flat stats go straight to `itembonuses`
(same fields `AddItemBonuses` fills, without the per-item caps), percentage affixes go to
`dynamic_item_bonuses`. For that reason `Client::CalcBonuses` now runs `CheckArmorSetBonuses`
**before** `ApplySkillBonuses`. `#dynitem reload` also reloads the armor set tables and recalculates
every client in the zone.

---

## Merchant integration (`zone/client_packet.cpp`)

* `Handle_OP_AltCurrencyPurchase`
  * Rejects the purchase *before* currency is deducted or quest events fire if the buyer lacks the
    item's `class_id`: "*X can only be purchased by a Y.*"
  * After `CreateItem`, calls `DynamicItemsManager::SetSeed(*inst, GenerateNewItemSeed())`, then
    sends the owner a chat summary. Also added a null check on `CreateItem` (previously a crash).
* `Handle_OP_AltCurrencySell` and `Handle_OP_AltCurrencySellSelection`
  * For dynamic items, the refund is `alt_currency_cost × DynamicItemSellRefundPct / 100`,
    overriding the item's `LDoNSellBackRate`. Both the price shown and the payout use it.

New rule (`common/ruletypes.h`): `Merchant:DynamicItemSellRefundPct` (INT, default 20).

---

## Commands (`zone/gm_commands/dynamic_items.cpp`)

| Command | Status | Purpose |
|---|---|---|
| `#dyninfo` | Player (0) | Numbered overview by slot group with better/worse counts vs equipped; `#dyninfo <slot>` side-by-side; `#dyninfo compare <#> <#>`. Includes bank and shared bank. Popups stay under the 4,095-character limit. |
| `#dynitem` | GMMgmt (200) | `give`, `info`, `reroll`, `setseed`, `reload` (acts on target if a client is targeted) |

`#dynitem reload` re-reads both tables and calls `DynamicItemsManager::RefreshClient` for every
client in the zone: re-applies rolls to possessions, bags, cursor, bank and shared bank, resends
carried items, and recalculates bonuses. Other zones keep the old tables until restarted or
reloaded.

---

## Files

**New**
* `common/dynamic_item_structs.h`: affix enum (`eDynamicAffixID`), `DynamicItemBonuses`
* `zone/dynamic_items.h`, `zone/dynamic_items.cpp`: `DynamicItemsManager` (loader, affix
  codes/names, seed get/set, `Roll`, `ApplyToInstance`, % affix application, describe/summary,
  refresh, sell refund)
* `zone/dynamic_seed_parser.cpp`: `GenerateNewItemSeed`, `RollDynamicItem`
* `zone/gm_commands/dynamic_items.cpp`: `#dynitem`, `#dyninfo`

**Modified**
* `common/item_instance.h/.cpp`: `dynamic_stats_hook`, `DYNAMIC_SEED_KEY`,
  `SetDynamicItemData`/`ClearDynamicItemData`/`HasDynamicStats`, `m_dynamic_stats`,
  `m_display_id`/`GetDisplayID`/`SetDisplayID` (copied in the copy constructor)
* `common/patches/{rof2,rof,uf,sod,sof,titanium}.cpp`: send rolled stats and the display ID for
  dynamic items
* `common/database/database_update_manifest_custom.cpp`, `common/version.h`: migrations v26-v28
* `common/ruletypes.h`: `Merchant:DynamicItemSellRefundPct`, `Custom:DynamicItemDisplayIds`
* `zone/main.cpp`: load tables, install hook
* `zone/bonuses.cpp`: reset `dynamic_item_bonuses`, % affix totals, `ApplySkillBonuses`
* `zone/effects.cpp`, `zone/client_mods.cpp`: spell, heal, instrument hooks
* `zone/client_packet.cpp`: purchase/sell handling; display ID resolution in the item lookup handlers
* `zone/command.cpp/.h`, `zone/CMakeLists.txt`: registration
* `zone/client.h`: `dynamic_item_bonuses` member

### Related fixes on this branch
* `Client::CheckArmorSetBonuses` now checks `GetClassesBits()` instead of `GetClass()`. On the
  multiclass server `GetClass()` is Bard for everyone, so class-restricted set pieces only counted
  for Bards.
* Reverted an earlier round of `const`-removal edits in `inventory.cpp`, `exp.cpp`,
  `client_evolving_items.cpp`, `item_instance.cpp` and `evolving_items.*` that broke the build and
  silently removed item placement, evolving-item checks and `ItemInstance::SetTimer`.

---

## Testing

* `RollDynamicItem` was checked standalone against the real 273 seed rows: 5,000 seeds × 16 classes
  for determinism, every guaranteed line present, values in range and non-zero, Mana/Endurance by
  class type, no duplicate affixes, range-edit isolation, pool-edit isolation, multiplier scaling,
  slot masks and weight bias.
* In-game testing: see the checklist in [USER_GUIDE.md](USER_GUIDE.md#test-checklist).

## Known limitations

* Clicking a dynamic item link shows that copy's rolls only in a zone that has loaded the copy;
  elsewhere the clicker gets a short message.
* Display IDs are a hash of the seed; two copies colliding (and so sharing a client cache entry) is
  possible but extremely unlikely.
* Chat output from `Client::Message` is printf-style; all rolled text is passed as a `%s` argument.
* `#summonitem` and quest-script item grants do **not** roll; use `#dynitem give`, or call
  `DynamicItemsManager::SetSeed` from new quest API bindings if needed.
* Changing pool membership or weights after launch re-picks random affixes on existing items of
  that class. Range edits are safe.
* Instrument % works in tenths of the instrument mod, so tiny bonuses on low mods can round to 0.

## Building on low-memory machines

The top-level `CMakeLists.txt` adds `/MP`, which spawns one compiler per CPU core regardless of
MSBuild's `-m`. Cap it:

```
cmake --build Build --config RelWithDebInfo --target zone -- -m:1 -p:CL_MPCount=2
```

In Visual Studio: Tools → Options → Projects and Solutions → VC++ Project Settings →
*Maximum concurrent C++ compilations*.
