-- Queued DB fixes, round 4 (apply on "ok to update")

-- #1 The Void on the bazaar map ("Attune the Map", NMS Waypoints), under the Special tab next to the
--    Wall of Slaughter. Lands where the rift portal returns players (moved there 2026-10-24). Like every waypoint it's greyed out until the
--    character unlocks it (walking near the waypoint trigger in The Void, or a progression unlock).
INSERT INTO nms_waypoints (shortname, long_name, category, x, y, z, heading)
SELECT 'thevoida', 'The Void', 8, 25.33, 78.03, 1.21, 241.75 FROM DUAL
WHERE NOT EXISTS (SELECT 1 FROM nms_waypoints WHERE shortname = 'thevoida');

-- #3 Permanent server buffs: the rule read 'True', and rule booleans are case-sensitive ("true"),
--    so every zone boot took the disabled branch and wiped the Echo buffs
UPDATE rule_values SET rule_value = 'true' WHERE rule_name = 'Custom:PermanentServerBuffsEnabled' AND BINARY rule_value = 'True';
