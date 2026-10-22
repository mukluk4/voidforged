-- Queued DB fixes, round 3 (apply on "ok to update")

-- Rift respawn 90s -> 180s (+-30s). ~100 kills clear a rift, so groups that can finish kill 7-8 a minute;
-- 3 minutes keeps cleared rooms clear behind them without the zone running dry.
UPDATE spawn2 SET respawntime = 180, variance = 30 WHERE version = 1 AND spawngroupID = 5003834;
