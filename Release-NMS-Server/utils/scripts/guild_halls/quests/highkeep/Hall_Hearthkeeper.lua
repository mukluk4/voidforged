-- Hall Hearthkeeper (guild hall in Highkeep, version 60): a regen pool, like the guild hall's Aporia.
-- Deploy to: quests/highkeep/Hall_Hearthkeeper.lua   (npc_types.id 1200122). Standing near it restores health
-- and mana quickly; leaving ends it.

local RANGE = 30

function event_spawn(e)
    local x, y, z = e.self:GetX(), e.self:GetY(), e.self:GetZ()
    eq.set_proximity(x - RANGE, x + RANGE, y - RANGE, y + RANGE, z - RANGE, z + RANGE)
end

function event_enter(e)
    e.other:EnableAreaRegens(2000)
    e.other:SendColoredText(0, "The hearth's warmth restores you.")
    e.other:AddNimbusEffect(412)
end

function event_exit(e)
    e.other:DisableAreaRegens()
    e.other:SendColoredText(0, "The warmth fades as you leave the hearth.")
    e.other:RemoveNimbusEffect(412)
end

function event_trade(e)
    local item_lib = require("items")
    item_lib.return_items(e.self, e.other, e.trade)
end
