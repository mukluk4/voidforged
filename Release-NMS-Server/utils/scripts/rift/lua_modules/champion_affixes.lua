-- ============================================================================
-- CHAMPION AFFIX SYSTEM (champion_affixes.lua)
-- Deploy to: quests/lua_modules/champion_affixes.lua
--
-- A module, not an NPC script: timers and death events are delivered to the
-- archetype's own script, which forwards them here.
-- ============================================================================

local affixes = {}

local AFFIX_TYPES = { "Molten", "Vortex", "Arcane" }
local CHAMPION_CHANCE = 15   -- percent

-- Existing point-blank AoE spells (cast instantly via SpellFinished)
local SPELL = {
    MOLTEN_PULSE = 1659,     -- Scintillation: fire, 342 dmg, 25 range
    MOLTEN_DEATH = 733,      -- Supernova: fire, 854 dmg, 35 range
    ARCANE_BOLTS = 1653,     -- Jyll's Static Pulse: magic, 495 dmg, 25 range
}

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

    eq.set_timer("affix_pulse", 4000)
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
        -- Pull the current target to the champion
        local target = npc:GetTarget()
        if target.valid and target:IsClient() then
            npc:Emote(string.format("'s vortex drags %s in!", target:GetCleanName()))
            target:GMMove(npc:GetX(), npc:GetY(), npc:GetZ(), target:GetHeading())
        end

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
