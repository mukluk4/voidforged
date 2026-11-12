"""Tier weapon procs and sympathetics (docs/dynamic_items/WEAPON_PROCS.md).

    python gen_weapon_procs.py

Writes utils/scripts/rift/sql/2026_11_11_01_tier_weapon_procs.sql and zone/nms_weapon_procs_ids.h. Every spell is a
copy of a template spell (for its icon, animation and sounds) with its own effects. Numbers are tier 1; tier N
multiplies damage, healing, runes and mana by the gear tier's multiplier (tier_gear.json). Edit this file, not the
outputs.

Spell ids: 44100 + (proc - 1) * 15 + role * 5 + (tier - 1), role 0 = the proc itself, 1 and 2 = spells it fires.
Items: the tier copies of tier_gear.json "weapons" (item_base + (tier - 1) * tier_item_step + weapon_block + n * 20 +
class). Weapons get the proc as their proc effect; shields and range items get it in item_sympathetics, which the
zone adds as a sympathetic (the item keeps its own focus).
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
tg = json.load(open(os.path.join(HERE, 'tier_gear.json'), encoding='utf-8'))

SPELL_BASE = 44100
TIERS = [t['mult'] for t in tg['tiers']]
ROMAN = ['I', 'II', 'III', 'IV', 'V']

# Target types (common/spdat.h)
TT_AE_CASTER, TT_TARGET, TT_SELF, TT_AE_TARGET, TT_TAP, TT_PET, TT_BEAM, TT_TARGETS_TARGET, TT_GROUP = 4, 5, 6, 8, 13, 14, 44, 46, 41
# Resists
R_NONE, R_MAGIC, R_COLD, R_DISEASE, R_CORRUPT = 0, 1, 3, 5, 9

# Templates: copied for icon, animation and sound
T_NUKE, T_DOT, T_SELF, T_GROUP, T_HEAL, T_SYMP_DMG, T_SYMP_HEAL = 3630, 3877, 6662, 8479, 1825, 24351, 44020

# Effect values: a number is fixed, ('s', n) scales with the tier multiplier, ('id', role) is a spell id of this proc
S = lambda n: ('s', n)
ID = lambda role: ('id', role)


def spell(name, tmpl, tt, effects, good=0, ticks=0, resist=R_CORRUPT, aoe=0, rng=200, maxtargets=0, **extra):
    return dict(name=name, tmpl=tmpl, tt=tt, effects=effects, good=good, ticks=ticks, resist=resist, aoe=aoe, rng=rng,
                maxtargets=maxtargets, extra=extra)


# n: (weapon source item, name, [role 0, role 1, role 2]); effects are (spa, base, limit, max)
PROCS = {
    1: (2022986, 'Severed Moment', [
        spell('Severed Moment', T_NUKE, TT_TARGET, [(0, S(-560), 0, 0), (340, 25, ID(1), 0)]),
        spell('Severed Moment Echo', T_NUKE, TT_TARGET, [(0, S(-560), 0, 0)]),
    ]),
    2: (2022999, 'Maw of the Void', [
        spell('Maw of the Void', T_NUKE, TT_TARGET, [(92, S(600), 0, 0), (374, 100, ID(1), 0)]),
        spell('Maw of the Void Rune', T_SELF, TT_SELF, [(55, S(800), 0, 0), (373, ID(2), 0, 0)], good=1, ticks=3),
        spell('Maw of the Void Burst', T_NUKE, TT_AE_CASTER, [(0, S(-400), 0, 0)], aoe=30),
    ]),
    3: (2023498, 'Voidlight Mend', [
        spell('Voidlight Mend', T_NUKE, TT_TARGET, [(0, S(-500), 0, 0), (374, 100, ID(1), 0)]),
        spell('Voidlight Promise', T_GROUP, TT_GROUP, [(289, ID(2), 0, 0)], good=1, ticks=1, aoe=100),
        spell('Voidlight Promise Heal', T_HEAL, TT_SELF, [(0, S(400), 0, 0)], good=1),
    ]),
    4: (2024570, 'Hollow Refrain', [
        spell('Hollow Refrain', T_GROUP, TT_GROUP, [(119, 10, 0, 0), (15, S(15), 0, 0)], good=1, ticks=2, aoe=100),
    ]),
    5: (2024573, 'Starless Volley', [
        spell('Starless Volley', T_NUKE, TT_AE_TARGET, [(0, S(-350), 0, 0)], aoe=25, maxtargets=3),
    ]),
    6: (2026092, 'Unmaking', [
        # zone: below 25% health the target takes the damage twice
        spell('Unmaking', T_NUKE, TT_TARGET, [(0, S(-900), 0, 0), (374, 100, ID(2), 0)]),
        spell('Unmaking Frenzy', T_SELF, TT_SELF, [(2, S(100), 0, 0), (119, 10, 0, 0)], good=1, ticks=3),
        # A kill within 12 seconds of the proc starts the frenzy
        spell('Unmaking Hunger', T_SELF, TT_SELF, [(360, 100, ID(1), 0)], good=1, ticks=2),
    ]),
    7: (2068199, 'Tide of Nothing', [
        spell('Tide of Nothing', T_NUKE, TT_TARGET, [(0, S(-400), 0, 0), (3, -40, 0, 0), (340, 100, ID(1), 0)], ticks=2,
              pushback=12, pushup=2),
        spell('Tide of Nothing Undertow', T_DOT, TT_TARGET, [(0, S(-150), 0, 0)], ticks=3),
    ]),
    8: (2024692, 'Gap in Time', [
        spell('Gap in Time', T_NUKE, TT_TARGET, [(0, S(-450), 0, 0), (92, -1500, 0, 0), (374, 100, ID(1), 0)]),
        spell('Gap in Time Opening', T_SELF, TT_SELF, [(169, 100, 8, 0)], good=1, ticks=1),
    ]),
    9: (2024699, 'Void Rot', [
        spell('Void Rot', T_DOT, TT_TARGET, [(0, S(-120), 0, 0), (11, 85, 0, 0)], ticks=4, resist=R_DISEASE),
    ]),
    10: (2024776, 'Whispers from Beyond', [
        spell('Whispers from Beyond', T_DOT, TT_TARGET, [(216, -30, 0, 0), (2, -50, 0, 0), (15, S(-100), 0, 0)], ticks=3),
    ]),
    11: (2020627, 'Null Absolution', [
        spell('Null Absolution', T_SELF, TT_TARGETS_TARGET, [(442, ID(1), 250, 0)], good=1, ticks=3),
        spell('Null Absolution Mend', T_HEAL, TT_SELF, [(0, S(1200), 0, 0)], good=1),
    ]),
    12: (2021876, 'Entropic Spark', [
        spell('Entropic Spark', T_NUKE, TT_TARGET, [(0, S(-650), 0, 0), (374, 100, ID(1), 0)]),
        spell('Entropic Return', T_HEAL, TT_SELF, [(15, S(150), 0, 0)], good=1),
    ]),
    13: (2021886, 'Hourless Ward', [
        spell('Hourless Ward', T_GROUP, TT_GROUP, [(323, ID(1), 200, 0)], good=1, ticks=3, aoe=100),
        spell('Hourless Mend', T_HEAL, TT_SELF, [(0, S(250), 0, 0)], good=1),
    ]),
    14: (2022883, 'Shatterpoint', [
        spell('Shatterpoint', T_DOT, TT_TARGET, [(1, S(-40), 0, 0)], ticks=4),
        spell('Shatterpoint Burst', T_NUKE, TT_TARGET, [(0, S(-300), 0, 0)]),  # zone: a spell landing on the mark
    ]),
    15: (2022890, 'Hungering Dark', [
        spell('Hungering Dark', T_NUKE, TT_TAP, [(0, S(-600), 0, 0), (374, 100, ID(1), 0)]),
        spell('Hungering Dark Feast', T_HEAL, TT_PET, [(0, S(600), 0, 0)], good=1),
    ]),
    16: (2022894, 'Collapse', [
        spell('Collapse', T_NUKE, TT_TARGET, [(0, S(-300), 0, 0), (21, 1000, 0, 0)]),  # zone: pulls nearby enemies in
    ]),
    17: (2022959, 'Mind Unmoored', [
        spell('Mind Unmoored', T_NUKE, TT_TARGET, [(0, S(-200), 0, 0)]),  # zone: mesmerizes two enemies near the target
        spell('Mind Unmoored Trance', T_DOT, TT_TARGET, [(31, 2, 0, 75)], ticks=3, resist=R_MAGIC),
    ]),
    18: (2022998, 'Phase Strike', [
        spell('Phase Strike', T_NUKE, TT_TARGET, [(0, S(-400), 0, 0), (374, 100, ID(1), 0)]),
        spell('Phased', T_SELF, TT_SELF, [(323, ID(2), 200, 0)], good=1, ticks=3),
        spell('Phase Retort', T_NUKE, TT_TARGET, [(0, S(-300), 0, 0)]),
    ]),
    19: (2015867, 'Event Horizon', [
        spell('Event Horizon', T_DOT, TT_AE_TARGET, [(0, S(-200), 0, 0)], ticks=2, aoe=25),
    ]),
    20: (2024793, 'Void Siphon', [
        spell('Void Siphon', T_GROUP, TT_GROUP, [(294, 5, 100, 0), (15, S(20), 0, 0)], good=1, ticks=3, aoe=100),
    ]),
    21: (2024999, 'Empty Palm', [
        spell('Empty Palm', T_NUKE, TT_TARGET, [(0, S(-650), 0, 0), (374, 100, ID(1), 0)]),
        spell('Empty Palm Stance', T_SELF, TT_SELF, [(279, 10, 0, 0), (177, 20, 0, 0)], good=1, ticks=2),
    ]),
    22: (2029172, 'Dream of the Void', [
        spell('Dream of the Void', T_DOT, TT_TARGET, [(0, S(-250), 0, 0), (2, S(-100), 0, 0)], ticks=4),  # zone: jumps on death
    ]),
    23: (2027818, 'Riftshot', [
        spell('Riftshot', T_NUKE, TT_BEAM, [(0, S(-700), 0, 0)], aoe=60, rng=200),
    ]),
    24: (2026740, 'Thousand Silences', [
        spell('Thousand Silences', T_NUKE, TT_TARGET, [(0, S(-400), 0, 0), (96, 1, 0, 0), (21, 500, 0, 0)], ticks=1),
    ]),
    # Sympathetics: role 0 is the focus (a copy of a set bonus sympathetic with its trigger swapped), 1+ what it fires
    25: (2020496, 'Unravelling Thread', [
        spell('Unravelling Thread', T_SYMP_DMG, None, 'trigger'),
        spell('Unravelling Thread Strand', T_DOT, TT_TARGET, [(0, S(-150), 0, 0)], ticks=3),
    ]),
    26: (2026987, 'Rift Pulse', [
        spell('Rift Pulse', T_SYMP_DMG, None, 'trigger'),
        spell('Rift Pulse Wave', T_NUKE, TT_AE_TARGET, [(0, S(-300), 0, 0)], aoe=20),
    ]),
    27: (2025998, 'Vortex Echo', [
        spell('Vortex Echo', T_SYMP_DMG, None, 'trigger'),
        spell('Vortex Echo Strike', T_NUKE, TT_TARGET, [(0, S(-350), 0, 0), (374, 100, ID(2), 0)]),
        spell('Vortex Echo Return', T_HEAL, TT_SELF, [(15, S(100), 0, 0)], good=1),
    ]),
    28: (2027274, 'Voidfrost', [
        spell('Voidfrost', T_SYMP_DMG, None, 'trigger'),
        spell('Voidfrost Bite', T_NUKE, TT_TARGET, [(0, S(-300), 0, 0), (3, -40, 0, 0)], ticks=2, resist=R_COLD),
    ]),
    29: (2027280, 'Tidewell', [
        spell('Tidewell', T_SYMP_HEAL, None, 'trigger'),
        spell('Tidewell Promise', T_SELF, TT_TARGET, [(289, ID(2), 0, 0)], good=1, ticks=1),
        spell('Tidewell Surge', T_HEAL, TT_SELF, [(0, S(300), 0, 0)], good=1),
    ]),
    30: (2027298, 'Bulwark of Nothing', [
        spell('Bulwark of Nothing', T_SYMP_DMG, None, 'trigger_any'),
        spell('Bulwark of Nothing Rune', T_SELF, TT_SELF, [(55, S(600), 0, 0), (373, ID(2), 0, 0)], good=1, ticks=3),
        spell('Bulwark of Nothing Pulse', T_NUKE, TT_AE_CASTER, [(0, S(-200), 0, 0), (92, S(400), 0, 0)], aoe=30),
    ]),
    31: (2028656, 'Void Ward', [
        spell('Void Ward', T_SYMP_HEAL, None, 'trigger_rare'),
        spell('Void Ward Shelter', T_GROUP, TT_GROUP, [(150, 1, 0, 0)], good=1, ticks=10, aoe=100),
    ]),
    32: (2028657, 'Crystal Ember', [
        spell('Crystal Ember', T_SYMP_DMG, None, 'trigger'),
        spell('Crystal Embers', T_SELF, TT_SELF, [(323, ID(2), 200, 0)], good=1, ticks=3),
        spell('Crystal Shards', T_NUKE, TT_TARGET, [(0, S(-250), 0, 0)]),
    ]),
}

# Zone hooks (zone/nms_weapon_procs.cpp): proc number -> what the zone does
HOOKS = {6: 'execute', 14: 'mark', 16: 'pull', 17: 'trance', 22: 'spread'}


def spell_id(n, role, tier):
    return SPELL_BASE + (n - 1) * 15 + role * 5 + (tier - 1)


def value(v, n, tier):
    if isinstance(v, tuple):
        kind, x = v
        return round(x * TIERS[tier - 1]) if kind == 's' else spell_id(n, x, tier)
    return v


def sql_str(s):
    return "'" + s.replace("'", "''") + "'"


out = ['-- Tier weapon procs and sympathetics (generated by utils/scripts/dynamic_items/gen_weapon_procs.py; edit that).',
       '-- docs/dynamic_items/WEAPON_PROCS.md. Spells load at server start and the item changes need shared_memory:',
       '-- restart after applying. The client needs these spells in spells_us.txt (export them, Sunday patch).', '',
       'START TRANSACTION;', '',
       f'DELETE FROM spells_new WHERE id BETWEEN {SPELL_BASE} AND {spell_id(32, 2, 5)};', '']

for n, (src, title, roles) in PROCS.items():
    out.append(f'-- {n}. {title}')
    for role, sp in enumerate(roles):
        for tier in range(1, 6):
            sid = spell_id(n, role, tier)
            name = f"{sp['name']} {ROMAN[tier - 1]}"
            sets = [f'id = {sid}', f'name = {sql_str(name)}']
            if sp['effects'] in ('trigger', 'trigger_any', 'trigger_rare'):
                # A copy of a set bonus sympathetic: only its trigger spell changes (slot 1 limit)
                sets.append(f'effect_limit_value1 = {spell_id(n, 1, tier)}')
                if sp['effects'] == 'trigger_rare':
                    sets.append('effect_base_value1 = 15')
                if sp['effects'] == 'trigger_any':
                    # No detrimental/beneficial limit: any damage or heal spell
                    sets += ['effectid2 = 254', 'effect_base_value2 = 0', 'effect_limit_value2 = 0']
            else:
                for slot in range(1, 13):
                    if slot <= len(sp['effects']):
                        spa, base, limit, mx = sp['effects'][slot - 1]
                        sets += [f'effectid{slot} = {spa}', f'effect_base_value{slot} = {value(base, n, tier)}',
                                 f'effect_limit_value{slot} = {value(limit, n, tier)}', f'max{slot} = {mx}',
                                 f'formula{slot} = 100']
                    else:
                        sets += [f'effectid{slot} = 254', f'effect_base_value{slot} = 0', f'effect_limit_value{slot} = 0',
                                 f'max{slot} = 0', f'formula{slot} = 100']
                sets += [f"targettype = {sp['tt']}", f"goodEffect = {sp['good']}", f"resisttype = {sp['resist'] if not sp['good'] else 0}",
                         f"buffduration = {sp['ticks']}", f"buffdurationformula = {11 if sp['ticks'] else 0}",
                         f"aoerange = {sp['aoe']}", f"`range` = {sp['rng']}", f"aemaxtargets = {sp['maxtargets']}",
                         f"maxtargets = {sp['maxtargets']}", 'ResistDiff = 0', 'numhits = 0', 'numhitstype = 0',
                         'cast_time = 0', 'mana = 0', 'recast_time = 0', 'recovery_time = 0', 'nimbuseffect = 0',
                         'descnum = 0', 'effectdescnum = 0', 'RecourseLink = 0', 'CastRestriction = 0',
                         'pushback = {}'.format(sp['extra'].get('pushback', 0)), 'pushup = {}'.format(sp['extra'].get('pushup', 0)),
                         "you_cast = ''", "other_casts = ''", "cast_on_you = 'The Void touches you.'",
                         "cast_on_other = ' is touched by the Void.'", "spell_fades = 'The Void lets go.'"]
            sets += [f'classes{c} = 255' for c in range(1, 17)]
            out.append(f"CREATE TEMPORARY TABLE wp SELECT * FROM spells_new WHERE id = {sp['tmpl']};")
            out.append('UPDATE wp SET ' + ', '.join(sets) + ';')
            out.append('INSERT INTO spells_new SELECT * FROM wp;')
            out.append('DROP TEMPORARY TABLE wp;')
    out.append('')

# Items: the tier copies of each source
index = {src: i for i, src in enumerate(tg['weapons'])}
out.append('-- Weapons: each tier copy procs its tier of the new proc, at the normal rate')
out.append('CREATE TABLE IF NOT EXISTS item_sympathetics (item_id INT UNSIGNED NOT NULL PRIMARY KEY, '
           'spell_id INT UNSIGNED NOT NULL) ENGINE=InnoDB;')
symp_rows = []
for n, (src, title, roles) in PROCS.items():
    for tier in range(1, 6):
        lo = tg['item_base'] + (tier - 1) * tg['tier_item_step'] + tg['weapon_block'] + index[src] * 20
        if n <= 24:
            out.append(f'UPDATE items SET proceffect = {spell_id(n, 0, tier)}, proctype = 0, proclevel = 0, proclevel2 = 0, '
                       f'procrate = 20 WHERE id BETWEEN {lo + 1} AND {lo + 16};  -- {title} {ROMAN[tier - 1]}')
        else:
            symp_rows.append((lo, tier, n, title))
out.append(f"DELETE FROM item_sympathetics WHERE spell_id BETWEEN {SPELL_BASE} AND {spell_id(32, 2, 5)};")
for lo, tier, n, title in symp_rows:
    out.append(f'INSERT INTO item_sympathetics (item_id, spell_id) SELECT id, {spell_id(n, 0, tier)} FROM items '
               f'WHERE id BETWEEN {lo + 1} AND {lo + 16};  -- {title} {ROMAN[tier - 1]}')
out += ['', 'COMMIT;', '']

sql_path = os.path.join(ROOT, 'utils', 'scripts', 'rift', 'sql', '2026_11_11_01_tier_weapon_procs.sql')
open(sql_path, 'w', encoding='utf-8', newline='\n').write('\n'.join(out))

# Header for the zone hooks
h = ['// Generated by utils/scripts/dynamic_items/gen_weapon_procs.py; edit that.', '#pragma once', '',
     f'constexpr int NMS_WEAPON_PROC_SPELL_FIRST = {SPELL_BASE};',
     f'constexpr int NMS_WEAPON_PROC_SPELL_LAST = {spell_id(32, 2, 5)};', '',
     '// Proc spell id of (proc number, role, tier 1-5)',
     f'constexpr int NmsWeaponProcSpell(int n, int role, int tier) {{ return {SPELL_BASE} + (n - 1) * 15 + role * 5 + (tier - 1); }}',
     '']
for n, hook in HOOKS.items():
    h.append(f'constexpr int NMS_WEAPON_PROC_{hook.upper()} = {n};  // {PROCS[n][1]}')
open(os.path.join(ROOT, 'zone', 'nms_weapon_procs_ids.h'), 'w', encoding='utf-8', newline='\n').write('\n'.join(h) + '\n')
print(f'wrote {sql_path}: {sum(len(r) for _, _, r in PROCS.values()) * 5} spells, {len(symp_rows) * 16} max sympathetic items')
