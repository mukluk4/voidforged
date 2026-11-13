-- Ancient Cragbeast Matriarch, raid portal version (loaded by txevu/script_init_v50.lua)
-- A copy of acm.lua with three changes:
--   * a bearer's damage, resist and regeneration buffs are set against her raid stats, not as fixed numbers
--   * the hatchling and AE signals start the timers the matriarch actually runs
--     (acm.lua starts "hatchling" and "ae" but checks "hatchlings" and "aoe")
--   * the portal's wipe reset puts the bearers and handlers back
local portal = require("raid_portal")

local bearer_list = {297173, 297055, 297190, 297051, 297054, 297050, 297208, 297142, 297053, 297052, 297141, 297191}
local handler = 297048
local matriarch = 297056
local hatchling = 297210

local bearer_sw = {669.00, -115.00, -351.52, 0}
local bearer_s = {641.00, -115.00, -351.52, 0}
local bearer_se = {615.00, -115.00, -350.9, 0}
local bearer_nw = {669.00, 115.00, -353.9, 256}
local bearer_n = {641.00, 115.00, -353.9, 256}
local bearer_ne = {615.00, 115.00, -353.9, 256}
local handler_n = {641.00, 80.00, -364.9, 256}
local handler_s = {641.00, -80.00, -360.27, 0}

-- Skills the Bonecrusher bearer (297173) makes her shrug off
local MELEE_SKILLS = {0, 1, 2, 3, 7, 8, 10, 21, 23, 26, 28, 30, 36, 38, 52, 74}

local sw = 0
local s = 0
local se = 0
local nw = 0
local n = 0
local ne = 0

local adds = false
local combat = false
local ae = false

function chain_agro(e)
	local target = eq.get_entity_list():GetRandomClient(e.self:GetX(), e.self:GetY(), e.self:GetZ(), 200 * 200)
	e.self:AddToHateList(target, 1)
end

function pick_bearer(locs)
	--Pick a random bearer
	local idx = math.random(#bearer_list)
	local spawn_id = table.remove(bearer_list, idx)
	eq.spawn2(spawn_id, 0, 0, unpack(locs))
	return spawn_id
end

function reset()
	--Despawn everything
	eq.depop_all(handler)
	eq.depop_all(hatchling)

	bearer_list = {297173, 297055, 297190, 297051, 297054, 297050, 297208, 297142, 297053, 297052, 297141, 297191}
	for i = 1, #bearer_list do
		eq.depop_all(bearer_list[i])
		eq.signal(matriarch, bearer_list[i] + 1000000)
	end
end

function init_event()
	eq.spawn2(handler, 0, 0, unpack(handler_n)) --North Handler
	eq.spawn2(handler, 0, 0, unpack(handler_s)) --South Handler
	sw = pick_bearer(bearer_sw) --Southwest Bearer
	s = pick_bearer(bearer_s) --South Bearer
	se = pick_bearer(bearer_se) --Southeast Bearer
	nw = pick_bearer(bearer_nw) --Northwest Bearer
	n = pick_bearer(bearer_n) --North Bearer
	ne = pick_bearer(bearer_ne) --Northeast Bearer
end

function bearer_spawn(e)
	eq.signal(matriarch, e.self:GetNPCTypeID())
end

function bearer_death(e)
	if sw == e.self:GetNPCTypeID() then
		sw = 0
		eq.signal(matriarch, 1)
	elseif s == e.self:GetNPCTypeID() then
		s = 0
		eq.signal(matriarch, 2)
	elseif se == e.self:GetNPCTypeID() then
		se = 0
		eq.signal(matriarch, 3)
	elseif nw == e.self:GetNPCTypeID() then
		nw = 0
		eq.signal(matriarch, 4)
	elseif n == e.self:GetNPCTypeID() then
		n = 0
		eq.signal(matriarch, 5)
	elseif ne == e.self:GetNPCTypeID() then
		ne = 0
		eq.signal(matriarch, 6)
	end
	eq.signal(matriarch, e.self:GetNPCTypeID() + 1000000)
	table.insert(bearer_list, e.self:GetNPCTypeID())
end

function handler_combat(e)
	if e.joined then
		eq.set_timer('leash', 3 * 1000)
		eq.signal(handler, 1)
		eq.signal(matriarch, 10)
	else
		eq.stop_timer('leash')
	end
end

function handler_signal(e)
	if e.signal == 1 then
		chain_agro(e)
	end
end

function handler_death(e)
	eq.stop_timer('leash')
end

function handler_timer(e)
	if e.timer == 'leash' then
		if e.self:GetX() >= 700 then
			e.self:GotoBind()
			e.self:WipeHateList()
		end
	end
end

-- The bearers spawn when the zone loads, before the controller spawns her, so she picks up their
-- buffs once the global scaling has set her raid stats
function matriarch_spawn(e)
	eq.set_timer('bearers', 1000)
end

function matriarch_death(e)
	-- Despawn everything because the players won
	eq.stop_all_timers()
	reset()
end

function matriarch_combat(e)
	if e.joined then
		combat = true
		if adds then
			eq.set_timer('hatchlings', 35 * 1000)
		end
		if ae then
			eq.set_timer('aoe', math.random(5, 60) * 1000)
		end
		eq.stop_timer("reset")
		eq.signal(handler, 1)
	else
		combat = false
		eq.stop_timer("hatchlings")
		eq.stop_timer("aoe")
		eq.set_timer("reset", 2 * 60 * 1000)
	end
end

function matriarch_timer(e)
	if e.timer == "bearers" then
		eq.stop_timer('bearers')
		for _, bearer in ipairs({sw, s, se, nw, n, ne}) do
			if bearer ~= 0 and eq.get_entity_list():IsMobSpawnedByNpcTypeID(bearer) then
				eq.signal(matriarch, bearer)
			end
		end
	elseif e.timer == "hatchlings" then
		local x,y,z,h = e.self:GetX(), e.self:GetY(), e.self:GetZ(), e.self:GetHeading();
		eq.zone_emote(MT.Yellow, "In search of food, cragebeast hatchlings scurry out from beneath the Ancient Cragbeast Matriarch")
		eq.spawn2(hatchling, 0, 0, x, y + 5, z, h)
		eq.spawn2(hatchling, 0, 0, x + 5, y, z, h)
		eq.spawn2(hatchling, 0, 0, x + 5, y + 5, z, h)
	elseif e.timer == 'aoe' then
		eq.stop_timer('aoe')
		local target = e.self:GetTarget()
		if target.valid then
			e.self:CastSpell(1239, target:GetID()) --Devouring Conflagration
		end
	elseif e.timer == 'sw' then
		eq.stop_timer('sw')
		sw = pick_bearer(bearer_sw)
	elseif e.timer == 's' then
		eq.stop_timer('s')
		s = pick_bearer(bearer_s)
	elseif e.timer == 'se' then
		eq.stop_timer('se')
		se = pick_bearer(bearer_se)
	elseif e.timer == 'nw' then
		eq.stop_timer('nw')
		nw = pick_bearer(bearer_nw)
	elseif e.timer == 'n' then
		eq.stop_timer('n')
		n = pick_bearer(bearer_n)
	elseif e.timer == 'ne' then
		eq.stop_timer('ne')
		ne = pick_bearer(bearer_ne)
	elseif e.timer == 'reset' then
		eq.stop_timer('reset')
		e.self:CastSpell(3230, e.self:GetID())
		e.self:WipeHateList()
		reset()
		eq.set_timer('respawn', 2 * 1000)
	elseif e.timer == 'respawn' then
		eq.stop_timer('respawn')
		init_event()
	end
end

function matriarch_signal(e)
	local repop = 2 * 60 * 1000
	if e.signal == portal.RESET_SIGNAL then
		eq.stop_all_timers()
		reset()
		eq.set_timer('respawn', 2 * 1000)
	elseif e.signal == 1 then
		eq.set_timer('sw', repop)
	elseif e.signal == 2 then
		eq.set_timer('s', repop)
	elseif e.signal == 3 then
		eq.set_timer('se', repop)
	elseif e.signal == 4 then
		eq.set_timer('nw', repop)
	elseif e.signal == 5 then
		eq.set_timer('n', repop)
	elseif e.signal == 6 then
		eq.set_timer('ne', repop)
	elseif e.signal == 10 then
		chain_agro(e)
	elseif e.signal == 297173 then
		for _, skill in ipairs(MELEE_SKILLS) do e.self:ModSkillDmgTaken(skill, -75) end
	elseif e.signal == 1297173 then
		for _, skill in ipairs(MELEE_SKILLS) do e.self:ModSkillDmgTaken(skill, 0) end
	elseif e.signal == 297055 then
		e.self:SetSpecialAbility(SpecialAbility.rampage, 1)
		e.self:SetSpecialAbilityParam(SpecialAbility.rampage, 0, 27)
	elseif e.signal == 1297055 then
		e.self:SetSpecialAbility(SpecialAbility.rampage, 0)
	elseif e.signal == 297190 then
		e.self:SetSpecialAbility(SpecialAbility.flurry, 1)
		e.self:SetSpecialAbilityParam(SpecialAbility.flurry, 0, 50)
	elseif e.signal == 1297190 then
		e.self:SetSpecialAbility(SpecialAbility.flurry, 0)
	elseif e.signal == 297051 then
		-- open world 850-2750 raised to 1150-4000
		portal.set_damage(e.self, 1150 / 850, 4000 / 2750)
	elseif e.signal == 1297051 then
		portal.set_damage(e.self, 1, 1)
	elseif e.signal == 297054 then
		-- open world 6000 per tick on 2,105,145 HP (0.285%), kept as the same share of her raid HP
		e.self:ModifyNPCStat("combat_hp_regen", tostring(math.floor(e.self:GetMaxHP() * 6000 / 2105145)))
	elseif e.signal == 1297054 then
		e.self:ModifyNPCStat("combat_hp_regen", "100")
	elseif e.signal == 297050 then
		adds = true
		if combat then
			eq.set_timer("hatchlings", 35 * 1000)
		end
	elseif e.signal == 1297050 then
		adds = false
		eq.stop_timer("hatchlings")
	elseif e.signal == 297208 then
		ae = true
		if combat then
			eq.set_timer("aoe", math.random(5, 60) * 1000)
		end
	elseif e.signal == 1297208 then
		ae = false
		eq.stop_timer("aoe")
	elseif e.signal == 297142 then
		e.self:ModifyNPCStat("attack_delay", "12")
	elseif e.signal == 1297142 then
		e.self:ModifyNPCStat("attack_delay", "17")
	elseif e.signal == 297053 then
		e.self:SetSpecialAbility(SpecialAbility.area_rampage, 1)
		e.self:SetSpecialAbilityParam(SpecialAbility.area_rampage, 0, 17)
	elseif e.signal == 1297053 then
		e.self:SetSpecialAbility(SpecialAbility.area_rampage, 0)
	elseif e.signal == 297052 then
		e.self:AddAISpell(0, 1248, 1024, -1, 30, -1) -- Spiritual Echo, in combat buff
	elseif e.signal == 1297052 then
		e.self:RemoveAISpell(1248) -- Spiritual Echo
	elseif e.signal == 297141 then
		-- open world 136 raised to 236
		portal.set_resists(e.self, 100)
	elseif e.signal == 1297141 then
		portal.set_resists(e.self, 0)
	elseif e.signal == 297191 then
		e.self:AddAISpell(0, 1249, 1024, -1, 30, -1) -- Bristling Armament, in combat buff
	elseif e.signal == 1297191 then
		e.self:RemoveAISpell(1249) -- Bristling Armament
	end
end

function event_encounter_load(e)
	eq.register_npc_event("acm_portal", Event.spawn, matriarch, matriarch_spawn)
	eq.register_npc_event("acm_portal", Event.signal, matriarch, matriarch_signal)
	eq.register_npc_event("acm_portal", Event.timer, matriarch, matriarch_timer)
	eq.register_npc_event("acm_portal", Event.combat, matriarch, matriarch_combat)
	eq.register_npc_event("acm_portal", Event.death, matriarch, matriarch_death)

	eq.register_npc_event("acm_portal", Event.timer, handler, handler_timer)
	eq.register_npc_event("acm_portal", Event.death, handler, handler_death)
	eq.register_npc_event("acm_portal", Event.combat, handler, handler_combat)
	eq.register_npc_event("acm_portal", Event.signal, handler, handler_signal)

	for i = 1, #bearer_list do
		eq.register_npc_event("acm_portal", Event.death, bearer_list[i], bearer_death)
		eq.register_npc_event("acm_portal", Event.spawn, bearer_list[i], bearer_spawn)
	end
	reset()
	init_event()
end
