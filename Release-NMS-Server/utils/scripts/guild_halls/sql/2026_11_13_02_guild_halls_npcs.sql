-- The Guild Steward and the Guild Hall Porter in the Bazaar, west of the safe spot. Run `#zoneshutdown zone bazaar`
-- first (or apply while the server is down): a running zone saves its NPCs on shutdown and restores them on boot,
-- ignoring new spawn2 rows. Both are modeled on the Raid Portal row (unkillable, no aggro) with a human look.

START TRANSACTION;

CREATE TEMPORARY TABLE gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200124;
INSERT INTO gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200118 WHERE id = 1200071;
UPDATE gh_npcs SET race = 1, gender = 0, texture = 1, helmtexture = 0, size = 6, bodytype = 1,
	lastname = 'Guild Treasury', name = 'Guild_Steward' WHERE id = 1200124;
UPDATE gh_npcs SET race = 1, gender = 1, texture = 1, helmtexture = 0, size = 6, bodytype = 1,
	lastname = 'Guild Halls', name = 'Guild_Hall_Porter' WHERE id = 1200118;
INSERT INTO npc_types SELECT * FROM gh_npcs;
DROP TEMPORARY TABLE gh_npcs;

INSERT INTO spawngroup (id, name) VALUES (5003915, 'Guild_Steward'), (5003916, 'Guild_Hall_Porter');
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES (5003915, 1200124, 100), (5003916, 1200118, 100);
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, path_when_zone_idle, _condition, cond_value, animation, min_expansion, max_expansion) VALUES
	(5003915, 'bazaar', 0, -145.00, -785.00, 3.75, 128.00, 30, 0, 0, 0, 0, 1, 0, -1, -1),
	(5003916, 'bazaar', 0, -125.00, -785.00, 3.75, 128.00, 30, 0, 0, 0, 0, 1, 0, -1, -1);

DELETE FROM zone_state_spawns WHERE zone_id = 151;

COMMIT;
