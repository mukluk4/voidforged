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
