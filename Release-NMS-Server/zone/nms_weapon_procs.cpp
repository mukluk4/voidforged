#include "nms_weapon_procs.h"
#include "nms_weapon_procs_ids.h"

#include "../common/spdat.h"
#include "../common/strings.h"
#include "../common/timer.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "npc.h"
#include "zonedb.h"

#include <cmath>
#include <cstdlib>
#include <unordered_map>

extern EntityList entity_list;

namespace {
	std::unordered_map<uint32, uint16> g_item_sympathetics;

	// Shatterpoint bursts: target entity id -> when it may burst again
	std::unordered_map<uint16, uint32> g_mark_ready;
	constexpr uint32 MARK_COOLDOWN_MS = 3000;

	constexpr float PULL_RADIUS    = 30.0f;
	constexpr float PULL_FRACTION  = 0.5f;
	constexpr float TRANCE_RADIUS  = 40.0f;
	constexpr int   TRANCE_TARGETS = 2;
	constexpr float SPREAD_RADIUS  = 40.0f;

	struct ProcSpell {
		int n    = 0; // proc number in WEAPON_PROCS.md
		int role = 0; // 0 = the proc itself, 1+ = the spells it fires
		int tier = 0; // 1-5
	};

	bool Decode(uint16 spell_id, ProcSpell &out)
	{
		if (spell_id < NMS_WEAPON_PROC_SPELL_FIRST || spell_id > NMS_WEAPON_PROC_SPELL_LAST) {
			return false;
		}

		const int off = spell_id - NMS_WEAPON_PROC_SPELL_FIRST;
		out.n    = off / 15 + 1;
		out.role = (off % 15) / 5;
		out.tier = off % 5 + 1;
		return true;
	}

	// The tier of the buff from proc n, role 0, on the mob, and the buff's caster; 0 if it has none
	int FindProcBuff(Mob *m, int n, uint16 *caster_id = nullptr)
	{
		auto buffs = m->GetBuffs();
		if (!buffs) {
			return 0;
		}

		for (int i = 0; i < m->GetMaxTotalSlots(); ++i) {
			ProcSpell p;
			if (Decode(buffs[i].spellid, p) && p.n == n && p.role == 0) {
				if (caster_id) {
					*caster_id = buffs[i].casterid;
				}
				return p.tier;
			}
		}

		return 0;
	}

	// An NPC near the target that the caster may hit and that is already fighting, so these never pull new mobs
	bool IsNearbyFoe(Mob *caster, Mob *target, NPC *npc, float radius)
	{
		return npc && npc != target && !npc->IsCorpse() && npc->GetHP() > 0 && npc->IsEngaged() &&
			!npc->IsRaidTarget() && caster->IsAttackAllowed(npc, true) &&
			DistanceSquared(npc->GetPosition(), target->GetPosition()) <= radius * radius &&
			npc->CheckLosFN(target);
	}

	void Execute(Mob *caster, Mob *target, uint16 spell_id, float hp_before)
	{
		if (hp_before >= 25.0f || target->IsCorpse() || target->GetHP() <= 0) {
			return;
		}

		// The second hit: the proc's damage again, unmodified
		const int64 damage = std::abs(static_cast<int64>(spells[spell_id].base_value[0]));
		if (damage > 0) {
			target->Damage(caster, damage, spell_id, EQ::skills::SkillEvocation, false);
		}
	}

	void Mark(Mob *caster, Mob *target, uint16 spell_id)
	{
		if (!target->IsNPC() || !caster->GetOwnerOrSelf()->IsClient() || !IsDetrimentalSpell(spell_id)) {
			return;
		}

		const int tier = FindProcBuff(target, NMS_WEAPON_PROC_MARK);
		if (!tier) {
			return;
		}

		const uint32 now = Timer::GetCurrentTime();
		auto it = g_mark_ready.find(target->GetID());
		if (it != g_mark_ready.end() && now < it->second) {
			return;
		}

		g_mark_ready[target->GetID()] = now + MARK_COOLDOWN_MS;
		const uint16 burst = NmsWeaponProcSpell(NMS_WEAPON_PROC_MARK, 1, tier);
		caster->SpellFinished(burst, target, EQ::spells::CastingSlot::Item, 0, 0xFFFFFFFF, spells[burst].resist_difficulty, true);
	}

	void Pull(Mob *caster, Mob *target)
	{
		const auto &dest = target->GetPosition();
		for (const auto &[id, npc] : entity_list.GetNPCList()) {
			if (!IsNearbyFoe(caster, target, npc, PULL_RADIUS) || npc->IsRooted()) {
				continue;
			}

			const auto &from = npc->GetPosition();
			glm::vec3 to(
				from.x + (dest.x - from.x) * PULL_FRACTION,
				from.y + (dest.y - from.y) * PULL_FRACTION,
				from.z + (dest.z - from.z) * PULL_FRACTION
			);
			to.z = npc->GetFixedZ(to);
			npc->GMMove(to.x, to.y, to.z, from.w, false);
		}
	}

	void Trance(Mob *caster, Mob *target, int tier)
	{
		const uint16 mez = NmsWeaponProcSpell(NMS_WEAPON_PROC_TRANCE, 1, tier);
		int count = 0;
		for (const auto &[id, npc] : entity_list.GetNPCList()) {
			if (count >= TRANCE_TARGETS) {
				break;
			}

			if (!IsNearbyFoe(caster, target, npc, TRANCE_RADIUS) || npc->IsMezzed()) {
				continue;
			}

			caster->SpellFinished(mez, npc, EQ::spells::CastingSlot::Item, 0, 0xFFFFFFFF, spells[mez].resist_difficulty, true);
			++count;
		}
	}
}

void NmsLoadItemSympathetics()
{
	g_item_sympathetics.clear();

	auto results = database.QueryDatabase("SELECT item_id, spell_id FROM item_sympathetics");
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		const uint32 item_id = Strings::ToUnsignedInt(row[0]);
		const uint16 spell = static_cast<uint16>(Strings::ToUnsignedInt(row[1]));
		if (!IsValidSpell(spell)) {
			LogError("item_sympathetics item [{}] has invalid spell [{}], skipping", item_id, spell);
			continue;
		}

		g_item_sympathetics[item_id] = spell;
	}

	LogInfo("Loaded {} item sympathetics.", g_item_sympathetics.size());
}

void NmsAddItemSympathetics(Client *c, std::vector<uint16> &out)
{
	if (!c || g_item_sympathetics.empty()) {
		return;
	}

	for (const int16 slot : {EQ::invslot::slotPrimary, EQ::invslot::slotSecondary, EQ::invslot::slotRange}) {
		const auto inst = c->GetInv().GetItem(slot);
		if (!inst) {
			continue;
		}

		const auto it = g_item_sympathetics.find(inst->GetID());
		if (it != g_item_sympathetics.end()) {
			out.push_back(it->second);
		}
	}
}

void NmsWeaponProcLanded(Mob *caster, Mob *target, uint16 spell_id, float target_hp_before)
{
	if (!caster || !target || caster == target) {
		return;
	}

	ProcSpell p;
	if (!Decode(spell_id, p)) {
		// Any other spell can set off a Shatterpoint mark
		Mark(caster, target, spell_id);
		return;
	}

	if (p.role != 0) {
		if (p.n != NMS_WEAPON_PROC_MARK) {
			Mark(caster, target, spell_id);
		}
		return;
	}

	switch (p.n) {
		case NMS_WEAPON_PROC_EXECUTE:
			Execute(caster, target, spell_id, target_hp_before);
			break;
		case NMS_WEAPON_PROC_PULL:
			Pull(caster, target);
			break;
		case NMS_WEAPON_PROC_TRANCE:
			Trance(caster, target, p.tier);
			break;
		default:
			break;
	}

	if (p.n != NMS_WEAPON_PROC_MARK) {
		Mark(caster, target, spell_id);
	}
}

void NmsWeaponProcNpcDied(NPC *npc)
{
	if (!npc) {
		return;
	}

	g_mark_ready.erase(npc->GetID());

	uint16 caster_id = 0;
	const int tier = FindProcBuff(npc, NMS_WEAPON_PROC_SPREAD, &caster_id);
	if (!tier) {
		return;
	}

	Mob *caster = entity_list.GetMob(caster_id);
	if (!caster || caster->IsCorpse()) {
		return;
	}

	const uint16 dream = NmsWeaponProcSpell(NMS_WEAPON_PROC_SPREAD, 0, tier);
	NPC *nearest = nullptr;
	float best = SPREAD_RADIUS * SPREAD_RADIUS;
	for (const auto &[id, other] : entity_list.GetNPCList()) {
		if (!IsNearbyFoe(caster, npc, other, SPREAD_RADIUS) || FindProcBuff(other, NMS_WEAPON_PROC_SPREAD)) {
			continue;
		}

		const float d = DistanceSquared(other->GetPosition(), npc->GetPosition());
		if (d <= best) {
			best = d;
			nearest = other;
		}
	}

	if (nearest) {
		caster->SpellFinished(dream, nearest, EQ::spells::CastingSlot::Item, 0, 0xFFFFFFFF, spells[dream].resist_difficulty, true);
	}
}
