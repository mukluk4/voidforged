-- Class sets: rename the class test pieces to their set theme, make one armor set per class
-- (set_id = 100 + class id) and give each set bonuses at 2, 4 and 8 pieces.
START TRANSACTION;

CREATE TEMPORARY TABLE tmp_theme (cid INT, theme VARCHAR(32));
INSERT INTO tmp_theme VALUES
(1,'Juggernaut'),(2,'Divine Conduit'),(3,'Bulwark'),(4,'Wildstalker'),(5,'Dread Harbinger'),(6,'Verdant Storm'),
(7,'Iron Fist'),(8,'Maestro'),(9,'Shadowblade'),(10,'Spiritcaller'),(11,'Lich Lord'),(12,'Archmage'),
(13,'Elementalist'),(14,'Mesmerist'),(15,'Feral Bond'),(16,'Bloodrage');

CREATE TEMPORARY TABLE tmp_piece (blk INT, piece VARCHAR(20));
INSERT INTO tmp_piece VALUES
(9002,'Boots'),(9003,'Helm'),(9004,'Chestguard'),(9006,'Sleeves'),(9007,'Bracer'),(9008,'Gloves'),(9009,'Leggings');

UPDATE items i
JOIN dynamic_items d ON d.item_id = i.id
JOIN tmp_theme t ON t.cid = d.class_id
JOIN tmp_piece p ON p.blk = FLOOR(i.id / 100)
SET i.Name = CONCAT(t.theme, ' ', p.piece),
    i.lore = CONCAT(t.theme, ' ', p.piece)
WHERE i.id BETWEEN 900201 AND 900916;

UPDATE dynamic_items SET notes = 'Class set piece' WHERE item_id BETWEEN 900201 AND 900916;

DELETE FROM armor_set_items WHERE set_id BETWEEN 101 AND 116;
INSERT INTO armor_set_items (item_id, set_id, class_mask)
SELECT item_id, 100 + class_id, 0 FROM dynamic_items WHERE item_id BETWEEN 900201 AND 900916;

DELETE FROM armor_set_bonuses WHERE set_id BETWEEN 101 AND 116;
-- (class, pieces, stat_id, affix, value); stat_id 7 = haste, which has no affix code
CREATE TEMPORARY TABLE tmp_bonus (cid INT, pieces INT, stat_id INT, affix VARCHAR(32), val INT);
INSERT INTO tmp_bonus VALUES
-- Warrior: Juggernaut
(1,2,0,'hp',250),(1,2,0,'ac',15),
(1,4,0,'ac',40),(1,4,0,'hp_regen',10),
(1,8,0,'hp',750),(1,8,0,'shielding',5),(1,8,0,'stun_resist',10),
-- Cleric: Divine Conduit
(2,2,0,'mana',250),(2,2,0,'mana_regen',5),
(2,4,0,'wis',30),(2,4,0,'heal_amount_pct',8),
(2,8,0,'mana_regen',10),(2,8,0,'heal_crit_chance_pct',5),
-- Paladin: Bulwark
(3,2,0,'hp',200),(3,2,0,'mana',150),
(3,4,0,'ac',30),(3,4,0,'heal_amount_pct',5),
(3,8,0,'hp',500),(3,8,0,'shielding',3),(3,8,0,'stun_resist',10),
-- Ranger: Wildstalker
(4,2,0,'dex',20),(4,2,0,'attack',25),
(4,4,0,'skill_archery_pct',10),
(4,8,7,'',15),(4,8,0,'archery_crit_chance_pct',4),
-- Shadowknight: Dread Harbinger
(5,2,0,'hp',250),(5,2,0,'mana',150),
(5,4,0,'ac',30),(5,4,0,'spell_poison_pct',10),(5,4,0,'spell_disease_pct',10),
(5,8,0,'hp',500),(5,8,0,'dot_crit_chance_pct',5),
-- Druid: Verdant Storm
(6,2,0,'mana',250),(6,2,0,'mana_regen',5),
(6,4,0,'spell_fire_pct',8),(6,4,0,'spell_cold_pct',8),(6,4,0,'heal_amount_pct',5),
(6,8,0,'damage_shield',25),(6,8,0,'dot_crit_chance_pct',5),
-- Monk: Iron Fist
(7,2,0,'agi',20),(7,2,0,'attack',25),
(7,4,0,'flying_kick_skill_pct',10),
(7,8,7,'',15),(7,8,0,'melee_crit_chance_pct',3),(7,8,0,'avoidance',5),
-- Bard: Maestro
(8,2,0,'cha',25),(8,2,0,'mana',150),
(8,4,0,'bard_stringed_pct',10),(8,4,0,'bard_wind_pct',10),(8,4,0,'bard_brass_pct',10),
(8,4,0,'bard_percussion_pct',10),(8,4,0,'bard_singing_pct',10),
(8,8,0,'hp',500),(8,8,7,'',15),(8,8,0,'end_regen',5),
-- Rogue: Shadowblade
(9,2,0,'dex',20),(9,2,0,'attack',25),
(9,4,0,'backstab_skill_pct',10),
(9,8,7,'',15),(9,8,0,'melee_crit_chance_pct',4),(9,8,0,'backstab_dmg_pct',10),
-- Shaman: Spiritcaller
(10,2,0,'mana',250),(10,2,0,'hp',200),
(10,4,0,'sta',20),(10,4,0,'spell_cold_pct',10),(10,4,0,'spell_poison_pct',10),(10,4,0,'spell_disease_pct',10),
(10,8,0,'mana_regen',10),(10,8,0,'heal_amount_pct',5),(10,8,0,'dot_crit_chance_pct',5),
-- Necromancer: Lich Lord
(11,2,0,'mana',300),(11,2,0,'mana_regen',5),
(11,4,0,'spell_poison_pct',10),(11,4,0,'spell_disease_pct',10),
(11,8,0,'mana_regen',10),(11,8,0,'dot_crit_chance_pct',5),(11,8,0,'dot_crit_dmg_pct',10),
-- Wizard: Archmage
(12,2,0,'mana',300),(12,2,0,'int',30),
(12,4,0,'spell_fire_pct',10),(12,4,0,'spell_cold_pct',10),(12,4,0,'spell_magic_pct',10),
(12,8,0,'spell_crit_chance_pct',5),(12,8,0,'spell_crit_dmg_pct',15),
-- Magician: Elementalist
(13,2,0,'mana',300),(13,2,0,'sta',20),
(13,4,0,'spell_fire_pct',10),(13,4,0,'spell_magic_pct',10),
(13,8,0,'spell_crit_chance_pct',5),(13,8,0,'damage_shield',25),
-- Enchanter: Mesmerist
(14,2,0,'mana',300),(14,2,0,'cha',25),
(14,4,0,'mana_regen',10),(14,4,0,'spell_magic_pct',10),
(14,8,0,'spell_shield',5),(14,8,0,'spell_crit_chance_pct',5),
-- Beastlord: Feral Bond
(15,2,0,'str',20),(15,2,0,'agi',20),(15,2,0,'mana',150),
(15,4,0,'skill_h2h_pct',10),(15,4,0,'spell_cold_pct',10),(15,4,0,'spell_poison_pct',10),
(15,8,7,'',15),(15,8,0,'melee_crit_chance_pct',3),
-- Berserker: Bloodrage
(16,2,0,'hp',250),(16,2,0,'str',25),
(16,4,0,'frenzy_skill_pct',10),(16,4,0,'attack',30),
(16,8,7,'',20),(16,8,0,'melee_crit_chance_pct',5),(16,8,0,'melee_crit_dmg_pct',15);

INSERT INTO armor_set_bonuses (set_id, min_pieces, class_mask, stat_id, affix, stat_value)
SELECT 100 + cid, pieces, 0, stat_id, affix, val FROM tmp_bonus ORDER BY cid, pieces;

DROP TEMPORARY TABLE tmp_bonus;
DROP TEMPORARY TABLE tmp_piece;
DROP TEMPORARY TABLE tmp_theme;

COMMIT;
