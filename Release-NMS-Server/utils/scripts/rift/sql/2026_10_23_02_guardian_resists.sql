-- Rift Guardian base resists 185 -> 55: at 185 players' spells were resisted most of the time from tier 1.
-- Tiers add +1 per 4 tiers on top (resist_scaling in rift_config.lua).
UPDATE npc_types SET mr = 55, fr = 55, cr = 55, pr = 55, dr = 55 WHERE id = 1200001;
