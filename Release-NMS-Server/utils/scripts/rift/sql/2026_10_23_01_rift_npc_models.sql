-- Rift utility NPC models (needs the GlobalLoad.txt in utils/scripts/rift on the client)
UPDATE npc_types SET race = 665, gender = 2, texture = 0, size = 6  WHERE id = 1200005; -- Rift_Pylon: Grandfather Clock (gfc)
UPDATE npc_types SET race = 329, gender = 2, texture = 0, size = 12 WHERE id = 1200009; -- Rift_Portal: BoT Portal (bothunder_chr; race 426 prt.eqg renders as a default human from GlobalLoad)
UPDATE npc_types SET race = 248, gender = 2, texture = 0, size = 6  WHERE id = 1200010; -- Chronal_Quartermaster: Clockwork Golem
