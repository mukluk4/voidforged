-- ============================================================================
-- CHAMPION AFFIX SYSTEM (champion_affixes.lua)
-- Deploy to: quests/lua_modules/champion_affixes.lua
--
-- A module, not an NPC script: timers and death events are delivered to the
-- archetype's own script, which forwards them here.
-- ============================================================================

local affixes = {}

local cfg = require("rift_config").champion

local AFFIX_TYPES = { "Molten", "Vortex", "Arcane" }
local CHAMPION_CHANCE = cfg.chance   -- percent

-- Point-blank AoE spells (cast instantly via SpellFinished); set in rift_config.lua
local SPELL = {
    MOLTEN_PULSE = cfg.spells.molten_pulse,
    MOLTEN_DEATH = cfg.spells.molten_death,
    ARCANE_BOLTS = cfg.spells.arcane_bolts,
}

-- Vortex pull: how often (in pulses), who counts as far away, and where they land
local VORTEX = cfg.vortex

function affixes.get(npc)
    if npc:EntityVariableExists("champion_affix") then
        return npc:GetEntityVariable("champion_affix")
    end
    return nil
end

-- Call from the NPC's event_spawn so the pulse timer is bound to that NPC
function affixes.try_initialize(npc)
    if math.random(1, 100) > CHAMPION_CHANCE then return false end

    local affix = AFFIX_TYPES[math.random(1, #AFFIX_TYPES)]
    npc:SetEntityVariable("champion_affix", affix)

    -- Show the affix as a surname under the mob's name
    npc:ChangeLastName(affix .. " Champion")

    npc:ModifyNPCStat("max_hp", tostring(math.floor(npc:GetMaxHP() * 1.5)))
    npc:Heal()

    eq.set_timer("affix_pulse", cfg.pulse_ms)
    return true
end

function affixes.on_timer(e)
    if e.timer ~= "affix_pulse" then return end

    local npc = e.self
    if not npc:IsEngaged() then return end

    local affix = affixes.get(npc)
    if affix == "Molten" then
        npc:Emote("'s body erupts with searing heat!")
        npc:SpellFinished(SPELL.MOLTEN_PULSE, npc)

    elseif affix == "Vortex" then
        -- Every few pulses, drag one player who is keeping their distance to just in front of the champion.
        -- Whoever is already in melee range (the tank) is never pulled.
        local pulses = tonumber(npc:GetEntityVariable("vortex_pulses")) or 0
        pulses = pulses + 1
        npc:SetEntityVariable("vortex_pulses", tostring(pulses))
        if pulses % VORTEX.every_pulses ~= 0 then return end

        local x, y, z = npc:GetX(), npc:GetY(), npc:GetZ()
        local far = {}
        for ent in npc:GetHateList().entries do
            local m = ent.ent
            if m.valid and m:IsClient() and m:CalculateDistance(x, y, z) > VORTEX.min_distance then
                table.insert(far, m)
            end
        end
        if #far == 0 then return end

        local target = far[math.random(1, #far)]
        local dx, dy = target:GetX() - x, target:GetY() - y
        local len    = math.max(math.sqrt(dx * dx + dy * dy), 1) -- far but straight above/below: no NaN
        local lx, ly = x + dx / len * VORTEX.land_distance, y + dy / len * VORTEX.land_distance
        npc:Emote(string.format("'s vortex drags %s in!", target:GetCleanName()))
        target:GMMove(lx, ly, z, target:CalculateHeadingToTarget(x, y))

    elseif affix == "Arcane" then
        npc:Emote(" unleashes spinning arcs of arcane energy!")
        npc:SpellFinished(SPELL.ARCANE_BOLTS, npc)
    end
end

-- Call from event_death (the NPC is still alive enough to cast)
function affixes.on_death(npc)
    if affixes.get(npc) == "Molten" then
        npc:Emote(" explodes in a cataclysmic burst of fire!")
        npc:SpellFinished(SPELL.MOLTEN_DEATH, npc)
    end
end

return affixes
