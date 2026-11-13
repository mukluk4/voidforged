-- ============================================================================
-- TALLON ZEK, RAID PORTAL VERSION
-- Deploy to: quests/potimeb/v50/223077.lua
-- Not a portal boss of his own: Vallon Zek (223078.lua) calls him in at 50%. The global scaling sets
-- him as an add, so he copies Vallon's raid HP and damage. The raid is paid once both are dead
-- (plugins/raid_portal.pl, RAID_PARTNERS). No zone status signals, no loot lockout.
-- ============================================================================

local VALLON = 223078

function event_spawn(e)
    -- The global scaling runs after event_spawn, so the stats are copied from a short timer
    eq.set_timer("raid_init", 500)
end

function event_timer(e)
    if e.timer == "raid_init" then
        eq.stop_timer(e.timer)
        local vallon = eq.get_entity_list():GetNPCByNPCTypeID(VALLON)
        if vallon.valid then
            e.self:ModifyNPCStat("max_hp", tostring(vallon:GetMaxHP()))
            e.self:ModifyNPCStat("min_hit", tostring(vallon:GetMinDMG()))
            e.self:ModifyNPCStat("max_hit", tostring(vallon:GetMaxDMG()))
        end
        e.self:Heal()
    end
end
