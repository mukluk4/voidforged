-- Class test armor sets: 16 classes x 6 pieces, copied from the Rogue Test Boots shell (900101)
-- with the slot and look of a plain cloth item. IDs = 900000 + piece block + class id.
START TRANSACTION;

CREATE TEMPORARY TABLE tmp_class (cid INT, cname VARCHAR(20));
INSERT INTO tmp_class VALUES
(1,'Warrior'),(2,'Cleric'),(3,'Paladin'),(4,'Ranger'),(5,'Shadowknight'),(6,'Druid'),(7,'Monk'),(8,'Bard'),
(9,'Rogue'),(10,'Shaman'),(11,'Necromancer'),(12,'Wizard'),(13,'Magician'),(14,'Enchanter'),(15,'Beastlord'),(16,'Berserker');

CREATE TEMPORARY TABLE tmp_piece (blk INT, piece VARCHAR(20), tpl INT);
INSERT INTO tmp_piece VALUES
(300,'Helm',1001),(400,'Chest',1004),(600,'Sleeves',1008),(700,'Bracer',1009),(800,'Gloves',1010),(900,'Leggings',1011);

CREATE TEMPORARY TABLE tmp_items AS
SELECT i.*, c.cid, c.cname, p.blk, p.piece, p.tpl
FROM items i JOIN tmp_class c JOIN tmp_piece p
WHERE i.id = 900101;

UPDATE tmp_items t JOIN items src ON src.id = t.tpl SET
  t.id       = 900000 + t.blk + t.cid,
  t.Name     = CONCAT(t.cname, ' Test ', t.piece),
  t.lore     = CONCAT(t.cname, ' Test ', t.piece),
  t.classes  = 1 << (t.cid - 1),
  t.slots    = src.slots,
  t.material = src.material,
  t.icon     = src.icon,
  t.idfile   = src.idfile,
  t.weight   = src.weight,
  t.size     = src.size;

INSERT INTO dynamic_items (item_id, class_id, tier, min_affixes, max_affixes, stat_multiplier, notes)
SELECT id, cid, 1, 2, 3, 1.0, 'Class test item' FROM tmp_items;

ALTER TABLE tmp_items DROP COLUMN cid, DROP COLUMN cname, DROP COLUMN blk, DROP COLUMN piece, DROP COLUMN tpl;
INSERT INTO items SELECT * FROM tmp_items;

DROP TEMPORARY TABLE tmp_items;
DROP TEMPORARY TABLE tmp_piece;
DROP TEMPORARY TABLE tmp_class;

COMMIT;
