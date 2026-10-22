-- Queued DB fixes, round 2 (apply on "ok to update", after the zone/world build with the new rule)

-- #8 Rift archetypes get real roles (all four were identical level-56 Warriors with no spells)
--    Vanguard (1200020) stays a Warrior
UPDATE npc_types SET class = 12, npc_spells_id = 2 WHERE id = 1200021;            -- Evoker: Wizard, Default Wizard List
UPDATE npc_types SET class = 10, npc_spells_id = 6 WHERE id = 1200022;            -- Dominator: Shaman, Default Shaman List (slows, roots, DoTs)
UPDATE npc_types SET special_abilities = '3,1^5,1' WHERE id = 1200023;            -- Executioner: Rampage, Flurry

-- #3 Kills no longer change faction (quest faction still works). Set back to 'true' to restore.
REPLACE INTO rule_values (ruleset_id, rule_name, rule_value, notes)
VALUES (1, 'Custom:KillFactionHits', 'false', 'Killing an NPC adjusts the killers faction. False: kills never change faction');
