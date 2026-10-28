-- ============================================================================
-- CHRONAL ARTIFICER - REFORGING (thevoida)
-- Deploy to: quests/thevoida/1200050.lua   (npc_types.id 1200050)
-- The reforging itself is the #reforge command (zone/gm_commands/reforge.cpp); it
-- only lets players pay and choose while they target this NPC up close.
-- ============================================================================

function event_say(e)
    if e.message:findi("hail") then
        e.self:Say(string.format(
            "Every Fracture leaves its threads loose. Pay me in platinum and shards and I will pull one thread of a rolled item and weave it anew. " ..
            "You choose between two new weavings or keep the old. [%s]",
            eq.say_link("#reforge", true, "Show me your items")))
        e.other:Message(15, "Reforging: pick one random (*) line of a dynamic item. Once you reforge a line, it's the only one on that item " ..
            "that can be reforged again, and each reforge of the same item costs more. Higher tier items cost more.")
    end
end
