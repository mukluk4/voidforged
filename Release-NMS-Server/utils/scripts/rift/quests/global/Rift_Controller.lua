-- ============================================================================
-- GREATER RIFT SYSTEM: Zone Controller NPC (zone_main.lua)
-- Deploy to: quests/global/Rift_Controller.lua   (npc_types.name Rift_Controller, ID rift.NPC.CONTROLLER)
--
-- The controller is an invisible, untargetable NPC (e.g. race 127, bodytype 11)
-- with one spawn2 row at rift.INSTANCE_VERSION in every zone of the pool.
-- It owns the master timer; all rift state and logic lives in rift_core.lua.
-- ============================================================================

local rift = require("rift_core")

function event_spawn(e)
    -- Does nothing if spawned outside a Keeper-created instance
    if not rift.in_rift() then return end

    if rift.start() then
        eq.set_timer("rift_master_tick", 1000)
    end
end

function event_timer(e)
    if e.timer == "rift_master_tick" and not rift.tick() then
        eq.stop_timer(e.timer)
    end
end
