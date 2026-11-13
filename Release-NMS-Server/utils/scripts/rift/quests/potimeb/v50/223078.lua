-- ============================================================================
-- VALLON ZEK, RAID PORTAL VERSION
-- Deploy to: quests/potimeb/v50/223078.lua
-- Keeps his fight: two copies of him at 51% (#Vallon_Zek, whose open-world script tethers them), and
-- a tether to his arena that sends him home, wipes his hate and casts Balance of the Nameless.
-- At 50% Tallon Zek joins at his open-world spot (223077.lua copies Vallon's raid HP and damage).
-- The raid is paid once both brothers are dead (plugins/raid_portal.pl, RAID_PARTNERS). No zone status signals.
-- ============================================================================

local portal = require("raid_portal")
local box = require("aa_box")

local COPY = 223164
local TALLON = 223077
local tether_box = box(229.2, -229.2, 246, 446.2)

function event_spawn(e)
    eq.set_next_hp_event(51)
    eq.set_timer("tether", 6 * 1000)
end

function event_timer(e)
    if e.timer == "tether" and not tether_box:contains(e.self:GetX(), e.self:GetY()) then
        e.self:GotoBind()
        e.self:WipeHateList()
        e.self:CastSpell(3230, e.self:GetID()) -- Balance of the Nameless
    end
end

function event_hp(e)
    if e.hp_event == 51 then
        eq.spawn2(COPY, 0, 0, 360, 75, 358, 384)
        eq.spawn2(COPY, 0, 0, 300, 75, 348, 384)
        eq.set_next_hp_event(50)
    elseif e.hp_event == 50 and not eq.get_entity_list():IsMobSpawnedByNpcTypeID(TALLON) then
        local top = e.self:GetHateTop()
        portal.spawn_add(TALLON, 405, -84, 358, 384, top.valid and top or nil)
        e.self:Shout("Brother! To arms!")
    end
end

function event_signal(e)
    if e.signal == portal.RESET_SIGNAL then
        eq.depop_all(COPY)
        eq.depop_all(TALLON)
        eq.set_next_hp_event(51)
    end
end
