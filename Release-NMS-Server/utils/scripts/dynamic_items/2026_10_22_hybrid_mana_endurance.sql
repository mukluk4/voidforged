-- Hybrids (Paladin, Ranger, Shadowknight, Bard, Beastlord): 50% of a caster's mana (360-440) and
-- 50% of a melee's endurance (290-350), instead of the 25% / 75% blend
UPDATE dynamic_affix_pool SET min_value = 180, max_value = 220 WHERE affix = 'mana'      AND class_id IN (3, 4, 5, 8, 15);
UPDATE dynamic_affix_pool SET min_value = 145, max_value = 175 WHERE affix = 'endurance' AND class_id IN (3, 4, 5, 8, 15);
