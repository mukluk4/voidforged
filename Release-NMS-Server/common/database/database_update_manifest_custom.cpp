#include "database_update.h"

std::vector<ManifestEntry> manifest_entries_custom = {
	ManifestEntry{
		.version = 1,
		.description = "2025_05_16_new_database_check_test",
		.check = "SHOW TABLES LIKE 'new_table'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `new_table`  (
  `id` int NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`)
);
)",
		.content_schema_update = false,
	},

	// Content schema tables
	ManifestEntry{
		.version = 2,
		.description = "2025_05_16_nms_waypoints_content_tables",
		.check = "SHOW TABLES LIKE 'nms_waypoints_categories'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE nms_waypoints_categories (
    id INT PRIMARY KEY,
    name VARCHAR(32) NOT NULL UNIQUE
);

CREATE TABLE nms_waypoints (
    id INT AUTO_INCREMENT PRIMARY KEY,
    shortname VARCHAR(32) UNIQUE NOT NULL,
    long_name VARCHAR(64) NOT NULL,
    category INT NOT NULL,
    x FLOAT NOT NULL,
    y FLOAT NOT NULL,
    z FLOAT NOT NULL,
    heading FLOAT NOT NULL,
    INDEX idx_nms_waypoints_shortname (shortname),
    INDEX idx_nms_waypoints_category (category)
);

CREATE TABLE nms_waypoints_default (
    id INT AUTO_INCREMENT PRIMARY KEY,
    waypoint_id INT NOT NULL,
    race_id INT NOT NULL DEFAULT 0,
    class_mask INT UNSIGNED NOT NULL DEFAULT 65535,
    min_level INT NOT NULL DEFAULT 1,
    max_level INT NOT NULL DEFAULT 255,
    UNIQUE KEY unique_default_waypoint (waypoint_id, race_id, class_mask, min_level),
    INDEX idx_nms_waypoints_default_race (race_id),
    INDEX idx_nms_waypoints_default_class (class_mask),
    INDEX idx_nms_waypoints_default_level (min_level, max_level)
);
)",
		.content_schema_update = true,
	},

	// PEQ schema tables
	ManifestEntry{
		.version = 3,
		.description = "2025_05_16_nms_waypoints_peq_tables",
		.check = "SHOW TABLES LIKE 'nms_waypoints_character'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE nms_waypoints_character (
    id INT AUTO_INCREMENT PRIMARY KEY,
    character_id BIGINT UNSIGNED NOT NULL,
    waypoint_id INT NOT NULL,
    unlock_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY unique_character_waypoint (character_id, waypoint_id),
    INDEX idx_nms_waypoints_character_id (character_id)
);

CREATE TABLE nms_waypoints_account (
    id INT AUTO_INCREMENT PRIMARY KEY,
    account_id BIGINT UNSIGNED NOT NULL,
    waypoint_id INT NOT NULL,
    unlock_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY unique_account_waypoint (account_id, waypoint_id),
    INDEX idx_nms_waypoints_account_id (account_id)
);
)",
		.content_schema_update = false,
	},

	ManifestEntry{
		.version = 4,
		.description = "2025_05_20_nms_zone_npc_update_range",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'npc_update_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `npc_update_range` int(11) NOT NULL DEFAULT 600 AFTER `npc_max_aggro_dist`;
)",
		.content_schema_update = true,
	},

	ManifestEntry{
		.version = 5,
		.description = "2025_05_20_nms_zone_max_movement_range",
		.check = "SHOW COLUMNS FROM `zone` LIKE 'max_movement_update_range'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `zone` ADD COLUMN `max_movement_update_range` int(11) NOT NULL DEFAULT 600 AFTER `npc_update_range`;
)",
		.content_schema_update = true,
	},

	ManifestEntry{
		.version = 6,
		.description = "2025_05_20_nms_global_buffs",
		.check = "SHOW TABLES LIKE 'global_buffs'",
		.condition = "empty",
		.match = "",
		.sql = R"(
create table global_buffs(spell_id int(11) primary key, duration int(11));
)",
		.content_schema_update = false,
	},

	ManifestEntry{
		.version = 7,
		.description = "2025_05_20_nms_account_kill_counts",
		.check = "SHOW TABLES LIKE 'account_kill_counts'",
		.condition = "empty",
		.match = "",
		.sql = R"(
create table account_kill_counts(account_id int(11) primary key, race_id int(11), count int(11));
)",
		.content_schema_update = false,
	},

	ManifestEntry{
		.version = 8,
		.description = "2025_05_20_nms_character_pet_class_id",
		.check = "SHOW COLUMNS FROM `character_pet_name` LIKE 'class_id'",
		.condition = "empty",
		.match = "",
		.sql = R"(
	ALTER TABLE `character_pet_name`
		DROP PRIMARY KEY,
		ADD COLUMN `class_id` TINYINT(11) NOT NULL DEFAULT -1,
		ADD PRIMARY KEY (`character_id`, `class_id`);
	)",
		.content_schema_update = false,
	},

	ManifestEntry{
		.version = 9,
		.description = "2025_05_20_nms_account_alt_currency",
		.check = "SHOW TABLES LIKE 'account_alt_currency'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `account_alt_currency` (
  `account_id` int(10) unsigned NOT NULL,
  `currency_id` int(10) unsigned NOT NULL,
  `amount` int(10) unsigned NOT NULL,
  PRIMARY KEY (`account_id`, `currency_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO account_alt_currency (account_id, currency_id, amount)
SELECT
  cd.account_id,
  cac.currency_id,
  SUM(cac.amount) AS amount
FROM character_alt_currency cac
JOIN character_data cd ON cd.id = cac.char_id
GROUP BY cd.account_id, cac.currency_id
ON DUPLICATE KEY UPDATE
  amount = VALUES(amount);
)",
		.content_schema_update = false,
	},

	// New familiar_names table
	ManifestEntry{
		.version = 10,
		.description = "2025_05_20_familiar_names_table",
		.check = "SHOW TABLES LIKE 'familiar_names'",
		.condition = "empty",
		.match = "",
		.sql = R"(
	CREATE TABLE `familiar_names` (
	  `spell_id` int(10) NOT NULL,
	  `name_list` text NOT NULL,
	  `size_mod` int(10) NOT NULL DEFAULT -1,
	  PRIMARY KEY (`spell_id`)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
	)",
		.content_schema_update = true,
	},

	// Alter character_pet_name to change class_id from TINYINT to INT
	ManifestEntry{
		.version = 11,
		.description = "2025_05_20_modify_character_pet_name_class_id",
		.check = "SHOW COLUMNS FROM `character_pet_name` WHERE `Field` = 'class_id' AND `Type` LIKE 'tinyint%'",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
			ALTER TABLE `character_pet_name`
			MODIFY COLUMN `class_id` INT NOT NULL DEFAULT 0
			)",
		.content_schema_update = false,
	},

	// Add character_aa_disabled table
	ManifestEntry{
		.version = 12,
		.description = "2025_05_20_nms_character_aa_disabled",
		.check = "SHOW TABLES LIKE 'character_aa_disabled'",
		.condition = "empty",
		.match = "",
		.sql = R"(
	CREATE TABLE `character_aa_disabled` (
	`aa_id` int(10) NOT NULL,
	`character_id` int(10) NOT NULL,
	`disabled` tinyint(4) NOT NULL,
	PRIMARY KEY (`aa_id`,`character_id`),
	KEY `idx_character_id` (`character_id`)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
	)",
		.content_schema_update = false,
	},

	// Add table for pet command states
	ManifestEntry{
		.version = 13,
		.description = "2025_05_26_character_pet_command_states_table",
		.check = "SHOW TABLES LIKE 'character_pet_command_states'",
		.condition = "empty",
		.match = "",
		.sql = R"(
	CREATE TABLE `character_pet_command_states` (
	`character_id` int(10) NOT NULL,
	`pet_class` tinyint(4) NOT NULL,
	`command_id` tinyint(4) NOT NULL,
	`command_state` tinyint(4) NOT NULL,
	PRIMARY KEY (`character_id`,`pet_class`,`command_id`),
	KEY `idx_char_petclass` (`character_id`,`pet_class`)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
	)",
		.content_schema_update = false,
	},

	// Add character_dynamic_aa_timers table
	ManifestEntry{
		.version = 14,
		.description = "2025_05_27_character_dynamic_aa_timers_table",
		.check = "SHOW TABLES LIKE 'character_dynamic_aa_timers'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `character_dynamic_aa_timers` (
  `character_id` int(10) NOT NULL,
  `aa_id` int(10) NOT NULL,
  `timer_id` int(10) NOT NULL,
  PRIMARY KEY (`character_id`,`aa_id`),
  UNIQUE KEY `character_id` (`character_id`,`timer_id`),
  KEY `aa_id` (`aa_id`),
  KEY `timer_id` (`timer_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)",
		.content_schema_update = false,
	},

/*
	// Create account_character_sets table
	ManifestEntry{
		.version = 15,
		.description = "2025_05_23_create_account_character_sets_table",
		.check = "SHOW TABLES LIKE 'account_character_sets'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `account_character_sets` (
	`account_id` int(11) NOT NULL,
	`set_id` int(11) NOT NULL,
	`set_name` varchar(255) NOT NULL,
	`created_at` timestamp DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (`account_id`, `set_id`),
	UNIQUE KEY `unique_account_set_name` (`account_id`, `set_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)",
		.content_schema_update = false,
	},

	// Create account_character_set_members table
	ManifestEntry{
		.version = 16,
		.description = "2025_05_23_create_account_character_set_members_table",
		.check = "SHOW TABLES LIKE 'account_character_set_members'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `account_character_set_members` (
	`account_id` int(11) NOT NULL,
	`set_id` int(11) NOT NULL,
	`character_id` int(11) NOT NULL,
	PRIMARY KEY (`account_id`, `set_id`, `character_id`),
	INDEX `idx_set_id` (`set_id`),
	INDEX `idx_character_id` (`character_id`),
	INDEX `idx_account_set` (`account_id`, `set_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)",
		.content_schema_update = false,
	},

	// Create limits table
	ManifestEntry{
		.version = 17,
		.description = "2025_05_23_create_account_character_set_limits_table",
		.check = "SHOW TABLES LIKE 'account_character_set_limits'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE `account_character_set_limits` (
	`account_id` int(11) NOT NULL,
	`eom_sets` int(11) NOT NULL DEFAULT 0,
	`bonus_sets` int(11) NOT NULL DEFAULT 0,
	`default_set` int(11) NOT NULL DEFAULT 0,
	`eom_slots` int(11) NOT NULL DEFAULT 0,
	`bonus_slots` int(11) NOT NULL DEFAULT 0,
	PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
	)",
	},
*/

	ManifestEntry{
		.version = 18,
		.description = "2026_06_13_update_aa339_whitelist_and_logs.sql",
		.check = "SELECT `rule_value` FROM `rule_values` WHERE `rule_name` = 'Custom:AA339Whitelist'",
		.condition = "missing",
		.match = "aa516",
		.sql = R"(
-- Upsert, not UPDATE: on a DB where the rule row does not exist yet, a bare UPDATE matches
-- zero rows and silently does nothing (this bit us in production - see versions 30-32).
INSERT INTO rule_values (ruleset_id, rule_name, rule_value, notes)
VALUES (1, 'Custom:AA339Whitelist', '16121,16122,16123,16675,16676,16677,30887,30888,30889,aa545,aa516,aa861,aa8332,8165,8166,8167,8168,8169', 'AA339 whitelist')
ON DUPLICATE KEY UPDATE rule_value = VALUES(rule_value);

UPDATE logsys_categories 
SET log_to_file = 0 
WHERE log_category_description NOT IN ('Error', 'Warning', 'Crash', 'MySQL Error', 'QuestErrors');

-- 1. Create the Custom NPC (Female Vah Shir)
INSERT INTO npc_types (id, name, lastname, level, race, class, bodytype, hp, mana, gender, texture, helmtexture, size, hp_regen_rate, hp_regen_per_second, mana_regen_rate, loottable_id, merchant_id, STR, STA, DEX, AGI, _INT, WIS, CHA, npc_faction_id, face)
VALUES (1120001300, 'A_Beastlord_Spell_Merchant', 'Spells 51-60', 60, 130, 41, 1, 1000, 1000, 1, 1, 0, 7, 0, 0, 0, 0, 1120001300, 75, 75, 75, 75, 75, 75, 75, 0, 3)
ON DUPLICATE KEY UPDATE class=VALUES(class), race=VALUES(race), gender=VALUES(gender), texture=VALUES(texture), size=VALUES(size), face=VALUES(face);

-- 2. Populate the Level 51-60 Merchantlist (38 scrolls)
INSERT INTO merchantlist (merchantid, slot, item) VALUES
(1120001300, 1, 15063),  -- Spell: Resist Disease (51)
(1120001300, 2, 15046),  -- Spell: Ultravision (51)
(1120001300, 3, 7740),   -- Spell: Spirit of Wind (51)
(1120001300, 4, 19363),  -- Spell: Summon: Muzzle of Mardu (51)
(1120001300, 5, 19499),  -- Spell: Spiritual Radiance (52)
(1120001300, 6, 7722),   -- Spell: Aid of Khurenz (52)
(1120001300, 7, 15435),  -- Spell: Venom of the Snake (52)
(1120001300, 8, 15161),  -- Spell: Health (52)
(1120001300, 9, 7741),   -- Spell: Spirit of the Storm (53)
(1120001300, 10, 15167), -- Spell: Talisman of Tnarg (53)
(1120001300, 11, 15152), -- Spell: Deftness (53)
(1120001300, 12, 26957), -- Spell: Ice Shard (54)
(1120001300, 13, 19531), -- Spell: Spirit of Snow (54)
(1120001300, 14, 7723),  -- Spell: Spirit of Omakin (54)
(1120001300, 15, 15062), -- Spell: Resist Poison (54)
(1120001300, 16, 15153), -- Spell: Furious Strength (54)
(1120001300, 17, 15145), -- Spell: Chloroplast (55)
(1120001300, 18, 7724),  -- Spell: Sha's Restoration (55)
(1120001300, 19, 15163), -- Spell: Incapacitate (56)
(1120001300, 20, 7726),  -- Spell: Spirit of Zehkes (56)
(1120001300, 21, 15431), -- Spell: Shifting Shield (56)
(1120001300, 22, 19530), -- Spell: Spirit of Flame (56)
(1120001300, 23, 15157), -- Spell: Dexterity (57)
(1120001300, 24, 15158), -- Spell: Stamina (57)
(1120001300, 25, 15015), -- Spell: Greater Healing (57)
(1120001300, 26, 15049), -- Spell: Nullify Magic (58)
(1120001300, 27, 59617), -- Spell: Guard of Calliav (58)
(1120001300, 28, 7727),  -- Spell: Spirit of Khurenz (58)
(1120001300, 29, 15168), -- Spell: Talisman of Altuna (58)
(1120001300, 30, 15510), -- Spell: Blizzard Blast (59)
(1120001300, 31, 7729),  -- Spell: Spiritual Purity (59)
(1120001300, 32, 19507), -- Spell: Chloroblast (59)
(1120001300, 33, 7728),  -- Spell: Sha's Ferocity (59)
(1120001300, 34, 15170), -- Spell: Alacrity (60)
(1120001300, 35, 7730),  -- Spell: Spiritual Strength (60)
(1120001300, 36, 19537), -- Spell: Savagery (60)
(1120001300, 37, 7731),  -- Spell: Spirit of Khati Sha (60)
(1120001300, 38, 19538)
ON DUPLICATE KEY UPDATE item=VALUES(item);

-- 3. Create Spawngroups
INSERT INTO spawngroup (name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer, wp_spawns)
VALUES ('fv_Beastlord_Spell_Merchant', 1, 0, 0, 0, 0, 0, 45000, 15000, 0, 100, 0),
       ('ot_Beastlord_Spell_Merchant', 1, 0, 0, 0, 0, 0, 45000, 15000, 0, 100, 0)
ON DUPLICATE KEY UPDATE name=VALUES(name);

-- 4. Link Spawngroups to Custom NPC
INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES
((SELECT id FROM spawngroup WHERE name = 'fv_Beastlord_Spell_Merchant'), 1120001300, 100),
((SELECT id FROM spawngroup WHERE name = 'ot_Beastlord_Spell_Merchant'), 1120001300, 100)
ON DUPLICATE KEY UPDATE chance=VALUES(chance);

-- 5. Place in Zones (spawn2)
INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, path_when_zone_idle, _condition, cond_value, animation) VALUES
((SELECT id FROM spawngroup WHERE name = 'fv_Beastlord_Spell_Merchant'), 'firiona', 0, -3356.63, 388.16, 158.98, 87.75, 640, 0, 0, 0, 0, 1, 0),
((SELECT id FROM spawngroup WHERE name = 'ot_Beastlord_Spell_Merchant'), 'overthere', 0, 2341.93, 2809.66, -51.22, 195, 640, 0, 0, 0, 0, 1, 0)
ON DUPLICATE KEY UPDATE spawngroupID=VALUES(spawngroupID);

-- 6. Update Uggrig Skullchomper to be a Beastlord GM (Class 34)
UPDATE npc_types SET class = 34 WHERE id = 93152;

-- 7. Update Quegmor O`Cranic to be a Beastlord GM (Class 34) and move/ensure spawn
UPDATE npc_types SET class = 34 WHERE id = 84202;
UPDATE spawn2 SET x = -3311.55, y = 415.63, z = 158.89, heading = 373 WHERE id = 14745;
UPDATE spawnentry SET chance = 100 WHERE spawngroupID = 9696 AND npcID = 84202;
)",
		.content_schema_update = false,
	},
	ManifestEntry{
		.version = 19,
		.description = "2026_06_27_fix_broken_npc_factions.sql",
		.check = "SELECT npc_faction_id FROM npc_types WHERE id = 46016",
		.condition = "match",
		.match = "19471",
		.sql = R"(
UPDATE npc_types SET npc_faction_id = 929 WHERE id IN (46016, 46017, 46061, 46089);
UPDATE npc_types SET npc_faction_id = 79 WHERE id = 2000507;
UPDATE rule_values SET rule_value = '1.0' WHERE rule_name = 'Merchant:BuyCostMod';
UPDATE db_str SET value = 'This passive ability grants you a chance to have your damage over time spells hit for additional damage. Each level increases both the chance and amount of damage.' WHERE id = 15594 AND type = 4;
)",
		.content_schema_update = true,
	},
	ManifestEntry{
		.version = 20,
		.description = "2026_07_01_bazaar_spawn_sateal.sql",
		.check = "SELECT id FROM npc_types WHERE name = 'Sateal_Deirosap' AND lastname = 'Smithing Supplies'",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- 1. Spawn Sateal Deirosap in Bazaar version 0 (Classic Bazaar)
INSERT INTO `spawn2` (`spawngroupID`, `zone`, `version`, `x`, `y`, `z`, `heading`, `respawntime`, `variance`, `pathgrid`, `_condition`, `cond_value`)
VALUES (111116, 'bazaar', 0, -38.000000, -676.000000, 4.750000, 126.000000, 640, 0, 0, 0, 1);

-- 2. Update Sateal's title to "Smithing Supplies"
UPDATE `npc_types` SET `lastname` = 'Smithing Supplies' WHERE `name` = 'Sateal_Deirosap';

-- 3. Update Yekan's Quickening scroll base price to target 1p 4g final merchant price (667 copper base)
UPDATE items SET price = 667 WHERE id = 7719;
)",
		.content_schema_update = true,
	},

	ManifestEntry{
		.version = 21,
		.description = "2026_07_12_bazaar_aa_and_data_fixes.sql",
		.check = "SELECT 1",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- The Bazaar is a trade hub but was the only hub with combat enabled (cancombat = 1); PoK,
-- Nexus, Guild Lobby and Plane of Tranquility are all cancombat = 0. That let players kill
-- NPCs (and each other) in the Bazaar. Flag it safe like the other hubs (covers both version
-- rows). IsAttackAllowed() gates all melee on zone->CanDoCombat().
UPDATE `zone` SET `cancombat` = 0 WHERE `short_name` = 'bazaar';

-- Hastened Gathering ranks 4 and 5 (custom ranks 12899/12900) reduced Gather Mana to nothing.
--
-- SE_HastenedAASkill (SPA 264) reductions are FLAT SECONDS, and each Hastened Gathering rank carries
-- two effect slots: slot 1 targets Gather Mana (aa_ability 57, base recast 3600s) and slot 2 targets
-- Mana Draw (616), whose upgraded ranks are 8640s. The custom ranks were scaled for the 8640s target
-- -- 3456 is ~40% of 8640, but 96% of 3600. So rank 4 cut Gather Mana to 144s and rank 5 (-4320)
-- exceeded its base entirely, clamping the timer to 0 and making it free.
--
-- Slot 1 only ever affects Gather Mana (GetAlternateAdvancementCooldownReduction filters on
-- limit_value), so rescaling it leaves the Mana Draw side untouched. The vanilla ranks step +400/rank
-- against the 3600s base (400/800/1200); 1600/2000 continue that line, giving 33 and 27 minutes.
-- Vanilla ranks 471/472/473 are deliberately NOT touched -- they are correct.
--
-- Dormant when written (both ranks are level_req 80; the cap is well below that), fixed now so it
-- cannot surprise us at the next expansion bump.
UPDATE `aa_rank_effects` SET `base1` = 1600 WHERE `rank_id` = 12899 AND `slot` = 1 AND `base2` = 57;
UPDATE `aa_rank_effects` SET `base1` = 2000 WHERE `rank_id` = 12900 AND `slot` = 1 AND `base2` = 57;

-- aa_ranks 12900 pointed next_id at rank 12901, which does not exist. Harmless (the chain just ends)
-- but it leaves a dangling reference in the rank list.
UPDATE `aa_ranks` SET `next_id` = -1 WHERE `id` = 12900;

-- Move the Firiona Vie Beastlord trainer and spell merchant out of the drolvarg camp, into the city.
--
-- This REVERSES a placement made by v21's own predecessor. Version 18 (above, "7. Update Quegmor
-- O`Cranic to be a Beastlord GM") made Quegmor a Beastlord GM -- he is not one in stock PEQ -- and
-- parked him at (-3311.55, 415.63, z=158.89), then placed the new Beastlord Spell Merchant beside him
-- at (-3356.63, 388.16, z=158.98).
--
-- Those coordinates are the ABANDONED pre-revamp Firiona Vie outpost. Firiona Vie was revamped and the
-- outpost moved south to z = -76; roughly 30 drolvarg now spawn on the old site. Evidence the south is
-- the live city: it holds 64 of the zone's 68 doors, has ~4x the map geometry, contains the zone's safe
-- point (1440, -2392), and is where 28 of the 30 named city NPCs already stand.
--
-- So Beastlords were the only class that had to fight through a mob camp to train or buy spells -- and
-- this merchant carries ALL 38 Beastlord scrolls, making that camp the only source in the zone. Every
-- other class trains and buys in the city.
--
-- Why v18 chose those coordinates: the common Firiona Vie map packs still label the OLD outpost with
-- the city's NPC names (their geometry was updated after the revamp; their hand-maintained label files
-- never were). The map says "town" there. It is wrong, and it is almost certainly the original source
-- of this bug.
--
-- The two UPDATEs target differently ON PURPOSE -- do not "clean this up" into one style:
--   * Quegmor's spawn2 row is stock PEQ data, so id 14745 is identical on every server. v18 relies on
--     this exact assumption one entry above, so it is proven on real installs.
--   * The merchant's spawn2 row is INSERTed by v18, so its id comes from AUTO_INCREMENT and WILL differ
--     per server (it is 3388065 on dev -- meaningless anywhere else). It must be resolved through its
--     spawngroup name, exactly as v18 inserts it. A hardcoded id here would silently move the wrong
--     spawn, or nothing at all.
--
-- Ordering is safe either way: the manifest applies versions in ascending order, so on a fresh install
-- v18 creates these at the old coordinates and v21 immediately corrects them; on a server already past
-- v18, v21 corrects them in place.
UPDATE spawn2 SET x = 2794.85, y = -3350.75, z = -76.12, heading = 385.5
  WHERE id = 14745;
UPDATE spawn2 SET x = 2810.72, y = -3378.81, z = -76.12, heading = 380.75
  WHERE spawngroupID IN (SELECT id FROM spawngroup WHERE name = 'fv_Beastlord_Spell_Merchant');

-- Remove "Boomerang of Wonders" and its variants. Its model (idfile IT11383) exists in NO RoF2-era
-- client -- not even the newest reference client -- so it always rendered empty-handed. It is also
-- completely unreachable in-game: not in any loot table, merchant, tradeskill recipe, quest, or plugin,
-- and the Purveyor of Glamour cannot produce its glamour. The hand-in path needs the base item, which
-- is unobtainable; the random-glamour path (get_random_glamour) skips it because it is a RANGE-slot
-- (2048) item with itemtype 10, while that query only accepts range items with itemtype 5. So these
-- four rows are pure orphaned data. Deletes base (109036), Enchanted (1109036), Legendary (2109036)
-- and the Glamour ornament (704681).
DELETE FROM items WHERE id IN (109036, 1109036, 2109036, 704681);

-- Move the Bazaar safe point OUT of the PVP arena. Rebuilding the .map files activated the RoF2 client's
-- hardcoded arena PVP region (fun, intended), but the old safe point sat inside it -- so anyone returned
-- to the safe point (AFK camp, /unstuck, failed feign, some ports) landed flagged for PVP in the pit.
-- New spot is by the Shadowhaven entrance, well clear of the arena. Updates BOTH bazaar rows (v0 default
-- and v1 map_file_name='bazaar_old') so whichever version the server boots is safe.
UPDATE zone SET safe_x = -134.13, safe_y = -815.98, safe_z = 3.75, safe_heading = 70.25
  WHERE short_name = 'bazaar';
)",
		.content_schema_update = false,
	},

	// ------------------------------------------------------------------------------------------------
	// Version 22: REPAIR PASS for the v21 rollout.
	//
	// v21 originally shipped as one multi-statement entry whose check only tested its FIRST
	// statement. Any failure after statement 1 - such as a MariaDB-only collation on other
	// engines - killed world mid-update; on restart the check passed, the whole entry was
	// skipped, and custom_version was stamped past it with the rest of the payload silently missing.
	// This entry re-delivers the v19/v21 data payloads behind an always-run check, so a healthy
	// server converges to the same values and a half-applied server gets the missing pieces.
	//
	// RULES (see also the header of this file):
	//  * ONE entry per version. The runner queues one apply-round per matching entry, and each
	//    round runs EVERY entry of that version - two entries sharing a version each execute twice.
	//  * Universal SQL only. No MariaDB-only syntax (uca1400 collations, ADD COLUMN IF NOT EXISTS).
	//  * The check must guard the exact SQL in the same entry, never a neighbor's.
	//  * Additive/idempotent only in repairs - no DROP/TRUNCATE/DELETE of player data.
	// ------------------------------------------------------------------------------------------------

	// DATA RESYNC. Re-delivers every data payload from v19 and v21 that a half-applied server may
	// have lost. The check is "SELECT 1" so it always runs ONCE during the bump to this version -
	// that is safe precisely because every statement here is idempotent DML (INSERT IGNORE or an
	// UPDATE that converges to the same values) and cannot error on any schema state. A server
	// that already has all of it is rewritten to the identical values; a half-applied server gets
	// the missing pieces. NO DDL in this entry, ever - DDL belongs in its own guarded entry.
	//
	// Deliberately NOT re-delivered:
	//   * v21's DELETE of the Boomerang of Wonders items - destructive, each operator's call.
	//   * v18's AA339 whitelist - INSERT IGNORE only, so an operator's tuned value is respected.
	ManifestEntry{
		.version = 22,
		.description = "2026_07_19_resync_v19_v21_data_payload",
		.check = "SELECT 1",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- v21: Bazaar is a no-combat hub like PoK/Nexus/Guild Lobby/PoT (covers both version rows).
UPDATE `zone` SET `cancombat` = 0 WHERE `short_name` = 'bazaar';

-- v21: Hastened Gathering ranks 4/5 rescale (custom ranks 12899/12900, Gather Mana slot only -
-- base2 = 57). The old values were scaled against Mana Draw's 8640s base instead of Gather
-- Mana's 3600s, making rank 5 free. Vanilla ranks 471-473 are correct and untouched.
UPDATE `aa_rank_effects` SET `base1` = 1600 WHERE `rank_id` = 12899 AND `slot` = 1 AND `base2` = 57;
UPDATE `aa_rank_effects` SET `base1` = 2000 WHERE `rank_id` = 12900 AND `slot` = 1 AND `base2` = 57;

-- v21: rank 12900 pointed next_id at nonexistent 12901.
UPDATE `aa_ranks` SET `next_id` = -1 WHERE `id` = 12900;

-- v21: Firiona Vie Beastlord trainer + spell merchant out of the drolvarg camp, into the live
-- city (see the v21 entry for the full story). Quegmor's row is stock PEQ so id 14745 is the
-- same everywhere; the merchant's row is v18-inserted so it MUST resolve via spawngroup name
-- (its AUTO_INCREMENT id differs per server). IN, not =, so a duplicated group name can never
-- turn this into a fatal 1242 multi-row error.
UPDATE spawn2 SET x = 2794.85, y = -3350.75, z = -76.12, heading = 385.5
  WHERE id = 14745;
UPDATE spawn2 SET x = 2810.72, y = -3378.81, z = -76.12, heading = 380.75
  WHERE spawngroupID IN (SELECT id FROM spawngroup WHERE name = 'fv_Beastlord_Spell_Merchant');

-- v21: Bazaar safe point out of the PVP arena (both bazaar rows).
UPDATE zone SET safe_x = -134.13, safe_y = -815.98, safe_z = 3.75, safe_heading = 70.25
  WHERE short_name = 'bazaar';

-- v19 orphans: v19's check tests the npc faction its first statement fixes, so on any server
-- whose dump already carried faction 929 the WHOLE entry was skipped and these never applied.
UPDATE npc_types SET npc_faction_id = 929 WHERE id IN (46016, 46017, 46061, 46089);
UPDATE npc_types SET npc_faction_id = 79 WHERE id = 2000507;
UPDATE rule_values SET rule_value = '1.0' WHERE rule_name = 'Merchant:BuyCostMod';
UPDATE db_str SET value = 'This passive ability grants you a chance to have your damage over time spells hit for additional damage. Each level increases both the chance and amount of damage.' WHERE id = 15594 AND type = 4;
)",
		.content_schema_update = false,
	},

	// ------------------------------------------------------------------------------------------------
	// Versions 23-25: REPAIR PASS for the v18 rollout.
	//
	// Found 2026-07-19 on a community server: db_version.custom_version was stamped >= 18 while the
	// DATA predated v18 entirely (baseline dump cut in the window where version stamp and content
	// were out of sync on dev). The runner only evaluates entries ABOVE the stored version, so v18
	// was never looked at again - no Beastlord spell merchant, no scrolls, no spawns, GMs left at
	// stock class, whitelist without aa516. Discovered only because v22's FV-merchant move returned
	// zero rows. These re-deliver v18's payload behind checks that test the CONTENT (never the
	// version), so healthy servers no-op through all of them. Same recipe as the v22 resync above.
	//
	// Same rules as always: one entry per version, universal SQL only, checks guard their own SQL,
	// everything idempotent.
	// ------------------------------------------------------------------------------------------------

	// v18's Beastlord spell merchant NPC + all 38 scrolls (51-60).
	// The check tests the LAST merchantlist row, not the NPC: if a run dies between the two
	// statements, the check still reads missing and the whole entry re-runs (both statements are
	// ON DUPLICATE KEY idempotent), instead of the NPC's existence masking an empty merchant list.
	ManifestEntry{
		.version = 23,
		.description = "2026_07_19_repair_v18_beastlord_merchant_and_scrolls",
		.check = "SELECT item FROM merchantlist WHERE merchantid = 1120001300 AND slot = 38",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO npc_types (id, name, lastname, level, race, class, bodytype, hp, mana, gender, texture, helmtexture, size, hp_regen_rate, hp_regen_per_second, mana_regen_rate, loottable_id, merchant_id, STR, STA, DEX, AGI, _INT, WIS, CHA, npc_faction_id, face)
VALUES (1120001300, 'A_Beastlord_Spell_Merchant', 'Spells 51-60', 60, 130, 41, 1, 1000, 1000, 1, 1, 0, 7, 0, 0, 0, 0, 1120001300, 75, 75, 75, 75, 75, 75, 75, 0, 3)
ON DUPLICATE KEY UPDATE class=VALUES(class), race=VALUES(race), gender=VALUES(gender), texture=VALUES(texture), size=VALUES(size), face=VALUES(face);

INSERT INTO merchantlist (merchantid, slot, item) VALUES
(1120001300, 1, 15063),  -- Spell: Resist Disease (51)
(1120001300, 2, 15046),  -- Spell: Ultravision (51)
(1120001300, 3, 7740),   -- Spell: Spirit of Wind (51)
(1120001300, 4, 19363),  -- Spell: Summon: Muzzle of Mardu (51)
(1120001300, 5, 19499),  -- Spell: Spiritual Radiance (52)
(1120001300, 6, 7722),   -- Spell: Aid of Khurenz (52)
(1120001300, 7, 15435),  -- Spell: Venom of the Snake (52)
(1120001300, 8, 15161),  -- Spell: Health (52)
(1120001300, 9, 7741),   -- Spell: Spirit of the Storm (53)
(1120001300, 10, 15167), -- Spell: Talisman of Tnarg (53)
(1120001300, 11, 15152), -- Spell: Deftness (53)
(1120001300, 12, 26957), -- Spell: Ice Shard (54)
(1120001300, 13, 19531), -- Spell: Spirit of Snow (54)
(1120001300, 14, 7723),  -- Spell: Spirit of Omakin (54)
(1120001300, 15, 15062), -- Spell: Resist Poison (54)
(1120001300, 16, 15153), -- Spell: Furious Strength (54)
(1120001300, 17, 15145), -- Spell: Chloroplast (55)
(1120001300, 18, 7724),  -- Spell: Sha's Restoration (55)
(1120001300, 19, 15163), -- Spell: Incapacitate (56)
(1120001300, 20, 7726),  -- Spell: Spirit of Zehkes (56)
(1120001300, 21, 15431), -- Spell: Shifting Shield (56)
(1120001300, 22, 19530), -- Spell: Spirit of Flame (56)
(1120001300, 23, 15157), -- Spell: Dexterity (57)
(1120001300, 24, 15158), -- Spell: Stamina (57)
(1120001300, 25, 15015), -- Spell: Greater Healing (57)
(1120001300, 26, 15049), -- Spell: Nullify Magic (58)
(1120001300, 27, 59617), -- Spell: Guard of Calliav (58)
(1120001300, 28, 7727),  -- Spell: Spirit of Khurenz (58)
(1120001300, 29, 15168), -- Spell: Talisman of Altuna (58)
(1120001300, 30, 15510), -- Spell: Blizzard Blast (59)
(1120001300, 31, 7729),  -- Spell: Spiritual Purity (59)
(1120001300, 32, 19507), -- Spell: Chloroblast (59)
(1120001300, 33, 7728),  -- Spell: Sha's Ferocity (59)
(1120001300, 34, 15170), -- Spell: Alacrity (60)
(1120001300, 35, 7730),  -- Spell: Spiritual Strength (60)
(1120001300, 36, 19537), -- Spell: Savagery (60)
(1120001300, 37, 7731),  -- Spell: Spirit of Khati Sha (60)
(1120001300, 38, 19538)
ON DUPLICATE KEY UPDATE item=VALUES(item);
)",
		.content_schema_update = false,
	},

	// v18's spawn groups / entries / world placement for the merchant, both zones.
	// Every statement is INSERT ... SELECT ... WHERE NOT EXISTS (v18's ON DUPLICATE KEY UPDATE on
	// spawngroup was a NO-OP guard - spawngroup.name has no unique index, so a re-run duplicated
	// rows and then every name-subquery in later entries became a fatal 1242 multi-row error).
	// The Firiona Vie spawn goes DIRECTLY at the corrected city location (v22's coordinates) -
	// re-creating v18's drolvarg-camp placement just for v22 to have already run would plant the
	// merchant in the wrong spot with nothing left to move him.
	// Check = the LAST artifact created (overthere spawn2): any partial death re-runs the entry;
	// WHERE NOT EXISTS makes the re-run clean.
	ManifestEntry{
		.version = 24,
		.description = "2026_07_19_repair_v18_beastlord_merchant_spawns",
		.check = "SELECT s.id FROM spawn2 s WHERE s.spawngroupID IN (SELECT id FROM spawngroup WHERE name = 'ot_Beastlord_Spell_Merchant')",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT INTO spawngroup (name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer, wp_spawns)
SELECT 'fv_Beastlord_Spell_Merchant', 1, 0, 0, 0, 0, 0, 45000, 15000, 0, 100, 0 FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM spawngroup WHERE name = 'fv_Beastlord_Spell_Merchant');

INSERT INTO spawngroup (name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer, wp_spawns)
SELECT 'ot_Beastlord_Spell_Merchant', 1, 0, 0, 0, 0, 0, 45000, 15000, 0, 100, 0 FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM spawngroup WHERE name = 'ot_Beastlord_Spell_Merchant');

INSERT INTO spawnentry (spawngroupID, npcID, chance)
SELECT sg.id, 1120001300, 100 FROM spawngroup sg
WHERE sg.name = 'fv_Beastlord_Spell_Merchant'
  AND NOT EXISTS (SELECT 1 FROM spawnentry se WHERE se.spawngroupID = sg.id AND se.npcID = 1120001300);

INSERT INTO spawnentry (spawngroupID, npcID, chance)
SELECT sg.id, 1120001300, 100 FROM spawngroup sg
WHERE sg.name = 'ot_Beastlord_Spell_Merchant'
  AND NOT EXISTS (SELECT 1 FROM spawnentry se WHERE se.spawngroupID = sg.id AND se.npcID = 1120001300);

INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, path_when_zone_idle, _condition, cond_value, animation)
SELECT sg.id, 'firiona', 0, 2810.72, -3378.81, -76.12, 380.75, 640, 0, 0, 0, 0, 1, 0 FROM spawngroup sg
WHERE sg.name = 'fv_Beastlord_Spell_Merchant'
  AND NOT EXISTS (SELECT 1 FROM spawn2 s WHERE s.spawngroupID = sg.id);

INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, path_when_zone_idle, _condition, cond_value, animation)
SELECT sg.id, 'overthere', 0, 2341.93, 2809.66, -51.22, 195, 640, 0, 0, 0, 0, 1, 0 FROM spawngroup sg
WHERE sg.name = 'ot_Beastlord_Spell_Merchant'
  AND NOT EXISTS (SELECT 1 FROM spawn2 s WHERE s.spawngroupID = sg.id);
)",
		.content_schema_update = false,
	},

	// v18's remaining data payload, always-run-once (same pattern as v22): every statement is
	// idempotent DML that converges to the same values, so re-application is harmless and nothing
	// can be skipped by a half-applied state.
	ManifestEntry{
		.version = 25,
		.description = "2026_07_19_resync_v18_data_payload",
		.check = "SELECT 1",
		.condition = "not_empty",
		.match = "",
		.sql = R"(
-- AA339 whitelist as an UPSERT - v18's bare UPDATE silently no-opped on servers where the rule row
-- did not exist, which is exactly how the whitelist ended up without aa516 in the wild.
INSERT INTO rule_values (ruleset_id, rule_name, rule_value, notes)
VALUES (1, 'Custom:AA339Whitelist', '16121,16122,16123,16675,16676,16677,30887,30888,30889,aa545,aa516,aa861,aa8332,8165,8166,8167,8168,8169', 'AA339 whitelist')
ON DUPLICATE KEY UPDATE rule_value = VALUES(rule_value);

-- Uggrig Skullchomper / Quegmor O`Cranic -> Beastlord GMs (class 34), as v18 intended.
UPDATE npc_types SET class = 34 WHERE id IN (93152, 84202);

-- Quegmor's spawn entry always up (v18 step 7). His spawn2 position move is v22's job.
UPDATE spawnentry SET chance = 100 WHERE spawngroupID = 9696 AND npcID = 84202;

-- Log noise reduction (v18 step 2).
UPDATE logsys_categories
SET log_to_file = 0
WHERE log_category_description NOT IN ('Error', 'Warning', 'Crash', 'MySQL Error', 'QuestErrors');
)",
		.content_schema_update = false,
	},

	// Dynamic (seeded, class-weighted) items. dynamic_items marks which items roll on purchase and
	// which class's pool they roll from (items are single-class, even on multiclass servers);
	// dynamic_affix_pool holds every stat line a class can roll. guaranteed = 1 lines always roll,
	// guaranteed = 0 lines form the random affix pool (picked by weight). A class with no row for a
	// stat never gets it. Only the seed is stored on the item, so editing ranges here retunes
	// existing items.
	ManifestEntry{
		.version = 26,
		.description = "2026_10_21_dynamic_items_tables",
		.check = "SHOW TABLES LIKE 'dynamic_affix_pool'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE IF NOT EXISTS `dynamic_items` (
  `item_id` INT UNSIGNED NOT NULL,
  `class_id` TINYINT UNSIGNED NOT NULL,
  `tier` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `min_affixes` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_affixes` TINYINT UNSIGNED NOT NULL DEFAULT 2,
  `stat_multiplier` FLOAT NOT NULL DEFAULT 1.0,
  `notes` VARCHAR(255) NULL,
  PRIMARY KEY (`item_id`)
);

CREATE TABLE IF NOT EXISTS `dynamic_affix_pool` (
  `class_id` TINYINT UNSIGNED NOT NULL,
  `affix` VARCHAR(32) NOT NULL,
  `guaranteed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `weight` INT UNSIGNED NOT NULL DEFAULT 100,
  `min_value` INT NOT NULL DEFAULT 0,
  `max_value` INT NOT NULL DEFAULT 0,
  `slot_mask` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`class_id`, `affix`)
);

INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
-- Warrior
(1, 'hp', 1, 0, 360, 440, 0),
(1, 'ac', 1, 0, 80, 104, 0),
(1, 'endurance', 1, 0, 290, 350, 0),
(1, 'heroic_str', 1, 0, 19, 25, 0),
(1, 'heroic_sta', 1, 0, 19, 25, 0),
(1, 'heroic_dex', 1, 0, 11, 15, 0),
(1, 'heroic_agi', 1, 0, 12, 15, 0),
(1, 'heroic_int', 1, 0, 5, 10, 0),
(1, 'heroic_wis', 1, 0, 5, 10, 0),
(1, 'skill_1hs_pct', 0, 100, 3, 12, 0),
(1, 'skill_2hs_pct', 0, 100, 3, 12, 0),
(1, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(1, 'skill_2hb_pct', 0, 100, 3, 12, 0),
(1, 'skill_pierce_pct', 0, 100, 3, 12, 0),
(1, 'skill_defense_pct', 0, 100, 2, 8, 0),
(1, 'skill_parry_pct', 0, 100, 2, 8, 0),
(1, 'skill_riposte_pct', 0, 100, 2, 8, 0),
(1, 'bash_skill_pct', 0, 100, 3, 10, 0),
(1, 'bash_dmg_pct', 0, 100, 5, 18, 0),
-- Cleric
(2, 'hp', 1, 0, 300, 370, 0),
(2, 'ac', 1, 0, 80, 104, 0),
(2, 'mana', 1, 0, 360, 440, 0),
(2, 'heroic_str', 1, 0, 11, 17, 0),
(2, 'heroic_sta', 1, 0, 11, 17, 0),
(2, 'heroic_dex', 1, 0, 5, 10, 0),
(2, 'heroic_agi', 1, 0, 5, 10, 0),
(2, 'heroic_int', 1, 0, 5, 10, 0),
(2, 'heroic_wis', 1, 0, 19, 25, 0),
(2, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(2, 'skill_2hb_pct', 0, 100, 3, 12, 0),
(2, 'skill_defense_pct', 0, 100, 2, 8, 0),
(2, 'heal_amount_pct', 0, 250, 3, 15, 0),
(2, 'spell_magic_pct', 0, 100, 3, 15, 0),
-- Paladin
(3, 'hp', 1, 0, 345, 422, 0),
(3, 'ac', 1, 0, 72, 94, 0),
(3, 'mana', 1, 0, 180, 220, 0),
(3, 'endurance', 1, 0, 145, 175, 0),
(3, 'heroic_str', 1, 0, 17, 23, 0),
(3, 'heroic_sta', 1, 0, 17, 23, 0),
(3, 'heroic_dex', 1, 0, 10, 14, 0),
(3, 'heroic_agi', 1, 0, 10, 14, 0),
(3, 'heroic_int', 1, 0, 5, 10, 0),
(3, 'heroic_wis', 1, 0, 8, 14, 0),
(3, 'skill_1hs_pct', 0, 100, 2, 9, 0),
(3, 'skill_2hs_pct', 0, 100, 2, 9, 0),
(3, 'skill_1hb_pct', 0, 100, 2, 9, 0),
(3, 'skill_2hb_pct', 0, 100, 2, 9, 0),
(3, 'skill_defense_pct', 0, 100, 2, 6, 0),
(3, 'skill_parry_pct', 0, 100, 2, 6, 0),
(3, 'skill_riposte_pct', 0, 100, 2, 6, 0),
(3, 'bash_skill_pct', 0, 100, 2, 8, 0),
(3, 'bash_dmg_pct', 0, 100, 4, 14, 0),
(3, 'heal_amount_pct', 0, 100, 1, 4, 0),
-- Ranger
(4, 'hp', 1, 0, 345, 422, 0),
(4, 'ac', 1, 0, 61, 81, 0),
(4, 'mana', 1, 0, 180, 220, 0),
(4, 'endurance', 1, 0, 145, 175, 0),
(4, 'heroic_str', 1, 0, 16, 21, 0),
(4, 'heroic_sta', 1, 0, 17, 23, 0),
(4, 'heroic_dex', 1, 0, 10, 14, 0),
(4, 'heroic_agi', 1, 0, 10, 14, 0),
(4, 'heroic_int', 1, 0, 5, 10, 0),
(4, 'heroic_wis', 1, 0, 8, 14, 0),
(4, 'skill_1hs_pct', 0, 100, 2, 9, 0),
(4, 'skill_2hs_pct', 0, 100, 2, 9, 0),
(4, 'skill_pierce_pct', 0, 100, 2, 9, 0),
(4, 'skill_archery_pct', 0, 250, 3, 12, 0),
(4, 'skill_defense_pct', 0, 100, 2, 6, 0),
(4, 'skill_parry_pct', 0, 100, 2, 6, 0),
(4, 'skill_riposte_pct', 0, 100, 2, 6, 0),
(4, 'skill_dodge_pct', 0, 100, 2, 6, 0),
(4, 'spell_fire_pct', 0, 100, 1, 4, 0),
(4, 'spell_cold_pct', 0, 100, 1, 4, 0),
-- Shadowknight
(5, 'hp', 1, 0, 345, 422, 0),
(5, 'ac', 1, 0, 64, 85, 0),
(5, 'mana', 1, 0, 180, 220, 0),
(5, 'endurance', 1, 0, 145, 175, 0),
(5, 'heroic_str', 1, 0, 16, 21, 0),
(5, 'heroic_sta', 1, 0, 17, 23, 0),
(5, 'heroic_dex', 1, 0, 10, 14, 0),
(5, 'heroic_agi', 1, 0, 10, 14, 0),
(5, 'heroic_int', 1, 0, 8, 14, 0),
(5, 'heroic_wis', 1, 0, 5, 10, 0),
(5, 'skill_1hs_pct', 0, 100, 2, 9, 0),
(5, 'skill_2hs_pct', 0, 100, 2, 9, 0),
(5, 'skill_1hb_pct', 0, 100, 2, 9, 0),
(5, 'skill_2hb_pct', 0, 100, 2, 9, 0),
(5, 'skill_defense_pct', 0, 100, 2, 6, 0),
(5, 'skill_parry_pct', 0, 100, 2, 6, 0),
(5, 'skill_riposte_pct', 0, 100, 2, 6, 0),
(5, 'bash_skill_pct', 0, 100, 2, 8, 0),
(5, 'bash_dmg_pct', 0, 100, 4, 14, 0),
(5, 'spell_disease_pct', 0, 100, 1, 4, 0),
(5, 'spell_poison_pct', 0, 100, 1, 4, 0),
(5, 'spell_magic_pct', 0, 100, 1, 4, 0),
-- Druid
(6, 'hp', 1, 0, 300, 370, 0),
(6, 'ac', 1, 0, 62, 84, 0),
(6, 'mana', 1, 0, 360, 440, 0),
(6, 'heroic_str', 1, 0, 5, 10, 0),
(6, 'heroic_sta', 1, 0, 11, 17, 0),
(6, 'heroic_dex', 1, 0, 5, 10, 0),
(6, 'heroic_agi', 1, 0, 5, 10, 0),
(6, 'heroic_int', 1, 0, 5, 10, 0),
(6, 'heroic_wis', 1, 0, 19, 25, 0),
(6, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(6, 'skill_defense_pct', 0, 100, 2, 8, 0),
(6, 'spell_fire_pct', 0, 150, 3, 15, 0),
(6, 'spell_cold_pct', 0, 150, 3, 15, 0),
(6, 'spell_magic_pct', 0, 100, 3, 15, 0),
(6, 'heal_amount_pct', 0, 150, 3, 15, 0),
-- Monk
(7, 'hp', 1, 0, 360, 440, 0),
(7, 'ac', 1, 0, 62, 84, 0),
(7, 'endurance', 1, 0, 290, 350, 0),
(7, 'heroic_str', 1, 0, 19, 25, 0),
(7, 'heroic_sta', 1, 0, 19, 25, 0),
(7, 'heroic_dex', 1, 0, 12, 15, 0),
(7, 'heroic_agi', 1, 0, 12, 15, 0),
(7, 'heroic_int', 1, 0, 5, 10, 0),
(7, 'heroic_wis', 1, 0, 5, 10, 0),
(7, 'skill_h2h_pct', 0, 250, 3, 12, 0),
(7, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(7, 'skill_2hb_pct', 0, 100, 3, 12, 0),
(7, 'skill_defense_pct', 0, 100, 2, 8, 0),
(7, 'skill_block_pct', 0, 100, 2, 8, 0),
(7, 'skill_dodge_pct', 0, 100, 2, 8, 0),
(7, 'skill_riposte_pct', 0, 100, 2, 8, 0),
(7, 'flying_kick_skill_pct', 0, 150, 3, 10, 0),
(7, 'flying_kick_dmg_pct', 0, 200, 5, 18, 0),
-- Bard
(8, 'hp', 1, 0, 345, 422, 0),
(8, 'ac', 1, 0, 57, 76, 0),
(8, 'mana', 1, 0, 180, 220, 0),
(8, 'endurance', 1, 0, 145, 175, 0),
(8, 'heroic_str', 1, 0, 16, 21, 0),
(8, 'heroic_sta', 1, 0, 17, 23, 0),
(8, 'heroic_dex', 1, 0, 10, 14, 0),
(8, 'heroic_agi', 1, 0, 10, 14, 0),
(8, 'heroic_int', 1, 0, 8, 14, 0),
(8, 'heroic_wis', 1, 0, 5, 10, 0),
(8, 'skill_1hs_pct', 0, 100, 2, 9, 0),
(8, 'skill_pierce_pct', 0, 100, 2, 9, 0),
(8, 'skill_1hb_pct', 0, 100, 2, 9, 0),
(8, 'skill_defense_pct', 0, 100, 2, 6, 0),
(8, 'skill_dodge_pct', 0, 100, 2, 6, 0),
(8, 'skill_parry_pct', 0, 100, 2, 6, 0),
(8, 'skill_riposte_pct', 0, 100, 2, 6, 0),
(8, 'bard_stringed_pct', 0, 150, 3, 15, 0),
(8, 'bard_wind_pct', 0, 150, 3, 15, 0),
(8, 'bard_brass_pct', 0, 150, 3, 15, 0),
(8, 'bard_percussion_pct', 0, 150, 3, 15, 0),
(8, 'bard_singing_pct', 0, 150, 3, 15, 0),
-- Rogue
(9, 'hp', 1, 0, 360, 440, 0),
(9, 'ac', 1, 0, 70, 92, 0),
(9, 'endurance', 1, 0, 290, 350, 0),
(9, 'heroic_str', 1, 0, 19, 25, 0),
(9, 'heroic_sta', 1, 0, 19, 25, 0),
(9, 'heroic_dex', 1, 0, 12, 15, 0),
(9, 'heroic_agi', 1, 0, 12, 15, 0),
(9, 'heroic_int', 1, 0, 5, 10, 0),
(9, 'heroic_wis', 1, 0, 5, 10, 0),
(9, 'skill_pierce_pct', 0, 250, 3, 12, 0),
(9, 'skill_1hs_pct', 0, 100, 3, 12, 0),
(9, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(9, 'skill_defense_pct', 0, 100, 2, 8, 0),
(9, 'skill_dodge_pct', 0, 100, 2, 8, 0),
(9, 'skill_parry_pct', 0, 100, 2, 8, 0),
(9, 'skill_riposte_pct', 0, 100, 2, 8, 0),
(9, 'backstab_skill_pct', 0, 150, 3, 10, 0),
(9, 'backstab_dmg_pct', 0, 250, 5, 18, 0),
-- Shaman
(10, 'hp', 1, 0, 300, 370, 0),
(10, 'ac', 1, 0, 70, 92, 0),
(10, 'mana', 1, 0, 360, 440, 0),
(10, 'heroic_str', 1, 0, 11, 17, 0),
(10, 'heroic_sta', 1, 0, 11, 17, 0),
(10, 'heroic_dex', 1, 0, 5, 10, 0),
(10, 'heroic_agi', 1, 0, 5, 10, 0),
(10, 'heroic_int', 1, 0, 5, 10, 0),
(10, 'heroic_wis', 1, 0, 19, 25, 0),
(10, 'skill_1hb_pct', 0, 100, 3, 12, 0),
(10, 'skill_2hb_pct', 0, 100, 3, 12, 0),
(10, 'skill_defense_pct', 0, 100, 2, 8, 0),
(10, 'spell_cold_pct', 0, 100, 3, 15, 0),
(10, 'spell_poison_pct', 0, 100, 3, 15, 0),
(10, 'spell_disease_pct', 0, 100, 3, 15, 0),
(10, 'heal_amount_pct', 0, 150, 3, 15, 0),
-- Necromancer
(11, 'hp', 1, 0, 300, 370, 0),
(11, 'ac', 1, 0, 45, 64, 0),
(11, 'mana', 1, 0, 360, 440, 0),
(11, 'heroic_str', 1, 0, 5, 10, 0),
(11, 'heroic_sta', 1, 0, 11, 17, 0),
(11, 'heroic_dex', 1, 0, 5, 10, 0),
(11, 'heroic_agi', 1, 0, 5, 10, 0),
(11, 'heroic_int', 1, 0, 19, 25, 0),
(11, 'heroic_wis', 1, 0, 5, 10, 0),
(11, 'spell_fire_pct', 0, 100, 3, 15, 0),
(11, 'spell_poison_pct', 0, 150, 3, 15, 0),
(11, 'spell_disease_pct', 0, 150, 3, 15, 0),
(11, 'spell_magic_pct', 0, 100, 3, 15, 0),
-- Wizard
(12, 'hp', 1, 0, 300, 370, 0),
(12, 'ac', 1, 0, 45, 64, 0),
(12, 'mana', 1, 0, 360, 440, 0),
(12, 'heroic_str', 1, 0, 5, 10, 0),
(12, 'heroic_sta', 1, 0, 11, 17, 0),
(12, 'heroic_dex', 1, 0, 5, 10, 0),
(12, 'heroic_agi', 1, 0, 5, 10, 0),
(12, 'heroic_int', 1, 0, 19, 25, 0),
(12, 'heroic_wis', 1, 0, 5, 10, 0),
(12, 'spell_fire_pct', 0, 150, 3, 15, 0),
(12, 'spell_cold_pct', 0, 150, 3, 15, 0),
(12, 'spell_magic_pct', 0, 150, 3, 15, 0),
-- Magician
(13, 'hp', 1, 0, 300, 370, 0),
(13, 'ac', 1, 0, 45, 64, 0),
(13, 'mana', 1, 0, 360, 440, 0),
(13, 'heroic_str', 1, 0, 5, 10, 0),
(13, 'heroic_sta', 1, 0, 11, 17, 0),
(13, 'heroic_dex', 1, 0, 5, 10, 0),
(13, 'heroic_agi', 1, 0, 5, 10, 0),
(13, 'heroic_int', 1, 0, 19, 25, 0),
(13, 'heroic_wis', 1, 0, 5, 10, 0),
(13, 'spell_fire_pct', 0, 200, 3, 15, 0),
(13, 'spell_magic_pct', 0, 150, 3, 15, 0),
(13, 'spell_cold_pct', 0, 100, 3, 15, 0),
-- Enchanter
(14, 'hp', 1, 0, 300, 370, 0),
(14, 'ac', 1, 0, 45, 64, 0),
(14, 'mana', 1, 0, 360, 440, 0),
(14, 'heroic_str', 1, 0, 5, 10, 0),
(14, 'heroic_sta', 1, 0, 11, 17, 0),
(14, 'heroic_dex', 1, 0, 5, 10, 0),
(14, 'heroic_agi', 1, 0, 5, 10, 0),
(14, 'heroic_int', 1, 0, 19, 25, 0),
(14, 'heroic_wis', 1, 0, 5, 10, 0),
(14, 'spell_magic_pct', 0, 200, 3, 15, 0),
(14, 'spell_poison_pct', 0, 100, 3, 15, 0),
(14, 'spell_cold_pct', 0, 100, 3, 15, 0),
-- Beastlord
(15, 'hp', 1, 0, 345, 422, 0),
(15, 'ac', 1, 0, 63, 83, 0),
(15, 'mana', 1, 0, 180, 220, 0),
(15, 'endurance', 1, 0, 145, 175, 0),
(15, 'heroic_str', 1, 0, 17, 23, 0),
(15, 'heroic_sta', 1, 0, 17, 23, 0),
(15, 'heroic_dex', 1, 0, 10, 14, 0),
(15, 'heroic_agi', 1, 0, 10, 14, 0),
(15, 'heroic_int', 1, 0, 5, 10, 0),
(15, 'heroic_wis', 1, 0, 8, 14, 0),
(15, 'skill_h2h_pct', 0, 200, 2, 9, 0),
(15, 'skill_1hb_pct', 0, 100, 2, 9, 0),
(15, 'skill_2hb_pct', 0, 100, 2, 9, 0),
(15, 'skill_pierce_pct', 0, 100, 2, 9, 0),
(15, 'skill_block_pct', 0, 100, 2, 6, 0),
(15, 'skill_dodge_pct', 0, 100, 2, 6, 0),
(15, 'skill_parry_pct', 0, 100, 2, 6, 0),
(15, 'skill_riposte_pct', 0, 100, 2, 6, 0),
(15, 'spell_cold_pct', 0, 100, 1, 4, 0),
(15, 'spell_poison_pct', 0, 100, 1, 4, 0),
(15, 'heal_amount_pct', 0, 100, 1, 4, 0),
-- Berserker
(16, 'hp', 1, 0, 360, 440, 0),
(16, 'ac', 1, 0, 70, 92, 0),
(16, 'endurance', 1, 0, 290, 350, 0),
(16, 'heroic_str', 1, 0, 19, 25, 0),
(16, 'heroic_sta', 1, 0, 19, 25, 0),
(16, 'heroic_dex', 1, 0, 12, 15, 0),
(16, 'heroic_agi', 1, 0, 11, 15, 0),
(16, 'heroic_int', 1, 0, 5, 10, 0),
(16, 'heroic_wis', 1, 0, 5, 10, 0),
(16, 'skill_2hs_pct', 0, 100, 3, 12, 0),
(16, 'skill_2hb_pct', 0, 100, 3, 12, 0),
(16, 'skill_defense_pct', 0, 100, 2, 8, 0),
(16, 'skill_parry_pct', 0, 100, 2, 8, 0),
(16, 'skill_riposte_pct', 0, 100, 2, 8, 0),
(16, 'skill_dodge_pct', 0, 100, 2, 8, 0),
(16, 'frenzy_skill_pct', 0, 150, 3, 10, 0),
(16, 'frenzy_dmg_pct', 0, 250, 5, 18, 0);
)",
		.content_schema_update = true,
	},

	// Dynamic item name suffixes ("Rogue Boots of the Assassin"): an item is named after its best
	// random affix roll. Editable; an affix with no row gives no suffix.
	ManifestEntry{
		.version = 27,
		.description = "2026_10_21_dynamic_affix_suffixes",
		.check = "SHOW TABLES LIKE 'dynamic_affix_suffixes'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE IF NOT EXISTS `dynamic_affix_suffixes` (
  `affix` VARCHAR(32) NOT NULL,
  `suffix` VARCHAR(48) NOT NULL,
  PRIMARY KEY (`affix`)
);

INSERT IGNORE INTO `dynamic_affix_suffixes` (`affix`, `suffix`) VALUES
('skill_1hs_pct', 'of Slashing'),
('skill_2hs_pct', 'of Cleaving'),
('skill_1hb_pct', 'of Crushing'),
('skill_2hb_pct', 'of Smashing'),
('skill_pierce_pct', 'of Piercing'),
('skill_h2h_pct', 'of the Fist'),
('skill_archery_pct', 'of the Hawk'),
('skill_defense_pct', 'of Defense'),
('skill_parry_pct', 'of Parrying'),
('skill_riposte_pct', 'of Retaliation'),
('skill_dodge_pct', 'of Evasion'),
('skill_block_pct', 'of Blocking'),
('flying_kick_skill_pct', 'of the Crane'),
('flying_kick_dmg_pct', 'of the Tiger'),
('backstab_skill_pct', 'of Shadows'),
('backstab_dmg_pct', 'of the Assassin'),
('frenzy_skill_pct', 'of Fury'),
('frenzy_dmg_pct', 'of the Berserker'),
('bash_skill_pct', 'of the Ram'),
('bash_dmg_pct', 'of Battering'),
('spell_fire_pct', 'of Flames'),
('spell_cold_pct', 'of Frost'),
('spell_magic_pct', 'of Sorcery'),
('spell_poison_pct', 'of Venom'),
('spell_disease_pct', 'of Plague'),
('heal_amount_pct', 'of Mending'),
('bard_stringed_pct', 'of Strings'),
('bard_wind_pct', 'of the Wind'),
('bard_brass_pct', 'of Brass'),
('bard_percussion_pct', 'of the Drum'),
('bard_singing_pct', 'of Song');
)",
		.content_schema_update = true,
	},

	// Dynamic items: regular ("white") stats and advanced stats (Spell Damage, Heal Amount, Attack,
	// regen, shielding, avoidance, crits...) as guaranteed class-weighted lines - ranges starting at 0
	// can roll 0, meaning the item doesn't get that stat - and regular/heroic resists (incl.
	// Corruption) as low-weight random affixes. INSERT IGNORE leaves rows tuned by hand alone.
	ManifestEntry{
		.version = 28,
		.description = "2026_10_21_dynamic_items_stats_resists_advanced",
		.check = "SELECT 1 FROM dynamic_affix_pool WHERE affix = 'str' LIMIT 1",
		.condition = "empty",
		.match = "",
		.sql = R"(
INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
-- Warrior
(1, 'str', 1, 0, 14, 26, 0),
(1, 'sta', 1, 0, 14, 26, 0),
(1, 'dex', 1, 0, 9, 16, 0),
(1, 'agi', 1, 0, 14, 26, 0),
(1, 'int', 1, 0, 3, 8, 0),
(1, 'wis', 1, 0, 3, 8, 0),
(1, 'cha', 1, 0, 3, 8, 0),
(1, 'mr', 1, 0, 15, 25, 0),
(1, 'fr', 1, 0, 15, 25, 0),
(1, 'cr', 1, 0, 15, 25, 0),
(1, 'dr', 1, 0, 15, 25, 0),
(1, 'pr', 1, 0, 15, 25, 0),
(1, 'corruption', 1, 0, 15, 25, 0),
(1, 'heroic_mr', 0, 20, 2, 10, 0),
(1, 'heroic_fr', 0, 20, 2, 10, 0),
(1, 'heroic_cr', 0, 20, 2, 10, 0),
(1, 'heroic_dr', 0, 20, 2, 10, 0),
(1, 'heroic_pr', 0, 20, 2, 10, 0),
(1, 'heroic_corruption', 0, 20, 2, 10, 0),
(1, 'spelldmg', 1, 0, 1, 5, 0),
(1, 'healamt', 1, 0, 1, 5, 0),
(1, 'attack', 1, 0, 12, 30, 0),
(1, 'hp_regen', 1, 0, 0, 3, 0),
(1, 'end_regen', 1, 0, 0, 3, 0),
(1, 'combat_effects', 1, 0, 0, 3, 0),
(1, 'spell_shield', 1, 0, 0, 3, 0),
(1, 'shielding', 1, 0, 0, 3, 0),
(1, 'damage_shield', 1, 0, 0, 3, 0),
(1, 'dot_shielding', 1, 0, 0, 3, 0),
(1, 'ds_mitigation', 1, 0, 0, 3, 0),
(1, 'avoidance', 1, 0, 0, 3, 0),
(1, 'accuracy', 1, 0, 0, 3, 0),
(1, 'stun_resist', 1, 0, 0, 3, 0),
(1, 'strike_through', 1, 0, 0, 3, 0),
(1, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(1, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(1, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(1, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(1, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(1, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(1, 'melee_crit_chance_pct', 1, 0, 0, 6, 0),
(1, 'melee_crit_dmg_pct', 1, 0, 0, 10, 0),
(1, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Cleric
(2, 'str', 1, 0, 9, 16, 0),
(2, 'sta', 1, 0, 9, 16, 0),
(2, 'dex', 1, 0, 3, 8, 0),
(2, 'agi', 1, 0, 3, 8, 0),
(2, 'int', 1, 0, 3, 8, 0),
(2, 'wis', 1, 0, 14, 26, 0),
(2, 'cha', 1, 0, 3, 8, 0),
(2, 'mr', 1, 0, 15, 25, 0),
(2, 'fr', 1, 0, 15, 25, 0),
(2, 'cr', 1, 0, 15, 25, 0),
(2, 'dr', 1, 0, 15, 25, 0),
(2, 'pr', 1, 0, 15, 25, 0),
(2, 'corruption', 1, 0, 15, 25, 0),
(2, 'heroic_mr', 0, 20, 2, 10, 0),
(2, 'heroic_fr', 0, 20, 2, 10, 0),
(2, 'heroic_cr', 0, 20, 2, 10, 0),
(2, 'heroic_dr', 0, 20, 2, 10, 0),
(2, 'heroic_pr', 0, 20, 2, 10, 0),
(2, 'heroic_corruption', 0, 20, 2, 10, 0),
(2, 'spelldmg', 1, 0, 5, 15, 0),
(2, 'healamt', 1, 0, 10, 30, 0),
(2, 'attack', 1, 0, 1, 5, 0),
(2, 'hp_regen', 1, 0, 0, 3, 0),
(2, 'mana_regen', 1, 0, 0, 3, 0),
(2, 'spell_shield', 1, 0, 0, 3, 0),
(2, 'dot_shielding', 1, 0, 0, 3, 0),
(2, 'avoidance', 1, 0, 0, 3, 0),
(2, 'stun_resist', 1, 0, 0, 3, 0),
(2, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(2, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(2, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(2, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(2, 'heal_crit_chance_pct', 1, 0, 0, 6, 0),
(2, 'hot_crit_chance_pct', 1, 0, 0, 6, 0),
(2, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(2, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(2, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Paladin
(3, 'str', 1, 0, 13, 24, 0),
(3, 'sta', 1, 0, 13, 24, 0),
(3, 'dex', 1, 0, 8, 14, 0),
(3, 'agi', 1, 0, 11, 22, 0),
(3, 'int', 1, 0, 3, 8, 0),
(3, 'wis', 1, 0, 6, 13, 0),
(3, 'cha', 1, 0, 3, 8, 0),
(3, 'mr', 1, 0, 15, 25, 0),
(3, 'fr', 1, 0, 15, 25, 0),
(3, 'cr', 1, 0, 15, 25, 0),
(3, 'dr', 1, 0, 15, 25, 0),
(3, 'pr', 1, 0, 15, 25, 0),
(3, 'corruption', 1, 0, 15, 25, 0),
(3, 'heroic_mr', 0, 20, 2, 10, 0),
(3, 'heroic_fr', 0, 20, 2, 10, 0),
(3, 'heroic_cr', 0, 20, 2, 10, 0),
(3, 'heroic_dr', 0, 20, 2, 10, 0),
(3, 'heroic_pr', 0, 20, 2, 10, 0),
(3, 'heroic_corruption', 0, 20, 2, 10, 0),
(3, 'spelldmg', 1, 0, 2, 8, 0),
(3, 'healamt', 1, 0, 3, 11, 0),
(3, 'attack', 1, 0, 9, 24, 0),
(3, 'hp_regen', 1, 0, 0, 3, 0),
(3, 'mana_regen', 1, 0, 0, 1, 0),
(3, 'end_regen', 1, 0, 0, 2, 0),
(3, 'combat_effects', 1, 0, 0, 2, 0),
(3, 'spell_shield', 1, 0, 0, 3, 0),
(3, 'shielding', 1, 0, 0, 2, 0),
(3, 'damage_shield', 1, 0, 0, 2, 0),
(3, 'dot_shielding', 1, 0, 0, 3, 0),
(3, 'ds_mitigation', 1, 0, 0, 2, 0),
(3, 'avoidance', 1, 0, 0, 3, 0),
(3, 'accuracy', 1, 0, 0, 2, 0),
(3, 'stun_resist', 1, 0, 0, 3, 0),
(3, 'strike_through', 1, 0, 0, 2, 0),
(3, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(3, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(3, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(3, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(3, 'heal_crit_chance_pct', 1, 0, 0, 3, 0),
(3, 'hot_crit_chance_pct', 1, 0, 0, 3, 0),
(3, 'melee_crit_chance_pct', 1, 0, 0, 5, 0),
(3, 'melee_crit_dmg_pct', 1, 0, 0, 8, 0),
(3, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Ranger
(4, 'str', 1, 0, 8, 14, 0),
(4, 'sta', 1, 0, 9, 16, 0),
(4, 'dex', 1, 0, 13, 23, 0),
(4, 'agi', 1, 0, 11, 22, 0),
(4, 'int', 1, 0, 3, 8, 0),
(4, 'wis', 1, 0, 6, 13, 0),
(4, 'cha', 1, 0, 3, 8, 0),
(4, 'mr', 1, 0, 15, 25, 0),
(4, 'fr', 1, 0, 15, 25, 0),
(4, 'cr', 1, 0, 15, 25, 0),
(4, 'dr', 1, 0, 15, 25, 0),
(4, 'pr', 1, 0, 15, 25, 0),
(4, 'corruption', 1, 0, 15, 25, 0),
(4, 'heroic_mr', 0, 20, 2, 10, 0),
(4, 'heroic_fr', 0, 20, 2, 10, 0),
(4, 'heroic_cr', 0, 20, 2, 10, 0),
(4, 'heroic_dr', 0, 20, 2, 10, 0),
(4, 'heroic_pr', 0, 20, 2, 10, 0),
(4, 'heroic_corruption', 0, 20, 2, 10, 0),
(4, 'spelldmg', 1, 0, 2, 8, 0),
(4, 'healamt', 1, 0, 3, 11, 0),
(4, 'attack', 1, 0, 9, 24, 0),
(4, 'hp_regen', 1, 0, 0, 3, 0),
(4, 'mana_regen', 1, 0, 0, 1, 0),
(4, 'end_regen', 1, 0, 0, 2, 0),
(4, 'combat_effects', 1, 0, 0, 2, 0);
)"
			R"(
INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
(4, 'spell_shield', 1, 0, 0, 3, 0),
(4, 'shielding', 1, 0, 0, 2, 0),
(4, 'dot_shielding', 1, 0, 0, 3, 0),
(4, 'ds_mitigation', 1, 0, 0, 2, 0),
(4, 'avoidance', 1, 0, 0, 3, 0),
(4, 'accuracy', 1, 0, 0, 2, 0),
(4, 'stun_resist', 1, 0, 0, 3, 0),
(4, 'strike_through', 1, 0, 0, 2, 0),
(4, 'spell_crit_chance_pct', 1, 0, 0, 3, 0),
(4, 'spell_crit_dmg_pct', 1, 0, 0, 4, 0),
(4, 'dot_crit_chance_pct', 1, 0, 0, 3, 0),
(4, 'dot_crit_dmg_pct', 1, 0, 0, 5, 0),
(4, 'heal_crit_chance_pct', 1, 0, 0, 3, 0),
(4, 'hot_crit_chance_pct', 1, 0, 0, 3, 0),
(4, 'melee_crit_chance_pct', 1, 0, 0, 5, 0),
(4, 'melee_crit_dmg_pct', 1, 0, 0, 8, 0),
(4, 'archery_crit_chance_pct', 1, 0, 0, 6, 0),
-- Shadowknight
(5, 'str', 1, 0, 11, 22, 0),
(5, 'sta', 1, 0, 13, 24, 0),
(5, 'dex', 1, 0, 8, 14, 0),
(5, 'agi', 1, 0, 11, 22, 0),
(5, 'int', 1, 0, 6, 13, 0),
(5, 'wis', 1, 0, 3, 8, 0),
(5, 'cha', 1, 0, 3, 8, 0),
(5, 'mr', 1, 0, 15, 25, 0),
(5, 'fr', 1, 0, 15, 25, 0),
(5, 'cr', 1, 0, 15, 25, 0),
(5, 'dr', 1, 0, 15, 25, 0),
(5, 'pr', 1, 0, 15, 25, 0),
(5, 'corruption', 1, 0, 15, 25, 0),
(5, 'heroic_mr', 0, 20, 2, 10, 0),
(5, 'heroic_fr', 0, 20, 2, 10, 0),
(5, 'heroic_cr', 0, 20, 2, 10, 0),
(5, 'heroic_dr', 0, 20, 2, 10, 0),
(5, 'heroic_pr', 0, 20, 2, 10, 0),
(5, 'heroic_corruption', 0, 20, 2, 10, 0),
(5, 'spelldmg', 1, 0, 3, 11, 0),
(5, 'healamt', 1, 0, 1, 5, 0),
(5, 'attack', 1, 0, 9, 24, 0),
(5, 'hp_regen', 1, 0, 0, 3, 0),
(5, 'mana_regen', 1, 0, 0, 1, 0),
(5, 'end_regen', 1, 0, 0, 2, 0),
(5, 'combat_effects', 1, 0, 0, 2, 0),
(5, 'spell_shield', 1, 0, 0, 3, 0),
(5, 'shielding', 1, 0, 0, 2, 0),
(5, 'damage_shield', 1, 0, 0, 2, 0),
(5, 'dot_shielding', 1, 0, 0, 3, 0),
(5, 'ds_mitigation', 1, 0, 0, 2, 0),
(5, 'avoidance', 1, 0, 0, 3, 0),
(5, 'accuracy', 1, 0, 0, 2, 0),
(5, 'stun_resist', 1, 0, 0, 3, 0),
(5, 'strike_through', 1, 0, 0, 2, 0),
(5, 'spell_crit_chance_pct', 1, 0, 0, 3, 0),
(5, 'spell_crit_dmg_pct', 1, 0, 0, 4, 0),
(5, 'dot_crit_chance_pct', 1, 0, 0, 3, 0),
(5, 'dot_crit_dmg_pct', 1, 0, 0, 5, 0),
(5, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(5, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(5, 'melee_crit_chance_pct', 1, 0, 0, 5, 0),
(5, 'melee_crit_dmg_pct', 1, 0, 0, 8, 0),
(5, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Druid
(6, 'str', 1, 0, 3, 8, 0),
(6, 'sta', 1, 0, 9, 16, 0),
(6, 'dex', 1, 0, 3, 8, 0),
(6, 'agi', 1, 0, 3, 8, 0),
(6, 'int', 1, 0, 3, 8, 0),
(6, 'wis', 1, 0, 14, 26, 0),
(6, 'cha', 1, 0, 3, 8, 0),
(6, 'mr', 1, 0, 15, 25, 0),
(6, 'fr', 1, 0, 15, 25, 0),
(6, 'cr', 1, 0, 15, 25, 0),
(6, 'dr', 1, 0, 15, 25, 0),
(6, 'pr', 1, 0, 15, 25, 0),
(6, 'corruption', 1, 0, 15, 25, 0),
(6, 'heroic_mr', 0, 20, 2, 10, 0),
(6, 'heroic_fr', 0, 20, 2, 10, 0),
(6, 'heroic_cr', 0, 20, 2, 10, 0),
(6, 'heroic_dr', 0, 20, 2, 10, 0),
(6, 'heroic_pr', 0, 20, 2, 10, 0),
(6, 'heroic_corruption', 0, 20, 2, 10, 0),
(6, 'spelldmg', 1, 0, 5, 15, 0),
(6, 'healamt', 1, 0, 10, 30, 0),
(6, 'attack', 1, 0, 1, 5, 0),
(6, 'hp_regen', 1, 0, 0, 3, 0),
(6, 'mana_regen', 1, 0, 0, 3, 0),
(6, 'spell_shield', 1, 0, 0, 3, 0),
(6, 'dot_shielding', 1, 0, 0, 3, 0),
(6, 'avoidance', 1, 0, 0, 3, 0),
(6, 'stun_resist', 1, 0, 0, 3, 0),
(6, 'spell_crit_chance_pct', 1, 0, 0, 4, 0),
(6, 'spell_crit_dmg_pct', 1, 0, 0, 6, 0),
(6, 'dot_crit_chance_pct', 1, 0, 0, 6, 0),
(6, 'dot_crit_dmg_pct', 1, 0, 0, 10, 0),
(6, 'heal_crit_chance_pct', 1, 0, 0, 6, 0),
(6, 'hot_crit_chance_pct', 1, 0, 0, 6, 0),
(6, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(6, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(6, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Monk
(7, 'str', 1, 0, 14, 26, 0),
(7, 'sta', 1, 0, 9, 16, 0),
(7, 'dex', 1, 0, 16, 28, 0),
(7, 'agi', 1, 0, 14, 26, 0),
(7, 'int', 1, 0, 3, 8, 0),
(7, 'wis', 1, 0, 3, 8, 0),
(7, 'cha', 1, 0, 3, 8, 0),
(7, 'mr', 1, 0, 15, 25, 0),
(7, 'fr', 1, 0, 15, 25, 0),
(7, 'cr', 1, 0, 15, 25, 0),
(7, 'dr', 1, 0, 15, 25, 0),
(7, 'pr', 1, 0, 15, 25, 0),
(7, 'corruption', 1, 0, 15, 25, 0),
(7, 'heroic_mr', 0, 20, 2, 10, 0),
(7, 'heroic_fr', 0, 20, 2, 10, 0),
(7, 'heroic_cr', 0, 20, 2, 10, 0),
(7, 'heroic_dr', 0, 20, 2, 10, 0),
(7, 'heroic_pr', 0, 20, 2, 10, 0),
(7, 'heroic_corruption', 0, 20, 2, 10, 0),
(7, 'spelldmg', 1, 0, 1, 5, 0),
(7, 'healamt', 1, 0, 1, 5, 0),
(7, 'attack', 1, 0, 12, 30, 0),
(7, 'hp_regen', 1, 0, 0, 3, 0),
(7, 'end_regen', 1, 0, 0, 3, 0),
(7, 'combat_effects', 1, 0, 0, 3, 0),
(7, 'spell_shield', 1, 0, 0, 3, 0),
(7, 'shielding', 1, 0, 0, 3, 0),
(7, 'dot_shielding', 1, 0, 0, 3, 0),
(7, 'ds_mitigation', 1, 0, 0, 3, 0),
(7, 'avoidance', 1, 0, 0, 3, 0),
(7, 'accuracy', 1, 0, 0, 3, 0),
(7, 'stun_resist', 1, 0, 0, 3, 0),
(7, 'strike_through', 1, 0, 0, 3, 0),
(7, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(7, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(7, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(7, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(7, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(7, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(7, 'melee_crit_chance_pct', 1, 0, 0, 6, 0),
(7, 'melee_crit_dmg_pct', 1, 0, 0, 10, 0),
(7, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Bard
(8, 'str', 1, 0, 8, 14, 0),
(8, 'sta', 1, 0, 9, 16, 0),
(8, 'dex', 1, 0, 13, 23, 0),
(8, 'agi', 1, 0, 11, 22, 0),
(8, 'int', 1, 0, 6, 13, 0),
(8, 'wis', 1, 0, 3, 8, 0),
(8, 'cha', 1, 0, 14, 26, 0),
(8, 'mr', 1, 0, 15, 25, 0),
(8, 'fr', 1, 0, 15, 25, 0),
(8, 'cr', 1, 0, 15, 25, 0);
)"
			R"(
INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
(8, 'dr', 1, 0, 15, 25, 0),
(8, 'pr', 1, 0, 15, 25, 0),
(8, 'corruption', 1, 0, 15, 25, 0),
(8, 'heroic_mr', 0, 20, 2, 10, 0),
(8, 'heroic_fr', 0, 20, 2, 10, 0),
(8, 'heroic_cr', 0, 20, 2, 10, 0),
(8, 'heroic_dr', 0, 20, 2, 10, 0),
(8, 'heroic_pr', 0, 20, 2, 10, 0),
(8, 'heroic_corruption', 0, 20, 2, 10, 0),
(8, 'spelldmg', 1, 0, 3, 11, 0),
(8, 'healamt', 1, 0, 1, 5, 0),
(8, 'attack', 1, 0, 9, 24, 0),
(8, 'hp_regen', 1, 0, 0, 3, 0),
(8, 'mana_regen', 1, 0, 0, 1, 0),
(8, 'end_regen', 1, 0, 0, 2, 0),
(8, 'combat_effects', 1, 0, 0, 2, 0),
(8, 'spell_shield', 1, 0, 0, 3, 0),
(8, 'shielding', 1, 0, 0, 2, 0),
(8, 'dot_shielding', 1, 0, 0, 3, 0),
(8, 'ds_mitigation', 1, 0, 0, 2, 0),
(8, 'avoidance', 1, 0, 0, 3, 0),
(8, 'accuracy', 1, 0, 0, 2, 0),
(8, 'stun_resist', 1, 0, 0, 3, 0),
(8, 'strike_through', 1, 0, 0, 2, 0),
(8, 'spell_crit_chance_pct', 1, 0, 0, 3, 0),
(8, 'spell_crit_dmg_pct', 1, 0, 0, 4, 0),
(8, 'dot_crit_chance_pct', 1, 0, 0, 3, 0),
(8, 'dot_crit_dmg_pct', 1, 0, 0, 4, 0),
(8, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(8, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(8, 'melee_crit_chance_pct', 1, 0, 0, 5, 0),
(8, 'melee_crit_dmg_pct', 1, 0, 0, 8, 0),
(8, 'archery_crit_chance_pct', 1, 0, 0, 4, 0),
-- Rogue
(9, 'str', 1, 0, 9, 16, 0),
(9, 'sta', 1, 0, 9, 16, 0),
(9, 'dex', 1, 0, 16, 28, 0),
(9, 'agi', 1, 0, 14, 26, 0),
(9, 'int', 1, 0, 3, 8, 0),
(9, 'wis', 1, 0, 3, 8, 0),
(9, 'cha', 1, 0, 3, 8, 0),
(9, 'mr', 1, 0, 15, 25, 0),
(9, 'fr', 1, 0, 15, 25, 0),
(9, 'cr', 1, 0, 15, 25, 0),
(9, 'dr', 1, 0, 15, 25, 0),
(9, 'pr', 1, 0, 15, 25, 0),
(9, 'corruption', 1, 0, 15, 25, 0),
(9, 'heroic_mr', 0, 20, 2, 10, 0),
(9, 'heroic_fr', 0, 20, 2, 10, 0),
(9, 'heroic_cr', 0, 20, 2, 10, 0),
(9, 'heroic_dr', 0, 20, 2, 10, 0),
(9, 'heroic_pr', 0, 20, 2, 10, 0),
(9, 'heroic_corruption', 0, 20, 2, 10, 0),
(9, 'spelldmg', 1, 0, 1, 5, 0),
(9, 'healamt', 1, 0, 1, 5, 0),
(9, 'attack', 1, 0, 12, 30, 0),
(9, 'hp_regen', 1, 0, 0, 3, 0),
(9, 'end_regen', 1, 0, 0, 3, 0),
(9, 'combat_effects', 1, 0, 0, 3, 0),
(9, 'spell_shield', 1, 0, 0, 3, 0),
(9, 'shielding', 1, 0, 0, 3, 0),
(9, 'dot_shielding', 1, 0, 0, 3, 0),
(9, 'ds_mitigation', 1, 0, 0, 3, 0),
(9, 'avoidance', 1, 0, 0, 3, 0),
(9, 'accuracy', 1, 0, 0, 3, 0),
(9, 'stun_resist', 1, 0, 0, 3, 0),
(9, 'strike_through', 1, 0, 0, 3, 0),
(9, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(9, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(9, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(9, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(9, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(9, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(9, 'melee_crit_chance_pct', 1, 0, 0, 6, 0),
(9, 'melee_crit_dmg_pct', 1, 0, 0, 10, 0),
(9, 'archery_crit_chance_pct', 1, 0, 0, 4, 0),
-- Shaman
(10, 'str', 1, 0, 9, 16, 0),
(10, 'sta', 1, 0, 9, 16, 0),
(10, 'dex', 1, 0, 3, 8, 0),
(10, 'agi', 1, 0, 3, 8, 0),
(10, 'int', 1, 0, 3, 8, 0),
(10, 'wis', 1, 0, 14, 26, 0),
(10, 'cha', 1, 0, 3, 8, 0),
(10, 'mr', 1, 0, 15, 25, 0),
(10, 'fr', 1, 0, 15, 25, 0),
(10, 'cr', 1, 0, 15, 25, 0),
(10, 'dr', 1, 0, 15, 25, 0),
(10, 'pr', 1, 0, 15, 25, 0),
(10, 'corruption', 1, 0, 15, 25, 0),
(10, 'heroic_mr', 0, 20, 2, 10, 0),
(10, 'heroic_fr', 0, 20, 2, 10, 0),
(10, 'heroic_cr', 0, 20, 2, 10, 0),
(10, 'heroic_dr', 0, 20, 2, 10, 0),
(10, 'heroic_pr', 0, 20, 2, 10, 0),
(10, 'heroic_corruption', 0, 20, 2, 10, 0),
(10, 'spelldmg', 1, 0, 5, 15, 0),
(10, 'healamt', 1, 0, 10, 30, 0),
(10, 'attack', 1, 0, 1, 5, 0),
(10, 'hp_regen', 1, 0, 0, 3, 0),
(10, 'mana_regen', 1, 0, 0, 3, 0),
(10, 'spell_shield', 1, 0, 0, 3, 0),
(10, 'dot_shielding', 1, 0, 0, 3, 0),
(10, 'avoidance', 1, 0, 0, 3, 0),
(10, 'stun_resist', 1, 0, 0, 3, 0),
(10, 'spell_crit_chance_pct', 1, 0, 0, 4, 0),
(10, 'spell_crit_dmg_pct', 1, 0, 0, 6, 0),
(10, 'dot_crit_chance_pct', 1, 0, 0, 4, 0),
(10, 'dot_crit_dmg_pct', 1, 0, 0, 6, 0),
(10, 'heal_crit_chance_pct', 1, 0, 0, 6, 0),
(10, 'hot_crit_chance_pct', 1, 0, 0, 6, 0),
(10, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(10, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(10, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Necromancer
(11, 'str', 1, 0, 3, 8, 0),
(11, 'sta', 1, 0, 9, 16, 0),
(11, 'dex', 1, 0, 3, 8, 0),
(11, 'agi', 1, 0, 3, 8, 0),
(11, 'int', 1, 0, 14, 26, 0),
(11, 'wis', 1, 0, 3, 8, 0),
(11, 'cha', 1, 0, 3, 8, 0),
(11, 'mr', 1, 0, 15, 25, 0),
(11, 'fr', 1, 0, 15, 25, 0),
(11, 'cr', 1, 0, 15, 25, 0),
(11, 'dr', 1, 0, 15, 25, 0),
(11, 'pr', 1, 0, 15, 25, 0),
(11, 'corruption', 1, 0, 15, 25, 0),
(11, 'heroic_mr', 0, 20, 2, 10, 0),
(11, 'heroic_fr', 0, 20, 2, 10, 0),
(11, 'heroic_cr', 0, 20, 2, 10, 0),
(11, 'heroic_dr', 0, 20, 2, 10, 0),
(11, 'heroic_pr', 0, 20, 2, 10, 0),
(11, 'heroic_corruption', 0, 20, 2, 10, 0),
(11, 'spelldmg', 1, 0, 10, 30, 0),
(11, 'healamt', 1, 0, 1, 5, 0),
(11, 'attack', 1, 0, 1, 5, 0),
(11, 'hp_regen', 1, 0, 0, 3, 0),
(11, 'mana_regen', 1, 0, 0, 3, 0),
(11, 'spell_shield', 1, 0, 0, 3, 0),
(11, 'dot_shielding', 1, 0, 0, 3, 0),
(11, 'avoidance', 1, 0, 0, 3, 0),
(11, 'stun_resist', 1, 0, 0, 3, 0),
(11, 'spell_crit_chance_pct', 1, 0, 0, 4, 0),
(11, 'spell_crit_dmg_pct', 1, 0, 0, 6, 0),
(11, 'dot_crit_chance_pct', 1, 0, 0, 6, 0),
(11, 'dot_crit_dmg_pct', 1, 0, 0, 10, 0),
(11, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(11, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(11, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(11, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(11, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Wizard
(12, 'str', 1, 0, 3, 8, 0);
)"
			R"(
INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
(12, 'sta', 1, 0, 9, 16, 0),
(12, 'dex', 1, 0, 3, 8, 0),
(12, 'agi', 1, 0, 3, 8, 0),
(12, 'int', 1, 0, 14, 26, 0),
(12, 'wis', 1, 0, 3, 8, 0),
(12, 'cha', 1, 0, 3, 8, 0),
(12, 'mr', 1, 0, 15, 25, 0),
(12, 'fr', 1, 0, 15, 25, 0),
(12, 'cr', 1, 0, 15, 25, 0),
(12, 'dr', 1, 0, 15, 25, 0),
(12, 'pr', 1, 0, 15, 25, 0),
(12, 'corruption', 1, 0, 15, 25, 0),
(12, 'heroic_mr', 0, 20, 2, 10, 0),
(12, 'heroic_fr', 0, 20, 2, 10, 0),
(12, 'heroic_cr', 0, 20, 2, 10, 0),
(12, 'heroic_dr', 0, 20, 2, 10, 0),
(12, 'heroic_pr', 0, 20, 2, 10, 0),
(12, 'heroic_corruption', 0, 20, 2, 10, 0),
(12, 'spelldmg', 1, 0, 10, 30, 0),
(12, 'healamt', 1, 0, 1, 5, 0),
(12, 'attack', 1, 0, 1, 5, 0),
(12, 'hp_regen', 1, 0, 0, 3, 0),
(12, 'mana_regen', 1, 0, 0, 3, 0),
(12, 'spell_shield', 1, 0, 0, 3, 0),
(12, 'dot_shielding', 1, 0, 0, 3, 0),
(12, 'avoidance', 1, 0, 0, 3, 0),
(12, 'stun_resist', 1, 0, 0, 3, 0),
(12, 'spell_crit_chance_pct', 1, 0, 0, 6, 0),
(12, 'spell_crit_dmg_pct', 1, 0, 0, 10, 0),
(12, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(12, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(12, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(12, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(12, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(12, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(12, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Magician
(13, 'str', 1, 0, 3, 8, 0),
(13, 'sta', 1, 0, 9, 16, 0),
(13, 'dex', 1, 0, 3, 8, 0),
(13, 'agi', 1, 0, 3, 8, 0),
(13, 'int', 1, 0, 14, 26, 0),
(13, 'wis', 1, 0, 3, 8, 0),
(13, 'cha', 1, 0, 3, 8, 0),
(13, 'mr', 1, 0, 15, 25, 0),
(13, 'fr', 1, 0, 15, 25, 0),
(13, 'cr', 1, 0, 15, 25, 0),
(13, 'dr', 1, 0, 15, 25, 0),
(13, 'pr', 1, 0, 15, 25, 0),
(13, 'corruption', 1, 0, 15, 25, 0),
(13, 'heroic_mr', 0, 20, 2, 10, 0),
(13, 'heroic_fr', 0, 20, 2, 10, 0),
(13, 'heroic_cr', 0, 20, 2, 10, 0),
(13, 'heroic_dr', 0, 20, 2, 10, 0),
(13, 'heroic_pr', 0, 20, 2, 10, 0),
(13, 'heroic_corruption', 0, 20, 2, 10, 0),
(13, 'spelldmg', 1, 0, 10, 30, 0),
(13, 'healamt', 1, 0, 1, 5, 0),
(13, 'attack', 1, 0, 1, 5, 0),
(13, 'hp_regen', 1, 0, 0, 3, 0),
(13, 'mana_regen', 1, 0, 0, 3, 0),
(13, 'spell_shield', 1, 0, 0, 3, 0),
(13, 'dot_shielding', 1, 0, 0, 3, 0),
(13, 'avoidance', 1, 0, 0, 3, 0),
(13, 'stun_resist', 1, 0, 0, 3, 0),
(13, 'spell_crit_chance_pct', 1, 0, 0, 6, 0),
(13, 'spell_crit_dmg_pct', 1, 0, 0, 10, 0),
(13, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(13, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(13, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(13, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(13, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(13, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(13, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Enchanter
(14, 'str', 1, 0, 3, 8, 0),
(14, 'sta', 1, 0, 9, 16, 0),
(14, 'dex', 1, 0, 3, 8, 0),
(14, 'agi', 1, 0, 3, 8, 0),
(14, 'int', 1, 0, 14, 26, 0),
(14, 'wis', 1, 0, 3, 8, 0),
(14, 'cha', 1, 0, 14, 26, 0),
(14, 'mr', 1, 0, 15, 25, 0),
(14, 'fr', 1, 0, 15, 25, 0),
(14, 'cr', 1, 0, 15, 25, 0),
(14, 'dr', 1, 0, 15, 25, 0),
(14, 'pr', 1, 0, 15, 25, 0),
(14, 'corruption', 1, 0, 15, 25, 0),
(14, 'heroic_mr', 0, 20, 2, 10, 0),
(14, 'heroic_fr', 0, 20, 2, 10, 0),
(14, 'heroic_cr', 0, 20, 2, 10, 0),
(14, 'heroic_dr', 0, 20, 2, 10, 0),
(14, 'heroic_pr', 0, 20, 2, 10, 0),
(14, 'heroic_corruption', 0, 20, 2, 10, 0),
(14, 'spelldmg', 1, 0, 10, 30, 0),
(14, 'healamt', 1, 0, 1, 5, 0),
(14, 'attack', 1, 0, 1, 5, 0),
(14, 'hp_regen', 1, 0, 0, 3, 0),
(14, 'mana_regen', 1, 0, 0, 3, 0),
(14, 'spell_shield', 1, 0, 0, 3, 0),
(14, 'dot_shielding', 1, 0, 0, 3, 0),
(14, 'avoidance', 1, 0, 0, 3, 0),
(14, 'stun_resist', 1, 0, 0, 3, 0),
(14, 'spell_crit_chance_pct', 1, 0, 0, 4, 0),
(14, 'spell_crit_dmg_pct', 1, 0, 0, 6, 0),
(14, 'dot_crit_chance_pct', 1, 0, 0, 4, 0),
(14, 'dot_crit_dmg_pct', 1, 0, 0, 6, 0),
(14, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(14, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(14, 'melee_crit_chance_pct', 1, 0, 0, 2, 0),
(14, 'melee_crit_dmg_pct', 1, 0, 0, 3, 0),
(14, 'archery_crit_chance_pct', 1, 0, 0, 2, 0),
-- Beastlord
(15, 'str', 1, 0, 9, 16, 0),
(15, 'sta', 1, 0, 9, 16, 0),
(15, 'dex', 1, 0, 13, 23, 0),
(15, 'agi', 1, 0, 11, 22, 0),
(15, 'int', 1, 0, 3, 8, 0),
(15, 'wis', 1, 0, 6, 13, 0),
(15, 'cha', 1, 0, 3, 8, 0),
(15, 'mr', 1, 0, 15, 25, 0),
(15, 'fr', 1, 0, 15, 25, 0),
(15, 'cr', 1, 0, 15, 25, 0),
(15, 'dr', 1, 0, 15, 25, 0),
(15, 'pr', 1, 0, 15, 25, 0),
(15, 'corruption', 1, 0, 15, 25, 0),
(15, 'heroic_mr', 0, 20, 2, 10, 0),
(15, 'heroic_fr', 0, 20, 2, 10, 0),
(15, 'heroic_cr', 0, 20, 2, 10, 0),
(15, 'heroic_dr', 0, 20, 2, 10, 0),
(15, 'heroic_pr', 0, 20, 2, 10, 0),
(15, 'heroic_corruption', 0, 20, 2, 10, 0),
(15, 'spelldmg', 1, 0, 2, 8, 0),
(15, 'healamt', 1, 0, 3, 11, 0),
(15, 'attack', 1, 0, 9, 24, 0),
(15, 'hp_regen', 1, 0, 0, 3, 0),
(15, 'mana_regen', 1, 0, 0, 1, 0),
(15, 'end_regen', 1, 0, 0, 2, 0),
(15, 'combat_effects', 1, 0, 0, 2, 0),
(15, 'spell_shield', 1, 0, 0, 3, 0),
(15, 'shielding', 1, 0, 0, 2, 0),
(15, 'dot_shielding', 1, 0, 0, 3, 0),
(15, 'ds_mitigation', 1, 0, 0, 2, 0),
(15, 'avoidance', 1, 0, 0, 3, 0),
(15, 'accuracy', 1, 0, 0, 2, 0),
(15, 'stun_resist', 1, 0, 0, 3, 0),
(15, 'strike_through', 1, 0, 0, 2, 0),
(15, 'spell_crit_chance_pct', 1, 0, 0, 3, 0),
(15, 'spell_crit_dmg_pct', 1, 0, 0, 4, 0),
(15, 'dot_crit_chance_pct', 1, 0, 0, 3, 0),
(15, 'dot_crit_dmg_pct', 1, 0, 0, 4, 0),
(15, 'heal_crit_chance_pct', 1, 0, 0, 3, 0),
(15, 'hot_crit_chance_pct', 1, 0, 0, 3, 0);
)"
			R"(
INSERT IGNORE INTO `dynamic_affix_pool` (`class_id`, `affix`, `guaranteed`, `weight`, `min_value`, `max_value`, `slot_mask`) VALUES
(15, 'melee_crit_chance_pct', 1, 0, 0, 5, 0),
(15, 'melee_crit_dmg_pct', 1, 0, 0, 8, 0),
(15, 'archery_crit_chance_pct', 1, 0, 0, 4, 0),
-- Berserker
(16, 'str', 1, 0, 14, 26, 0),
(16, 'sta', 1, 0, 14, 26, 0),
(16, 'dex', 1, 0, 16, 28, 0),
(16, 'agi', 1, 0, 9, 16, 0),
(16, 'int', 1, 0, 3, 8, 0),
(16, 'wis', 1, 0, 3, 8, 0),
(16, 'cha', 1, 0, 3, 8, 0),
(16, 'mr', 1, 0, 15, 25, 0),
(16, 'fr', 1, 0, 15, 25, 0),
(16, 'cr', 1, 0, 15, 25, 0),
(16, 'dr', 1, 0, 15, 25, 0),
(16, 'pr', 1, 0, 15, 25, 0),
(16, 'corruption', 1, 0, 15, 25, 0),
(16, 'heroic_mr', 0, 20, 2, 10, 0),
(16, 'heroic_fr', 0, 20, 2, 10, 0),
(16, 'heroic_cr', 0, 20, 2, 10, 0),
(16, 'heroic_dr', 0, 20, 2, 10, 0),
(16, 'heroic_pr', 0, 20, 2, 10, 0),
(16, 'heroic_corruption', 0, 20, 2, 10, 0),
(16, 'spelldmg', 1, 0, 1, 5, 0),
(16, 'healamt', 1, 0, 1, 5, 0),
(16, 'attack', 1, 0, 12, 30, 0),
(16, 'hp_regen', 1, 0, 0, 3, 0),
(16, 'end_regen', 1, 0, 0, 3, 0),
(16, 'combat_effects', 1, 0, 0, 3, 0),
(16, 'spell_shield', 1, 0, 0, 3, 0),
(16, 'shielding', 1, 0, 0, 3, 0),
(16, 'dot_shielding', 1, 0, 0, 3, 0),
(16, 'ds_mitigation', 1, 0, 0, 3, 0),
(16, 'avoidance', 1, 0, 0, 3, 0),
(16, 'accuracy', 1, 0, 0, 3, 0),
(16, 'stun_resist', 1, 0, 0, 3, 0),
(16, 'strike_through', 1, 0, 0, 3, 0),
(16, 'spell_crit_chance_pct', 1, 0, 0, 2, 0),
(16, 'spell_crit_dmg_pct', 1, 0, 0, 3, 0),
(16, 'dot_crit_chance_pct', 1, 0, 0, 2, 0),
(16, 'dot_crit_dmg_pct', 1, 0, 0, 3, 0),
(16, 'heal_crit_chance_pct', 1, 0, 0, 2, 0),
(16, 'hot_crit_chance_pct', 1, 0, 0, 2, 0),
(16, 'melee_crit_chance_pct', 1, 0, 0, 6, 0),
(16, 'melee_crit_dmg_pct', 1, 0, 0, 10, 0),
(16, 'archery_crit_chance_pct', 1, 0, 0, 2, 0);
)"
			R"(
INSERT IGNORE INTO `dynamic_affix_suffixes` (`affix`, `suffix`) VALUES
('mr', 'of Spell Warding'),
('fr', 'of Warmth'),
('cr', 'of Insulation'),
('dr', 'of Health'),
('pr', 'of Antivenom'),
('corruption', 'of Purity'),
('heroic_mr', 'of the Magus'),
('heroic_fr', 'of the Salamander'),
('heroic_cr', 'of the Yeti'),
('heroic_dr', 'of the Healer'),
('heroic_pr', 'of the Serpent'),
('heroic_corruption', 'of the Saint');
)",
		.content_schema_update = true,
	},

	// Armor set bonuses can use any dynamic item affix code (crits, spell %, shielding...) in place
	// of stat_id. Also creates the armor set tables on servers that don't have them yet.
	ManifestEntry{
		.version = 29,
		.description = "2026_10_22_armor_set_bonus_affixes",
		.check = "SELECT 1 FROM information_schema.columns WHERE table_schema = DATABASE() AND table_name = 'armor_set_bonuses' AND column_name = 'affix'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE IF NOT EXISTS `armor_set_items` (
  `item_id` INT(11) NOT NULL,
  `set_id` INT(11) NOT NULL,
  `class_mask` INT(10) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`item_id`)
);

CREATE TABLE IF NOT EXISTS `armor_set_bonuses` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `set_id` INT(11) NOT NULL,
  `min_pieces` INT(11) NOT NULL,
  `class_mask` INT(11) NOT NULL,
  `stat_id` INT(11) NOT NULL,
  `stat_value` INT(11) NOT NULL,
  PRIMARY KEY (`id`)
);

ALTER TABLE `armor_set_bonuses` ADD COLUMN IF NOT EXISTS `affix` VARCHAR(32) NOT NULL DEFAULT '' AFTER `stat_id`;
)",
		.content_schema_update = true,
	},

	// Dynamic item pool rebalanced to ~122% of best-in-slot for the 8 dynamic slots (rebalance_to_bis.py):
	// guaranteed stats, Spell Damage, Heal Amount, Attack, regen/avoidance/accuracy and resists raised,
	// heroic resists made guaranteed, hybrid Mana raised. Berserkers lose the 1H skill lines they can't use.
	ManifestEntry{
		.version = 30,
		.description = "2026_10_23_dynamic_items_bis_rebalance",
		.check = "SELECT 1 FROM dynamic_affix_pool WHERE class_id = 3 AND affix = 'heroic_mr' AND guaranteed = 1",
		.condition = "empty",
		.match = "",
		.sql = R"(
DELETE FROM dynamic_affix_pool WHERE class_id = 16 AND affix IN ('skill_1hb_pct', 'skill_1hs_pct');
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 124, max_value = 161 WHERE class_id = 1 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 23 WHERE class_id = 1 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 50 WHERE class_id = 1 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 44 WHERE class_id = 1 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 1 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 1 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 1 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 33, max_value = 40 WHERE class_id = 1 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 1 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 1 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 1 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 1 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 1 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 1 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 1 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 1 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 424, max_value = 518 WHERE class_id = 1 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 1 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 1 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 1 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 1 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 27 WHERE class_id = 1 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 25, max_value = 37 WHERE class_id = 1 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 49, max_value = 61 WHERE class_id = 1 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 1 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 124, max_value = 161 WHERE class_id = 2 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 2 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 2 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 2 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 2 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 2 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 2 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 2 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 2 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 33, max_value = 53 WHERE class_id = 2 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 2 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 2 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 2 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 2 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 2 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 2 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 2 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 2 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 2 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 2 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 37 WHERE class_id = 2 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 2 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 44, max_value = 51 WHERE class_id = 2 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 34, max_value = 46 WHERE class_id = 2 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 111, max_value = 145 WHERE class_id = 3 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 15 WHERE class_id = 3 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 35, max_value = 46 WHERE class_id = 3 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 38 WHERE class_id = 3 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 3 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 3 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 3 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 38 WHERE class_id = 3 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 3 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 3 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 34 WHERE class_id = 3 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 3 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 3 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 3 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 3 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 3 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 406, max_value = 497 WHERE class_id = 3 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 3 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 3 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 3 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 3 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 30 WHERE class_id = 3 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 35 WHERE class_id = 3 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 48, max_value = 59 WHERE class_id = 3 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 33 WHERE class_id = 3 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 94, max_value = 125 WHERE class_id = 4 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 15 WHERE class_id = 4 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 35, max_value = 46 WHERE class_id = 4 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 38 WHERE class_id = 4 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 4 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 4 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 4 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 37, max_value = 47 WHERE class_id = 4 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 4 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 4 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 34 WHERE class_id = 4 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 4 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 4 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 4 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 4 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 4 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 406, max_value = 497 WHERE class_id = 4 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 4 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 4 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 4 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 4 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 30 WHERE class_id = 4 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 4 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 43, max_value = 49 WHERE class_id = 4 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 33 WHERE class_id = 4 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 99, max_value = 131 WHERE class_id = 5 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 15 WHERE class_id = 5 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 35, max_value = 46 WHERE class_id = 5 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 38 WHERE class_id = 5 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 5 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 5 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 5 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 38 WHERE class_id = 5 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 5 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 5 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 5 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 5 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 5 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 5 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 5 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 5 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 406, max_value = 497 WHERE class_id = 5 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 5 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 31 WHERE class_id = 5 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 5 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 5 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 25, max_value = 33 WHERE class_id = 5 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 35 WHERE class_id = 5 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 46, max_value = 57 WHERE class_id = 5 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 5 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 96, max_value = 130 WHERE class_id = 6 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 6 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 6 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 6 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 6 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 6 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 6 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 6 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 6 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 33, max_value = 53 WHERE class_id = 6 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 6 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 6 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 6 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 6 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 6 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 6 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 6 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 6 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 6 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 6 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 37 WHERE class_id = 6 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 6 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 43 WHERE class_id = 6 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 34, max_value = 46 WHERE class_id = 6 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 96, max_value = 130 WHERE class_id = 7 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 23 WHERE class_id = 7 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 50 WHERE class_id = 7 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 44 WHERE class_id = 7 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 7 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 7 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 7 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 40, max_value = 52 WHERE class_id = 7 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 7 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 7 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 7 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 7 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 7 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 7 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 7 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 7 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 424, max_value = 518 WHERE class_id = 7 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 7 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 7 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 7 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 7 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 27 WHERE class_id = 7 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 7 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 49, max_value = 61 WHERE class_id = 7 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 7 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 88, max_value = 117 WHERE class_id = 8 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 15 WHERE class_id = 8 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 35, max_value = 46 WHERE class_id = 8 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 38 WHERE class_id = 8 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 8 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 29, max_value = 41 WHERE class_id = 8 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 8 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 37, max_value = 47 WHERE class_id = 8 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 8 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 8 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 8 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 8 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 8 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 8 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 8 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 8 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 406, max_value = 497 WHERE class_id = 8 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 8 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 31 WHERE class_id = 8 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 8 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 8 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 25, max_value = 33 WHERE class_id = 8 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 8 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 43, max_value = 49 WHERE class_id = 8 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 8 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 108, max_value = 142 WHERE class_id = 9 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 23 WHERE class_id = 9 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 50 WHERE class_id = 9 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 44 WHERE class_id = 9 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 9 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 9 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 9 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 40, max_value = 52 WHERE class_id = 9 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 9 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 9 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 9 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 9 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 9 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 9 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 9 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 9 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 424, max_value = 518 WHERE class_id = 9 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 9 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 9 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 9 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 9 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 27 WHERE class_id = 9 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 9 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 44, max_value = 51 WHERE class_id = 9 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 9 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 108, max_value = 142 WHERE class_id = 10 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 10 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 10 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 10 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 10 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 10 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 10 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 10 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 10 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 33, max_value = 53 WHERE class_id = 10 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 10 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 10 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 10 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 10 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 10 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 10 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 10 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 10 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 10 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 10 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 37 WHERE class_id = 10 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 10 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 44, max_value = 51 WHERE class_id = 10 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 34, max_value = 46 WHERE class_id = 10 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 69, max_value = 99 WHERE class_id = 11 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 11 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 11 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 11 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 11 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 11 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 11 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 11 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 11 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 11 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 11 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 11 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 11 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 11 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 11 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 11 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 11 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 44 WHERE class_id = 11 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 11 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 11 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 52 WHERE class_id = 11 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 11 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 43 WHERE class_id = 11 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 11 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 69, max_value = 99 WHERE class_id = 12 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 12 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 12 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 12 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 12 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 12 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 12 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 12 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 12 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 12 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 12 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 12 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 12 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 12 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 12 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 12 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 12 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 44 WHERE class_id = 12 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 12 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 12 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 52 WHERE class_id = 12 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 12 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 43 WHERE class_id = 12 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 12 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 69, max_value = 99 WHERE class_id = 13 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 13 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 13 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 13 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 13 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 13 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 13 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 13 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 13 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 13 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 13 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 13 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 13 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 13 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 13 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 13 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 13 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 44 WHERE class_id = 13 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 13 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 13 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 52 WHERE class_id = 13 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 13 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 43 WHERE class_id = 13 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 13 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 69, max_value = 99 WHERE class_id = 14 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 14 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 15, max_value = 19 WHERE class_id = 14 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 14 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 29, max_value = 41 WHERE class_id = 14 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 14 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 27, max_value = 32 WHERE class_id = 14 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 14 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 14 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 14 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 14 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 14 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 14 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 14 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 14 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 353, max_value = 436 WHERE class_id = 14 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 14 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 44 WHERE class_id = 14 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 14 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 14 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 32, max_value = 52 WHERE class_id = 14 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 14 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 38, max_value = 43 WHERE class_id = 14 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 14 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 97, max_value = 128 WHERE class_id = 15 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 15 WHERE class_id = 15 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 35, max_value = 46 WHERE class_id = 15 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 38 WHERE class_id = 15 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 15 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 15 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 15 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 37, max_value = 47 WHERE class_id = 15 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 15 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 15 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 34 WHERE class_id = 15 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 15 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 15 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 15 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 15 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 15 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 406, max_value = 497 WHERE class_id = 15 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 15 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 15 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 15 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 15 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 30 WHERE class_id = 15 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 20, max_value = 27 WHERE class_id = 15 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 44, max_value = 51 WHERE class_id = 15 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 33 WHERE class_id = 15 AND affix = 'wis';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 108, max_value = 142 WHERE class_id = 16 AND affix = 'ac';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 23 WHERE class_id = 16 AND affix = 'accuracy';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 33, max_value = 40 WHERE class_id = 16 AND affix = 'agi';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 26, max_value = 44 WHERE class_id = 16 AND affix = 'attack';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 13 WHERE class_id = 16 AND affix = 'avoidance';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 18, max_value = 23 WHERE class_id = 16 AND affix = 'cha';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 16 AND affix = 'cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 40, max_value = 52 WHERE class_id = 16 AND affix = 'dex';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 16 AND affix = 'dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 16 AND affix = 'fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 24, max_value = 28 WHERE class_id = 16 AND affix = 'healamt';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 16 AND affix = 'heroic_cr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 16 AND affix = 'heroic_dr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 16 AND affix = 'heroic_fr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 16 AND affix = 'heroic_mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 6, max_value = 12 WHERE class_id = 16 AND affix = 'heroic_pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 424, max_value = 518 WHERE class_id = 16 AND affix = 'hp';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 0, max_value = 11 WHERE class_id = 16 AND affix = 'hp_regen';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 21, max_value = 26 WHERE class_id = 16 AND affix = 'int';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 16 AND affix = 'mr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 28, max_value = 42 WHERE class_id = 16 AND affix = 'pr';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 27 WHERE class_id = 16 AND affix = 'spelldmg';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 25, max_value = 37 WHERE class_id = 16 AND affix = 'sta';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 49, max_value = 61 WHERE class_id = 16 AND affix = 'str';
UPDATE dynamic_affix_pool SET guaranteed = 1, weight = 0, min_value = 23, max_value = 28 WHERE class_id = 16 AND affix = 'wis';
UPDATE dynamic_affix_pool SET min_value = 265, max_value = 320
WHERE affix = 'mana' AND class_id IN (3, 4, 5, 8, 15) AND min_value = 180 AND max_value = 220;
)",
		.content_schema_update = true,
	},

	// Armor set bonuses can grant a spell: a worn effect, a weapon proc or a spell-cast (sympathetic) proc
	ManifestEntry{
		.version = 31,
		.description = "2026_10_23_armor_set_bonus_spells",
		.check = "SELECT 1 FROM information_schema.columns WHERE table_schema = DATABASE() AND table_name = 'armor_set_bonuses' AND column_name = 'spell_type'",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE `armor_set_bonuses` ADD COLUMN IF NOT EXISTS `spell_id` INT(11) NOT NULL DEFAULT 0 AFTER `affix`;
ALTER TABLE `armor_set_bonuses` ADD COLUMN IF NOT EXISTS `spell_type` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 COMMENT '1 worn effect, 2 melee proc, 3 spell-cast (sympathetic) proc' AFTER `spell_id`;
)",
		.content_schema_update = true,
	},


	// ------------------------------------------------------------------
	// Nautilus Vault, ported from Triptych (their custom v93-v96, itself a port of
	// Rockin-Vik/NMS-Release). v32 storage table, v33 per-instance item state
	// (custom_data keeps dynamic item seeds), v34 the Nautilus Vault key item
	// (clone of 9011010 -> 9011009), v35 the feature rule.
	// Code: zone/nms_vault.cpp, zone/gm_commands/vault.cpp, common/nms_vault_item.h.
	// Client: the installed 3.5 MB dinput8.dll (Dimensional Vault window).
	// ------------------------------------------------------------------
	ManifestEntry{
		.version = 32,
		.description = "2026_10_02_character_nms_vault",
		.check = "SHOW TABLES LIKE 'character_nms_vault'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE IF NOT EXISTS character_nms_vault (
    character_id INT UNSIGNED NOT NULL,
    slot INT NOT NULL,
    bag_slot INT NOT NULL DEFAULT 0,
    item_id INT UNSIGNED NOT NULL,
    charges SMALLINT NOT NULL DEFAULT 1,
    aug1 INT UNSIGNED NOT NULL DEFAULT 0,
    aug2 INT UNSIGNED NOT NULL DEFAULT 0,
    aug3 INT UNSIGNED NOT NULL DEFAULT 0,
    aug4 INT UNSIGNED NOT NULL DEFAULT 0,
    aug5 INT UNSIGNED NOT NULL DEFAULT 0,
    aug6 INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (character_id, slot, bag_slot),
    KEY idx_character_nms_vault_slot (character_id, slot)
);
)",
		.content_schema_update = false,
	},
	ManifestEntry{
		.version = 33,
		.description = "2026_10_02_character_nms_vault_instance_state",
		// Sentinels on ALL six columns, not just the first: a check that only looked at
		// instnodrop would mark v94 applied after a partial ALTER and never retry, and the
		// vault's own EnsureTables() column count would then keep the vault disabled
		// forever.
		.check = "SELECT COUNT(*) AS n FROM information_schema.columns "
				 "WHERE table_schema = DATABASE() AND table_name = 'character_nms_vault' "
				 "AND column_name IN ('instnodrop', 'custom_data', 'ornament_icon', "
				 "'ornament_idfile', 'ornament_hero_model', 'guid') HAVING COUNT(*) = 6",
		.condition = "empty",
		.match = "",
		.sql = R"(
ALTER TABLE character_nms_vault
    ADD COLUMN instnodrop TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER aug6,
    ADD COLUMN custom_data TEXT DEFAULT NULL AFTER instnodrop,
    ADD COLUMN ornament_icon INT UNSIGNED NOT NULL DEFAULT 0 AFTER custom_data,
    ADD COLUMN ornament_idfile INT UNSIGNED NOT NULL DEFAULT 0 AFTER ornament_icon,
    ADD COLUMN ornament_hero_model INT UNSIGNED NOT NULL DEFAULT 0 AFTER ornament_idfile,
    ADD COLUMN guid BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER ornament_hero_model;
)",
		.content_schema_update = false,
	},
	ManifestEntry{
		.version = 34,
		.description = "2026_10_02_armarium_item",
		.check = "SELECT id FROM items WHERE id = 9011009 AND Name = 'Nautilus Vault' AND itemclass = 0 AND itemtype = 33 AND bagslots = 0 AND bagtype = 0 AND book = 0 AND slots = 0 AND nodrop = 0 AND norent = 1 AND notransfer = 1 AND fvnodrop = 1 AND attuneable = 0 AND loregroup = -1 AND clicktype = 1 AND clickeffect = 1 AND clickname = 'Open Nautilus Vault' AND casttime = 0 AND maxcharges = -1",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Nautilus Vault: bound vault key (VAULTDATA). Identity:
-- id 9011009, remainder 11009; runtime matching requires id >= 9011009 so
-- stock items sharing the remainder can never count as the vault key.
-- Cloned from Satchel of the Hero (9011010) through a temporary table so every live
-- items column is copied. Then converted to a non-container key: right-click must
-- fire the click, not open a bag. clickeffect 1 is a client dummy; the zone
-- intercepts the click and never casts that spell. nodrop = 0 is NO DROP in this
-- schema. norent = 1 is rentable: item_data.h treats NoRent == 0 as deleted after
-- a long camp. Empty CREATE TEMPORARY TABLE ... AS SELECT is not an SQL error, so
-- if both 9011010 and 9011009 are missing this INSERT of NULL into a NOT NULL
-- column fails and custom_version is not stamped. UPDATE after INSERT IGNORE
-- repairs a previously inserted row whose fields drifted (including norent = 0).
DROP TEMPORARY TABLE IF EXISTS nms_armarium_need_source;
CREATE TEMPORARY TABLE nms_armarium_need_source (must_exist TINYINT NOT NULL);
INSERT INTO nms_armarium_need_source (must_exist)
VALUES (CASE
    WHEN EXISTS (SELECT 1 FROM items WHERE id = 9011010) THEN 1
    WHEN EXISTS (SELECT 1 FROM items WHERE id = 9011009) THEN 1
    ELSE NULL
END);
DROP TEMPORARY TABLE IF EXISTS nms_armarium_need_source;
DROP TEMPORARY TABLE IF EXISTS nms_armarium_tmp;
CREATE TEMPORARY TABLE nms_armarium_tmp AS SELECT * FROM items WHERE id = 9011010;
UPDATE nms_armarium_tmp
SET id = 9011009,
    Name = 'Nautilus Vault',
    lore = 'Your Nautilus Vault. Right-click to open storage, bank, merchant, Proc Locker.',
    comment = 'NMS Nautilus Vault opener (custom v34)',
    itemclass = 0,
    itemtype = 33,
    bagsize = 0,
    bagslots = 0,
    bagtype = 0,
    bagwr = 0,
    book = 0,
    nodrop = 0,
    norent = 1,
    notransfer = 1,
    fvnodrop = 1,
    loregroup = -1,
    magic = 1,
    slots = 0,
    weight = 1,
    size = 1,
    price = 0,
    sellrate = 0,
    stackable = 0,
    potionbelt = 0,
    attuneable = 0,
    clicktype = 1,
    clickeffect = 1,
    clicklevel = 0,
    clicklevel2 = 0,
    clickname = 'Open Nautilus Vault',
    casttime = 0,
    casttime_ = 0,
    recastdelay = 0,
    recasttype = 0,
    maxcharges = -1;
INSERT IGNORE INTO items SELECT * FROM nms_armarium_tmp;
UPDATE items
SET Name = 'Nautilus Vault',
    lore = 'Your Nautilus Vault. Right-click to open storage, bank, merchant, Proc Locker.',
    comment = 'NMS Nautilus Vault opener (custom v34)',
    itemclass = 0,
    itemtype = 33,
    bagsize = 0,
    bagslots = 0,
    bagtype = 0,
    bagwr = 0,
    book = 0,
    nodrop = 0,
    norent = 1,
    notransfer = 1,
    fvnodrop = 1,
    loregroup = -1,
    magic = 1,
    slots = 0,
    weight = 1,
    size = 1,
    price = 0,
    sellrate = 0,
    stackable = 0,
    potionbelt = 0,
    attuneable = 0,
    clicktype = 1,
    clickeffect = 1,
    clicklevel = 0,
    clicklevel2 = 0,
    clickname = 'Open Nautilus Vault',
    casttime = 0,
    casttime_ = 0,
    recastdelay = 0,
    recasttype = 0,
    maxcharges = -1
WHERE id = 9011009;
DROP TEMPORARY TABLE IF EXISTS nms_armarium_tmp;
)",
		.content_schema_update = true,
	},
	ManifestEntry{
		.version = 35,
		.description = "2026_10_02_nautilus_vault_rule_on",
		.check = R"(
SELECT rv.rule_value
FROM rule_values rv
WHERE rv.rule_name = 'Custom:NautilusVault'
  AND rv.rule_value IN ('true', '1')
  AND rv.ruleset_id = COALESCE(
    (
      SELECT rs.ruleset_id
      FROM rule_sets rs
      INNER JOIN variables v ON v.varname = 'RuleSet' AND v.value = rs.`name`
      LIMIT 1
    ),
    (
      SELECT rs.ruleset_id
      FROM rule_sets rs
      WHERE rs.`name` = 'default'
      LIMIT 1
    ),
    1
  )
)",
		.condition = "empty",
		.match = "",
		.sql = R"(
-- Nautilus Vault. Compiled default is now true. Live servers read
-- rule_values, so a leftover false/0 row would keep the vault sealed. Only
-- rewrite false/0; a deliberate true is not stomped. INSERT if no row exists.
UPDATE rule_values
SET rule_value = 'true',
    notes = 'Enable the Nautilus Vault (#vault_*) storage, Proc Locker, clicky autoload, vault bank/merchant, and the Nautilus Vault inventory key. Off = no vault commands, grant, or combat hooks.'
WHERE rule_name = 'Custom:NautilusVault'
  AND rule_value IN ('false', '0');
INSERT INTO rule_values (ruleset_id, rule_name, rule_value, notes)
SELECT 1,
       'Custom:NautilusVault',
       'true',
       'Enable the Nautilus Vault (#vault_*) storage, Proc Locker, clicky autoload, vault bank/merchant, and the Nautilus Vault inventory key. Off = no vault commands, grant, or combat hooks.'
FROM DUAL
WHERE NOT EXISTS (
    SELECT 1 FROM rule_values WHERE rule_name = 'Custom:NautilusVault'
);
)",
		.content_schema_update = false,
	},

	// /nmsloot personal loot offers (Rockin-Vik/NMS-Release v36-v40 folded into one table).
	// Code: zone/nms_loot_offers.cpp. Rule Custom:NmsLootOffers (default off).
	ManifestEntry{
		.version = 36,
		.description = "2026_10_24_character_nms_loot_offers",
		.check = "SHOW TABLES LIKE 'character_nms_loot_offers'",
		.condition = "empty",
		.match = "",
		.sql = R"(
CREATE TABLE IF NOT EXISTS character_nms_loot_offers (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    character_id INT UNSIGNED NOT NULL,
    zone_id INT UNSIGNED NOT NULL DEFAULT 0,
    instance_id INT UNSIGNED NOT NULL DEFAULT 0,
    corpse_id INT UNSIGNED NOT NULL DEFAULT 0,
    corpse_serial BIGINT UNSIGNED NOT NULL DEFAULT 0,
    item_id INT UNSIGNED NOT NULL,
    icon INT UNSIGNED NOT NULL DEFAULT 0,
    charges SMALLINT NOT NULL DEFAULT 1,
    bonus TINYINT UNSIGNED NOT NULL DEFAULT 0,
    name VARCHAR(64) NOT NULL DEFAULT '',
    aug1 INT UNSIGNED NOT NULL DEFAULT 0,
    aug2 INT UNSIGNED NOT NULL DEFAULT 0,
    aug3 INT UNSIGNED NOT NULL DEFAULT 0,
    aug4 INT UNSIGNED NOT NULL DEFAULT 0,
    aug5 INT UNSIGNED NOT NULL DEFAULT 0,
    aug6 INT UNSIGNED NOT NULL DEFAULT 0,
    passed TINYINT UNSIGNED NOT NULL DEFAULT 0,
    passed_from VARCHAR(64) NOT NULL DEFAULT '',
    expires_at TIMESTAMP NOT NULL,
    PRIMARY KEY (id),
    KEY idx_character_nms_loot_offers_char (character_id, zone_id, expires_at)
);
)",
		.content_schema_update = false,
	},

	// Used for testing
	//	ManifestEntry{
	//		.version = 9229,
	//		.description = "new_database_check_test",
	//		.check = "SHOW TABLES LIKE 'new_table'",
	//		.condition = "empty",
	//		.match = "",
	//		.sql = R"(
	// CREATE TABLE `new_table`  (
	//  `id` int NOT NULL AUTO_INCREMENT,
	//  PRIMARY KEY (`id`)
	//);
	// CREATE TABLE `new_table1`  (
	//  `id` int NOT NULL AUTO_INCREMENT,
	//  PRIMARY KEY (`id`)
	//);
	// CREATE TABLE `new_table2`  (
	//  `id` int NOT NULL AUTO_INCREMENT,
	//  PRIMARY KEY (`id`)
	//);
	// CREATE TABLE `new_table3`  (
	//  `id` int NOT NULL AUTO_INCREMENT,
	//  PRIMARY KEY (`id`)
	//);
	//)",
	//	}
};

// see struct definitions for what each field does
// struct ManifestEntry {
// 	int         version{};     // database version of the migration
// 	std::string description{}; // description of the migration ex: "add_new_table" or "add_index_to_table"
// 	std::string check{};       // query that checks against the condition
// 	std::string condition{};   // condition or "match_type" - Possible values [contains|match|missing|empty|not_empty]
// 	std::string match{};       // match field that is not always used, but works in conjunction with "condition" values [missing|match|contains]
// 	std::string sql{};         // the SQL DDL that gets ran when the condition is true
// };
