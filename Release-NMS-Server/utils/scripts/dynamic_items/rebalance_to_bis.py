# Scales the guaranteed dynamic_affix_pool lines so a full 8-piece dynamic set lands at TARGET x the
# best-in-slot gear for those slots (measured on a Paladin/Monk/Warrior with #dyninfo worn).
#
# HP, AC and the lines that can roll 0 are multiplied, so armor types and roll-0 chances keep their shape.
# Attributes, Spell Damage, Heal Amount and Attack get the same flat amount added for every class, so the
# class leads stay (a Wizard's Spell Damage isn't multiplied by the factor a Warrior's 1-5 needs).
# Left alone: heroics (every piece has all of them by design, BiS doesn't), Mana and Endurance (full vs
# hybrid pools make one target meaningless), Shielding and Stun Resist (already about on target).
#
#   python rebalance_to_bis.py pool.tsv [--sql out.sql]
#   pool.tsv: SELECT class_id,affix,guaranteed,weight,min_value,max_value,slot_mask FROM dynamic_affix_pool
import csv, sys
from collections import defaultdict

TARGET = 1.22               # middle of the 115-130% band
SLOTS  = 7.8                # summed stat_multiplier: boots 1, helm 1, chest 1.25, sleeves 0.9, 2x bracer 0.8, gloves 0.9, legs 1.15
REF    = (1, 3, 7)          # Warrior, Paladin, Monk: the classes the BiS gear was measured on

# Best-in-slot totals for the 8 dynamic slots (first #dyninfo worn screenshot, 2026-10-23)
BIS = {
	'hp': 2970, 'ac': 816, 'attack': 214,
	'str': 350, 'sta': 180, 'dex': 250, 'agi': 274, 'int': 148, 'wis': 174, 'cha': 130,
	'spelldmg': 161, 'healamt': 174, 'hp_regen': 36, 'avoidance': 40, 'accuracy': 64,
}
MULTIPLY = {'hp', 'ac', 'hp_regen', 'avoidance', 'accuracy'}

# Resists use one shared target so no element stands out; heroic resists become guaranteed lines
RESISTS        = ['mr', 'fr', 'cr', 'dr', 'pr']
BIS_RESIST     = (220 + 160 + 290 + 230 + 220) / 5
HEROIC_RESISTS = ['heroic_mr', 'heroic_fr', 'heroic_cr', 'heroic_dr', 'heroic_pr']
BIS_HEROIC_RES = (57 + 42 + 75 + 60 + 57) / 5

rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
for r in rows:
	for k in ('class_id', 'guaranteed', 'weight', 'min_value', 'max_value', 'slot_mask'):
		r[k] = int(r[k])
pool = {(r['class_id'], r['affix']): r for r in rows}

def avg(r):
	return (r['min_value'] + r['max_value']) / 2

def ref_total(affix):
	vals = [avg(pool[(c, affix)]) * SLOTS for c in REF if (c, affix) in pool and pool[(c, affix)]['guaranteed']]
	return sum(vals) / len(vals) if vals else 0

def scaled(lo, hi, f):
	nlo, nhi = round(lo * f), round(hi * f)
	if lo > 0:
		nlo = max(nlo, 1)
	return nlo, max(nhi, nlo)

report, changes = [], {}
for affix, bis in BIS.items():
	cur = ref_total(affix)
	f   = TARGET * bis / cur
	add = round((TARGET * bis - cur) / SLOTS)
	for (c, a), r in pool.items():
		if a == affix and r['guaranteed']:
			rng = scaled(r['min_value'], r['max_value'], f) if affix in MULTIPLY else (r['min_value'] + add, r['max_value'] + add)
			changes[(c, a)] = (1, 0, *rng)
	report.append((affix, bis, cur, f))

# Resists: shared range for all classes (they're the same everywhere today)
per_piece = TARGET * BIS_RESIST / SLOTS
lo, hi    = round(per_piece * 0.8), round(per_piece * 1.2)
for (c, a), r in pool.items():
	if a in RESISTS:
		changes[(c, a)] = (1, 0, lo, hi)
report.append(('resists (each)', BIS_RESIST, avg(pool[(3, 'mr')]) * SLOTS, (lo + hi) / 2 * SLOTS / (avg(pool[(3, 'mr')]) * SLOTS)))

per_piece = TARGET * BIS_HEROIC_RES / SLOTS
hlo, hhi  = round(per_piece * 0.7), round(per_piece * 1.3)
for (c, a), r in pool.items():
	if a in HEROIC_RESISTS:
		changes[(c, a)] = (1, 0, hlo, hhi)
report.append(('heroic resists (each)', BIS_HEROIC_RES, 0, 0))

def new_avg(c, a):
	g, wt, lo_, hi_ = changes.get((c, a), (None, None, pool[(c, a)]['min_value'], pool[(c, a)]['max_value']))
	return (lo_ + hi_) / 2 * SLOTS

print(f'{"stat":<22}{"BiS":>7}{"now":>8}{"now%":>6}{"new":>8}{"new%":>6}   Paladin range now -> new')
for affix, bis, cur, f in report:
	if affix.startswith('resists'):
		print(f'{affix:<22}{bis:>7.0f}{cur:>8.0f}{cur / bis * 100:>5.0f}%{(lo + hi) / 2 * SLOTS:>8.0f}{(lo + hi) / 2 * SLOTS / bis * 100:>5.0f}%   15-25 -> {lo}-{hi}')
		continue
	if affix.startswith('heroic resists'):
		print(f'{affix:<22}{bis:>7.0f}{"~4":>8}{"~5":>5}%{(hlo + hhi) / 2 * SLOTS:>8.0f}{(hlo + hhi) / 2 * SLOTS / bis * 100:>5.0f}%   random 2-10 (weight 20) -> guaranteed {hlo}-{hhi}')
		continue
	new = sum(new_avg(c, affix) for c in REF if (c, affix) in pool) / max(1, sum(1 for c in REF if (c, affix) in pool))
	p   = pool.get((3, affix))
	pr  = f'{p["min_value"]}-{p["max_value"]} -> {changes[(3, affix)][2]}-{changes[(3, affix)][3]}' if p else ''
	print(f'{affix:<22}{bis:>7}{cur:>8.0f}{cur / bis * 100:>5.0f}%{new:>8.0f}{new / bis * 100:>5.0f}%   {pr}')

if '--sql' in sys.argv:
	out = open(sys.argv[sys.argv.index('--sql') + 1], 'w', newline='\n')
	out.write('-- Dynamic items: guaranteed lines scaled to ~122% of best-in-slot (rebalance_to_bis.py)\n')
	out.write('-- Heroic resists move from the random pool to guaranteed lines, which reshuffles existing items\' random picks\n')
	out.write('CREATE TABLE IF NOT EXISTS dynamic_affix_pool_backup_20261023_bis AS SELECT * FROM dynamic_affix_pool;\n')
	for (c, a), (g, wt, lo_, hi_) in sorted(changes.items()):
		out.write(f"UPDATE dynamic_affix_pool SET guaranteed = {g}, weight = {wt}, min_value = {lo_}, max_value = {hi_} WHERE class_id = {c} AND affix = '{a}';\n")
	print(f'\n{len(changes)} rows -> {sys.argv[sys.argv.index("--sql") + 1]}')

# Every class's 8-piece expected total, before -> after
NAMES = {1: 'WAR', 2: 'CLR', 3: 'PAL', 4: 'RNG', 5: 'SHD', 6: 'DRU', 7: 'MNK', 8: 'BRD', 9: 'ROG', 10: 'SHM',
	11: 'NEC', 12: 'WIZ', 13: 'MAG', 14: 'ENC', 15: 'BST', 16: 'BER'}
if '--classes' in sys.argv:
	cols = ['hp', 'ac', 'attack', 'str', 'sta', 'dex', 'agi', 'int', 'wis', 'cha', 'spelldmg', 'healamt', 'mr']
	print('\n8-piece totals per class, now -> new')
	print('    ' + ''.join(f'{c:>12}' for c in cols))
	for c, n in NAMES.items():
		cells = []
		for a in cols:
			if (c, a) not in pool:
				cells.append(f'{"-":>12}')
				continue
			cells.append(f'{avg(pool[(c, a)]) * SLOTS:>5.0f}->{new_avg(c, a):<5.0f}'.rjust(12))
		print(f'{n} ' + ''.join(cells))
