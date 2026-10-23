-- ============================================================================
-- PYLON INTERACTION SCRIPT (Rift_Pylon.lua)
-- Deploy to: quests/global/Rift_Pylon.lua   (npc_types.name Rift_Pylon, ID rift.NPC.PYLON)
--
-- rift_core spawns the pylon at 25/50/75% progress; the pylon picks its own
-- type. Players activate it by walking into it or hailing it. Types, spells and
-- pulse timing are set in rift_config.lua (pylons).
--
-- Pulsing pylons stay up, hidden, until their last pulse. Each pulse is cast
-- fresh: Conduit centres on wherever the activator stands, Cleansing reaches every
-- group member in the rift. Pulses stop if the activator dies or leaves.
-- ============================================================================

local rift = require("rift_core")
local cfg  = require("rift_config").pylons

local ACTIVATE_RANGE = 10

local function pylon_data(npc)
    return cfg[tonumber(npc:GetEntityVariable("pylon_type")) or 1]
end

-- The activator, if still alive in this zone
local function activator(npc)
    local client = eq.get_entity_list():GetClientByID(tonumber(npc:GetEntityVariable("activator")) or 0)
    if client.valid and not client:IsDead() then
        return client
    end
    return nil
end

-- The activator and their group members in this zone
local function group_of(client)
    local members = {}
    local group   = client:GetGroup()
    if client:IsGrouped() and group.valid then
        for i = 0, 5 do
            local member = group:GetMember(i)
            if member.valid and member:IsClient() and not member:CastToClient():IsDead() then
                table.insert(members, member:CastToClient())
            end
        end
    else
        table.insert(members, client)
    end
    return members
end

local function pulse(npc, data, client)
    if data.kind == "nuke" then
        -- Point-blank AoE from the activator: hits the mobs around them, never players
        client:SpellFinished(data.spell_id, client)
    elseif data.kind == "splash" then
        -- Ground-targeted in game, so applied to each member directly instead
        for _, member in ipairs(group_of(client)) do
            member:SpellEffect(member, data.spell_id, 100)
        end
    else
        -- Buffs are cast by each member on themselves, so self-only spells land
        for _, member in ipairs(group_of(client)) do
            member:SpellFinished(data.spell_id, member)
        end
    end
end

function event_spawn(e)
    local npc       = e.self
    local pylon_type = math.random(1, #cfg)
    local data      = cfg[pylon_type]

    npc:SetEntityVariable("pylon_type", tostring(pylon_type))
    npc:TempName((string.gsub(data.name, " ", "_")))

    local x, y = npc:GetX(), npc:GetY()
    eq.set_proximity(x - ACTIVATE_RANGE, x + ACTIVATE_RANGE, y - ACTIVATE_RANGE, y + ACTIVATE_RANGE)

    rift.announce(15, string.format("A %s has appeared in the Rift!", data.name))
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
    npc:SetEntityVariable("activator", tostring(client:GetID()))

    local data = pylon_data(npc)
    rift.announce(13, string.format("%s activated a %s! It %s", client:GetCleanName(), data.name, data.emote))

    pulse(npc, data, client)

    local pulses = data.pulses or 1
    if pulses <= 1 then
        npc:Depop(false)
        return
    end

    -- Keep pulsing from a hidden pylon
    npc:SetEntityVariable("pulses_left", tostring(pulses - 1))
    npc:SetTargetable(false)
    npc:SetInvisible(1)
    eq.clear_proximity()
    eq.set_timer("pylon_pulse", data.interval_ms)
end

function event_timer(e)
    if e.timer ~= "pylon_pulse" then return end

    local npc    = e.self
    local client = activator(npc)
    local left   = tonumber(npc:GetEntityVariable("pulses_left")) or 0
    if not client or left <= 0 then
        eq.stop_timer("pylon_pulse")
        npc:Depop(false)
        return
    end

    pulse(npc, pylon_data(npc), client)
    npc:SetEntityVariable("pulses_left", tostring(left - 1))
end
