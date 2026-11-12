-- Tier weapons, shields and range items cost more now that they carry the new procs and sympathetics (2026-11-12):
-- 100 / 200 / 400 / 800 / 1600 Chronal Shards at gear tiers 1-5 (was 30 / 60 / 120 / 240 / 480). Tier arrows keep
-- their price (15 / 30 / 60 / 120 / 240). Zones load merchant lists at boot: #reload merchants or restart the zone.
UPDATE merchantlist SET alt_currency_cost = 100  WHERE merchantid = 1200044 AND alt_currency_cost = 30;
UPDATE merchantlist SET alt_currency_cost = 200  WHERE merchantid = 1200045 AND alt_currency_cost = 60;
UPDATE merchantlist SET alt_currency_cost = 400  WHERE merchantid = 1200046 AND alt_currency_cost = 120;
UPDATE merchantlist SET alt_currency_cost = 800  WHERE merchantid = 1200047 AND alt_currency_cost = 240;
UPDATE merchantlist SET alt_currency_cost = 1600 WHERE merchantid = 1200048 AND alt_currency_cost = 480;
