-- ============================================================================
-- GREATER RIFT CORE MODULE (rift_core.lua)
-- Deploy to: quests/lua_modules/rift_core.lua
--
-- Every rift script does `local rift = require("rift_core")`.
-- EQEmu runs each quest script in its own sandbox, so a global function
-- defined in one script is NOT visible from another. A require'd module is
-- shared by every script in the same zone process, and each instance runs in
-- its own zone process, so the state below is effectively per-rift.
-- ============================================================================

local rift = {}

-- ----------------------------------------------------------------------------
-- Configuration: every server-specific value lives in rift_config.lua
-- ----------------------------------------------------------------------------
local cfg = require("rift_config")

local SPECIAL_CASTING_RESIST_DIFFICULTY = 43 -- SpecialAbility::CastingResistDifficulty
local SPECIAL_COUNTER_AVOID_DAMAGE      = 44 -- SpecialAbility::CounterAvoidDamage (param 0: all avoidance)

-- base + per_tier * tier, never past max (the penetration settings in rift_config.lua)
local function by_tier(c, tier)
    return math.floor(math.min(c.base + c.per_tier * tier, c.max))
end
rift.CONFIG = cfg

rift.HUB_ZONE_ID = cfg.hub.zone_id
rift.HUB_X, rift.HUB_Y, rift.HUB_Z, rift.HUB_H = cfg.hub.x, cfg.hub.y, cfg.hub.z, cfg.hub.h

rift.INSTANCE_VERSION = cfg.instance_version
rift.TIME_LIMIT       = cfg.time_limit
rift.INSTANCE_GRACE   = cfg.instance_grace
rift.TARGET_PROGRESS  = cfg.target_progress
rift.MAX_TIER         = cfg.max_tier

rift.NPC = {
    CONTROLLER = cfg.npc.controller,
    GUARDIAN   = cfg.npc.guardian,
    PYLON      = cfg.npc.pylon,
    PORTAL     = cfg.npc.portal,
    VENDOR     = cfg.npc.vendor,
    KEEPER     = cfg.npc.keeper,
    MELEE      = cfg.npc.melee,
    CASTER     = cfg.npc.caster,
    DISRUPT    = cfg.npc.disrupt,
    ELITE      = cfg.npc.elite,
}

rift.PROGRESS = { TRASH = cfg.progress.trash, ELITE = cfg.progress.elite }

-- Clear reward: amount = BASE + PER_TIER * tier, +SPEED_BONUS% if cleared in under SPEED_SECONDS
rift.REWARD = {
    CURRENCY_ID   = cfg.reward.currency_id,
    CURRENCY_NAME = cfg.reward.currency_name,
    BASE          = cfg.reward.base,
    PER_TIER      = cfg.reward.per_tier,
    SPEED_SECONDS = cfg.reward.speed_seconds,
    SPEED_BONUS   = cfg.reward.speed_bonus,
    PLATINUM_PER_TIER = cfg.reward.platinum_per_tier or 0,
    FORTUNE_TRIBUTE_ID   = cfg.reward.fortune_tribute_id or 0,
    FORTUNE_PCT_PER_TIER = cfg.reward.fortune_pct_per_tier or 0,
}

local PYLON_THRESHOLDS = cfg.pylon_at
local TIME_WARNINGS    = cfg.time_warnings
local PROGRESS_STEP    = cfg.progress_step

-- Data bucket keys (all written with an expiry so they clean themselves up)
function rift.key_tier(instance_id)  return "gr_tier_"   .. instance_id end
function rift.key_start(instance_id) return "gr_start_"  .. instance_id end
function rift.key_active(char_id)    return "gr_active_" .. char_id end
function rift.key_deepest(char_id)   return "gr_deepest_" .. char_id end -- no expiry: progression
function rift.key_state(instance_id) return "gr_state_"  .. instance_id end -- progress, so a zone crash doesn't reset it

-- ----------------------------------------------------------------------------
-- Per-instance state
-- ----------------------------------------------------------------------------
local state = { instance_id = -1 }

-- "x,y,z,h" <-> { x, y, z, h }
local function loc_string(loc) return loc and string.format("%.1f,%.1f,%.1f,%.1f", loc[1], loc[2], loc[3], loc[4]) or "" end
local function parse_loc(s)
    local x, y, z, h = (s or ""):match("^([^,]+),([^,]+),([^,]+),([^,]+)$")
    return x and { tonumber(x), tonumber(y), tonumber(z), tonumber(h) } or nil
end

-- Saved as "key=value;..." under gr_state_<instance id>, expiring with the instance
local function save_state()
    local pylons = {}
    for threshold in pairs(state.pylons_done) do pylons[#pylons + 1] = tostring(threshold) end
    local value = string.format("progress=%d;step=%d;pylons=%s;boss=%s;over=%d;portal=%s",
        math.floor(state.progress), state.last_step, table.concat(pylons, ","), loc_string(state.boss_loc),
        state.is_over and 1 or 0, loc_string(state.portal_loc))
    local expires = state.start_time + rift.TIME_LIMIT + rift.INSTANCE_GRACE - os.time()
    if expires > 0 then
        eq.set_data(rift.key_state(state.instance_id), value, tostring(expires))
    end
end

local function load_state(instance_id)
    state = {
        instance_id   = instance_id,
        tier          = tonumber(eq.get_data(rift.key_tier(instance_id))) or 1,
        start_time    = tonumber(eq.get_data(rift.key_start(instance_id))) or os.time(),
        progress      = 0,
        last_step     = 0,
        is_active     = false,
        is_over       = false,
        boss_spawned  = false,
        boss_loc      = nil,
        portal_loc    = nil,
        allow_spawn   = false,
        pylons_done   = {},
        warnings_done = {},
    }

    -- After a zone crash or restart, pick up where the rift left off
    local saved = {}
    for k, v in (eq.get_data(rift.key_state(instance_id)) or ""):gmatch("(%w+)=([^;]*)") do saved[k] = v end
    state.progress  = tonumber(saved.progress) or 0
    state.last_step = tonumber(saved.step) or 0
    for threshold in (saved.pylons or ""):gmatch("%d+") do state.pylons_done[tonumber(threshold)] = true end
    state.boss_loc     = parse_loc(saved.boss)
    state.boss_spawned = state.boss_loc ~= nil
    state.is_over      = saved.over == "1"
    state.portal_loc   = parse_loc(saved.portal)

    -- Time warnings already past stay quiet
    local remaining = state.start_time + rift.TIME_LIMIT - os.time()
    for _, t in ipairs(TIME_WARNINGS) do
        if remaining < t then state.warnings_done[t] = true end
    end
end

local function ensure_state()
    local instance_id = eq.get_zone_instance_id()
    if state.instance_id ~= instance_id then
        load_state(instance_id)
    end
end

-- ----------------------------------------------------------------------------
-- Helpers
-- ----------------------------------------------------------------------------
local function each_client(fn)
    for client in eq.get_entity_list():GetClientList().entries do
        if client.valid then fn(client) end
    end
end

local function first_client()
    for client in eq.get_entity_list():GetClientList().entries do
        if client.valid then return client end
    end
    return nil
end

-- Message every player in this instance (one zone process == one instance)
function rift.announce(color, msg)
    each_client(function(client) client:Message(color, msg) end)
end

-- Progression: a character can open any tier up to one past their deepest clear
function rift.get_deepest(char_id)
    return tonumber(eq.get_data(rift.key_deepest(char_id))) or 0
end

function rift.max_tier(char_id)
    return math.min(rift.get_deepest(char_id) + 1, rift.MAX_TIER)
end

function rift.is_allowed_tier(char_id, tier)
    return tier ~= nil and tier >= 1 and tier <= rift.max_tier(char_id)
end

-- A clear only counts toward a character's own progression if they could have opened that tier
-- themselves; being carried deeper pays the same rewards but unlocks nothing
local function record_clear(client, tier)
    local deepest = rift.get_deepest(client:CharacterID())
    if tier == deepest + 1 then
        eq.set_data(rift.key_deepest(client:CharacterID()), tostring(tier))
        -- Character-scoped copy: the tier gear vendors hide items until it reaches their unlock tier
        client:SetBucket(cfg.gear_unlock_bucket, tostring(tier))
        client:Message(15, string.format("Your deepest Fracture is now Tier %d. Tier %d is open to you.", tier, math.min(tier + 1, rift.MAX_TIER)))
    elseif tier > deepest + 1 then
        client:Message(15, string.format("This Fracture was beyond your own reach; your deepest remains Tier %d.", deepest))
    end
end

-- True only inside an instance the Rift Keeper created
function rift.in_rift()
    local instance_id = eq.get_zone_instance_id()
    return instance_id > 0 and eq.get_data(rift.key_tier(instance_id)) ~= ""
end

function rift.get_tier()
    ensure_state()
    return state.tier
end

function rift.remaining()
    ensure_state()
    return state.start_time + rift.TIME_LIMIT - os.time()
end

function rift.percent()
    ensure_state()
    return math.floor(state.progress * 100 / rift.TARGET_PROGRESS)
end

-- Archetypes that (re)spawn after the Guardian is up should remove themselves
function rift.spawn_blocked()
    ensure_state()
    return state.boss_spawned and not state.allow_spawn
end

-- ----------------------------------------------------------------------------
-- Stat scaling (shared by trash, elites and the Guardian)
-- ----------------------------------------------------------------------------
function rift.apply_tier_scaling(npc, tier, is_boss)
    -- Spell and melee penetration apply from tier 1 (see rift_config.lua)
    npc:SetSpecialAbility(SPECIAL_CASTING_RESIST_DIFFICULTY, 1)
    npc:SetSpecialAbilityParam(SPECIAL_CASTING_RESIST_DIFFICULTY, 0, -by_tier(cfg.spell_penetration, tier or 1))
    npc:ModifyNPCStat("heroic_strikethrough", tostring(by_tier(cfg.heroic_strikethrough, tier or 1)))
    npc:SetSpecialAbility(SPECIAL_COUNTER_AVOID_DAMAGE, 1)
    npc:SetSpecialAbilityParam(SPECIAL_COUNTER_AVOID_DAMAGE, 0, by_tier(cfg.counter_avoid, tier or 1))

    local steps = (tier or 1) - 1
    if steps <= 0 then return end

    local hp_step, dmg_step = 0.15, 0.08
    if is_boss then
        hp_step, dmg_step = 0.25, 0.10
    end

    local hp_factor  = 1.0 + hp_step * steps
    local dmg_factor = 1.0 + dmg_step * steps
    local def_factor = 1.0 + 0.03 * steps

    npc:ModifyNPCStat("max_hp",  tostring(math.floor(npc:GetMaxHP() * hp_factor)))
    npc:ModifyNPCStat("min_hit", tostring(math.floor(npc:GetMinDMG() * dmg_factor)))
    npc:ModifyNPCStat("max_hit", tostring(math.floor(npc:GetMaxDMG() * dmg_factor)))
    -- Spell damage (nukes, DoTs, champion and Guardian AoEs) scales like melee damage
    npc:ModifyNPCStat("spellscale", tostring(math.floor(100 * dmg_factor)))
    npc:ModifyNPCStat("ac",      tostring(math.floor(npc:GetAC() * def_factor)))
    -- Resists grow by a flat amount, not a factor, so players' spells still land at high tiers
    local resist_add = by_tier(cfg.resist_scaling, tier)
    npc:ModifyNPCStat("mr",      tostring(npc:GetMR() + resist_add))
    npc:ModifyNPCStat("fr",      tostring(npc:GetFR() + resist_add))
    npc:ModifyNPCStat("cr",      tostring(npc:GetCR() + resist_add))
    npc:ModifyNPCStat("pr",      tostring(npc:GetPR() + resist_add))
    npc:ModifyNPCStat("dr",      tostring(npc:GetDR() + resist_add))
    npc:Heal()
end

-- ----------------------------------------------------------------------------
-- Rift lifecycle (driven by the controller NPC's timer)
-- ----------------------------------------------------------------------------
function rift.start()
    ensure_state()

    -- Cleared or failed before a zone restart: just bring the way out back
    if state.is_over then
        local p = state.portal_loc
        if p and not eq.get_entity_list():IsMobSpawnedByNpcTypeID(rift.NPC.PORTAL) then
            eq.spawn2(rift.NPC.PORTAL, 0, 0, p[1], p[2], p[3], p[4])
        end
        return false
    end

    local remaining = rift.remaining()
    if remaining <= 0 then return false end

    state.is_active = true

    -- Resumed after a zone restart with the Guardian already called: bring it back if it didn't return
    if state.boss_spawned then
        local b = state.boss_loc
        if not eq.get_entity_list():IsMobSpawnedByNpcTypeID(rift.NPC.GUARDIAN) then
            eq.spawn2(rift.NPC.GUARDIAN, 0, 0, b[1], b[2], b[3], b[4])
        end
        rift.announce(15, string.format("The Fracture holds. Defeat the Rift Guardian! %d minute(s) remain.",
            math.ceil(remaining / 60)))
        return true
    end

    if state.progress > 0 then
        rift.announce(15, string.format("The Fracture holds. Greater Rift Tier %d resumes at %d%% with %d minute(s) left.",
            state.tier, rift.percent(), math.ceil(remaining / 60)))
        return true
    end

    rift.announce(15, string.format("Greater Rift Tier %d initiated! The Chronal Fabric will hold for %d minute(s).",
        state.tier, math.ceil(remaining / 60)))
    return true
end

-- Returns false once the rift is over so the controller can stop its timer
function rift.tick()
    ensure_state()
    if not state.is_active then return false end

    local remaining = rift.remaining()

    for _, t in ipairs(TIME_WARNINGS) do
        if remaining <= t and not state.warnings_done[t] then
            state.warnings_done[t] = true
            rift.announce(13, string.format("Greater Rift: %d minute(s) remaining! Progress: %d%%",
                math.ceil(remaining / 60), rift.percent()))
        end
    end

    if remaining <= 0 then
        rift.fail()
        return false
    end
    return true
end

-- Called from a dying archetype; x/y/z/h is where it died
function rift.add_progress(amount, x, y, z, h)
    ensure_state()
    if not state.is_active or state.boss_spawned then return end

    state.progress = math.min(rift.TARGET_PROGRESS, state.progress + amount)
    local percent = math.floor(state.progress * 100 / rift.TARGET_PROGRESS)

    for _, threshold in ipairs(PYLON_THRESHOLDS) do
        if percent >= threshold and not state.pylons_done[threshold] then
            state.pylons_done[threshold] = true
            eq.spawn2(rift.NPC.PYLON, 0, 0, x, y, z, h)
        end
    end

    if state.progress >= rift.TARGET_PROGRESS then
        rift.spawn_guardian(x, y, z, h)
        return
    end

    local step = math.floor(percent / PROGRESS_STEP)
    if step > state.last_step then
        state.last_step = step
        rift.announce(15, string.format("Greater Rift progress: %d%%", step * PROGRESS_STEP))
    end
    save_state()
end

function rift.spawn_guardian(x, y, z, h)
    state.boss_spawned = true
    state.boss_loc = { x, y, z, h }
    save_state()
    rift.announce(15, "An Anachronism tears through the Fabric! Defeat the Rift Guardian to seal the Rift!")

    -- Clear the remaining trash and any unclaimed pylons
    for _, id in ipairs({ rift.NPC.MELEE, rift.NPC.CASTER, rift.NPC.DISRUPT, rift.NPC.ELITE, rift.NPC.PYLON }) do
        eq.depop_all(id)
    end

    eq.spawn2(rift.NPC.GUARDIAN, 0, 0, x, y, z, h)
end

-- Lets the Guardian summon archetype adds without them depopping themselves
function rift.spawn_minion(npc_type_id, x, y, z, h)
    state.allow_spawn = true
    eq.spawn2(npc_type_id, 0, 0, x, y, z, h)
    state.allow_spawn = false
end

function rift.complete(x, y, z, h)
    ensure_state()
    if not state.is_active then return end
    state.is_active = false

    local clear_time = os.time() - state.start_time
    rift.announce(15, string.format("GREATER RIFT CLEARED! The Chrono-Anchor re-stitches the Fabric. Time: %02d:%02d (Tier %d)",
        math.floor(clear_time / 60), clear_time % 60, state.tier))

    each_client(function(client)
        eq.delete_data(rift.key_active(client:CharacterID()))
        rift.grant_rewards(client, state.tier, clear_time)
        record_clear(client, state.tier)
    end)

    eq.spawn2(rift.NPC.PORTAL, 0, 0, x + 5, y + 5, z, h)
    rift.announce(15, "A rift portal leading back to The Void has opened!")
    state.is_over = true
    state.portal_loc = { x + 5, y + 5, z, h }
    save_state()
end

function rift.fail()
    state.is_active = false
    state.is_over = true
    rift.announce(13, string.format("The Rift's energy dissipates... You failed to complete the Rift in time. (Reached %d%%)", rift.percent()))

    -- Portal so nobody is stuck waiting for the instance to expire
    local client = first_client()
    if client then
        state.portal_loc = { client:GetX() + 5, client:GetY() + 5, client:GetZ(), client:GetHeading() }
        eq.spawn2(rift.NPC.PORTAL, 0, 0, state.portal_loc[1], state.portal_loc[2], state.portal_loc[3], state.portal_loc[4])
        rift.announce(15, "A rift portal leading back to The Void has opened!")
    end
    save_state()
end

-- Called once per player in the instance on a clear. Pays alternate currency
-- so players can bank it and buy several rolled items at once from the vendor.
function rift.reward_amount(tier, clear_time)
    local r = rift.REWARD
    local amount = r.BASE + r.PER_TIER * tier
    if clear_time < r.SPEED_SECONDS then
        amount = amount + math.floor(amount * r.SPEED_BONUS / 100)
    end
    return amount
end

-- Chronal Fortune's tribute tier if it's active for this player in this rift, else 0.
-- Bucket: "<instance id>|<tribute id>:<tier>,..."
local function fortune_tier(client)
    local value = client:GetBucket("chronal_tributes") or ""
    local instance, list = value:match("^(%d+)|(.*)$")
    if tonumber(instance) ~= eq.get_zone_instance_id() then
        return 0
    end
    for id, t in list:gmatch("(%d+):(%d+)") do
        if tonumber(id) == rift.REWARD.FORTUNE_TRIBUTE_ID then
            return tonumber(t)
        end
    end
    return 0
end

function rift.grant_rewards(client, tier, clear_time)
    local r = rift.REWARD
    local amount = rift.reward_amount(tier, clear_time)
    local fortune = fortune_tier(client) * r.FORTUNE_PCT_PER_TIER
    if fortune > 0 then
        local extra = math.floor(amount * fortune / 100)
        amount = amount + extra
        client:Message(15, string.format("Chronal Fortune adds %d%% (%d) to your shards.", fortune, extra))
    end
    client:AddAlternateCurrencyValue(r.CURRENCY_ID, amount)
    client:Message(15, string.format("You receive %d %s. (Total: %d)",
        amount, r.CURRENCY_NAME, client:GetAlternateCurrencyValue(r.CURRENCY_ID)))

    -- Platinum: running rifts leaves no time for farming coin
    local platinum = r.PLATINUM_PER_TIER * tier
    if platinum > 0 then
        client:AddMoneyToPP(0, 0, 0, platinum, true)
        client:Message(15, string.format("You receive %d platinum.", platinum))
    end
end

function rift.send_to_hub(client)
    client:Message(15, "Stepping through the rift, you return to The Void...")
    client:MovePC(rift.HUB_ZONE_ID, rift.HUB_X, rift.HUB_Y, rift.HUB_Z, rift.HUB_H)
end

return rift
