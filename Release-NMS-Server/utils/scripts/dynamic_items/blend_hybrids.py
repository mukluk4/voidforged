# Post-processes the generated pool rows (v26 file + v28 file, rewritten in place):
#  1. Melee DPS (Rogue, Monk, Berserker) get a standout DEX band.
#  2. Hybrids are blended from two templates: 75% of a melee class + 25% of a caster class.
#     Guaranteed lines: range = 0.75 * melee range + 0.25 * caster range (missing = 0-0).
#     Random affixes keep the hybrid's own list; melee-type values scale to 75%, caster-type to 25%.
#     Signature lines keep their full value.
import re
import sys

V26, V28 = sys.argv[1], sys.argv[2]
ROW = re.compile(r"^\((\d+), '(\w+)', (\d), (\d+), (-?\d+), (-?\d+), (\d+)\),?;?$")

DPS = {7, 9, 16}
DPS_DEX = {'dex': (16, 28), 'heroic_dex': (4, 9)}

TEMPLATES = {  # hybrid: (melee 75%, caster 25%)
    3: (1, 2),    # Paladin: Warrior + Cleric
    5: (1, 11),   # Shadowknight: Warrior + Necromancer
    4: (9, 6),    # Ranger: Rogue + Druid
    15: (9, 10),  # Beastlord: Rogue + Shaman
    8: (9, 14),   # Bard: Rogue + Enchanter
}
SIGNATURE = {
    8: {'cha', 'bard_stringed_pct', 'bard_wind_pct', 'bard_brass_pct', 'bard_percussion_pct', 'bard_singing_pct'},
    4: {'archery_crit_chance_pct', 'skill_archery_pct'},
}
CASTER_AFFIXES = {'spell_fire_pct', 'spell_cold_pct', 'spell_magic_pct', 'spell_poison_pct',
                  'spell_disease_pct', 'heal_amount_pct'}
UNSCALED_RANDOM = {'heroic_mr', 'heroic_fr', 'heroic_cr', 'heroic_dr', 'heroic_pr', 'heroic_corruption'}


def rnd(x):
    return int(x + 0.5)


def load(path):
    text = open(path).read()
    pool, sep, rest = (text.partition('\n@@SPLIT@@\n') if '@@SPLIT@@' in text else (text, '', ''))
    rows = []
    for line in pool.split('\n'):
        m = ROW.match(line.strip())
        if m:
            c, a, g, w, mn, mx, sl = m.groups()
            rows.append([int(c), a, int(g), int(w), int(mn), int(mx), int(sl)])
    return rows, sep, rest


def save(path, rows, sep, rest, names):
    out, last_class = [], None
    for r in rows:
        if r[0] != last_class:
            out.append(f'-- {names[r[0]]}')
            last_class = r[0]
        out.append(f"({r[0]}, '{r[1]}', {r[2]}, {r[3]}, {r[4]}, {r[5]}, {r[6]}),")
    body = '\n'.join(out).rstrip(',') + ';'
    open(path, 'w').write(body + (sep + rest if sep else '\n'))


names = {1: 'Warrior', 2: 'Cleric', 3: 'Paladin', 4: 'Ranger', 5: 'Shadowknight', 6: 'Druid', 7: 'Monk',
         8: 'Bard', 9: 'Rogue', 10: 'Shaman', 11: 'Necromancer', 12: 'Wizard', 13: 'Magician',
         14: 'Enchanter', 15: 'Beastlord', 16: 'Berserker'}

rows26, _, _ = load(V26)
rows28, sep28, rest28 = load(V28)
codes26 = {r[1] for r in rows26}

allrows = {(r[0], r[1]): r for r in rows26 + rows28}

# 1. Standout DEX for melee DPS
for c in DPS:
    for code, (lo, hi) in DPS_DEX.items():
        allrows[(c, code)][4:6] = [lo, hi]

# 2. Blend hybrids
def guaranteed(c):
    return {a: r for (cc, a), r in allrows.items() if cc == c and r[2] == 1}

for h, (m, cst) in TEMPLATES.items():
    sig = SIGNATURE.get(h, set())
    mg, cg, hg = guaranteed(m), guaranteed(cst), guaranteed(h)
    for a in set(mg) | set(cg):
        if a in sig and a in hg:
            continue
        ml, mh = (mg[a][4], mg[a][5]) if a in mg else (0, 0)
        cl, ch = (cg[a][4], cg[a][5]) if a in cg else (0, 0)
        lo, hi = rnd(0.75 * ml + 0.25 * cl), rnd(0.75 * mh + 0.25 * ch)
        hi = max(hi, lo)
        if hi == 0:
            continue
        allrows[(h, a)] = [h, a, 1, 0, lo, hi, 0]
    # guaranteed lines the templates don't have, and that aren't signature, go away
    for a in list(hg):
        if a not in mg and a not in cg and a not in sig:
            del allrows[(h, a)]
    # random affixes: scale values
    for (cc, a), r in list(allrows.items()):
        if cc != h or r[2] == 1 or a in sig or a in UNSCALED_RANDOM:
            continue
        f = 0.25 if a in CASTER_AFFIXES else 0.75
        lo, hi = max(1, rnd(r[4] * f)), max(1, rnd(r[5] * f))
        r[4], r[5] = lo, max(hi, lo)

# Hybrids always have less AC than the pure tank: 90% of their blended AC
for h in TEMPLATES:
    r = allrows[(h, 'ac')]
    r[4], r[5] = rnd(r[4] * 0.9), rnd(r[5] * 0.9)

new26 = sorted([r for r in allrows.values() if r[1] in codes26], key=lambda r: r[0])
new28 = sorted([r for r in allrows.values() if r[1] not in codes26], key=lambda r: r[0])
save(V26, new26, '', '', names)
save(V28, new28, sep28, rest28, names)
print(len(new26), 'v26 rows,', len(new28), 'v28 rows')
