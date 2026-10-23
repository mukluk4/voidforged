-- ============================================================================
-- RIFT ARCHETYPE MORPH ENGINE (rift_archetypes.lua)
-- Deploy to: quests/global/Rift_Anachronism.lua   (npc_types.name Rift_Anachronism, IDs 1200020-1200023)
--
-- All four archetype npc_types share the DB name "Rift_Anachronism" so this
-- one script serves them all in any rift zone.
-- Archetypes should carry no loot so their corpses vanish immediately
-- (rule NPC:EmptyNPCCorpseDecayTimeMS = 0).
-- ============================================================================

local rift = require("rift_core")
local affixes = require("champion_affixes")

-- Every spawn takes a random look from the creatures native to this zone (plus the client's global
-- models). The client only draws models loaded for the current zone; anything else shows as the
-- default model, so the pool is per zone.
local APPEARANCES = require("rift_appearances")

local function zone_pool()
    local pool = {}
    for _, entry in ipairs(APPEARANCES.zones[eq.get_zone_short_name()] or {}) do table.insert(pool, entry) end
    for _, entry in ipairs(APPEARANCES.global) do table.insert(pool, entry) end
    return pool
end

local ARCHETYPE_SUFFIX = {
    [rift.NPC.MELEE]   = "Vanguard",
    [rift.NPC.CASTER]  = "Evoker",
    [rift.NPC.DISRUPT] = "Dominator",
    [rift.NPC.ELITE]   = "Executioner",
}

function event_spawn(e)
    local npc = e.self
    if not rift.in_rift() then return end

    -- Respawns after the Guardian is up remove themselves
    if rift.spawn_blocked() then
        eq.set_timer("rift_depop", 100)
        return
    end

    local is_elite = (npc:GetNPCTypeID() == rift.NPC.ELITE)

    -- 1. Dynamic appearance
    ApplyRandomAppearance(npc)

    -- 2. Tier stat scaling (elites use the faster boss curve)
    rift.apply_tier_scaling(npc, rift.get_tier(), is_elite)

    -- 3. 15% of archetypes roll a champion affix
    affixes.try_initialize(npc)
end

function event_timer(e)
    if e.timer == "rift_depop" then
        eq.stop_timer(e.timer)
        e.self:Depop(false)
        return
    end
    affixes.on_timer(e)
end

function event_death(e)
    affixes.on_death(e.self)
end

function event_death_complete(e)
    local npc = e.self
    if not rift.in_rift() then return end

    local amount = rift.PROGRESS.TRASH
    if npc:GetNPCTypeID() == rift.NPC.ELITE or affixes.get(npc) then
        amount = rift.PROGRESS.ELITE
    end

    rift.add_progress(amount, npc:GetX(), npc:GetY(), npc:GetZ(), npc:GetHeading())
end

function ApplyRandomAppearance(npc)
    local pool = zone_pool()
    if #pool == 0 then return end
    local entry = pool[math.random(1, #pool)]

    -- Executioners (elites) stand a third taller than their kind so they're easy to spot. Tight zones
    -- cap everything: regulars at 3/4 of the cap, Executioners at 9/10, leaving the top for the Guardian
    local cap  = APPEARANCES.size_cap[eq.get_zone_short_name()]
    local size = entry.size
    if npc:GetNPCTypeID() == rift.NPC.ELITE then
        size = math.min(math.floor(size * 1.33 + 0.5), cap and math.floor(cap * 0.9) or 16)
    elseif cap then
        size = math.min(size, math.floor(cap * 0.75))
    end

    npc:SendIllusionPacket({
        race        = entry.race,
        gender      = entry.gender,
        texture     = entry.texture,
        helmtexture = entry.helmtexture,
        size        = size,
    })

    -- The look's real creature type (Undead, Animal...) so body-type-limited spells work on it
    if entry.bodytype then
        npc:SetBodyType(entry.bodytype, true)
    end

    local suffix = ARCHETYPE_SUFFIX[npc:GetNPCTypeID()]
    if suffix then
        npc:TempName((string.gsub(entry.name .. " " .. suffix, " ", "_")))
    end
end
