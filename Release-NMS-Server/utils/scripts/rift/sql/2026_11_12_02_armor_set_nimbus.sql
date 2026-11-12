-- Lasting nimbus for a tier armor set at its full bonus (2026-11-12): one per class the set is made for and gear
-- tier (1 Chronal .. 5 Timeless). Read with the armor set bonuses at zone boot; players hide theirs with #setnimbus off.
-- Edit rows freely; a zone reload or restart picks them up.
CREATE TABLE IF NOT EXISTS armor_set_nimbus (
  class_id      TINYINT UNSIGNED NOT NULL,
  gear_tier     TINYINT UNSIGNED NOT NULL,
  nimbus_effect INT UNSIGNED NOT NULL,
  PRIMARY KEY (class_id, gear_tier)
);

REPLACE INTO armor_set_nimbus (class_id, gear_tier, nimbus_effect) VALUES
  (1, 1, 582), (1, 2, 612), (1, 3, 667), (1, 4, 470), (1, 5, 569),  -- Warrior
  (2, 1, 511), (2, 2, 514), (2, 3, 551), (2, 4, 509), (2, 5, 643),  -- Cleric
  (3, 1, 510), (3, 2, 654), (3, 3, 95), (3, 4, 610), (3, 5, 226),  -- Paladin
  (4, 1, 475), (4, 2, 611), (4, 3, 649), (4, 4, 661), (4, 5, 671),  -- Ranger
  (5, 1, 168), (5, 2, 464), (5, 3, 512), (5, 4, 521), (5, 5, 634),  -- Shadow Knight
  (6, 1, 448), (6, 2, 476), (6, 3, 578), (6, 4, 614), (6, 5, 579),  -- Druid
  (7, 1, 455), (7, 2, 547), (7, 3, 516), (7, 4, 457), (7, 5, 474),  -- Monk
  (8, 1, 450), (8, 2, 471), (8, 3, 502), (8, 4, 656), (8, 5, 156),  -- Bard
  (9, 1, 465), (9, 2, 466), (9, 3, 468), (9, 4, 451), (9, 5, 571),  -- Rogue
  (10, 1, 467), (10, 2, 608), (10, 3, 633), (10, 4, 10), (10, 5, 13),  -- Shaman
  (11, 1, 200), (11, 2, 446), (11, 3, 513), (11, 4, 545), (11, 5, 609),  -- Necromancer
  (12, 1, 461), (12, 2, 204), (12, 3, 576), (12, 4, 607), (12, 5, 644),  -- Wizard
  (13, 1, 463), (13, 2, 469), (13, 3, 517), (13, 4, 641), (13, 5, 447),  -- Magician
  (14, 1, 225), (14, 2, 459), (14, 3, 570), (14, 4, 456), (14, 5, 581),  -- Enchanter
  (15, 1, 572), (15, 2, 585), (15, 3, 453), (15, 4, 663), (15, 5, 548),  -- Beastlord
  (16, 1, 546), (16, 2, 567), (16, 3, 568), (16, 4, 94), (16, 5, 574);  -- Berserker
