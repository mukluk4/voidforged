-- Berserkers can't use one-handed weapons: drop their 1H skill % affixes.
-- Seed-only storage, so existing items lose the line (and reroll their random picks) on #dynitem reload / zone boot.
DELETE FROM dynamic_affix_pool WHERE class_id = 16 AND affix IN ('skill_1hb_pct', 'skill_1hs_pct');
