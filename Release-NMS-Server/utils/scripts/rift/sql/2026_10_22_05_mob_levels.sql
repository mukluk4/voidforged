-- Queued DB fixes, round 5

-- #2 Rift mob levels: 56 conned light blue to a 65 and couldn't land spells.
UPDATE npc_types SET level = 65 WHERE id IN (1200020, 1200021, 1200022);   -- Vanguard, Evoker, Dominator (white)
UPDATE npc_types SET level = 67 WHERE id = 1200023;                        -- Executioner (yellow)
UPDATE npc_types SET level = 70 WHERE id = 1200001;                        -- Rift Guardian (red)
