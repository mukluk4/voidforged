# Dynamic item pool generators

These scripts produce the starting `dynamic_affix_pool` data in custom migrations v26 and v28
(`common/database/database_update_manifest_custom.cpp`). The live table can be tuned by hand; these
only matter when you want to regenerate the defaults.

```
python gen_v26_pool.py > v26_rows.sql                     # base lines + random class affixes
python gen_v28_pool.py gen_v26_pool.py v28_rows.sql       # regular stats, resists, advanced stats, crits, suffixes
python blend_hybrids.py v26_rows.sql v28_rows.sql         # DPS DEX, hybrid 75/25 blend, hybrid AC at 90%
```

Paste the rows from `v26_rows.sql` into the v26 entry and `v28_rows.sql` into v28. Keep each raw
string literal under ~15,000 characters (MSVC error C2026): v28 is split into several `INSERT`
statements of about 150 rows each.

Balance notes (2026-10-21): ranges are set a little above Plane of Time drops (about 180 HP, 175
mana, 30 AC and 12 per stat, no heroics, all five resists near 13).

Later passes change the live table directly and save their SQL here:

| Date | Script | SQL | What it does |
|---|---|---|---|
| 2026-10-22 | `rebalance_heroics.py` | `2026_10_22_heroics_base_rebalance.sql` | Heroic bands and base HP / Mana / AC targets |
| 2026-10-23 | `rebalance_to_bis.py` | `2026_10_23_bis_rebalance.sql` | 8 visible pieces at ~122% of melee best-in-slot (migration v30) |
| 2026-10-24 | `rebalance_casters.py` | `2026_10_24_caster_rebalance.sql` | Casters at ~122% of caster best-in-slot: INT/WIS 480, Spell Damage 480 (priests 350), Heal Amount 480 for priests; caster STR, DEX, AGI, Attack, AC and HP regen down. **Not in a migration yet.** |

Each rebalance first copies the table to `dynamic_affix_pool_backup_<date>_<name>`.
