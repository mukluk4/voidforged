-- Tier jewelers: race 88 (clockwork gnome) isn't loaded in The Void, so the client drew them as plain humans.
-- They now use the Quartermasters' clockwork golem (race 248). Applied to the live DB 2026-11-11; the old rows
-- are in npc_types_bak_2026_11_11_jewelers.
UPDATE npc_types SET race = 248, gender = 2, texture = 0, size = 6
WHERE id IN (1200030, 1200033, 1200036, 1200039, 1200042) AND race = 88;
