-- ============================================================================
-- CHAMPION AFFIX SYSTEM (champion_affixes.lua)
-- Deploy to: quests/lua_modules/champion_affixes.lua
--
-- A module, not an NPC script: timers, damage and death events are delivered to the
-- archetype's (or the Guardian's) own script, which forwards them here.
--
-- Affixes: Molten, Vortex, Arcane, Warded, Summoner, Splitter, Thorns, Reflective, Juggernaut,
-- Blinker, Nullifier, Vampiric, Phasing, Enraged, Storm. A mob rolls one affix, plus one more for
-- every 100 rift tiers (cfg.per_tiers), all different. Adds a Summoner calls and the copy a
-- Splitter makes are "summoned": they roll no affix and give no rift progress.
-- ============================================================================

local affixes = {}

local cfg = require("rift_config").champion
local rift = require("rift_core")

local AFFIX_TYPES = {
    "Molten", "Vortex", "Arcane", "Warded", "Summoner", "Splitter", "Thorns", "Reflective",
    "Juggernaut", "Blinker", "Nullifier", "Vampiric", "Phasing", "Enraged", "Storm",
}
local CHAMPION_CHANCE = cfg.chance   -- percent

-- Point-blank AoE spells (cast instantly via SpellFinished); set in rift_config.lua
local SPELL = {
    MOLTEN_PULSE = cfg.spells.molten_pulse,
    MOLTEN_DEATH = cfg.spells.molten_death,
    ARCANE_BOLTS = cfg.spells.arcane_bolts,
}

local VORTEX, WARDED, SUMMONER, SPLITTER = cfg.vortex, cfg.warded, cfg.summoner, cfg.splitter
local THORNS, REFLECTIVE, JUGGERNAUT = cfg.thorns, cfg.reflective, cfg.juggernaut
local BLINKER, NULLIFIER, VAMPIRIC = cfg.blinker, cfg.nullifier, cfg.vampiric
local PHASING, ENRAGED, STORM = cfg.phasing, cfg.enraged, cfg.storm

-- ----------------------------------------------------------------------------
-- Helpers
-- ----------------------------------------------------------------------------

local function is_spell(spell_id)
    return spell_id and spell_id > 0 and spell_id < 65535
end

-- Damage from Thorns, Reflective and Storm is queued and dealt from a short timer, never from
-- inside a damage event: [npc entity id] = { [target entity id] = { amount, spell_id } }
local pending = {}

local function queue_damage(npc, target, amount, spell_id)
    if amount <= 0 then return end
    local by_npc = pending[npc:GetID()] or {}
    pending[npc:GetID()] = by_npc
    local entry = by_npc[target:GetID()]
    if entry and entry.spell_id == spell_id then
        entry.amount = entry.amount + amount
    else
        by_npc[target:GetID()] = { amount = amount, spell_id = spell_id }
    end
    eq.set_timer("affix_retaliate", 100)
end

local function deal_pending(npc)
    local by_npc = pending[npc:GetID()]
    pending[npc:GetID()] = nil
    if not by_npc then return end
    for target_id, entry in pairs(by_npc) do
        local mob = eq.get_entity_list():GetMobID(target_id)
        if mob.valid and mob:GetHP() > 0 then
            mob:Damage(npc, entry.amount, entry.spell_id, 0)
        end
    end
end

-- Players (and their pets) on the hate list
local function hated_players(npc)
    local list = {}
    for ent in npc:GetHateList().entries do
        local m = ent.ent
        if m.valid and (m:IsClient() or (m:IsPet() and m:GetOwner().valid and m:GetOwner():IsClient())) then
            table.insert(list, m)
        end
    end
    return list
end

local function hated_clients(npc)
    local list = {}
    for _, m in ipairs(hated_players(npc)) do
        if m:IsClient() then table.insert(list, m) end
    end
    return list
end

-- ----------------------------------------------------------------------------
-- Affix list on an NPC (entity variable "champion_affix" = "Warded,Storm")
-- ----------------------------------------------------------------------------

-- Set while a champion spawns its adds or its copy, so the new NPC's event_spawn marks itself
-- summoned (spawn2 runs event_spawn before it returns; every rift script shares this module)
affixes.spawning = false

-- The affix list as text, or nil when the NPC has none (callers use it as "is a champion")
function affixes.get(npc)
    if npc:EntityVariableExists("champion_affix") then
        return npc:GetEntityVariable("champion_affix")
    end
    return nil
end

function affixes.has(npc, name)
    local list = affixes.get(npc)
    if not list then return false end
    for affix in string.gmatch(list, "[^,]+") do
        if affix == name then return true end
    end
    return false
end

function affixes.is_summoned(npc)
    return npc:EntityVariableExists("rift_summoned")
end

-- Spawns a summoned NPC: no affix roll, no progress. Spawned as a Guardian minion so it also
-- stays up once the Guardian is out (trash that spawns then normally removes itself).
local function spawn_summoned(npc_id, x, y, z, h)
    affixes.spawning = true
    local ok, add = pcall(rift.spawn_minion, npc_id, x, y, z, h)
    affixes.spawning = false
    if ok and add and add.valid then return add end
    return nil
end

-- One affix, plus one per cfg.per_tiers tiers
function affixes.count_for_tier(tier)
    return 1 + math.floor((tier or 1) / (cfg.per_tiers or 100))
end

-- Call from the NPC's event_spawn so the pulse timer is bound to that NPC.
-- opts (all optional): chance (percent, default the champion chance), hp_mult (default 1.5),
-- exclude (affix names it can't roll), title (surname suffix after the affix names, default
-- "Champion"; false for none)
function affixes.try_initialize(npc, opts)
    opts = opts or {}
    if affixes.spawning then
        npc:SetEntityVariable("rift_summoned", "1")
        return false
    end
    if math.random(1, 100) > (opts.chance or CHAMPION_CHANCE) then return false end

    local pool = {}
    for _, name in ipairs(AFFIX_TYPES) do
        if not (opts.exclude and opts.exclude[name]) then table.insert(pool, name) end
    end
    if #pool == 0 then return false end

    local chosen = {}
    for i = 1, math.min(affixes.count_for_tier(rift.get_tier()), #pool) do
        table.insert(chosen, table.remove(pool, math.random(1, #pool)))
    end
    npc:SetEntityVariable("champion_affix", table.concat(chosen, ","))

    -- Show the affixes as a surname under the mob's name
    local title = opts.title
    if title == nil then title = "Champion" end
    local surname = table.concat(chosen, " ")
    if title then surname = surname .. " " .. title end
    npc:ChangeLastName(surname)

    local hp_mult = opts.hp_mult or 1.5
    if affixes.has(npc, "Juggernaut") then
        hp_mult = hp_mult * JUGGERNAUT.hp_mult
        for _, ability in ipairs({ SpecialAbility.unstunable, SpecialAbility.unsnareable, SpecialAbility.unslowable,
                                   SpecialAbility.unmezable, SpecialAbility.uncharmable, SpecialAbility.unfearable }) do
            npc:SetSpecialAbility(ability, 1)
        end
    end
    if hp_mult ~= 1 then
        npc:ModifyNPCStat("max_hp", tostring(math.floor(npc:GetMaxHP() * hp_mult)))
    end
    npc:Heal()

    eq.set_timer("affix_pulse", cfg.pulse_ms)
    return true
end

-- ----------------------------------------------------------------------------
-- Affix actions
-- ----------------------------------------------------------------------------

local function summon_adds(npc)
    -- Count the adds from earlier calls that are still up
    local alive, kept = 0, {}
    for id in string.gmatch(npc:GetEntityVariable("summoned_ids") or "", "%d+") do
        local add = eq.get_entity_list():GetNPCByID(tonumber(id))
        if add.valid and not add:IsCorpse() then
            alive = alive + 1
            table.insert(kept, id)
        end
    end
    if alive >= SUMMONER.max_alive then return end

    local target = npc:GetHateTop()
    local x, y, z, h = npc:GetX(), npc:GetY(), npc:GetZ(), npc:GetHeading()
    local count = math.min(SUMMONER.adds, SUMMONER.max_alive - alive)
    for i = 1, count do
        local npc_id = (math.random(2) == 1) and rift.NPC.MELEE or rift.NPC.CASTER
        local angle = math.random() * 2 * math.pi
        local add = spawn_summoned(npc_id, x + math.cos(angle) * 10, y + math.sin(angle) * 10, z, h)
        if add then
            table.insert(kept, tostring(add:GetID()))
            if target.valid then add:AddToHateList(target, 1) end
        end
    end
    npc:SetEntityVariable("summoned_ids", table.concat(kept, ","))
    npc:Emote(" tears open the rift and calls for aid!")
end

-- Vortex: drag one player who is keeping their distance to just in front of it. Whoever is
-- already in melee range (the tank) is never pulled.
local function vortex_pull(npc)
    local x, y, z = npc:GetX(), npc:GetY(), npc:GetZ()
    local far = {}
    for _, m in ipairs(hated_clients(npc)) do
        if m:CalculateDistance(x, y, z) > VORTEX.min_distance then table.insert(far, m) end
    end
    if #far == 0 then return end

    local target = far[math.random(1, #far)]
    local dx, dy = target:GetX() - x, target:GetY() - y
    local len    = math.max(math.sqrt(dx * dx + dy * dy), 1) -- far but straight above/below: no NaN
    local lx, ly = x + dx / len * VORTEX.land_distance, y + dy / len * VORTEX.land_distance
    npc:Emote(string.format("'s vortex drags %s in!", target:GetCleanName()))
    target:GMMove(lx, ly, z, target:CalculateHeadingToTarget(x, y))
end

-- Blinker: jump to the farthest player on its hate list
local function blink(npc)
    local x, y, z = npc:GetX(), npc:GetY(), npc:GetZ()
    local target, best = nil, BLINKER.min_distance
    for _, m in ipairs(hated_clients(npc)) do
        local d = m:CalculateDistance(x, y, z)
        if d > best then target, best = m, d end
    end
    if not target then return end

    local angle = math.random() * 2 * math.pi
    npc:Emote(string.format(" blinks through time to %s!", target:GetCleanName()))
    npc:GMMove(target:GetX() + math.cos(angle) * 5, target:GetY() + math.sin(angle) * 5, target:GetZ(),
               npc:CalculateHeadingToTarget(target:GetX(), target:GetY()))
end

-- Nullifier: strip one beneficial buff from a random player on its hate list
local function nullify(npc)
    local clients = hated_clients(npc)
    if #clients == 0 then return end
    local target = clients[math.random(1, #clients)]

    local slots = {}
    for i, buff in ipairs(target:GetBuffs()) do
        local spell_id = buff:GetSpellID()
        if is_spell(spell_id) and eq.is_beneficial_spell(spell_id) then
            table.insert(slots, { slot = i - 1, spell_id = spell_id })
        end
    end
    if #slots == 0 then return end

    local pick = slots[math.random(1, #slots)]
    target:BuffFadeBySlot(pick.slot)
    npc:Emote(string.format(" unravels %s's %s!", target:GetCleanName(), eq.get_spell_name(pick.spell_id)))
end

-- Phasing: a warning, then immune to damage for a moment
local function phase_warn(npc)
    npc:Emote(" begins to flicker out of time...")
    eq.set_timer("affix_phase_in", PHASING.warn_ms)
end

-- Storm: chain lightning from a random player through the nearest players and pets
local function storm(npc)
    local targets = hated_players(npc)
    if #targets == 0 then return end

    local current = targets[math.random(1, #targets)]
    local damage = math.floor(npc:GetMaxDMG() * STORM.damage_mult)
    local hit = { [current:GetID()] = true }
    npc:Emote(string.format(" calls down a storm on %s!", current:GetCleanName()))

    for jump = 0, STORM.jumps do
        queue_damage(npc, current, damage, STORM.spell)
        damage = math.floor(damage * STORM.falloff_pct / 100)

        local next_target, best = nil, STORM.jump_range
        for _, m in ipairs(targets) do
            if not hit[m:GetID()] then
                local d = m:CalculateDistance(current:GetX(), current:GetY(), current:GetZ())
                if d <= best then next_target, best = m, d end
            end
        end
        if not next_target or damage <= 0 then break end
        hit[next_target:GetID()] = true
        current = next_target
    end
end

local function enrage(npc)
    npc:ModifyNPCStat("min_hit", tostring(math.floor(npc:GetMinDMG() * ENRAGED.damage_mult)))
    npc:ModifyNPCStat("max_hit", tostring(math.floor(npc:GetMaxDMG() * ENRAGED.damage_mult)))
    npc:ModifyNPCStat("attack_delay", tostring(math.max(1, math.floor(npc:GetAttackDelay() * ENRAGED.delay_mult / 100))))
    npc:SetSpecialAbility(SpecialAbility.flurry, 1)
    npc:Emote(" flies into a frenzied rage!")
end

-- ----------------------------------------------------------------------------
-- Events (forwarded from the NPC's script)
-- ----------------------------------------------------------------------------

function affixes.on_timer(e)
    local npc = e.self
    local t = e.timer

    if t == "affix_split" then
        eq.stop_timer(t)
        affixes.split(npc)
        return
    elseif t == "affix_retaliate" then
        eq.stop_timer(t)
        deal_pending(npc)
        return
    elseif t == "affix_enrage" then
        eq.stop_timer(t)
        enrage(npc)
        return
    elseif t == "affix_phase_in" then
        eq.stop_timer(t)
        npc:SetEntityVariable("phased", "1")
        npc:Emote(" phases out of time!")
        eq.set_timer("affix_phase_out", PHASING.phase_ms)
        return
    elseif t == "affix_phase_out" then
        eq.stop_timer(t)
        npc:DeleteEntityVariable("phased")
        npc:Emote(" snaps back into the present!")
        return
    end

    if t ~= "affix_pulse" then return end
    if not npc:IsEngaged() then return end

    -- One shared pulse counter; each affix acts on its own beat
    local pulses = (tonumber(npc:GetEntityVariable("affix_pulses")) or 0) + 1
    npc:SetEntityVariable("affix_pulses", tostring(pulses))

    for affix in string.gmatch(affixes.get(npc) or "", "[^,]+") do
        if affix == "Molten" then
            npc:Emote("'s body erupts with searing heat!")
            npc:SpellFinished(SPELL.MOLTEN_PULSE, npc)
        elseif affix == "Arcane" then
            npc:Emote(" unleashes spinning arcs of arcane energy!")
            npc:SpellFinished(SPELL.ARCANE_BOLTS, npc)
        elseif affix == "Vortex" and pulses % VORTEX.every_pulses == 0 then
            vortex_pull(npc)
        elseif affix == "Warded" and pulses % WARDED.every_pulses == 1 % WARDED.every_pulses then
            -- The first ward goes up on the first pulse of the fight, then every few pulses
            npc:SetEntityVariable("ward_pool", tostring(math.floor(npc:GetMaxHP() * WARDED.absorb_pct / 100)))
            npc:Emote(" is wrapped in a shimmering ward!")
        elseif affix == "Summoner" and pulses % SUMMONER.every_pulses == 1 % SUMMONER.every_pulses then
            summon_adds(npc)
        elseif affix == "Blinker" and pulses % BLINKER.every_pulses == 0 then
            blink(npc)
        elseif affix == "Nullifier" and pulses % NULLIFIER.every_pulses == 2 % NULLIFIER.every_pulses then
            nullify(npc)
        elseif affix == "Phasing" and pulses % PHASING.every_pulses == 3 % PHASING.every_pulses then
            phase_warn(npc)
        elseif affix == "Storm" and pulses % STORM.every_pulses == 0 then
            storm(npc)
        end
    end
end

-- Call from the NPC's event_damage_taken. Returns the damage override for that event:
-- 0 changes nothing, -1 cancels the hit, a positive number replaces the damage.
function affixes.on_damage(e)
    local npc = e.self
    local list = affixes.get(npc)
    if not list then return 0 end

    local damage = e.damage
    if damage <= 0 then return 0 end
    local original = damage
    local attacker = e.other
    local has_attacker = attacker and attacker.valid

    -- Phasing: nothing lands while it is out of time
    if npc:EntityVariableExists("phased") then
        return -1
    end

    -- Reflective: a direct spell hit or proc may be turned back on the caster
    if has_attacker and affixes.has(npc, "Reflective") and is_spell(e.spell_id) and not e.is_buff_tic
        and not e.is_damage_shield and math.random(1, 100) <= REFLECTIVE.chance then
        local amount = math.min(damage, math.floor(attacker:GetMaxHP() * REFLECTIVE.max_hp_pct / 100))
        queue_damage(npc, attacker, amount, e.spell_id)
        npc:Emote(string.format(" reflects %s back at %s!", eq.get_spell_name(e.spell_id), attacker:GetCleanName()))
        return -1
    end

    -- Thorns: melee hits only (no spells, procs or damage shields)
    if has_attacker and affixes.has(npc, "Thorns") and not is_spell(e.spell_id) and not e.is_damage_shield then
        local amount = math.min(math.floor(damage * THORNS.return_pct / 100),
                                math.floor(attacker:GetMaxHP() * THORNS.max_hp_pct / 100))
        queue_damage(npc, attacker, amount, THORNS.spell)
    end

    -- Warded: the ward soaks what it can
    if affixes.has(npc, "Warded") then
        local pool = tonumber(npc:GetEntityVariable("ward_pool")) or 0
        if pool > 0 then
            local absorbed = math.min(pool, damage)
            pool = pool - absorbed
            npc:SetEntityVariable("ward_pool", tostring(pool))
            if pool <= 0 then npc:Emote("'s ward shatters!") end
            damage = damage - absorbed
        end
    end

    -- HP thresholds, checked against what actually lands
    local hp_after = npc:GetHP() - damage
    if hp_after > 0 then
        if affixes.has(npc, "Splitter") and not npc:EntityVariableExists("split_done")
            and hp_after * 100 <= npc:GetMaxHP() * SPLITTER.at_pct then
            npc:SetEntityVariable("split_done", "1")
            eq.set_timer("affix_split", 100)
        end
        if affixes.has(npc, "Enraged") and not npc:EntityVariableExists("enrage_done")
            and hp_after * 100 <= npc:GetMaxHP() * ENRAGED.at_pct then
            npc:SetEntityVariable("enrage_done", "1")
            eq.set_timer("affix_enrage", 100)
        end
    end

    if damage == original then return 0 end
    return damage > 0 and damage or -1
end

-- Call from the NPC's event_damage_given: Vampiric heals for part of what it deals
function affixes.on_damage_given(e)
    local npc = e.self
    if e.damage <= 0 or not affixes.has(npc, "Vampiric") then return end
    if npc:GetHP() >= npc:GetMaxHP() then return end
    local heal = math.min(math.floor(e.damage * VAMPIRIC.heal_pct / 100),
                          math.floor(npc:GetMaxHP() * VAMPIRIC.max_hp_pct / 100))
    if heal > 0 then npc:HealDamage(heal) end
end

-- Splitter: a copy with the same look, name and max HP appears beside it, and the two share
-- the HP it had left
function affixes.split(npc)
    if npc:IsCorpse() or npc:GetHP() <= 0 then return end

    local x, y, z, h = npc:GetX(), npc:GetY(), npc:GetZ(), npc:GetHeading()
    local copy = spawn_summoned(npc:GetNPCTypeID(), x + 8, y + 8, z, h)
    if not copy then return end

    copy:SendIllusionPacket({
        race        = npc:GetRace(),
        gender      = npc:GetGender(),
        texture     = npc:GetTexture(),
        helmtexture = npc:GetHelmTexture(),
        size        = npc:GetSize(),
    })
    copy:SetBodyType(npc:GetBodyType(), true)
    copy:TempName((string.gsub(npc:GetCleanName(), " ", "_")))
    copy:ChangeLastName("Splinter")

    local share = math.max(1, math.floor(npc:GetHP() / 2))
    copy:ModifyNPCStat("max_hp", tostring(npc:GetMaxHP()))
    copy:SetHP(share)
    npc:SetHP(share)

    local target = npc:GetHateTop()
    if target.valid then copy:AddToHateList(target, 1) end
    npc:Emote(" splits in two!")
end

-- Call from event_death (the NPC is still alive enough to cast)
function affixes.on_death(npc)
    pending[npc:GetID()] = nil  -- its timer dies with it
    if affixes.has(npc, "Molten") then
        npc:Emote(" explodes in a cataclysmic burst of fire!")
        npc:SpellFinished(SPELL.MOLTEN_DEATH, npc)
    end
end

return affixes
