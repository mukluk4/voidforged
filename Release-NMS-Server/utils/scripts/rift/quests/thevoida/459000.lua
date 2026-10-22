-- ============================================================================
-- ZEBUXORUK - STORYTELLER AND GREATER RIFT KEEPER (thevoida, zone 459)
-- Deploy to: quests/thevoida/459000.lua   (npc_types.id 459000, #Zebuxoruk)
-- Rifts are free: hail, then click a tier. One open rift per character at a time.
-- ============================================================================

local rift = require("rift_core")
local RIFT_ZONE_POOL = require("rift_zones")

-- Active rift record: "instance_id,zone_id,x,y,z,h"
local function get_active_rift(char_id)
    local raw = eq.get_data(rift.key_active(char_id))
    if raw == "" then return nil end

    local instance_id, zone_id, x, y, z, h = raw:match("^(%d+),(%d+),([-%d.]+),([-%d.]+),([-%d.]+),([-%d.]+)$")
    if not instance_id then return nil end
    return {
        instance_id = tonumber(instance_id), zone_id = tonumber(zone_id),
        x = tonumber(x), y = tonumber(y), z = tonumber(z), h = tonumber(h),
    }
end

local function link(text)
    return "[" .. eq.say_link(text, true, text) .. "]"
end

-- Quick picks, deepest first: the next unlock, the last clear, then every 5th tier down to 1
local function tier_links(max_tier, deepest)
    local picks, seen = {}, {}
    local function add(tier)
        if tier >= 1 and not seen[tier] then
            seen[tier] = true
            table.insert(picks, tier)
        end
    end

    add(max_tier)
    add(max_tier - 1)
    local step = math.floor((max_tier - 1) / 5) * 5
    while #picks < 6 and step > 1 do
        add(step)
        step = step - 5
    end
    add(1)

    local links = {}
    for _, tier in ipairs(picks) do
        local label = "Tier " .. tier
        local text  = link(label)
        if tier == max_tier and tier > deepest and deepest > 0 then
            text = text .. " (new)"
        end
        table.insert(links, text)
    end
    return table.concat(links, " | ")
end

local function offer_rifts(npc, client)
    if get_active_rift(client:CharacterID()) then
        npc:Say("A Fracture already holds a piece of you. Say " .. link("re-enter") .. " and I will send you back to it, or " .. link("close") .. " to let it go.")
        return
    end

    local deepest  = rift.get_deepest(client:CharacterID())
    local max_tier = rift.max_tier(client:CharacterID())

    npc:Say("Choose how deep you would tear the Fabric, and I will open the way.")
    if deepest > 0 then
        client:Message(15, string.format("Your deepest Fracture: Tier %d", deepest))
    end
    client:Message(15, "Select a Tier: " .. tier_links(max_tier, deepest))
    if max_tier > 1 then
        client:Message(15, string.format("Or say \"tier #\" for any tier up to %d.", max_tier))
    end
end

function event_say(e)
    local client = e.other
    local message = e.message

    if message:findi("hail") then
        e.self:Say(string.format(
            "Ah, %s. Another mind drawn to the edge of everything. I am Zebuxoruk, and the Chronal Fabric is unraveling because of %s.",
            client:GetName(), link("me")
        ))
        e.self:Say("Ask, and I will tell you of " .. link("the Fabric") .. ", or step into the " .. link("rifts") .. " and mend it with your own hands.")

    elseif message:findi("^me$") or message:findi("who are you") then
        e.self:Say("For the crime of sharing what I knew, the Pantheon bound me in a prison woven through time itself, so that I could never escape it, not in any age.")
        e.self:Say("Mortals broke those bindings. I am grateful. But a cage woven through time does not simply open; it tears. And every thread that tore has been pulling at " .. link("the Fabric") .. " ever since.")

    elseif message:findi("fabric") then
        e.self:Say("The Fabric is the order of moments: this before that, cause before consequence. Where my prison was anchored, it is fraying into Fractures.")
        e.self:Say("Each Fracture is a place that remembers itself wrongly: a past that never ended, a future that came too soon. Things live there that should not, and they grow stronger the deeper the tear.")
        e.self:Say("I cannot mend them myself; the gods saw to that. But you can walk where I cannot. Will you enter the " .. link("rifts") .. "?")

    elseif message:findi("rift") then
        e.self:Say("Seal a Fracture before it closes on you, and the Fabric pays you in Chronal Shards. My Quartermaster trades them for gear shaped by the rifts themselves.")
        offer_rifts(e.self, client)

    elseif message:findi("^close$") or message:findi("close it") then
        -- Abandons this character's open rift (e.g. one that never closed); no reward, others unaffected
        local active = get_active_rift(client:CharacterID())
        if active then
            eq.delete_data(rift.key_active(client:CharacterID()))
            eq.remove_from_instance_by_char_id(active.instance_id, client:CharacterID())
            e.self:Say("Then I will let that thread fall. The Fracture forgets you.")
            offer_rifts(e.self, client)
        else
            e.self:Say("No Fracture holds you right now.")
        end

    elseif message:findi("re%-enter") or message:findi("portal") then
        local active = get_active_rift(client:CharacterID())
        if active then
            client:Message(15, "Re-entering your active Greater Rift instance...")
            client:MovePCInstance(active.zone_id, active.instance_id, active.x, active.y, active.z, active.h)
        else
            e.self:Say("No Fracture holds you right now.")
        end

    elseif message:findi("tier %d+") then
        local tier = tonumber(message:match("%d+"))
        if not rift.is_allowed_tier(client:CharacterID(), tier) then
            e.self:Say(string.format(
                "You are not ready for that depth. Seal Tier %d first; your deepest is Tier %d.",
                rift.max_tier(client:CharacterID()), rift.get_deepest(client:CharacterID())
            ))
            return
        end

        if get_active_rift(client:CharacterID()) then
            e.self:Say("A Fracture already holds a piece of you. Say " .. link("re-enter") .. " and I will send you back to it, or " .. link("close") .. " to let it go.")
            return
        end

        OpenRiftInstance(client, tier, e.self)
    end
end

function event_trade(e)
    local item_lib = require("items")
    item_lib.return_items(e.self, e.other, e.trade)
end

-- Runs inside event_say, so the quest initiator is `client`
function OpenRiftInstance(client, tier, npc)
    -- 1. Pick a random map from the master zone table
    local zone = RIFT_ZONE_POOL[math.random(1, #RIFT_ZONE_POOL)]

    -- 2. Create the instance; it outlives the rift timer so players can walk out after a fail
    local duration = rift.TIME_LIMIT + rift.INSTANCE_GRACE
    local instance_id = eq.create_instance(zone.short_name, rift.INSTANCE_VERSION, duration)
    if instance_id <= 0 then
        client:Message(13, "The energies of the Fabric fail to stabilize the instance. Please try again.")
        return
    end

    -- 3. Tier and start time, read by the zone controller when the instance boots
    local expires = tostring(duration)
    eq.set_data(rift.key_tier(instance_id), tostring(tier), expires)
    eq.set_data(rift.key_start(instance_id), tostring(os.time()), expires)

    -- 4. Who goes: the hailer and every group member standing here in The Void. Members anywhere
    --    else (including characters without Void access) take no part and aren't assigned.
    local party = { client }
    local group = client:GetGroup()
    if client:IsGrouped() and group.valid then
        for i = 0, 5 do
            local member = group:GetMember(i)
            if member.valid and member:IsClient() and member:GetID() ~= client:GetID() then
                table.insert(party, member:CastToClient())
            end
        end
    end

    for _, member in ipairs(party) do
        eq.assign_to_instance_by_char_id(instance_id, member:CharacterID())
    end

    npc:Say(string.format("The Fabric parts. Tier %d: %s. Go, and do not linger once it begins to close.", tier, zone.name))

    -- 5. Move them in
    local active = string.format("%d,%d,%s,%s,%s,%s", instance_id, zone.zone_id, zone.x, zone.y, zone.z, zone.h)
    local function send(member)
        eq.set_data(rift.key_active(member:CharacterID()), active, expires)
        member:MovePCInstance(zone.zone_id, instance_id, zone.x, zone.y, zone.z, zone.h)
    end

    for _, member in ipairs(party) do
        send(member)
    end
end
