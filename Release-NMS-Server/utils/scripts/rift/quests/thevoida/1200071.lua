-- ============================================================================
-- RAID PORTAL NPC (thevoida)
-- Deploy to: quests/thevoida/1200071.lua   (npc_types.id 1200071, Raid_Portal)
-- GM only while the portal is being tested. "test <boss> <size>" skips the raid requirements.
-- ============================================================================

local portal = require("raid_portal")
local rift = require("rift_core")

local function link(text)
    return "[" .. eq.say_link(text, true, text) .. "]"
end

-- Active record: "instance_id,zone_id,x,y,z,h"
local function get_active(char_id)
    local raw = eq.get_data(portal.key_active(char_id))
    local instance_id, zone_id, x, y, z, h = raw:match("^(%d+),(%d+),([-%d.]+),([-%d.]+),([-%d.]+),([-%d.]+)$")
    if not instance_id then return nil end
    return {
        instance_id = tonumber(instance_id), zone_id = tonumber(zone_id),
        x = tonumber(x), y = tonumber(y), z = tonumber(z), h = tonumber(h),
    }
end

-- The hailer and every raid or group member standing here in The Void
local function gather_party(client)
    local party, seen = { client }, { [client:CharacterID()] = true }
    local function add(member)
        if member.valid and not seen[member:CharacterID()] then
            seen[member:CharacterID()] = true
            table.insert(party, member)
        end
    end

    if client:IsRaidGrouped() then
        local raid = client:GetRaid()
        for i = 0, portal.MAX_RAID - 1 do
            add(raid:GetMember(i))
        end
    elseif client:IsGrouped() then
        local group = client:GetGroup()
        for i = 0, 5 do
            local member = group:GetMember(i)
            if member.valid and member:IsClient() then
                add(member:CastToClient())
            end
        end
    end
    return party
end

local function open_instance(npc, boss, size, party)
    local instance_id = eq.create_instance(boss.zone, portal.INSTANCE_VERSION, portal.INSTANCE_SECONDS)
    if instance_id <= 0 then
        npc:Say("The way will not hold. Try again.")
        return
    end

    local expires = tostring(portal.INSTANCE_SECONDS)
    eq.set_data(portal.key_instance(instance_id),
        string.format("%d,%d,%d,%d", boss.tier, size, boss.npc_id, boss.spellscale), expires)

    local e = boss.entry
    local active = string.format("%d,%d,%s,%s,%s,%s", instance_id, boss.zone_id, e[1], e[2], e[3], e[4])
    npc:Say(string.format("Raid tier %d: %s, sized for %d. The way is open for two hours.", boss.tier, boss.name, size))
    for _, member in ipairs(party) do
        eq.assign_to_instance_by_char_id(instance_id, member:CharacterID())
        eq.set_data(portal.key_active(member:CharacterID()), active, expires)
        member:MovePCInstance(boss.zone_id, instance_id, e[1], e[2], e[3], e[4])
    end
end

-- The real path: the raid leader opens a tier and the boss is picked at random
local function open_tier(npc, client, tier)
    if not client:IsRaidGrouped() then
        npc:Say("Only a raid may pass.")
        return
    end
    if not client:GetRaid():IsLeader(client) then
        npc:Say("Only the raid's leader may open the way.")
        return
    end
    if rift.get_deepest(client:CharacterID()) < tier * 100 then
        npc:Say(string.format("Raid tier %d needs a deepest Greater Rift clear of %d.", tier, tier * 100))
        return
    end

    local party = gather_party(client)
    if #party < portal.MIN_RAID or #party > portal.MAX_RAID then
        npc:Say(string.format("A raid here must number %d to %d. You have %d in The Void.", portal.MIN_RAID, portal.MAX_RAID, #party))
        return
    end

    local pool = portal.bosses_in_tier(tier)
    if #pool == 0 then
        npc:Say(string.format("Raid tier %d has no bosses yet.", tier))
        return
    end
    open_instance(npc, pool[math.random(1, #pool)], #party, party)
end

function event_say(e)
    local client = e.other
    if not client:GetGM() then return end

    -- Typed lines can carry stray spaces that a say link doesn't
    local message = e.message:lower():gsub("[%s%c]+", " "):match("^ *(.-) *$")

    if message:findi("hail") then
        e.self:Say(string.format("Open raid tier %s %s %s %s %s, or test a boss at a raid size: %s",
            link("tier 1"), link("tier 2"), link("tier 3"), link("tier 4"), link("tier 5"), link("bosses")))
        if get_active(client:CharacterID()) then
            e.self:Say("You also have a fight open: " .. link("re-enter") .. " or " .. link("close") .. ".")
        end

    elseif message == "bosses" then
        -- One line per tier, each boss a link to test it at size 6
        for tier = 1, 5 do
            local keys = {}
            for key, boss in pairs(portal.BOSSES) do
                if boss.tier == tier then table.insert(keys, key) end
            end
            if #keys > 0 then
                table.sort(keys)
                local links = {}
                for _, key in ipairs(keys) do
                    table.insert(links, "[" .. eq.say_link("test " .. key .. " 6", true, portal.BOSSES[key].name) .. "]")
                end
                -- Four links a line keeps each message well under the client's length limit
                for i = 1, #links, 4 do
                    e.self:Say(string.format("Tier %d: %s", tier, table.concat(links, " ", i, math.min(i + 3, #links))))
                end
            end
        end

    elseif message:findi("^re%-enter$") then
        local active = get_active(client:CharacterID())
        if active then
            client:MovePCInstance(active.zone_id, active.instance_id, active.x, active.y, active.z, active.h)
        else
            e.self:Say("No fight holds you right now.")
        end

    elseif message:findi("^close$") then
        local active = get_active(client:CharacterID())
        if active then
            eq.delete_data(portal.key_active(client:CharacterID()))
            eq.remove_from_instance_by_char_id(active.instance_id, client:CharacterID())
            rift.end_instance_if_unheld(active.instance_id)
            e.self:Say("That way is closed to you.")
        end

    elseif get_active(client:CharacterID()) and (message:findi("^tier %d+$") or message:findi("^test ")) then
        e.self:Say("You already have a fight open: " .. link("re-enter") .. " or " .. link("close") .. ".")

    elseif message:findi("^tier %d+$") then
        open_tier(e.self, client, tonumber(message:match("%d+")))

    elseif message:findi("^test ") then
        local key, size = message:match("^test (%a+) (%d+)$")
        local boss = key and portal.BOSSES[key]
        size = tonumber(size)
        if not boss or not size or size < 1 or size > portal.MAX_RAID then
            e.self:Say(string.format("Say test <boss> <raid size, 1 to 72>; %s lists them. I heard: %q", link("bosses"), e.message))
            return
        end
        open_instance(e.self, boss, size, gather_party(client))
    end
end

function event_trade(e)
    local item_lib = require("items")
    item_lib.return_items(e.self, e.other, e.trade)
end
