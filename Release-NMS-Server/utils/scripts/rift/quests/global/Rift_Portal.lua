-- ============================================================================
-- RETURN PORTAL NPC SCRIPT (void_portal.lua)
-- Deploy to: quests/global/Rift_Portal.lua   (npc_types.name Rift_Portal, ID rift.NPC.PORTAL)
-- ============================================================================

local rift = require("rift_core")

function event_say(e)
    if e.message:findi("hail") or e.message:findi("leave") then
        rift.send_to_hub(e.other)
    end
end
