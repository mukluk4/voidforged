# Platinum and AA sinks: writes the SQL for
#   - Chronal Tributes: tributes 60-63 with 5 tiers each, one per band of 100 rift tiers, and their benefit items
#   - Paragon AAs: long, rising-cost AA lines (HP, mana, endurance, AC, attack, melee damage, heroic stats)
#   - the Chronal Artificer in The Void, where #reforge is used
# and the client strings (dbstr_us.txt lines) for the Paragon AAs.
#
#   python utils/scripts/rift/gen_sinks.py
#
# Chronal Tribute tiers stack: tier III turns on bands I-III (rift tiers 1-300) and gives the benefit of the
# rift's own band. Writes sql/2026_10_27_01_plat_aa_sinks.sql and client/dbstr_paragon.txt next to this script.
# Nothing touches the database. The SQL can be rerun: it deletes its own rows first.
# Code side: zone/nms_chronal_tribute.cpp, zone/nms_paragon.cpp, zone/gm_commands/reforge.cpp.
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SQL_OUT = os.path.join(HERE, 'sql', '2026_10_27_01_plat_aa_sinks.sql')
DBSTR_OUT = os.path.join(HERE, 'client', 'dbstr_paragon.txt')

# ----------------------------------------------------------------------------- Chronal Tributes
BAND = 100                # rift tiers per tribute tier (Custom:ChronalTributeBandSize)
FAVOR_PER_TIER = 100      # favor per rift tier per run (Custom:ChronalTributeFavorPerTier)
TRIBUTE_TIERS = 5         # bands 1-100 ... 401-500 (rift cap 500)
TRIBUTE_ITEM_BASE = 980000
TRIBUTE_ITEM_TEMPLATE = 56300  # "Benefit: Mana Regeneration 01": copied for the item flags and icon
FORTUNE_PCT_PER_TIER = 5      # rift_config.lua reward.fortune_pct_per_tier

# id, name, description lead, stats per tribute tier (column -> amount at tier 1; tier N gets N x)
TRIBUTES = [
    (60, 'Chronal Might', 'Strength drawn from Fractures you have already closed sharpens your blows.',
     {'attack': 5, 'heroic_str': 3, 'heroic_dex': 3, 'hp': 50}),
    (61, 'Chronal Arcana', 'The unspent magic of collapsed timelines answers your spells.',
     {'spelldmg': 10, 'healamt': 10, 'mana': 50, 'heroic_int': 3, 'heroic_wis': 3}),
    (62, 'Chronal Bulwark', 'Moments that never happened harden around you.',
     {'ac': 5, 'hp': 75, 'heroic_sta': 3, 'heroic_mr': 2, 'heroic_fr': 2, 'heroic_cr': 2, 'heroic_dr': 2, 'heroic_pr': 2}),
    (63, 'Chronal Fortune', 'More of each Fracture\'s shattered time clings to you as it closes.',
     {}),
]

STAT_NAMES = {
    'attack': 'Attack', 'heroic_str': 'Heroic Strength', 'heroic_dex': 'Heroic Dexterity', 'hp': 'HP',
    'spelldmg': 'Spell Damage', 'healamt': 'Heal Amount', 'mana': 'Mana', 'heroic_int': 'Heroic Intelligence',
    'heroic_wis': 'Heroic Wisdom', 'ac': 'AC', 'heroic_sta': 'Heroic Stamina', 'heroic_mr': 'Heroic Magic Resist',
    'heroic_fr': 'Heroic Fire Resist', 'heroic_cr': 'Heroic Cold Resist', 'heroic_dr': 'Heroic Disease Resist',
    'heroic_pr': 'Heroic Poison Resist',
}

ROMAN = ['I', 'II', 'III', 'IV', 'V']

# ----------------------------------------------------------------------------- Paragon AAs
AA_ABILITY_BASE = 17800
AA_RANK_BASE = 19000      # also the client string ids (title_sid / desc_sid = the line's first rank id)
AA_LEVEL = 51
NMS_SPA = {'heroic_str': 9001, 'heroic_sta': 9002, 'heroic_agi': 9003, 'heroic_dex': 9004, 'heroic_int': 9005, 'heroic_wis': 9006}

# name, effect id, base2, per rank, ranks, cost of rank 1, cost added per rank, what it gives (for the text)
PARAGON = [
    ('Paragon of Vitality',  69,  0,  25,  50, 5,  1, '{} hit points'),
    ('Paragon of the Mind',  97,  0,  25,  50, 5,  1, '{} mana'),
    ('Paragon of Stamina',   190, 0,  25,  50, 5,  1, '{} endurance'),
    ('Paragon of Fortitude', 1,   0,  1,   50, 5,  1, '{} armor class'),
    ('Paragon of Might',     2,   0,  1,   50, 5,  1, '{} attack'),
    ('Paragon of Ferocity',  185, -1, 1,   10, 25, 10, '{}% melee damage'),
    ('Paragon of Strength',     NMS_SPA['heroic_str'], 0, 1, 50, 15, 3, '{} Heroic Strength'),
    ('Paragon of Endurance',    NMS_SPA['heroic_sta'], 0, 1, 50, 15, 3, '{} Heroic Stamina'),
    ('Paragon of Agility',      NMS_SPA['heroic_agi'], 0, 1, 50, 15, 3, '{} Heroic Agility'),
    ('Paragon of Dexterity',    NMS_SPA['heroic_dex'], 0, 1, 50, 15, 3, '{} Heroic Dexterity'),
    ('Paragon of Intellect',    NMS_SPA['heroic_int'], 0, 1, 50, 15, 3, '{} Heroic Intelligence'),
    ('Paragon of Wisdom',       NMS_SPA['heroic_wis'], 0, 1, 50, 15, 3, '{} Heroic Wisdom'),
]

# ----------------------------------------------------------------------------- Chronal Artificer
ARTIFICER_ID = 1200050
ARTIFICER_TEMPLATE = 1200010  # Chronal_Quartermaster
ARTIFICER_SPAWN = (86.0, -30.0, 4.2, 420.0)  # thevoida, beside Zebuxoruk (84.7, -48.7)


def q(s):
    return "'" + s.replace('\\', '\\\\').replace("'", "''") + "'"


def tribute_sql():
    out = ['-- ---------------------------------------------------------------- Chronal Tributes']
    first, last = TRIBUTES[0][0], TRIBUTES[-1][0]
    out.append(f'DELETE FROM tribute_levels WHERE tribute_id BETWEEN {first} AND {last};')
    out.append(f'DELETE FROM tributes WHERE id BETWEEN {first} AND {last};')
    out.append(f'DELETE FROM items WHERE id BETWEEN {TRIBUTE_ITEM_BASE} AND {TRIBUTE_ITEM_BASE + 99};')
    out.append('DROP TEMPORARY TABLE IF EXISTS sink_item;')

    for n, (tid, name, lead, stats) in enumerate(TRIBUTES):
        tiers = []
        for t in range(1, TRIBUTE_TIERS + 1):
            item_id = TRIBUTE_ITEM_BASE + n * 10 + t
            lo, hi = (t - 1) * BAND + 1, t * BAND
            item_name = f'Benefit: {name} {ROMAN[t - 1]}'
            sets = [f'id = {item_id}', f'Name = {q(item_name)}', f'lore = {q(item_name)}', 'favor = 0']
            sets += [f'{col} = {amount * t}' for col, amount in stats.items()]
            out.append(f'CREATE TEMPORARY TABLE sink_item SELECT * FROM items WHERE id = {TRIBUTE_ITEM_TEMPLATE};')
            out.append(f'UPDATE sink_item SET {", ".join(sets)};')
            out.append('INSERT INTO items SELECT * FROM sink_item;')
            out.append('DROP TEMPORARY TABLE sink_item;')

            if stats:
                benefit = ', '.join(f'{amount * t} {STAT_NAMES[col]}' for col, amount in stats.items())
            else:
                benefit = f'{FORTUNE_PCT_PER_TIER * t}% more Chronal Shards from a clear'
            tiers.append(f'{ROMAN[t - 1]} (rift tiers {lo}-{hi}): {benefit}')  # lo..hi is the band this tier adds
            # level is the character level the tier needs (all are open; it must be unique per tribute)
            out.append(f'INSERT INTO tribute_levels (tribute_id, level, cost, item_id) VALUES ({tid}, {t}, {FAVOR_PER_TIER * hi}, {item_id});')

        descr = (
            f'{lead}<br>Works only inside a Greater Rift. Tiers stack: tier III also turns on I and II, so it works in rift tiers 1-{3 * BAND}, '
            "giving the benefit of the rift's band. "
            f'Instead of a cost every 10 minutes, it takes {FAVOR_PER_TIER} favor x the rift tier once per run, on entering the rift. '
            f'The cost shown is for the top of the band.<br>Benefit -<br>' + '<br>'.join(tiers)
        )
        # unknown = the number of tiers (sent to the client as the tier count)
        out.append(f'INSERT INTO tributes (id, unknown, name, descr, isguild) VALUES ({tid}, {TRIBUTE_TIERS}, {q(name)}, {q(descr)}, 0);')
    return out


def paragon_sql():
    out = ['', '-- ---------------------------------------------------------------- Paragon AAs']
    out.append(f'DELETE FROM aa_rank_effects WHERE rank_id BETWEEN {AA_RANK_BASE} AND {AA_RANK_BASE + 1999};')
    out.append(f'DELETE FROM aa_rank_prereqs WHERE rank_id BETWEEN {AA_RANK_BASE} AND {AA_RANK_BASE + 1999};')
    out.append(f'DELETE FROM aa_ranks WHERE id BETWEEN {AA_RANK_BASE} AND {AA_RANK_BASE + 1999};')
    out.append(f'DELETE FROM aa_ability WHERE id BETWEEN {AA_ABILITY_BASE} AND {AA_ABILITY_BASE + 99};')
    dbstr = []
    rank_id = AA_RANK_BASE
    summary = []
    for n, (name, effect, base2, per, ranks, cost1, step, text) in enumerate(PARAGON):
        ability_id = AA_ABILITY_BASE + n
        first = rank_id
        out.append(
            'INSERT INTO aa_ability (id, name, category, classes, races, drakkin_heritage, deities, status, type, charges, '
            'grant_only, first_rank_id, enabled, reset_on_death, auto_grant_enabled) VALUES '
            f'({ability_id}, {q(name)}, -1, 65535, 65535, 127, 131071, 0, 1, 0, 0, {first}, 1, 0, 0);'
        )
        rows, effects = [], []
        total = 0
        for r in range(1, ranks + 1):
            cost = cost1 + step * (r - 1)
            total += cost
            prev_id = rank_id - 1 if r > 1 else -1
            next_id = rank_id + 1 if r < ranks else -1
            rows.append(f'({rank_id}, -1, -1, {first}, {first}, {cost}, {AA_LEVEL}, -1, 0, 0, 0, {prev_id}, {next_id})')
            # A rank's effect is the total at that rank
            effects.append(f'({rank_id}, 1, {effect}, {per * r}, {base2})')
            rank_id += 1
        out.append(
            'INSERT INTO aa_ranks (id, upper_hotkey_sid, lower_hotkey_sid, title_sid, desc_sid, cost, level_req, spell, '
            'spell_type, recast_time, expansion, prev_id, next_id) VALUES\n  ' + ',\n  '.join(rows) + ';'
        )
        out.append('INSERT INTO aa_rank_effects (rank_id, slot, effect_id, base1, base2) VALUES\n  ' + ',\n  '.join(effects) + ';')
        gain = text.format(f'+{per}')
        dbstr.append(f'{first}^1^{name}^0')
        dbstr.append(
            f'{first}^4^Each rank grants {gain}, up to {text.format(f"+{per * ranks}")} at rank {ranks}. '
            f'Rank 1 costs {cost1} points and each rank after costs {step} more ({total:,} points for all {ranks}).^0'
        )
        summary.append(f'-- {name}: {gain}/rank, {ranks} ranks, {cost1}-{cost1 + step * (ranks - 1)} AA/rank, {total:,} AA total')
    return out, dbstr, summary


def artificer_sql():
    x, y, z, h = ARTIFICER_SPAWN
    return ['', '-- ---------------------------------------------------------------- Chronal Artificer (#reforge)',
            f"DELETE s FROM spawn2 s JOIN spawnentry e ON e.spawngroupID = s.spawngroupID WHERE e.npcID = {ARTIFICER_ID};",
            f"DELETE g FROM spawngroup g JOIN spawnentry e ON e.spawngroupID = g.id WHERE e.npcID = {ARTIFICER_ID};",
            f'DELETE FROM spawnentry WHERE npcID = {ARTIFICER_ID};',
            f'DELETE FROM npc_types WHERE id = {ARTIFICER_ID};',
            'DROP TEMPORARY TABLE IF EXISTS sink_npc;',
            f'CREATE TEMPORARY TABLE sink_npc SELECT * FROM npc_types WHERE id = {ARTIFICER_TEMPLATE};',
            f"UPDATE sink_npc SET id = {ARTIFICER_ID}, name = 'Chronal_Artificer', lastname = 'Reforging', class = 1, "
            "merchant_id = 0, alt_currency_id = 0;",
            'INSERT INTO npc_types SELECT * FROM sink_npc;',
            'DROP TEMPORARY TABLE sink_npc;',
            'SET @sg = (SELECT MAX(id) + 1 FROM spawngroup);',
            f"INSERT INTO spawngroup (id, name, spawn_limit, dist, max_x, min_x, max_y, min_y, delay, mindelay, despawn, despawn_timer, wp_spawns) "
            f"VALUES (@sg, 'rift_artificer_{ARTIFICER_ID}', 0, 0, 0, 0, 0, 0, 45000, 15000, 0, 100, 0);",
            f'INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES (@sg, {ARTIFICER_ID}, 100);',
            "INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, _condition, cond_value, animation) "
            f"VALUES (@sg, 'thevoida', 0, {x}, {y}, {z}, {h}, 30, 0, 0, 0, 1, 0);"]


def main():
    paragon, dbstr, summary = paragon_sql()
    lines = ['-- Platinum and AA sinks (generated by utils/scripts/rift/gen_sinks.py; edit that, not this)',
             '-- Chronal Tributes 60-63 (items 980001-980035), Paragon AAs (abilities 17800+, ranks 19000+),',
             '-- the Chronal Artificer (npc 1200050) in The Void. Rerunnable. Needs shared_memory and a zone restart.',
             *summary, '']
    lines += tribute_sql() + paragon + artificer_sql()
    os.makedirs(os.path.dirname(SQL_OUT), exist_ok=True)
    os.makedirs(os.path.dirname(DBSTR_OUT), exist_ok=True)
    open(SQL_OUT, 'w', newline='\n').write('\n'.join(lines) + '\n')
    open(DBSTR_OUT, 'w', newline='\r\n').write('\n'.join(dbstr) + '\n')
    print(f'wrote {SQL_OUT}\nwrote {DBSTR_OUT}')
    print('\n'.join(summary))


if __name__ == '__main__':
    main()
