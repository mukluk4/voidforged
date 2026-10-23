-- Copy each character's deepest Greater Rift clear (global key gr_deepest_<char_id>) into a
-- character-scoped bucket "rift_deepest", which the tier gear vendors check. rift_core keeps it current.
INSERT INTO data_buckets (`key`, `value`, `expires`, `account_id`, `character_id`, `npc_id`, `bot_id`, `zone_id`, `instance_id`)
SELECT 'rift_deepest', d.`value`, 0, 0, CAST(SUBSTRING(d.`key`, 12) AS UNSIGNED), 0, 0, 0, 0
FROM data_buckets d
WHERE d.`key` LIKE 'gr\_deepest\_%' AND d.character_id = 0
  AND NOT EXISTS (SELECT 1 FROM data_buckets x WHERE x.`key` = 'rift_deepest' AND x.character_id = CAST(SUBSTRING(d.`key`, 12) AS UNSIGNED));
