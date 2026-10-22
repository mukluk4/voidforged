# Generates migration v28 rows: regular stats (guaranteed, class-weighted from each class's
# heroic weighting) and resists (random affixes, all classes) plus resist name suffixes.
import importlib.util
import sys

spec = importlib.util.spec_from_file_location('gen', sys.argv[1])
gen = importlib.util.module_from_spec(spec)
_print = print
import builtins
builtins.print = lambda *a, **k: None  # the original generator prints its SQL on import
spec.loader.exec_module(gen)
builtins.print = _print

H, M, L = gen.H, gen.M, gen.L
REGULAR = {H: (14, 26), M: (9, 16), L: (3, 8)}
STATS = ['str', 'sta', 'dex', 'agi', 'int', 'wis']   # same order as the heroic tuple
CHA_STRONG = {8, 14}                                 # Bard, Enchanter

RESISTS = [('mr', 30), ('fr', 30), ('cr', 30), ('dr', 30), ('pr', 30), ('corruption', 30)]
HEROIC_RESISTS = [('heroic_mr', 20), ('heroic_fr', 20), ('heroic_cr', 20), ('heroic_dr', 20),
                  ('heroic_pr', 20), ('heroic_corruption', 20)]

rows = []
for cid, (name, hp, ac, mana, end, heroics, pool) in gen.CLASSES.items():
    rows.append(f'-- {name}')
    for stat, band in zip(STATS, heroics):
        lo, hi = REGULAR[band]
        rows.append(f"({cid}, '{stat}', 1, 0, {lo}, {hi}, 0),")
    lo, hi = REGULAR[H] if cid in CHA_STRONG else REGULAR[L]
    rows.append(f"({cid}, 'cha', 1, 0, {lo}, {hi}, 0),")
    for code, weight in RESISTS:
        rows.append(f"({cid}, '{code}', 1, 0, 15, 25, 0),")  # resists: guaranteed, never 0
    for code, weight in HEROIC_RESISTS:
        rows.append(f"({cid}, '{code}', 0, {weight}, 2, 10, 0),")

# Advanced item stats and crits: guaranteed lines. Spell Damage / Heal Amount / Attack are on every
# item (never 0); the rest have ranges starting at 0, so an item may not get them.
MELEE, HYBRID, PRIEST, CASTER = {1, 7, 9, 16}, {3, 4, 5, 8, 15}, {2, 6, 10}, {11, 12, 13, 14}
ALL = MELEE | HYBRID | PRIEST | CASTER


def pick(cid, strong, mid, ranges):
    return ranges[0] if cid in strong else ranges[1] if cid in mid else ranges[2]


SPELL_HEAL = ((10, 30), (5, 15), (1, 5))
ATTACK = ((12, 30), (6, 15), (1, 5))

ZERO_LINES = [  # (code, classes, max)
    ('hp_regen', ALL, 3), ('mana_regen', PRIEST | CASTER | HYBRID, 3), ('end_regen', MELEE | HYBRID, 3),
    ('combat_effects', MELEE | HYBRID, 3), ('spell_shield', ALL, 3), ('shielding', MELEE | HYBRID, 3),
    ('damage_shield', {1, 3, 5}, 3), ('dot_shielding', ALL, 3), ('ds_mitigation', MELEE | HYBRID, 3),
    ('avoidance', ALL, 3), ('accuracy', MELEE | HYBRID, 3), ('stun_resist', ALL, 3),
    ('strike_through', MELEE | HYBRID, 3),
]

# Crits: every class can roll them (0 = none), weighted by how much the class uses them
CRIT_RATE, CRIT_DMG = (6, 4, 2), (10, 6, 3)  # strong / medium / weak maximum
CRITS = [  # (code, strong, medium, bands)
    ('spell_crit_chance_pct', {12, 13}, {11, 14, 6, 10, 4, 5, 15}, CRIT_RATE),
    ('spell_crit_dmg_pct', {12, 13}, {11, 14, 6, 10, 4, 5, 15}, CRIT_DMG),
    ('dot_crit_chance_pct', {11, 6}, {10, 5, 15, 14, 4}, CRIT_RATE),
    ('dot_crit_dmg_pct', {11, 6}, {10, 5, 15, 14, 4}, CRIT_DMG),
    ('heal_crit_chance_pct', {2, 6, 10}, {3, 4, 15}, CRIT_RATE),
    ('hot_crit_chance_pct', {2, 6, 10}, {3, 4, 15}, CRIT_RATE),
    ('melee_crit_chance_pct', MELEE, {3, 4, 5, 15, 8}, CRIT_RATE),
    ('melee_crit_dmg_pct', MELEE, {3, 4, 5, 15, 8}, CRIT_DMG),
    ('archery_crit_chance_pct', {4}, {9}, CRIT_RATE),
]

for cid, (name, *_rest) in gen.CLASSES.items():
    rows.append(f'-- {name}: advanced stats')
    lo, hi = pick(cid, CASTER, PRIEST | HYBRID, SPELL_HEAL)
    rows.append(f"({cid}, 'spelldmg', 1, 0, {lo}, {hi}, 0),")
    lo, hi = pick(cid, PRIEST, {3, 4, 15, 8}, SPELL_HEAL)
    rows.append(f"({cid}, 'healamt', 1, 0, {lo}, {hi}, 0),")
    lo, hi = pick(cid, MELEE | {3, 4, 5, 15}, {8}, ATTACK)
    rows.append(f"({cid}, 'attack', 1, 0, {lo}, {hi}, 0),")
    for code, classes, top in ZERO_LINES:
        if cid in classes:
            rows.append(f"({cid}, '{code}', 1, 0, 0, {top}, 0),")
    for code, strong, mid, bands in CRITS:
        top = pick(cid, strong, mid, bands)
        rows.append(f"({cid}, '{code}', 1, 0, 0, {top}, 0),")

pool_sql ='\n'.join(rows).rstrip(',') + ';'

suffixes = [
    ('mr', 'of Spell Warding'), ('fr', 'of Warmth'), ('cr', 'of Insulation'),
    ('dr', 'of Health'), ('pr', 'of Antivenom'), ('corruption', 'of Purity'),
    ('heroic_mr', 'of the Magus'), ('heroic_fr', 'of the Salamander'), ('heroic_cr', 'of the Yeti'),
    ('heroic_dr', 'of the Healer'), ('heroic_pr', 'of the Serpent'), ('heroic_corruption', 'of the Saint'),
]
suffix_sql = ',\n'.join(f"('{a}', '{s}')" for a, s in suffixes) + ';'

open(sys.argv[2], 'w').write(pool_sql + '\n@@SPLIT@@\n' + suffix_sql)
print(sum(1 for r in rows if r.startswith('(')), 'pool rows,', len(suffixes), 'suffixes')
