-- Hybrid Mana to ~122% of best-in-slot (1820 over the 8 dynamic slots): 180-220 -> 265-320 per piece.
-- Paladin, Ranger, Shadowknight, Bard, Beastlord. Pure casters (360-440) unchanged.
UPDATE dynamic_affix_pool SET min_value = 265, max_value = 320
WHERE affix = 'mana' AND class_id IN (3, 4, 5, 8, 15) AND min_value = 180 AND max_value = 220;
