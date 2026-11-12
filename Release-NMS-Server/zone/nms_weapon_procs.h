#ifndef NMS_WEAPON_PROCS_H
#define NMS_WEAPON_PROCS_H

#include "../common/types.h"
#include <vector>

class Client;
class Mob;
class NPC;

// Server side of the tier weapon procs (docs/dynamic_items/WEAPON_PROCS.md). The proc spells themselves are spell
// data; this adds the parts no spell effect can do, and the sympathetics on the tier shields and range items.

// Reads item_sympathetics (item id -> sympathetic spell). Called with the armor set bonus load at zone boot.
void NmsLoadItemSympathetics();

// Adds the sympathetics of the client's equipped primary, secondary and range items to out.
void NmsAddItemSympathetics(Client *c, std::vector<uint16> &out);

// A spell from caster landed on target. target_hp_before is the target's health percent before it landed.
void NmsWeaponProcLanded(Mob *caster, Mob *target, uint16 spell_id, float target_hp_before);

// An NPC is dying with its buffs still on it.
void NmsWeaponProcNpcDied(NPC *npc);

#endif
