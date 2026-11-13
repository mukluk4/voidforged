-- The T1 guild hall: Highkeep's third floor (noble quarters) as zone version 60. Each guild gets its own instance
-- (plugins/guild_halls.pl GuildHallEnter). Version 60 has none of Highkeep's NPCs or zone exits; doors are copied
-- unlocked. The hall NPCs stand near the ladies in waiting; positions are first guesses until the #loc walk-through.
-- Apply while Highkeep is not running (zone state saving), or run `#zoneshutdown zone highkeep` first.

START TRANSACTION;

-- Zone row: no requirements, arrival on the third floor
CREATE TEMPORARY TABLE gh_zone SELECT * FROM zone WHERE short_name = 'highkeep' AND version = 0;
UPDATE gh_zone SET id = 0, version = 60, flag_needed = '', min_level = 0, min_status = 0,
	safe_x = -163, safe_y = 80, safe_z = 65.63, safe_heading = 0;
INSERT INTO zone SELECT * FROM gh_zone;
DROP TEMPORARY TABLE gh_zone;

-- Doors, unlocked. No zone points: members leave through the Hall Doorman, a gate or an evacuation.
CREATE TEMPORARY TABLE gh_doors SELECT * FROM doors WHERE zone = 'highkeep' AND version = 0;
UPDATE gh_doors SET id = 0, version = 60, keyitem = 0, lockpick = 0;
INSERT INTO doors SELECT * FROM gh_doors;
DROP TEMPORARY TABLE gh_doors;

-- Hall NPCs, modeled on the Raid Portal row (unkillable, no aggro) with a human look:
-- 1200119 Guild_Hall_Mystic (buffs), 1200120 Hall_Banker (personal bank), 1200121 Hall_Tribute_Master
-- (guild tribute), 1200122 Hall_Hearthkeeper (regen pool), 1200123 Hall_Doorman (back to the Bazaar)
CREATE TEMPORARY TABLE gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200119;
INSERT INTO gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200120 WHERE id = 1200071;
INSERT INTO gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200121 WHERE id = 1200071;
INSERT INTO gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200122 WHERE id = 1200071;
INSERT INTO gh_npcs SELECT * FROM npc_types WHERE id = 1200071;
UPDATE gh_npcs SET id = 1200123 WHERE id = 1200071;
UPDATE gh_npcs SET race = 1, texture = 1, helmtexture = 0, size = 6, bodytype = 1;
UPDATE gh_npcs SET gender = 1, lastname = 'Guild Blessings', name = 'Guild_Hall_Mystic' WHERE id = 1200119;
UPDATE gh_npcs SET gender = 0, lastname = 'Banker', name = 'Hall_Banker', class = 40 WHERE id = 1200120;
UPDATE gh_npcs SET gender = 1, lastname = 'Guild Tribute', name = 'Hall_Tribute_Master', class = 64 WHERE id = 1200121;
UPDATE gh_npcs SET gender = 1, lastname = 'Hearth and Rest', name = 'Hall_Hearthkeeper' WHERE id = 1200122;
UPDATE gh_npcs SET gender = 0, lastname = 'Way Out', name = 'Hall_Doorman' WHERE id = 1200123;
INSERT INTO npc_types SELECT * FROM gh_npcs;
DROP TEMPORARY TABLE gh_npcs;

INSERT INTO spawngroup (id, name) VALUES
	(5003917, 'Guild_Hall_Mystic'), (5003918, 'Hall_Banker'), (5003919, 'Hall_Tribute_Master'),
	(5003920, 'Hall_Hearthkeeper'), (5003921, 'Hall_Doorman');
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES
	(5003917, 1200119, 100), (5003918, 1200120, 100), (5003919, 1200121, 100),
	(5003920, 1200122, 100), (5003921, 1200123, 100);
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, path_when_zone_idle, _condition, cond_value, animation, min_expansion, max_expansion) VALUES
	(5003917, 'highkeep', 60, -170.00, 70.00, 65.63, 0.00, 30, 0, 0, 0, 0, 1, 0, -1, -1),
	(5003918, 'highkeep', 60, -155.00, 70.00, 65.63, 0.00, 30, 0, 0, 0, 0, 1, 0, -1, -1),
	(5003919, 'highkeep', 60, -155.00, 90.00, 65.63, 256.00, 30, 0, 0, 0, 0, 1, 0, -1, -1),
	(5003920, 'highkeep', 60, -170.00, 90.00, 65.63, 256.00, 30, 0, 0, 0, 0, 1, 0, -1, -1),
	(5003921, 'highkeep', 60, -163.00, 72.00, 65.63, 0.00, 30, 0, 0, 0, 0, 1, 0, -1, -1);

DELETE FROM zone_state_spawns WHERE zone_id = 6;

COMMIT;
