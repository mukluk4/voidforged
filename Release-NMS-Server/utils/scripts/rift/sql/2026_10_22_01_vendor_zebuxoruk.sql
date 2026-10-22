-- Queued DB fixes (apply on "ok to update")

-- #1 Chronal_Quartermaster opens the alternate currency window (and token purchases roll stats)
UPDATE npc_types SET class = 70 WHERE id = 1200010;

-- #3 Zebuxoruk becomes storyteller + rift keeper, in the Keeper's spot
--    class 1: right-clicking an alt currency merchant with nothing for sale opened an empty window
UPDATE npc_types SET class = 1, lastname = 'Chronal Fabric' WHERE id = 459000;
UPDATE spawn2 SET x = 84.709999, y = -48.740002, z = 4.150000, heading = 437.750000 WHERE id = 130090;
--    The_Rift_Keeper's spawn is switched off, not deleted (delete this row to bring him back)
INSERT INTO spawn2_disabled (spawn2_id, instance_id, disabled)
SELECT 3388453, 0, 1 FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM spawn2_disabled WHERE spawn2_id = 3388453);
