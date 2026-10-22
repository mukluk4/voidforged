#include "../client.h"
#include "../command.h"
#include "../dialogue_window.h"
#include "../dynamic_items.h"
#include "../../common/say_link.h"

#include <algorithm>

namespace {
	// Every slot a player can be carrying a dynamic item in, except the bank
	std::vector<int16> CarriedSlots()
	{
		std::vector<int16> slots;
		for (int16 slot_id = EQ::invslot::POSSESSIONS_BEGIN; slot_id <= EQ::invslot::POSSESSIONS_END; ++slot_id) {
			slots.push_back(slot_id);
		}

		for (int16 slot_id = EQ::invbag::GENERAL_BAGS_BEGIN; slot_id <= EQ::invbag::GENERAL_BAGS_END; ++slot_id) {
			slots.push_back(slot_id);
		}

		return slots;
	}

	void SendDynamicItemSubCommands(Client *c)
	{
		c->Message(Chat::White, "Usage: #dynitem give [Item ID] [Seed] - Give your target (or you) a rolled dynamic item, random seed if omitted");
		c->Message(Chat::White, "Usage: #dynitem info - List your target's (or your) dynamic items with slots and seeds");
		c->Message(Chat::White, "Usage: #dynitem reroll [Slot ID] - Give the item in that slot a new random seed");
		c->Message(Chat::White, "Usage: #dynitem setseed [Slot ID] [Seed] - Set the seed of the item in that slot");
		c->Message(Chat::White, "Usage: #dynitem reload - Reload the dynamic item and armor set tables and refresh players in this zone");
	}

	// Saves and resends an item whose seed was just changed in place
	void ResaveDynamicItem(Client *t, int16 slot_id, EQ::ItemInstance *inst)
	{
		if (slot_id == EQ::invslot::slotCursor) {
			auto s = t->GetInv().cursor_cbegin(), e = t->GetInv().cursor_cend();
			database.SaveCursor(t->CharacterID(), s, e);
		} else {
			database.SaveInventory(t->CharacterID(), inst, slot_id);
		}

		t->SendItemPacket(slot_id, inst, ItemPacketTrade);
		t->CalcBonuses();
	}
}

void command_dynitem(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments) {
		SendDynamicItemSubCommands(c);
		return;
	}

	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	auto mgr = DynamicItemsManager::Instance();

	if (!strcasecmp(sep->arg[1], "give")) {
		if (!sep->IsNumber(2)) {
			SendDynamicItemSubCommands(c);
			return;
		}

		const uint32 item_id = Strings::ToUnsignedInt(sep->arg[2]);
		if (!mgr->IsDynamicItem(item_id)) {
			c->Message(Chat::Red, fmt::format("Item {} is not in dynamic_items.", item_id).c_str());
			return;
		}

		uint64 seed = arguments >= 3 && sep->IsNumber(3) ? Strings::ToUnsignedBigInt(sep->arg[3]) : 0;
		if (seed == 0) {
			seed = GenerateNewItemSeed();
		}

		auto inst = database.CreateItem(item_id);
		if (!inst) {
			c->Message(Chat::Red, fmt::format("Item {} does not exist.", item_id).c_str());
			return;
		}

		DynamicItemsManager::SetSeed(*inst, seed);
		t->PushItemOnCursor(*inst, true);
		mgr->SendSummary(t, *inst);
		safe_delete(inst);

		if (t != c) {
			c->Message(Chat::White, "%s", fmt::format("Gave {} item {} with seed {}.", t->GetCleanName(), item_id, seed).c_str());
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "info")) {
		int found = 0;
		for (const int16 slot_id : CarriedSlots()) {
			const auto inst = t->GetInv().GetItem(slot_id);
			if (!inst || !mgr->IsDynamicItem(inst->GetID())) {
				continue;
			}

			const auto lines = mgr->Describe(*inst);
			if (lines.empty()) {
				continue;
			}

			c->Message(
				Chat::Yellow,
				"%s",
				fmt::format("Slot {} | {} | Seed {}", slot_id, lines.front(), DynamicItemsManager::GetSeed(*inst)).c_str()
			);
			c->Message(Chat::White, "%s", Strings::Join(std::vector<std::string>(lines.begin() + 1, lines.end()), ", ").c_str());
			found++;
		}

		if (!found) {
			c->Message(Chat::White, fmt::format("{} is not carrying any dynamic items.", t->GetCleanName()).c_str());
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "reroll") || !strcasecmp(sep->arg[1], "setseed")) {
		const bool is_setseed = !strcasecmp(sep->arg[1], "setseed");
		if (!sep->IsNumber(2) || (is_setseed && !sep->IsNumber(3))) {
			SendDynamicItemSubCommands(c);
			return;
		}

		const auto slot_id = static_cast<int16>(Strings::ToInt(sep->arg[2]));
		auto inst = t->GetInv().GetItem(slot_id);
		if (!inst || !mgr->IsDynamicItem(inst->GetID())) {
			c->Message(Chat::Red, fmt::format("There is no dynamic item in slot {}.", slot_id).c_str());
			return;
		}

		const uint64 seed = is_setseed ? Strings::ToUnsignedBigInt(sep->arg[3]) : GenerateNewItemSeed();
		if (seed == 0) {
			c->Message(Chat::Red, "Seed must not be 0.");
			return;
		}

		DynamicItemsManager::SetSeed(*inst, seed);
		ResaveDynamicItem(t, slot_id, inst);
		mgr->SendSummary(t, *inst);

		if (t != c) {
			c->Message(Chat::White, fmt::format("Set seed of {}'s slot {} to {}.", t->GetCleanName(), slot_id, seed).c_str());
		}
		return;
	}

	if (!strcasecmp(sep->arg[1], "reload")) {
		if (!mgr->Load()) {
			c->Message(Chat::Red, "Failed to reload dynamic item tables, see the zone log.");
			return;
		}

		// Armor sets too, since dynamic items are usually set pieces
		LoadArmorSetBonuses();

		int players = 0, items = 0;
		for (auto &[id, client] : entity_list.GetClientList()) {
			const int updated = mgr->RefreshClient(client);
			if (updated) {
				players++;
				items += updated;
			}
			client->CalcBonuses();
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Dynamic item and armor set tables reloaded. Updated {} items on {} players in this zone (other zones need a restart or their own reload).",
				items,
				players
			).c_str()
		);
		return;
	}

	SendDynamicItemSubCommands(c);
}

namespace {
	// One dynamic item a player owns, numbered for #dyninfo compare
	struct DynInfoEntry {
		int                      number = 0;
		int16                    slot_id = 0;
		const EQ::ItemInstance   *inst = nullptr;
		std::string              where;
		int                      group = -1;
		bool                     equipped = false;
		std::vector<RolledAffix> rolls;
	};

	// Everything the player owns, in a stable order: worn, inventory, bags, bank, shared bank
	std::vector<DynInfoEntry> GatherDynamicItems(Client *c)
	{
		auto mgr = DynamicItemsManager::Instance();

		std::vector<std::pair<int16, const char *>> slots;
		for (int16 s = EQ::invslot::POSSESSIONS_BEGIN; s <= EQ::invslot::POSSESSIONS_END; ++s) {
			const bool worn = s >= EQ::invslot::EQUIPMENT_BEGIN && s <= EQ::invslot::EQUIPMENT_END;
			slots.emplace_back(s, worn ? "equipped" : (s == EQ::invslot::slotCursor ? "cursor" : "inventory"));
		}
		for (int16 s = EQ::invbag::GENERAL_BAGS_BEGIN; s <= EQ::invbag::GENERAL_BAGS_END; ++s) {
			slots.emplace_back(s, "bag");
		}
		for (int16 s = EQ::invslot::BANK_BEGIN; s <= EQ::invslot::BANK_END; ++s) {
			slots.emplace_back(s, "bank");
		}
		for (int16 s = EQ::invbag::BANK_BAGS_BEGIN; s <= EQ::invbag::BANK_BAGS_END; ++s) {
			slots.emplace_back(s, "bank");
		}
		for (int16 s = EQ::invslot::SHARED_BANK_BEGIN; s <= EQ::invslot::SHARED_BANK_END; ++s) {
			slots.emplace_back(s, "shared bank");
		}
		for (int16 s = EQ::invbag::SHARED_BANK_BAGS_BEGIN; s <= EQ::invbag::SHARED_BANK_BAGS_END; ++s) {
			slots.emplace_back(s, "shared bank");
		}

		std::vector<DynInfoEntry> entries;
		for (const auto &[slot_id, where] : slots) {
			const auto inst = c->GetInv().GetItem(slot_id);
			if (!inst || !mgr->IsDynamicItem(inst->GetID())) {
				continue;
			}

			DynInfoEntry e;
			e.number   = static_cast<int>(entries.size()) + 1;
			e.slot_id  = slot_id;
			e.inst     = inst;
			e.where    = where;
			e.equipped = slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END;
			e.group    = e.equipped
				? DynamicItemsManager::SlotGroupForEquipSlot(slot_id)
				: DynamicItemsManager::SlotGroupForSlots(inst->GetUnscaledItem()->Slots);
			e.rolls    = mgr->Roll(*inst);
			entries.push_back(std::move(e));
		}

		return entries;
	}

	// What a group's spares are compared against: the first dynamic item worn in that group
	const DynInfoEntry *GroupBaseline(const std::vector<DynInfoEntry> &entries, int group)
	{
		for (const auto &e : entries) {
			if (e.group == group && e.equipped) {
				return &e;
			}
		}

		return nullptr;
	}

	std::string EntryTitle(const DynInfoEntry &e)
	{
		return fmt::format("[{}] {} ({})", e.number, e.inst->GetItem()->Name, e.where);
	}

	// The popup drops anything over 4095 characters
	constexpr size_t POPUP_LIMIT = 4000;

	const DynInfoEntry *FindEntry(const std::vector<DynInfoEntry> &entries, int number)
	{
		for (const auto &e : entries) {
			if (e.number == number) {
				return &e;
			}
		}

		return nullptr;
	}

	// "Parry %" -> "Parry", is_pct = true
	std::string TableLabel(EQ::eDynamicAffixID affix, bool &is_pct)
	{
		std::string name = DynamicItemsManager::AffixName(affix);
		is_pct = name.size() > 2 && name.ends_with(" %");
		if (is_pct) {
			name.resize(name.size() - 2);
		}

		return name;
	}

	// One item's stats as a table, or a second item compared against it column by column. The popup holds
	// 4,095 characters (OnLevelMessage_Struct; longer text is dropped by the client), so a long table is split
	// into pages: page_command + " <page>" re-sends this view at that page (a clickable link in chat).
	void SendDynInfoTable(
		Client *c,
		const DynInfoEntry &first,
		const DynInfoEntry *second,
		const std::string &title,
		int page = 1,
		const std::string &page_command = ""
	)
	{
		std::vector<EQ::eDynamicAffixID> affixes;
		for (const auto *e : { &first, second }) {
			if (!e) {
				continue;
			}
			for (const auto &line : e->rolls) {
				if (std::find(affixes.begin(), affixes.end(), line.affix) == affixes.end()) {
					affixes.push_back(line.affix);
				}
			}
		}
		std::sort(affixes.begin(), affixes.end());

		auto find_line = [](const DynInfoEntry &e, EQ::eDynamicAffixID affix) -> const RolledAffix * {
			for (const auto &line : e.rolls) {
				if (line.affix == affix) {
					return &line;
				}
			}
			return nullptr;
		};

		auto header = DialogueWindow::TableCell("Stat") +
			DialogueWindow::TableCell(fmt::format("#{}{}", first.number, first.equipped ? " (worn)" : ""));
		if (second) {
			header += DialogueWindow::TableCell(fmt::format("#{}", second->number)) + DialogueWindow::TableCell("Change");
		}

		// Every row, with the totals counted over the whole comparison
		std::vector<std::string> rows;
		int                      better = 0, worse = 0;
		for (const auto affix : affixes) {
			bool       is_pct = false;
			const auto name   = TableLabel(affix, is_pct);
			const auto unit   = is_pct ? "%" : "";
			const auto a      = find_line(first, affix);
			const auto b      = second ? find_line(*second, affix) : nullptr;
			const bool random = (a && !a->guaranteed) || (b && !b->guaranteed);

			auto cell = [&](const RolledAffix *l) {
				return DialogueWindow::TableCell(l ? fmt::format("{}{}", l->value, unit) : "-");
			};

			std::string row = DialogueWindow::TableCell(fmt::format("{}{}", random ? "* " : "", name)) + cell(a);
			if (second) {
				const int diff = (b ? b->value : 0) - (a ? a->value : 0);
				std::string change;
				if (diff > 0) {
					better++;
					change = DialogueWindow::ColorMessage("green", fmt::format("{:+}{}", diff, unit));
				} else if (diff < 0) {
					worse++;
					change = DialogueWindow::ColorMessage("red", fmt::format("{:+}{}", diff, unit));
				}
				row += cell(b) + DialogueWindow::TableCell(change);
			}

			rows.push_back(DialogueWindow::TableRow(row));
		}

		std::string top = DialogueWindow::ColorMessage("gold", EntryTitle(first)) + DialogueWindow::Break();
		if (second) {
			top += "vs " + DialogueWindow::ColorMessage("white", EntryTitle(*second)) + DialogueWindow::Break();
		}

		std::string bottom;
		if (second) {
			bottom += fmt::format(
				"{} better, {} worse",
				DialogueWindow::ColorMessage("green", std::to_string(better)),
				DialogueWindow::ColorMessage("red", std::to_string(worse))
			) + DialogueWindow::Break();
		}
		bottom += "* = random affix";

		// Split the rows into pages that fit, leaving room for the page line
		constexpr size_t popup_limit = 4095;
		const std::string page_line_room = DialogueWindow::Break() + "Page 99 of 99";
		const size_t      fixed = top.size() + DialogueWindow::Table(DialogueWindow::TableRow(header)).size() +
			bottom.size() + page_line_room.size();

		std::vector<std::pair<size_t, size_t>> pages; // [first row, end)
		size_t                                 begin = 0, size = fixed;
		for (size_t i = 0; i < rows.size(); ++i) {
			if (i > begin && size + rows[i].size() > popup_limit) {
				pages.emplace_back(begin, i);
				begin = i;
				size  = fixed;
			}
			size += rows[i].size();
		}
		pages.emplace_back(begin, rows.size());

		const int page_count = static_cast<int>(pages.size());
		page = std::clamp(page, 1, page_count);
		const auto [from, to] = pages[page - 1];

		std::string table_rows = DialogueWindow::TableRow(header);
		for (size_t i = from; i < to; ++i) {
			table_rows += rows[i];
		}

		std::string text = top + DialogueWindow::Table(table_rows) + bottom;
		if (page_count > 1) {
			text += DialogueWindow::Break() + fmt::format("Page {} of {}", page, page_count);
		}

		c->SendPopupToClient(
			(page_count > 1 ? fmt::format("{} ({}/{})", title, page, page_count) : title).c_str(),
			text.c_str()
		);

		if (page_count > 1 && !page_command.empty()) {
			std::vector<std::string> links;
			for (int p = 1; p <= page_count; ++p) {
				if (p != page) {
					links.push_back(Saylink::Create(fmt::format("{} {}", page_command, p), true, fmt::format("Page {}", p)));
				}
			}
			c->Message(
				Chat::White,
				"%s",
				fmt::format("{} has {} pages: {}", title, page_count, Strings::Join(links, " ")).c_str()
			);
		}
	}

	// Clickable links in chat, one per item in each slot, that open its comparison window
	void SendDynInfoLinks(Client *c, const std::vector<DynInfoEntry> &entries, int only_group = -1)
	{
		for (int group = 0; group < DynamicItemsManager::SlotGroupCount(); ++group) {
			if (only_group >= 0 && group != only_group) {
				continue;
			}

			std::vector<std::string> links;
			for (const auto &e : entries) {
				if (e.group != group) {
					continue;
				}
				links.push_back(Saylink::Create(fmt::format("#dyninfo {}", e.number), true, fmt::format("#{}", e.number)));
			}

			if (!links.empty()) {
				c->Message(
					Chat::White,
					"%s",
					fmt::format("{}: {}", DynamicItemsManager::SlotGroupName(group), Strings::Join(links, " ")).c_str()
				);
			}
		}
	}

	void SendDynInfoOverview(Client *c, const std::vector<DynInfoEntry> &entries)
	{
		std::string text;
		bool truncated = false;

		for (int group = 0; group < DynamicItemsManager::SlotGroupCount(); ++group) {
			const bool has_items = std::any_of(
				entries.begin(),
				entries.end(),
				[&](const DynInfoEntry &e) { return e.group == group; }
			);
			if (!has_items) {
				continue;
			}

			const auto  baseline = GroupBaseline(entries, group);
			std::string block    = DialogueWindow::ColorMessage("gold", Strings::ToUpper(DynamicItemsManager::SlotGroupName(group)));
			block += baseline ? fmt::format(" - {}", EntryTitle(*baseline)) : " - nothing dynamic equipped";
			block += DialogueWindow::Break();

			for (const auto &e : entries) {
				if (e.group != group || &e == baseline) {
					continue;
				}

				block += DialogueWindow::Indent() + EntryTitle(e);
				if (baseline) {
					const auto cmp = DynamicItemsManager::Compare(baseline->rolls, e.rolls, false);
					block += fmt::format(
						"  {} better, {} worse",
						DialogueWindow::ColorMessage("green", std::to_string(cmp.better)),
						DialogueWindow::ColorMessage("red", std::to_string(cmp.worse))
					);
					if (!cmp.standout.empty()) {
						block += ", " + cmp.standout;
					}
				}
				block += DialogueWindow::Break();
			}

			block += DialogueWindow::Break();

			if (text.size() + block.size() > POPUP_LIMIT) {
				truncated = true;
				break;
			}

			text += block;
		}

		if (truncated) {
			text += "Too many items to list; use #dyninfo [slot]." + DialogueWindow::Break();
		}

		text += "Click an item number in chat (or #dyninfo [#]) to compare it with what you're wearing. #dyninfo compare [#] [#] for any two.";
		c->SendPopupToClient("Dynamic Items", text.c_str());
		SendDynInfoLinks(c, entries);
	}

	void SendDynInfoSlot(Client *c, const std::vector<DynInfoEntry> &entries, int group)
	{
		const auto  baseline = GroupBaseline(entries, group);
		std::string text;
		bool        any = false;

		auto add_block = [&](const std::string &block) {
			if (text.size() + block.size() > POPUP_LIMIT) {
				return false;
			}
			text += block;
			return true;
		};

		if (baseline) {
			std::string block = DialogueWindow::ColorMessage("gold", EntryTitle(*baseline)) + DialogueWindow::Break();
			for (const auto &line : baseline->rolls) {
				block += DialogueWindow::Indent() + DynamicItemsManager::FormatLine(line) + DialogueWindow::Break();
			}
			add_block(block + DialogueWindow::Break());
			any = true;
		}

		for (const auto &e : entries) {
			if (e.group != group || &e == baseline) {
				continue;
			}

			any = true;
			std::string block = DialogueWindow::ColorMessage("white", EntryTitle(e)) + DialogueWindow::Break();
			if (baseline) {
				for (const auto &line : DynamicItemsManager::Compare(baseline->rolls, e.rolls, true).lines) {
					block += DialogueWindow::Indent() + line + DialogueWindow::Break();
				}
			} else {
				for (const auto &line : e.rolls) {
					block += DialogueWindow::Indent() + DynamicItemsManager::FormatLine(line) + DialogueWindow::Break();
				}
			}

			if (!add_block(block + DialogueWindow::Break())) {
				text += "More items than fit; use #dyninfo compare [#] [#].";
				break;
			}
		}

		if (!any) {
			c->Message(
				Chat::White,
				"%s",
				fmt::format("You have no dynamic {} items.", DynamicItemsManager::SlotGroupName(group)).c_str()
			);
			return;
		}

		if (baseline) {
			text += "(differences are against your equipped item) * = random affix";
		}

		c->SendPopupToClient(fmt::format("Dynamic Items: {}", DynamicItemsManager::SlotGroupName(group)).c_str(), text.c_str());
		SendDynInfoLinks(c, entries, group);
	}

	void SendDynInfoCompare(Client *c, const std::vector<DynInfoEntry> &entries, int a, int b, int page = 1)
	{
		const auto first  = FindEntry(entries, a);
		const auto second = FindEntry(entries, b);
		if (!first || !second) {
			c->Message(Chat::Red, "Unknown item number. Type #dyninfo to see the numbers.");
			return;
		}

		SendDynInfoTable(c, *first, second, fmt::format("Compare #{} with #{}", b, a), page, fmt::format("#dyninfo compare {} {}", a, b));
	}

	// One item against whatever is worn in its slot (or on its own if it's the worn one)
	void SendDynInfoItem(Client *c, const std::vector<DynInfoEntry> &entries, int number, int page = 1)
	{
		const auto e = FindEntry(entries, number);
		if (!e) {
			c->Message(Chat::Red, "Unknown item number. Type #dyninfo to see the numbers.");
			return;
		}

		const auto worn = GroupBaseline(entries, e->group);
		if (!worn || worn == e) {
			SendDynInfoTable(c, *e, nullptr, fmt::format("Dynamic Item #{}", number), page, fmt::format("#dyninfo {}", number));
			return;
		}

		SendDynInfoTable(c, *worn, e, fmt::format("Compare #{} with worn #{}", number, worn->number), page, fmt::format("#dyninfo {}", number));
	}
}

void command_dyninfo(Client *c, const Seperator *sep)
{
	const auto entries = GatherDynamicItems(c);
	if (entries.empty()) {
		c->Message(Chat::White, "You don't have any dynamic items.");
		return;
	}

	if (sep->argnum == 0) {
		SendDynInfoOverview(c, entries);
		return;
	}

	if (sep->IsNumber(1)) {
		SendDynInfoItem(c, entries, Strings::ToInt(sep->arg[1]), sep->IsNumber(2) ? Strings::ToInt(sep->arg[2]) : 1);
		return;
	}

	if (!strcasecmp(sep->arg[1], "compare")) {
		if (!sep->IsNumber(2) || !sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #dyninfo compare [#] [#] [page] - numbers are shown by #dyninfo");
			return;
		}

		SendDynInfoCompare(
			c,
			entries,
			Strings::ToInt(sep->arg[2]),
			Strings::ToInt(sep->arg[3]),
			sep->IsNumber(4) ? Strings::ToInt(sep->arg[4]) : 1
		);
		return;
	}

	// Allow two-word slot names ("power source")
	std::string slot_name = sep->arg[1];
	if (sep->argnum >= 2) {
		slot_name += sep->arg[2];
	}

	int group = DynamicItemsManager::SlotGroupFromName(sep->arg[1]);
	if (group < 0) {
		group = DynamicItemsManager::SlotGroupFromName(slot_name);
	}

	if (group < 0) {
		c->Message(Chat::White, "Usage: #dyninfo - overview | #dyninfo [#] [page] - compare with worn | #dyninfo [slot] - e.g. feet | #dyninfo compare [#] [#] [page]");
		return;
	}

	SendDynInfoSlot(c, entries, group);
}
