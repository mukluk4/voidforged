-- Glamour looks: armor glamours that change the texture (a robe, a material) show to everyone, on every client.
--
-- EQEmu sends other players an armor slot's look from the base item: its Material, and the glamour augment's
-- Hero's Forge model. A glamour that sets only a Material (the robes: material 11, herosforgemodel 0) is shown only
-- if the wearer's own client sends an OP_WearChange with it, which the server relays and forgets. Players who zone
-- in later see the base item, and a client that never sends one (Hailstone) never shows the glamour to anyone.
--
-- This sends the look from the server instead: at zone-in, and whenever gear or an augment changes, each armor
-- slot whose item holds a glamour with a Material gets Mob:WearChange(slot, material, color, 0). WearChange also
-- keeps it as the slot's texture profile, so the spawn record players get when they zone in later has it too, and
-- the wearer's own client is sent it as well. When the glamour or the item goes, the slot is set back to its item.
--
-- Install: quests/global/encounters/glamour_looks.lua, and in quests/global/script_init.lua:
--   eq.load_encounter("glamour_looks")
-- then #reloadquest (or restart the zones).

local ENCOUNTER = "glamour_looks"
local DEBUG = false  -- true: tells the wearer what is sent (used 2026-11-13 to debug robe glamours)
local VAR = "glamour_look_"  -- entity variable per material slot: the material this script set ("" or none: none)

-- Armor material slot -> inventory slot (EQ::InventoryProfile::CalcSlotFromMaterial)
local SLOTS = {
	{ material = 0, inv = 2 },   -- head
	{ material = 1, inv = 17 },  -- chest
	{ material = 2, inv = 7 },   -- arms
	{ material = 3, inv = 9 },   -- wrist (the first wrist slot, as the server uses)
	{ material = 4, inv = 12 },  -- hands
	{ material = 5, inv = 18 },  -- legs
	{ material = 6, inv = 19 },  -- feet
}

-- Augment types 20 and 21 (ornamentation, armor glamour) are bits 19 and 20 of an augment's AugType.
local function is_glamour_type(aug_type)
	return math.floor(aug_type / 524288) % 4 ~= 0
end

local function valid(o)
	return o ~= nil and o.valid
end

-- The glamour Material in an item's sockets and the glamour's own colour, or 0, 0.
local function glamour_material(inst)
	for socket = 0, 5 do
		local aug = inst:GetAugment(socket)
		if valid(aug) then
			local item = aug:GetItem()
			if valid(item) and is_glamour_type(item:AugType()) and item:Material() > 0 then
				return item:Material(), item:Color()
			end
		end
	end
	return 0, 0
end

-- The item's colour: its dye, otherwise the item's own colour.
local function item_color(inst)
	local color = inst:GetColor()
	if color == 0 then
		local item = inst:GetItem()
		if valid(item) then color = item:Color() end
	end
	return color
end

-- The colour to show a glamour in: a dye the player put on the item wins; otherwise the glamour's own colour
-- (a black robe stays black on a blue chestguard), falling back to the item's colour.
local function glamour_color(inst, glamour_color_value)
	local item = inst:GetItem()
	local base = valid(item) and item:Color() or 0
	local worn = inst:GetColor()
	if worn ~= 0 and worn ~= base then return worn end
	if glamour_color_value ~= 0 then return glamour_color_value end
	return item_color(inst)
end

local function refresh(client, force)
	if not valid(client) then return end
	local inv = client:GetInventory()
	for _, s in ipairs(SLOTS) do
		local inst = inv:GetItem(s.inv)
		local material, glamour_tint = 0, 0
		if valid(inst) then material, glamour_tint = glamour_material(inst) end
		local key = VAR .. s.material
		local was = client:GetEntityVariable(key)
		if material > 0 then
			if force or was ~= tostring(material) then
				local tint = glamour_color(inst, glamour_tint)
				client:WearChange(s.material, material, tint, 0)
				if DEBUG then
					client:Message(15, string.format("[glamour debug] slot %d: material %d, color %08X%s", s.material,
						material, tint, force and " (recheck)" or ""))
				end
				client:SetEntityVariable(key, tostring(material))
			end
		elseif was ~= nil and was ~= "" then
			-- The glamour (or the item) is gone: the slot shows its item again.
			if valid(inst) then
				local item = inst:GetItem()
				local base = valid(item) and item:Material() or 0
				client:WearChange(s.material, base, item_color(inst), client:GetHerosForgeModel(s.material))
			else
				client:WearChange(s.material, 0, 0, 0)
			end
			client:SetEntityVariable(key, "")
		end
	end
end

-- Each change is looked at at once and again RECHECK_MS later, should the server send the event before the move
-- has finished.
local RECHECK = "glamour_recheck"
local RECHECK_MS = 500
local encounter
local pending = {}  -- client entity ids to look at again

local function on_event(e)
	if DEBUG and valid(e.self) then
		e.self:Message(15, "[glamour debug] checking your armor")
	end
	refresh(e.self)
	if valid(e.self) and encounter then
		pending[e.self:GetID()] = true
		eq.set_timer(RECHECK, RECHECK_MS, encounter)
	end
end

function event_timer(e)
	if e.timer ~= RECHECK then return end
	eq.stop_timer(RECHECK, encounter)
	local list = eq.get_entity_list()
	for id in pairs(pending) do
		refresh(list:GetClientByID(id), true)
	end
	pending = {}
end

local EVENTS = { "enter_zone", "equip_item_client", "unequip_item_client", "augment_insert_client", "augment_remove_client" }

function event_encounter_load(e)
	encounter = e.encounter
	for _, name in ipairs(EVENTS) do
		eq.register_player_event(ENCOUNTER, Event[name], on_event)
	end
end

function event_encounter_unload(e)
	for _, name in ipairs(EVENTS) do
		eq.unregister_player_event(ENCOUNTER, Event[name])
	end
end
