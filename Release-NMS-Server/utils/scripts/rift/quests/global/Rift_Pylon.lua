-- ============================================================================
-- PYLON INTERACTION SCRIPT (pylon_controller.lua)
-- Deploy to: quests/global/Rift_Pylon.lua   (npc_types.name Rift_Pylon, ID rift.NPC.PYLON)
--
-- rift_core spawns the pylon at 25/50/75% progress; the pylon picks its own
-- type. Players activate it by walking into it or hailing it.
-- ============================================================================

local rift = require("rift_core")

local PYLON_SPELLS = {
    [1] = { name = "Conduit Pylon",   spell_id = 90010, emote = "surges with raw electricity, striking down nearby foes!" },
    [2] = { name = "Power Pylon",     spell_id = 90011, emote = "radiates immense power, supercharging your attacks and spells!" },
    [3] = { name = "Shielding Pylon", spell_id = 90012, emote = "wraps your group in impenetrable divine protection!" },
    [4] = { name = "Speed Pylon",     spell_id = 90013, emote = "accelerates time around you to blinding speeds!" }
}

local ACTIVATE_RANGE = 10

function event_spawn(e)
    local npc = e.self
    local pylon_type = math.random(1, #PYLON_SPELLS)
    local pylon_data = PYLON_SPELLS[pylon_type]

    npc:SetEntityVariable("pylon_type", tostring(pylon_type))
    npc:TempName((string.gsub(pylon_data.name, " ", "_")))

    local x, y = npc:GetX(), npc:GetY()
    eq.set_proximity(x - ACTIVATE_RANGE, x + ACTIVATE_RANGE, y - ACTIVATE_RANGE, y + ACTIVATE_RANGE)

    rift.announce(15, string.format("A %s has appeared in the Rift!", pylon_data.name))
end

function event_enter(e)
    ActivatePylon(e.self, e.other)
end

function event_say(e)
    if e.message:findi("hail") then
        ActivatePylon(e.self, e.other)
    end
end

function ActivatePylon(npc, client)
    -- Two players can trigger it in the same tick; only the first counts
    if npc:EntityVariableExists("used") then return end
    npc:SetEntityVariable("used", "1")

    local pylon_data = PYLON_SPELLS[tonumber(npc:GetEntityVariable("pylon_type")) or 1]
    rift.announce(13, string.format("%s activated a %s! It %s", client:GetCleanName(), pylon_data.name, pylon_data.emote))

    -- Buff the activator and their group members in this instance
    local group = client:GetGroup()
    if client:IsGrouped() and group.valid then
        for i = 0, 5 do
            local member = group:GetMember(i)
            if member.valid and member:IsClient() then
                npc:SpellFinished(pylon_data.spell_id, member)
            end
        end
    else
        npc:SpellFinished(pylon_data.spell_id, client)
    end

    npc:Depop(false)
end
