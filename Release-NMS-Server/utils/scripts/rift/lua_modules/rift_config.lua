-- ============================================================================
-- GREATER RIFT SETTINGS (rift_config.lua)
-- Deploy to: lua_modules/rift_config.lua
--
-- Every server-specific value the rift scripts use, in one place. rift_core.lua, champion_affixes.lua
-- and Rift_Guardian.lua read this; edit here, then #reloadquest (open rift instances keep what they
-- loaded when they booted). Values below are this server's (NMS).
-- ============================================================================

return {
    -- Where players return when a rift ends (the rift portal), and where the rift NPCs stand
    hub = { zone_id = 459, x = 25.33, y = 78.03, z = 1.21, h = 241.75 },  -- The Void (thevoida), same spot as its map waypoint

    -- spawn2/doors/zone version that holds the rift copy of each zone (gen_rift_zones.py)
    instance_version = 1,

    time_limit     = 900,     -- seconds to kill the Guardian
    instance_grace = 180,     -- extra instance life so players can walk out after a fail
    max_tier       = 500,     -- hard cap; below it each player's own progression is the ceiling

    -- Character data bucket holding the deepest clear; tier gear vendor items are hidden until it reaches
    -- their unlock tier (merchantlist bucket_name / bucket_value, comparison 2 = >=)
    gear_unlock_bucket = "rift_deepest",

    -- Progress to a full rift (10000 = 100.00%) and what each kill is worth
    target_progress = 10000,
    progress        = { trash = 100, elite = 400 },                -- 1% / 4%
    progress_step   = 5,                                            -- announce every 5%
    pylon_at        = { 25, 50, 75 },                               -- % progress that spawns a pylon
    time_warnings   = { 720, 540, 360, 180, 60 },                   -- seconds left that get a warning

    -- Clear reward, paid in alternate currency: base + per_tier * tier, +speed_bonus% under speed_seconds
    reward = {
        currency_id   = 2,
        currency_name = "Chronal Shards",
        base          = 5,
        per_tier      = 2,
        speed_seconds = 600,
        speed_bonus   = 25,
        platinum_per_tier = 1000,  -- also paid on a clear: tier x this, to each player (no speed bonus)
    },

    -- Rift NPC types (npc_types ids)
    npc = {
        controller = 1200000,     -- invisible, untargetable zone controller
        guardian   = 1200001,
        pylon      = 1200005,
        portal     = 1200009,
        vendor     = 1200010,     -- token vendor in the hub
        keeper     = 459000,      -- who opens rifts (here: Zebuxoruk; his script is quests/thevoida/459000.lua)
        melee      = 1200020,
        caster     = 1200021,
        disrupt    = 1200022,
        elite      = 1200023,
    },

    -- Spell penetration: every rift NPC's spells are this much harder to resist (NPC special ability 43,
    -- Casting Resist Difficulty). The resist roll is 0-200 against resist + modifiers, so without this,
    -- players past ~200 resist shrug off everything. -(base + per_tier * tier), never past max.
    spell_penetration = { base = 200, per_tier = 2, max = 400 },

    -- Melee penetration for every rift NPC, also from tier 1 (base + per_tier * tier, never past max):
    --   heroic_strikethrough: each point cancels 25 of the player's heroic DEX (riposte/parry/block) and
    --                         heroic AGI (dodge) from gear, i.e. 1% off each of those avoid rolls
    --   counter_avoid:        % cut to the player's whole riposte/parry/block/dodge chance (special ability 44)
    heroic_strikethrough = { base = 0, per_tier = 0.05, max = 25 },  -- 0 until tier 20, then +1 every 20 tiers
    counter_avoid        = { base = 0, per_tier = 0.1,  max = 20 },  -- +1% every 10 tiers, up to 20% at tier 200

    -- The mobs' own resists: a flat amount added per tier (AC still grows 3% per tier). Multiplying them
    -- made players' spells unlandable past ~tier 50; this adds +25 by tier 100, +100 at most (tier 400)
    resist_scaling       = { base = 0, per_tier = 0.25, max = 100 },

    -- Pylons (one picked at random per spawn). Existing spells only, so no client spell file change.
    --   nuke:   point-blank AoE cast by the activator, centred on them each pulse
    --   splash: applied to every group member each pulse (heal + cure)
    --   buff:   each group member casts it on themselves
    -- pulses/interval_ms: how many times it fires and how far apart; stops if the activator dies or leaves
    pylons = {
        { name = "Conduit Pylon",   kind = "nuke",   spell_id = 35779, pulses = 6, interval_ms = 3000,   -- Circle of Flame Rk. III: 5493 fire, radius 35
          emote = "surges with chronal fire, scorching every foe around them!" },
        { name = "Cleansing Pylon", kind = "splash", spell_id = 34412, pulses = 5, interval_ms = 6000,   -- Splash of Cleansing Rk. III: 2276 heal, cures poison/disease/curse
          emote = "washes over the group, mending wounds and purging afflictions!" },
        { name = "Shielding Pylon", kind = "buff",   spell_id = 38306,                                   -- Elemental Shield: absorbs 100% up to 40000, +2000 HP
          emote = "wraps the group in a shimmering elemental ward!" },
        { name = "Speed Pylon",     kind = "buff",   spell_id = 39243,                                   -- Expedited Delivery: +125 overhaste, +300 run, 25% faster casts
          emote = "accelerates time around the group to blinding speeds!" },
    },

    -- Champion affixes on rift trash
    champion = {
        chance   = 15,            -- percent of rift trash that becomes a champion
        pulse_ms = 4000,
        spells   = {
            molten_pulse = 1659,  -- Scintillation: fire PBAoE, 342 dmg
            molten_death = 733,   -- Supernova: fire PBAoE, 854 dmg
            arcane_bolts = 1653,  -- Jyll's Static Pulse: magic PBAoE, 495 dmg
        },
        vortex   = {
            every_pulses  = 3,    -- pulls once every 3rd pulse (~12s)
            min_distance  = 30,   -- only players farther than this get pulled, never the one in melee
            land_distance = 10,   -- pulled players land this far in front of the champion
        },
    },

    -- Rift Guardian (boss)
    guardian = {
        enrage_ms       = 120000,
        pulse_ms        = 8000,
        hate_wipe_every = 3,      -- the phase-shift mechanic wipes hate every Nth pulse
        spells          = {
            enrage      = 3761,   -- Blind Rage: self, +65% haste, +250 ATK, -600 AC
            energy_wave = 3473,   -- Catastrophe: magic PBAoE, 850 dmg
        },
    },
}
