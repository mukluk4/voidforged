# Tier weapon procs (design, 2026-11-11)

The tier weapons sold by the Weaponsmiths are copies of the Legendary Plane of Time weapons. They keep the source's
proc at every tier, so a tier 5 weapon procs the same spell as tier 1 (tabled 2026-11-03). This replaces those with
new Void-themed procs: one per weapon, each with five tier versions that grow with the gear tier (x1.0 to x3.0).

Decided 2026-11-11: one proc per weapon (not shared by role), mixed effect types, Void theme, and some mechanics no
stock spell has.

## Scope

24 weapons can proc. The 6 shields and the 2 range items (Spool of Woven Time, Stone of Flowing Time) don't swing, so
they get sympathetics instead (below). Weapons without a proc today (Songblade, Time's Antithesis, Serpent of Vindication, Wand of
Impenetrable Force, Bow of the Tempest) get one too.

## How they're built

- **Spell data (S):** a normal proc spell: direct damage, DoT, debuff, lifetap, buff, or a spell that triggers
  others (SPA 340/374 trigger effects). Server and client both read it from the spell file.
- **Scripted (Q):** the proc spell carries the visual, the name and a base effect, and a small piece of server code
  (a spell script or a zone hook, like the existing rift scripts) does the unusual part: echoes, executes, chains,
  temporary pets, teleports.
- **Reactive (R):** the proc lands a buff or mark that fires later, as Ward of Tunare, Promised Renewal and Divine
  Guardian do (see Reactive mechanics). Each needs the buff and the spell it fires, so two or three spells per tier.
- Every proc is a new spell in the client range (as Hail of Arrows 43021-43025 was), so they ship to players in a
  Sunday client patch. 32 procs and sympathetics x 5 tiers, with the reactive ones' extra spells: about 250-300 spells.
- Damage and healing numbers below are tier 1; tier N multiplies them by the tier's gear multiplier (1.0, 1.5, 2.0,
  2.5, 3.0). Proc rate follows the weapon's delay as normal EQ procs do, so fast weapons get smaller procs.

## The procs

| # | Weapon (classes) | Proc | Effect | Build |
|---|---|---|---|---|
| 1 | Edge of Eternity (WAR RNG BRD ROG, 1H slash) | **Severed Moment** | Void damage. 25% of the time the same moment repeats one second later: the proc hits again. | Q |
| 2 | Darkblade of the Warlord (WAR, 1H slash) | **Maw of the Void** | A burst of hate on the target and a rune on the warrior that grows for each enemy within 30 feet. When the rune breaks it explodes in void damage around the warrior. | R + Q |
| 3 | Hopebringer (PAL, 1H slash) | **Voidlight Mend** | Light torn from the Void: damages the target and promises the paladin's group a heal that lands 6 seconds later. | R |
| 4 | Songblade of the Eternal (BRD, 1H slash) | **Hollow Refrain** | An echo of the bard's song: a short group buff with melee haste and mana regeneration. | S |
| 5 | Timespinner, Blade of the Hunter (RNG, 1H slash) | **Starless Volley** | Void arrows strike the target and up to two other enemies near it. | S |
| 6 | Greatblade of Chaos (PAL SHD BER, 2H slash) | **Unmaking** | Heavy void damage, doubled against a target below 25% health. A killing blow gives the wielder a short frenzy. | R + Q |
| 7 | Timeless Coral Greatsword (WAR PAL RNG SHD BER, 2H slash) | **Tide of Nothing** | A wave of void water: knockback, a snare and a damage over time. | S |
| 8 | Jagged Timeforged Blade (ROG, piercing) | **Gap in Time** | The rogue slips out of time: void damage, a large hate drop, and the next Backstab within 6 seconds is a certain critical. | S + Q |
| 9 | Time's Antithesis (SHM, piercing) | **Void Rot** | A disease DoT that also slows the target. | S |
| 10 | Dagger of Distraction (WAR RNG BRD ROG BST, piercing) | **Whispers from Beyond** | Voices from the Void: lowers the target's accuracy and drains mana from casters. | S |
| 11 | Hammer of Holy Vengeance (CLR, 1H blunt) | **Null Absolution** | A Divine Guardian-style ward on whoever the cleric's target is fighting (usually the tank): it heals them if they drop below 35% health. | R + Q |
| 12 | Wand of Temporal Power (NEC WIZ MAG ENC, 1H blunt) | **Entropic Spark** | Void damage, and the wielder gets back mana equal to part of it. | S |
| 13 | Hammer of Hours (CLR DRU SHM, 1H blunt) | **Hourless Ward** | A Ward of Tunare-style group buff: each time a member is struck, a chance to heal them. | R |
| 14 | Cudgel of Wrecking (WAR RNG MNK BRD ROG BST, 1H blunt) | **Shatterpoint** | Cracks the target's armor: an armor class debuff that stacks up to three times, and a mark that sets off a void burst whenever anyone hits the target with a spell. | R + Q, test |
| 15 | Cudgel of Venomous Hatred (SHD, 1H blunt) | **Hungering Dark** | A lifetap that also heals the shadow knight's pet, if it has one. | S + Q |
| 16 | Hammer of the Timeweaver (WAR RNG MNK BRD ROG BST, 1H blunt) | **Collapse** | The Void pulls inward: a short stun on the target, and nearby enemies are pulled toward it. | Q |
| 17 | Serpent of Vindication (ENC, 1H blunt) | **Mind Unmoored** | Mesmerizes up to two enemies near the target, never the target itself, so adds stop while the fight goes on. | Q |
| 18 | Ethereal Destroyer (WAR RNG MNK BRD ROG BST, 1H blunt) | **Phase Strike** | The wielder phases half out of the world: void damage, then for 18 seconds each time the wielder is struck, a chance to hit the attacker back with void damage. | R |
| 19 | Wand of Impenetrable Force (NEC WIZ, 1H blunt) | **Event Horizon** | Opens a void well under the target that hurts every enemy near it for 12 seconds. | Q |
| 20 | Greatstaff of Power (CLR DRU SHM NEC WIZ MAG ENC, 2H blunt) | **Void Siphon** | A short group buff: spell crit chance and mana regeneration. | S |
| 21 | Staff of Transcendence (MNK BST, 2H blunt) | **Empty Palm** | Void damage and a self buff that adds flurry and double attack for 12 seconds. | S |
| 22 | Vanazir, Dreamer's Despair (WAR PAL RNG SHD MNK BST BER, 2H blunt) | **Dream of the Void** | A nightmare DoT, and the target does less damage while it lasts. If the target dies, the nightmare jumps to the nearest enemy. | R, test |
| 23 | Bow of the Tempest (WAR PAL RNG SHD ROG, bow) | **Riftshot** | The arrow tears a rift: it hits the target and every enemy in a line behind it. | S |
| 24 | Shinai of the Ancients (MNK BST, hand to hand) | **Thousand Silences** | Void damage, interrupts the target's cast and silences it for 6 seconds. | S |

S = spell data only. R = reactive spell data. Q = needs server code as well. Test = NPC behaviour to check in game first.

Approved by the user 2026-11-11 ("let's go with these").

## Sympathetics for the items that don't swing

The 6 shields and 2 range items get a sympathetic proc instead: it fires when the wearer casts a spell (a damage
spell for a detrimental one, a heal for a beneficial one), as the 9-piece set bonus sympathetics do. Items have one
focus slot and 5 of these 8 already use it (Fury of Solusek, Fury of E`ci, Wrath of E`ci, Marr's Blessing, Furious
Bash VII), so the sympathetic is attached by the server the way set bonus sympathetics are (`m_set_bonus_sympathetic`)
and the item keeps its own focus. 8 sympathetics x 5 tiers = 40 more spells.

| # | Item (classes) | Sympathetic | Fires on | Effect |
|---|---|---|---|---|
| 25 | Spool of Woven Time (all, range) | **Unravelling Thread** | damage spell | A void thread on the target: a damage over time. |
| 26 | Stone of Flowing Time (WAR CLR PAL RNG SHD MNK BRD ROG SHM BST BER, range) | **Rift Pulse** | damage spell | A burst of void energy hits every enemy near the target. |
| 27 | Shield of the Vortex (CLR DRU SHM NEC WIZ MAG ENC) | **Vortex Echo** | damage spell | Void damage, and part of the spell's mana comes back. |
| 28 | Barrier of Freezing Winds (DRU SHM NEC WIZ MAG ENC) | **Voidfrost** | damage spell | Cold void damage and a snare. |
| 29 | Whorl of Unnatural Forces (CLR DRU SHM) | **Tidewell** | heal | A promised heal: whoever was healed is healed again 6 seconds later. |
| 30 | Shield of Strife (WAR PAL SHD) | **Bulwark of Nothing** | damage or heal spell | A rune on the wearer; when it breaks it pulses hate and void damage at nearby enemies. |
| 31 | Tactician's Shield (all) | **Void Ward** | heal | A rare group death save: a member who would die is healed instead. Fires at most once a minute. |
| 32 | Fiery Crystal Guard (WAR CLR PAL SHD BRD) | **Crystal Ember** | damage spell | For 18 seconds, each time the wearer is struck, a chance to throw crystal shards at the attacker. |

Sympathetics only fire on spells cast from a spell gem that damage or heal (`zone/spells.cpp`, the spell needs a
hit point effect); disciplines, buffs and clickies don't count. On Voidforged every character has three classes, so a
warrior with a caster class still triggers them; a pure-melee setup won't.

## Reactive mechanics

All are already in the server (`common/spdat.h`) and in RoF2 spells; the proc or sympathetic lands the buff or mark.

| Mechanic | Effect (SPA) | Stock example | Used by |
|---|---|---|---|
| When struck | Defensive proc (323) | Ward of Tunare, Ward of Retribution | 13, 18, 32 |
| Promised | Cast on fade (289) | Promised Renewal, Promised Restoration | 3, 29 |
| Below a health threshold | Trigger on target requirement (442) | Divine Guardian | 11 |
| Rune breaks | Cast on fade, rune or duration (373) | | 2, 30 |
| Death save | Death save (150) | Divine Intervention | 31 |
| Killing blow | Proc on kill shot (360) | | 6 |
| Buffed one dies | zone hook (361 is cast by the NPC) | | 22 |
| Hit by a spell | zone hook (481 is cast by the NPC) | | 14 |

Fade and land triggers on an NPC are cast by the NPC itself, so the mark (14) and the jump on death (22) are zone
hooks instead (see Build). Void Ward (31) fires rarely (15% sympathetic chance) instead of on a once-a-minute timer.

## Build (2026-11-11)

- `utils/scripts/dynamic_items/gen_weapon_procs.py` writes the spells and item changes to
  `utils/scripts/rift/sql/2026_11_11_01_tier_weapon_procs.sql` and the spell ids to `zone/nms_weapon_procs_ids.h`.
  Run it again after `gen_tier_gear.py`, since it finds the tier weapons by their item ids.
- Spell ids 44100-44579: `44100 + (n-1)*15 + role*5 + (tier-1)`, where role 0 is the proc itself and 1-2 are the
  spells it fires. 310 of them are used. Procs 1-24 are set as `proceffect` (rate 20) on the 16 class copies of each
  weapon; 25-32 go in the `item_sympathetics` table.
- `zone/nms_weapon_procs.cpp` does the parts no spell effect can:
  - **Item sympathetics:** loaded with the armor set bonuses at zone boot; the equipped primary, secondary and range
    items' entries join `m_set_bonus_sympathetic`, so they fire as set bonus sympathetics do.
  - **Unmaking (6):** a target below 25% health before the hit takes the proc's damage a second time.
  - **Shatterpoint (14):** any detrimental spell a player (or pet) lands on a marked NPC sets off Shatterpoint Burst
    of the mark's tier, at most once per 3 seconds per target.
  - **Collapse (16):** NPCs within 30 of the target are moved halfway toward it.
  - **Mind Unmoored (17):** the wielder casts the trance on up to 2 NPCs within 40 of the target.
  - **Dream of the Void (22):** when an NPC dies with it, the caster casts the same tier on the nearest NPC within 40.
  - Pull, trance and spread only pick NPCs that are already fighting, in line of sight of the target, that the
    caster may attack and that are not raid targets, so they never pull new mobs into a fight.
- The spells must reach players in a client patch together with the server change: export the new lines from the DB
  (`export_client_files`) into the reference client's `spells_us.txt`.
- The Vault's proc slots lend a parked weapon's proc as before; item sympathetics only count equipped items.

## Still to check in game

- Tier 1 numbers (damage, heal, rune size, durations) and whether the procs that hit many enemies (5, 16, 19, 23) do
  less damage per enemy.
- Riftshot's beam (target type 44) from a melee proc; 361 from a debuff on an NPC was replaced by the death hook.
- Visuals: each spell copies its template's particles; pick better ones from `#nimbustest` and `#castspell`.
- The old Legendary procs stay on the non-tier Legendary items; only the tier copies change.
