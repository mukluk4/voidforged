#include "../client.h"
#include "../command.h"
#include "../../common/say_link.h"
#include "../../common/spdat.h"
#include "../../common/strings.h"

#include <map>
#include <string>

// #nimbustest: put a nimbus (the lasting particle effect some buffs give) on yourself or your target, to see how each
// one looks in the client. "next"/"prev" step through every nimbus the spell data uses.
namespace {
	// Nimbus effect id -> the first spell that uses it, from the loaded spell data
	const std::map<uint32, uint16> &NimbusList()
	{
		static std::map<uint32, uint16> list;
		if (list.empty()) {
			for (int id = 1; id < SPDAT_RECORDS; ++id) {
				const auto effect = GetSpellNimbusEffect(id);
				if (effect > 0 && !list.count(effect)) {
					list[effect] = static_cast<uint16>(id);
				}
			}
		}

		return list;
	}

	// The last nimbus shown by each GM, so next/prev know where they are
	std::map<uint32, uint32> &Current()
	{
		static std::map<uint32, uint32> current;
		return current;
	}

	void Show(Client *c, Mob *target, uint32 effect)
	{
		target->RemoveAllNimbusEffects();
		target->SetNimbusEffect(effect);
		target->SendSpellEffect(effect, 500, 0, 1, 3000, true);
		Current()[c->CharacterID()] = effect;

		const auto &list = NimbusList();
		const auto it = list.find(effect);
		const std::string name = it != list.end() ? spells[it->second].name : "not used by any spell";
		c->Message(
			Chat::White,
			fmt::format(
				"Nimbus {} ({}) on {}. {} | {} | {}",
				effect,
				name,
				target->GetCleanName(),
				Saylink::Silent("#nimbustest prev", "Previous"),
				Saylink::Silent("#nimbustest next", "Next"),
				Saylink::Silent("#nimbustest off", "Off")
			).c_str()
		);
	}
}

void command_nimbustest(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget() ? c->GetTarget() : c;
	const auto &list = NimbusList();
	const std::string arg = Strings::ToLower(sep->arg[1]);

	if (arg.empty() || arg == "help") {
		c->Message(Chat::White, "Usage: #nimbustest [effect id | next | prev | off | list]");
		c->Message(Chat::White, fmt::format("{} nimbus effects are used by the spell data. Shows on your target, or you.", list.size()).c_str());
		return;
	}

	if (arg == "off") {
		target->RemoveAllNimbusEffects();
		c->Message(Chat::White, fmt::format("Nimbus effects removed from {}.", target->GetCleanName()).c_str());
		return;
	}

	if (arg == "list") {
		std::string line;
		for (const auto &[effect, spell_id] : list) {
			line += fmt::format("{} {}, ", Saylink::Silent(fmt::format("#nimbustest {}", effect), std::to_string(effect)), spells[spell_id].name);
			if (line.size() > 1500) {
				c->Message(Chat::White, line.c_str());
				line.clear();
			}
		}

		if (!line.empty()) {
			c->Message(Chat::White, line.c_str());
		}
		return;
	}

	if (list.empty()) {
		c->Message(Chat::White, "No spell uses a nimbus effect.");
		return;
	}

	if (arg == "next" || arg == "prev") {
		const auto cur = Current().count(c->CharacterID()) ? Current()[c->CharacterID()] : 0;
		auto it = list.upper_bound(cur);
		if (arg == "next") {
			if (it == list.end()) {
				it = list.begin();
			}
		} else {
			it = list.lower_bound(cur);
			it = it == list.begin() ? std::prev(list.end()) : std::prev(it);
		}

		Show(c, target, it->first);
		return;
	}

	if (!sep->IsNumber(1) || Strings::ToUnsignedInt(sep->arg[1]) == 0) {
		c->Message(Chat::White, "Usage: #nimbustest [effect id | next | prev | off | list]");
		return;
	}

	Show(c, target, Strings::ToUnsignedInt(sep->arg[1]));
}
