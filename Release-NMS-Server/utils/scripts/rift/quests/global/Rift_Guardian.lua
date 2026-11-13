-- ============================================================================
-- RIFT GUARDIAN CONTROLLER (rift_guardian.lua)
-- Deploy to: quests/global/Rift_Guardian.lua   (npc_types.name Rift_Guardian, ID rift.NPC.GUARDIAN)
-- ============================================================================

local rift = require("rift_core")
local affixes = require("champion_affixes")

-- Anachronisms: echoes of the zone's own bosses, torn from another point in time. Only models
-- loaded for this zone render (others show as the default model), so the pool is per zone.
local APPEARANCES = require("rift_appearances")

-- The zone's named bosses; if it has none, its biggest native creature
local function pick_model()
    local zone = eq.get_zone_short_name()
    local bosses = APPEARANCES.bosses[zone] or {}
    if #bosses > 0 then
        return bosses[math.random(1, #bosses)]
    end

    local biggest = nil
    for _, entry in ipairs(APPEARANCES.zones[zone] or {}) do
        if not biggest or entry.size > biggest.size then biggest = entry end
    end
    return biggest
end

-- Timers and spells are set in rift_config.lua
local cfg = require("rift_config").guardian

local ENRAGE_MS         = cfg.enrage_ms
local PULSE_MS          = cfg.pulse_ms
local HATE_WIPE_EVERY   = cfg.hate_wipe_every   -- mechanic 3 fires every Nth pulse

local SPELL = {
    ENRAGE      = cfg.spells.enrage,
    ENERGY_WAVE = cfg.spells.energy_wave,
}

function event_spawn(e)
    local npc = e.self

    local model = pick_model()
    if model then
        npc:SendIllusionPacket({
            race        = model.race,
            gender      = model.gender,
            texture     = model.texture,
            helmtexture = model.helmtexture,
            size        = APPEARANCES.size_cap[eq.get_zone_short_name()] or math.max(model.size, 12),
        })
        npc:TempName((string.gsub("Anachronism of " .. model.name, " ", "_")))
        if model.bodytype then
            npc:SetBodyType(model.bodytype, true)
        end
    end

    rift.apply_tier_scaling(npc, rift.get_tier(), true)

    -- 1 = PBAOE pulses, 2 = adds at 50%, 3 = hate-wipe phase shift
    npc:SetEntityVariable("boss_mechanic", tostring(math.random(1, 3)))
    npc:SetEntityVariable("pulse_count", "0")

    eq.set_timer("boss_pulse", PULSE_MS)

    -- Some Guardians also roll a champion affix on top of their mechanic (no extra HP, never Splitter)
    affixes.try_initialize(npc, { chance = cfg.affix_chance, hp_mult = 1, exclude = cfg.affix_exclude,
                                  title = "Guardian" })
end

-- Enrage clock starts on first engage, not on spawn
function event_combat(e)
    if e.joined and not e.self:EntityVariableExists("enrage_started") then
        e.self:SetEntityVariable("enrage_started", "1")
        eq.set_timer("enrage_timer", ENRAGE_MS)
    end
end

function event_timer(e)
    local npc = e.self

    if string.sub(e.timer, 1, 6) == "affix_" then
        affixes.on_timer(e)
        return
    end

    if e.timer == "enrage_timer" then
        eq.stop_timer("enrage_timer")
        rift.announce(13, string.format("%s ENRAGES! Its power reaches critical levels!", npc:GetCleanName()))
        npc:SpellFinished(SPELL.ENRAGE, npc)

    elseif e.timer == "boss_pulse" and npc:IsEngaged() then
        local mechanic = tonumber(npc:GetEntityVariable("boss_mechanic")) or 1
        local pulses = (tonumber(npc:GetEntityVariable("pulse_count")) or 0) + 1
        npc:SetEntityVariable("pulse_count", tostring(pulses))

        if mechanic == 1 then
            npc:Emote(" unleashes a devastating wave of energy!")
            npc:SpellFinished(SPELL.ENERGY_WAVE, npc)

        elseif mechanic == 2 then
            if npc:GetHPRatio() < 50 and not npc:EntityVariableExists("adds_spawned") then
                npc:SetEntityVariable("adds_spawned", "1")
                rift.announce(13, string.format("%s summons guardians to its aid!", npc:GetCleanName()))
                rift.spawn_minion(rift.NPC.MELEE, npc:GetX() + 10, npc:GetY(), npc:GetZ(), 0)
                rift.spawn_minion(rift.NPC.MELEE, npc:GetX() - 10, npc:GetY(), npc:GetZ(), 0)
            end

        elseif mechanic == 3 then
            if pulses % HATE_WIPE_EVERY == 0 then
                npc:Emote(" phases out of time and forgets its attackers!")
                npc:WipeHateList()
            end
        end
    end
end

-- Warded and the other affixes that react to hits; the return value overrides the damage
function event_damage_taken(e)
    return affixes.on_damage(e)
end

-- Vampiric heals itself for part of what it deals
function event_damage_given(e)
    affixes.on_damage_given(e)
end

function event_death(e)
    affixes.on_death(e.self)
end

function event_death_complete(e)
    local npc = e.self
    rift.complete(npc:GetX(), npc:GetY(), npc:GetZ(), npc:GetHeading())
end
