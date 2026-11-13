-- Guild halls, phase 1: Infamy, the guild treasury and hall ownership. Additive only: new tables and
-- the list of open-world raid bosses that earn Infamy. Nothing reads these until plugins/guild_halls.pl
-- is deployed, and players see nothing until a GM turns guild halls on at the Guild Steward.

START TRANSACTION;

-- One row per guild: running Infamy (never spent), treasury platinum, and the hall tier it owns (0 = none)
CREATE TABLE IF NOT EXISTS nms_guild_halls (
	guild_id       INT UNSIGNED    NOT NULL PRIMARY KEY,
	infamy         BIGINT UNSIGNED NOT NULL DEFAULT 0,
	treasury       BIGINT UNSIGNED NOT NULL DEFAULT 0,
	hall_tier      TINYINT UNSIGNED NOT NULL DEFAULT 0,
	hall_bought_at DATETIME        NULL,
	updated_at     TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Every Infamy award: source is rift, raid, boss or level
CREATE TABLE IF NOT EXISTS nms_guild_infamy_log (
	id         INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
	guild_id   INT UNSIGNED NOT NULL,
	amount     INT UNSIGNED NOT NULL,
	members    SMALLINT UNSIGNED NOT NULL,
	source     VARCHAR(16)  NOT NULL,
	detail     VARCHAR(128) NOT NULL DEFAULT '',
	created_at TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
	KEY guild_time (guild_id, created_at)
);

-- Donations (positive) and purchases (negative), in platinum
CREATE TABLE IF NOT EXISTS nms_guild_treasury_log (
	id         INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
	guild_id   INT UNSIGNED NOT NULL,
	char_id    INT UNSIGNED NOT NULL,
	amount     BIGINT       NOT NULL,
	reason     VARCHAR(64)  NOT NULL,
	created_at TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
	KEY guild_time (guild_id, created_at)
);

-- A guild's first kill of each boss pays 5x once
CREATE TABLE IF NOT EXISTS nms_guild_first_kills (
	guild_id   INT UNSIGNED NOT NULL,
	boss_key   VARCHAR(64)  NOT NULL,
	created_at TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (guild_id, boss_key)
);

-- Hall amenities a guild has bought, with the level owned (unlock 'buffs' = the Guild Hall Mystic's tier 1-5)
CREATE TABLE IF NOT EXISTS nms_guild_unlocks (
	guild_id  INT UNSIGNED     NOT NULL,
	unlock_id VARCHAR(32)      NOT NULL,
	level     TINYINT UNSIGNED NOT NULL DEFAULT 1,
	bought_at TIMESTAMP        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
	PRIMARY KEY (guild_id, unlock_id)
);

-- Characters that already paid the max level award
CREATE TABLE IF NOT EXISTS nms_guild_max_level (
	char_id    INT UNSIGNED NOT NULL PRIMARY KEY,
	guild_id   INT UNSIGNED NOT NULL,
	created_at TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Open-world raid bosses that earn Infamy, by era (0 Classic 10, 1 Kunark 15, 2 Velious 20, 3 Luclin 25,
-- 4 Planes of Power 30 per guild member). Several ids for one boss are its phases or zone copies; the plugin
-- pays one kill per boss name per zone copy per 30 minutes. Add or remove rows to change the list.
CREATE TABLE IF NOT EXISTS nms_guild_infamy_bosses (
	npc_id INT UNSIGNED     NOT NULL PRIMARY KEY,
	era    TINYINT UNSIGNED NOT NULL,
	note   VARCHAR(64)      NOT NULL DEFAULT ''
);

INSERT IGNORE INTO nms_guild_infamy_bosses (npc_id, era, note) VALUES
	-- Classic
	(32040, 0, 'Lord Nagafen'), (73057, 0, 'Lady Vox'), (64001, 0, 'Phinigel Autropos'),
	(72003, 0, 'Cazic Thule'), (76007, 0, 'Innoruuk'), (186107, 0, 'Innoruuk (hateplaneb)'),
	(71065, 0, 'Eye of Veeshan'), (39138, 0, 'Master Yael'),
	-- Kunark
	(89154, 1, 'Trakanon'), (89181, 1, 'Trakanon'), (102112, 1, 'Venril Sathir'), (102126, 1, 'Venril Sathir'),
	(105182, 1, 'Venril Sathir'), (86014, 1, 'Gorenaire'), (91093, 1, 'Talendor'), (94009, 1, 'Severilous'),
	(96073, 1, 'Faydedar'), (96089, 1, 'Faydedar'), (103056, 1, 'Overking Bathezid'),
	(108040, 1, 'Druushk'), (108512, 1, 'Druushk'), (108043, 1, 'Hoshkar'), (108517, 1, 'Hoshkar'),
	(108047, 1, 'Nexona'), (108513, 1, 'Nexona'), (108048, 1, 'Phara Dar'), (108510, 1, 'Phara Dar'),
	(108050, 1, 'Silverwing'), (108509, 1, 'Silverwing'), (108053, 1, 'Xygoz'), (108511, 1, 'Xygoz'),
	-- Velious
	(129003, 2, 'Dain Frostreaver IV'), (114583, 2, 'Kerafyrm'), (128089, 2, 'Kerafyrm'), (128095, 2, 'Kerafyrm'),
	(127098, 2, 'Tunare'), (124155, 2, 'Vulak`Aerr'), (124011, 2, 'Dagarn the Destroyer'),
	(113118, 2, 'Derakor the Vindicator'), (113215, 2, 'King Tormax'), (117073, 2, 'Kelorek`Dar'),
	(120084, 2, 'Klandicar'), (120005, 2, 'Sontalak'), (123115, 2, 'Zlandicar'), (119112, 2, 'Wuoshi'),
	(112049, 2, 'Lord Doljonijiarnimorinar'), (112025, 2, 'Velketor the Sorcerer'),
	(114106, 2, 'Lord Yelinak'), (114618, 2, 'Lord Yelinak'), (127096, 2, 'Prince Thirneg'),
	(128044, 2, 'Zeixshi-Kar the Ancient'), (120064, 2, 'Mraaka'),
	(124010, 2, 'Aaryonar'), (124071, 2, 'Cekenar'), (124037, 2, 'Dozekar the Cursed'), (124004, 2, 'Eashen of the Sky'),
	(124105, 2, 'Gozzrem'), (124001, 2, 'Ikatiar the Venom'), (124072, 2, 'Jorlleag'), (124077, 2, 'Lady Mirenilla'),
	(124076, 2, 'Lady Nevederia'), (124020, 2, 'Lendiniara the Keeper'), (124008, 2, 'Lord Feshlak'),
	(124103, 2, 'Lord Koi`Doken'), (124074, 2, 'Lord Kreizenn'), (124017, 2, 'Lord Vyemm'), (124075, 2, 'Sevalak'),
	(124104, 2, 'Telkorenar'), (124078, 2, 'Zemm'), (124073, 2, 'Zlexak'),
	-- Luclin
	(162065, 3, 'Emperor Ssraeshza'), (162227, 3, 'Emperor Ssraeshza'), (162189, 3, 'Blood of Ssraeshza'),
	(162076, 3, 'High Priest of Ssraeshza'), (162190, 3, 'Xerkizh the Creator'),
	(158006, 3, 'Aten Ha Ra'), (158096, 3, 'Aten Ha Ra'), (158016, 3, 'Thall Va Xakra'), (158125, 3, 'Thall Va Xakra'),
	(163075, 3, 'Grieg Veneficus'), (163097, 3, 'Grieg Veneficus'), (163156, 3, 'Grieg Veneficus'),
	(159690, 3, 'Lord Inquisitor Seru'), (159691, 3, 'Lord Inquisitor Seru'), (160375, 3, 'Lcea Katta'),
	-- Planes of Power
	(223201, 4, 'Quarm'), (214109, 4, 'Rallos Zek'), (214052, 4, 'Rallos Zek'), (214113, 4, 'Rallos Zek the Warlord'),
	(223168, 4, 'Rallos Zek'), (214110, 4, 'Vallon Zek'), (214111, 4, 'Vallon Zek'), (214116, 4, 'Vallon Zek'),
	(214117, 4, 'Vallon Zek'), (214118, 4, 'Vallon Zek'), (214083, 4, 'Vallon Zek'), (223164, 4, 'Vallon Zek'),
	(223078, 4, 'Vallon Zek'), (214108, 4, 'Tallon Zek'), (214026, 4, 'Tallon Zek'), (223077, 4, 'Tallon Zek'),
	(220006, 4, 'Lord Mithaniel Marr'), (220015, 4, 'Lord Mithaniel Marr'), (207001, 4, 'Saryrn'), (223076, 4, 'Saryrn'),
	(204065, 4, 'Terris Thule'), (221008, 4, 'Terris Thule'), (223075, 4, 'Terris Thule'),
	(200055, 4, 'Bertoxxulous'), (223142, 4, 'Bertoxxulous'), (223166, 4, 'Cazic Thule'), (223167, 4, 'Innoruuk'),
	(215056, 4, 'Xegony'), (216094, 4, 'Coirnav'), (216048, 4, 'Coirnav'), (217076, 4, 'Fennin Ro'), (217054, 4, 'Fennin Ro'),
	(222014, 4, 'Avatar of Earth'), (209026, 4, 'Agnarr the Storm Lord'), (209108, 4, 'Karana'),
	(212025, 4, 'Solusek Ro'), (212023, 4, 'Arlyxir'), (212063, 4, 'Rizlona'), (212026, 4, 'Rizlona'),
	(208074, 4, 'Aerin`Dar'), (205091, 4, 'Grummus'), (206074, 4, 'Manaetic Behemoth'), (206046, 4, 'Manaetic Behemoth');

COMMIT;
