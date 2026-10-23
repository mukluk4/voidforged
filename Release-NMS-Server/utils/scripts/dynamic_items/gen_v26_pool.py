# Generates the starting dynamic_affix_pool rows for migration v26.
# Ranges come from the original dynamic_seed_parser.cpp values.

H, M, L = (3, 8), (2, 5), (1, 3)            # heroic weighting bands (never 0); PoTime gear has no heroics
POOL_FULL, POOL_HYBRID = (200, 800), (120, 480)
AC = {'plate': (32, 52), 'chain': (28, 46), 'leather': (25, 42), 'cloth': (18, 32)}
HP_MELEE, HP_CASTER = (190, 260), (160, 220)

WEAPON, DEFENSE, ABIL_SKILL, ABIL_DMG, SPELL, BARD = (3, 12), (2, 8), (3, 10), (5, 18), (3, 15), (3, 15)

# class_id: (name, hp, ac, mana, endurance, (STR, STA, DEX, AGI, INT, WIS), random pool)
# random pool entries: (affix, (min, max), weight)
def w(a): return (a, WEAPON, 100)
def d(a): return (a, DEFENSE, 100)
def s(a, wt=100): return (a, SPELL, wt)
def b(a): return (a, BARD, 150)

CLASSES = {
	1:  ('Warrior', HP_MELEE, 'plate', None, POOL_FULL, (H, H, M, H, L, L),
		[w('skill_1hs_pct'), w('skill_2hs_pct'), w('skill_1hb_pct'), w('skill_2hb_pct'), w('skill_pierce_pct'),
		 d('skill_defense_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 ('bash_skill_pct', ABIL_SKILL, 100), ('bash_dmg_pct', ABIL_DMG, 100)]),
	2:  ('Cleric', HP_CASTER, 'plate', POOL_FULL, None, (M, M, L, L, L, H),
		[w('skill_1hb_pct'), w('skill_2hb_pct'), d('skill_defense_pct'),
		 s('heal_amount_pct', 250), s('spell_magic_pct')]),
	3:  ('Paladin', HP_MELEE, 'plate', POOL_HYBRID, POOL_HYBRID, (H, H, M, M, L, M),
		[w('skill_1hs_pct'), w('skill_2hs_pct'), w('skill_1hb_pct'), w('skill_2hb_pct'),
		 d('skill_defense_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 ('bash_skill_pct', ABIL_SKILL, 100), ('bash_dmg_pct', ABIL_DMG, 100), s('heal_amount_pct')]),
	4:  ('Ranger', HP_MELEE, 'chain', POOL_HYBRID, POOL_HYBRID, (H, M, H, M, L, M),
		[w('skill_1hs_pct'), w('skill_2hs_pct'), w('skill_pierce_pct'), ('skill_archery_pct', WEAPON, 250),
		 d('skill_defense_pct'), d('skill_parry_pct'), d('skill_riposte_pct'), d('skill_dodge_pct'),
		 s('spell_fire_pct'), s('spell_cold_pct')]),
	5:  ('Shadowknight', HP_MELEE, 'plate', POOL_HYBRID, POOL_HYBRID, (H, H, M, M, M, L),
		[w('skill_1hs_pct'), w('skill_2hs_pct'), w('skill_1hb_pct'), w('skill_2hb_pct'),
		 d('skill_defense_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 ('bash_skill_pct', ABIL_SKILL, 100), ('bash_dmg_pct', ABIL_DMG, 100),
		 s('spell_disease_pct'), s('spell_poison_pct'), s('spell_magic_pct')]),
	6:  ('Druid', HP_CASTER, 'leather', POOL_FULL, None, (L, M, L, L, L, H),
		[w('skill_1hb_pct'), d('skill_defense_pct'),
		 s('spell_fire_pct', 150), s('spell_cold_pct', 150), s('spell_magic_pct'), s('heal_amount_pct', 150)]),
	7:  ('Monk', HP_MELEE, 'leather', None, POOL_FULL, (H, M, M, H, L, L),
		[('skill_h2h_pct', WEAPON, 250), w('skill_1hb_pct'), w('skill_2hb_pct'),
		 d('skill_defense_pct'), d('skill_block_pct'), d('skill_dodge_pct'), d('skill_riposte_pct'),
		 ('flying_kick_skill_pct', ABIL_SKILL, 150), ('flying_kick_dmg_pct', ABIL_DMG, 200)]),
	8:  ('Bard', HP_MELEE, 'plate', POOL_HYBRID, POOL_HYBRID, (M, M, H, M, M, L),
		[w('skill_1hs_pct'), w('skill_pierce_pct'), w('skill_1hb_pct'),
		 d('skill_defense_pct'), d('skill_dodge_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 b('bard_stringed_pct'), b('bard_wind_pct'), b('bard_brass_pct'), b('bard_percussion_pct'), b('bard_singing_pct')]),
	9:  ('Rogue', HP_MELEE, 'chain', None, POOL_FULL, (M, M, H, H, L, L),
		[('skill_pierce_pct', WEAPON, 250), w('skill_1hs_pct'), w('skill_1hb_pct'),
		 d('skill_defense_pct'), d('skill_dodge_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 ('backstab_skill_pct', ABIL_SKILL, 150), ('backstab_dmg_pct', ABIL_DMG, 250)]),
	10: ('Shaman', HP_CASTER, 'chain', POOL_FULL, None, (M, M, L, L, L, H),
		[w('skill_1hb_pct'), w('skill_2hb_pct'), d('skill_defense_pct'),
		 s('spell_cold_pct'), s('spell_poison_pct'), s('spell_disease_pct'), s('heal_amount_pct', 150)]),
	11: ('Necromancer', HP_CASTER, 'cloth', POOL_FULL, None, (L, M, L, L, H, L),
		[s('spell_fire_pct'), s('spell_poison_pct', 150), s('spell_disease_pct', 150), s('spell_magic_pct')]),
	12: ('Wizard', HP_CASTER, 'cloth', POOL_FULL, None, (L, M, L, L, H, L),
		[s('spell_fire_pct', 150), s('spell_cold_pct', 150), s('spell_magic_pct', 150)]),
	13: ('Magician', HP_CASTER, 'cloth', POOL_FULL, None, (L, M, L, L, H, L),
		[s('spell_fire_pct', 200), s('spell_magic_pct', 150), s('spell_cold_pct')]),
	14: ('Enchanter', HP_CASTER, 'cloth', POOL_FULL, None, (L, M, L, L, H, L),
		[s('spell_magic_pct', 200), s('spell_poison_pct'), s('spell_cold_pct')]),
	15: ('Beastlord', HP_MELEE, 'leather', POOL_HYBRID, POOL_HYBRID, (H, M, M, M, L, M),
		[('skill_h2h_pct', WEAPON, 200), w('skill_1hb_pct'), w('skill_2hb_pct'), w('skill_pierce_pct'),
		 d('skill_block_pct'), d('skill_dodge_pct'), d('skill_parry_pct'), d('skill_riposte_pct'),
		 s('spell_cold_pct'), s('spell_poison_pct'), s('heal_amount_pct')]),
	16: ('Berserker', HP_MELEE, 'chain', None, POOL_FULL, (H, H, M, M, L, L),
		[w('skill_2hs_pct'), w('skill_2hb_pct'),  # no 1H: Berserkers can't use one-handed weapons
		 d('skill_defense_pct'), d('skill_parry_pct'), d('skill_riposte_pct'), d('skill_dodge_pct'),
		 ('frenzy_skill_pct', ABIL_SKILL, 150), ('frenzy_dmg_pct', ABIL_DMG, 250)]),
}

HEROICS = ['heroic_str', 'heroic_sta', 'heroic_dex', 'heroic_agi', 'heroic_int', 'heroic_wis']

out = []
for cid, (name, hp, ac, mana, end, heroics, pool) in CLASSES.items():
	rows = [('hp', hp, 1, 0), ('ac', AC[ac], 1, 0)]
	if mana: rows.append(('mana', (190, 260) if mana == POOL_FULL else (120, 170), 1, 0))
	if end:  rows.append(('endurance', (150, 210) if end == POOL_FULL else (100, 140), 1, 0))
	rows += [(h, r, 1, 0) for h, r in zip(HEROICS, heroics)]
	rows += [(a, r, 0, wt) for a, r, wt in pool]
	out.append(f'-- {name}')
	vals = [f"({cid}, '{a}', {g}, {wt}, {r[0]}, {r[1]}, 0)" for a, r, g, wt in rows]
	out.append(',\n'.join(vals) + ',')

sql = '\n'.join(out).rstrip(',') + ';'
print(sql)
