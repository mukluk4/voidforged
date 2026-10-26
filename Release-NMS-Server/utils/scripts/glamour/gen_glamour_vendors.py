# Sorts every "Glamour - '<item>'" ornament into vendor lists: plate, chain and leather vendors (all slots),
# exotic vendors per slot, robes, plus weapon vendors by weapon type. Each vendor sells one glamour per
# distinct look (armor: material + a fixed palette of tints; weapons: model), so every list fits a merchant
# window (500).
#
#   python gen_glamour_vendors.py <eqemu_config.json> report
#   python gen_glamour_vendors.py <eqemu_config.json> sql <out.sql> <first_npc_id> <merchant_id_base> <x> <y> <z> <heading>
#   --review <glamour_review.xlsx>  (any mode) leaves out rows set to Keep = N and names matching the Patterns tab
import json, os, subprocess, sys
from collections import defaultdict

review_path = None
if '--review' in sys.argv:
    i = sys.argv.index('--review')
    review_path = sys.argv[i + 1]
    del sys.argv[i:i + 2]

cfg = json.load(open(sys.argv[1]))['server']['database']
mode = sys.argv[2]
env = dict(os.environ, MYSQL_PWD=cfg['password'])
mysql = r'C:\Program Files\MariaDB 12.3\bin\mysql.exe'
WINDOW = 500


def query(sql):
    out = subprocess.run([mysql, '-h', cfg['host'], '-P', str(cfg['port']), '-u', cfg['username'], cfg['db'],
                          '-N', '-B', '-e', sql], env=env, capture_output=True, text=True, check=True).stdout
    return [line.split('\t') for line in out.splitlines()]


SLOTS = [(4, 'Helms'), (131072, 'Chests'), (128, 'Sleeves'), (1536, 'Bracers'), (4096, 'Gloves'),
         (262144, 'Leggings'), (524288, 'Boots')]
ARMOR_TYPES = {3: 'Plate', 2: 'Chain', 1: 'Leather'}      # anything else: 'Exotic'
ROBES = range(10, 17)                                     # chest materials 10-16 are robes

WEAPON_TYPES = {0: '1H Slashing', 3: '1H Blunt', 2: '1H Piercing', 8: 'Shields',
                1: 'Two-Handed', 4: 'Two-Handed', 35: 'Two-Handed',
                45: 'Fist & Ranged', 5: 'Fist & Ranged', 7: 'Fist & Ranged'}

# Several items often share one look (e.g. "Small Platemail Helm" and "Platemail Helm"). The vendor sells
# one per look: a Medium or plain-named one if there is any, else the lowest item ID.
import re
SIZED = re.compile(r"^Glamour - '(Small|Large)\s")


def pick_rank(gid, name):
    return (1 if SIZED.match(name) else 0, gid)


# Armor textures the RoF2 client may not have (untested, so left out), and Hero's Forge models (not wanted)
SKIP_MATERIALS = {5, 7, 8, 24, 240}

# Tints: most textures come in hundreds of near-identical tints. Each texture keeps the untinted look plus the
# tint closest to each palette color, chosen only from tints that exist in every slot of that texture, so a
# full set can always be bought in one matching color.
NATURAL = {0, 0xFF000000}
PALETTE = [0x141414, 0xDCDCDC, 0xB41414, 0x1432B4, 0x149614, 0xD2A01E, 0x781496]  # black, white, red, blue,
                                                                                  # green, gold, purple
# Plate, chain and leather are down to a few looks per slot, so each sells every slot from one vendor
MERGED_TYPES = {'Plate', 'Chain', 'Leather'}


def rgb_dist(a, b):
    return sum((((a >> s) & 255) - ((b >> s) & 255)) ** 2 for s in (16, 8, 0))


# Armor glamours: one per (slot, armor type, material, tint)
lists = defaultdict(list)
best = {}
for gid, name, slots, material, color in query(
        "SELECT id, Name, slots, material, color FROM items WHERE Name LIKE 'Glamour - %' "
        "AND NOT (slots & 26624) AND herosforgemodel = 0 ORDER BY id"):
    gid, slots, material, color = int(gid), int(slots), int(material), int(color)
    slot_name = next((n for bit, n in SLOTS if slots & bit), None)
    if not slot_name or material in SKIP_MATERIALS:
        continue
    if slot_name == 'Chests' and material in ROBES:
        kind = 'Robes'
    else:
        kind = ARMOR_TYPES.get(material, 'Exotic')
    key = (kind, slot_name, material, color)
    if key not in best or pick_rank(gid, name) < best[key]:
        best[key] = pick_rank(gid, name)

slot_tints = defaultdict(lambda: defaultdict(set))   # material -> slot -> tints
for kind, slot_name, material, color in best:
    slot_tints[material][slot_name].add(color)
tint_order = {}                                      # material -> {tint: palette position}
for material, by_slot in slot_tints.items():
    coverage = defaultdict(int)
    for tints in by_slot.values():
        for t in tints:
            coverage[t] += 1
    pool = [t for t, n in coverage.items() if n == max(coverage.values())]
    order = {t: -1 for t in pool if t in NATURAL}
    rest = [t for t in pool if t not in NATURAL]
    for i, target in enumerate(PALETTE):
        if rest:
            order.setdefault(min(rest, key=lambda t: rgb_dist(t, target)), i)
    tint_order[material] = order

slot_index = {n: i for i, (_, n) in enumerate(SLOTS)}
picked = [(key, gid) for key, (_r, gid) in best.items() if key[3] in tint_order[key[2]]]
# Vendor order: slot (helm to boots), then texture, then palette color
for (kind, slot_name, material, color), gid in sorted(
        picked, key=lambda kv: (slot_index[kv[0][1]], kv[0][2], tint_order[kv[0][2]][kv[0][3]])):
    if kind == 'Robes' or kind in MERGED_TYPES:
        vendor = kind
    else:
        vendor = f'{kind} {slot_name}'
    lists[vendor].append(gid)

# Weapon glamours: one per model (idfile), typed by the weapon the glamour copies
src_type = {name: int(t) for name, t in query(
    "SELECT Name, MIN(itemtype) FROM items WHERE itemtype IN (0,1,2,3,4,5,7,8,35,45) AND Name IN ("
    "SELECT SUBSTRING(Name, 12, CHAR_LENGTH(Name) - 12) FROM items WHERE Name LIKE 'Glamour - %' "
    "AND (slots & 26624)) GROUP BY Name")}
best_models = {}
for gid, name, idfile in query(
        "SELECT id, Name, idfile FROM items WHERE Name LIKE 'Glamour - %' AND (slots & 26624) "
        "AND idfile NOT IN ('IT0', 'IT63') ORDER BY id"):           # IT0: no model, IT63: dropped-item bag
    gid = int(gid)
    src = name[11:-1]
    vendor = WEAPON_TYPES.get(src_type.get(src), 'Unusual Weapons')
    key = (vendor, idfile)
    if key not in best_models or pick_rank(gid, name) < best_models[key]:
        best_models[key] = pick_rank(gid, name)
for (vendor, _f), (_r, gid) in sorted(best_models.items(), key=lambda kv: kv[1][1]):
    lists[vendor].append(gid)

# Review sheet edits: Keep = N rows, and Patterns (case-insensitive name substring, on one vendor or all)
if review_path:
    from openpyxl import load_workbook
    wb = load_workbook(review_path, read_only=True)
    dropped = set()
    for ws in wb.worksheets:
        if ws.title in ('How to use', 'Summary', 'Patterns'):
            continue
        for row in ws.iter_rows(min_row=2, values_only=True):
            if row and row[0] is not None and str(row[4]).strip().upper() == 'N':
                dropped.add(int(row[0]))
    rules = [(str(p).lower(), (str(v).strip() if v else '')) for p, v, *_ in
             wb['Patterns'].iter_rows(min_row=2, values_only=True) if p]
    if rules:
        names = dict(query("SELECT id, Name FROM items WHERE Name LIKE 'Glamour - %'"))
    for vendor, ids in lists.items():
        lists[vendor] = [i for i in ids if i not in dropped and not any(
            p in names[str(i)].lower() and (not v or v == vendor) for p, v in rules)]

# Anything over a merchant window gets split into numbered vendors
vendors = []
# Walking order: each armor type helm to boots, then robes, then weapons
ORDER = (['Plate', 'Chain', 'Leather'] + [f'Exotic {slot}' for _, slot in SLOTS] + ['Robes'] +
         ['1H Slashing', '1H Blunt', '1H Piercing', 'Two-Handed', 'Shields', 'Fist & Ranged', 'Unusual Weapons'])
for name in sorted(lists, key=lambda n: (ORDER.index(n) if n in ORDER else len(ORDER), n)):
    ids = lists[name]
    parts = [ids[i:i + WINDOW] for i in range(0, len(ids), WINDOW)]
    for i, part in enumerate(parts):
        vendors.append((name if len(parts) == 1 else f'{name} {i + 1}', part))

if mode == 'json':
    # vendor lists with item names, for the review workbook (build_review_sheet.py)
    names = dict(query("SELECT id, Name FROM items WHERE Name LIKE 'Glamour - %'"))
    json.dump([{'vendor': v, 'items': [[i, names.get(str(i), '')] for i in ids]} for v, ids in vendors],
              open(sys.argv[3], 'w'), indent=0)
    print('wrote', sys.argv[3])
    sys.exit(0)

if mode == 'report':
    for name, ids in vendors:
        print(f'{name:28} {len(ids)}')
    print(len(vendors), 'vendors,', sum(len(i) for _, i in vendors), 'glamours')
    sys.exit(0)

# ---------------------------------------------------------------------------------------------------
# sql mode: vendors lined up along both walls of a corridor, facing its center line.
#   sql <out.sql> <zone> <first_id> <end1 "y,x,z"> <end2 "y,x,z"> <half_width>
# The two ends are /loc readings from the middle of the floor (/loc prints Y, X, Z); half_width is how
# far from the center line the vendors stand. NPC, merchant list and spawngroup IDs are first_id + n.
import math

PRICE_COPPER = 1000 * 1000          # 1,000 platinum
TEMPLATE_NPC = 1120001103           # Purveyor_of_Glamour (bazaar): copied look
RESERVED_IDS = 100                  # first_id .. first_id + 99 belong to the glamour vendors


def eq_heading(x1, y1, x2, y2):
    """Heading (0-512) from point 1 toward point 2, same as CalculateHeadingAngleBetweenPositions."""
    x_diff, y_diff = abs(x1 - x2), max(abs(y1 - y2), 0.000001)
    angle = math.degrees(math.atan2(x_diff, y_diff))
    k = 511.5 / 360.0
    if y1 >= y2:
        if x2 >= x1:
            return (180.0 - angle) * k
        return (angle + 180.0) * k
    if x2 > x1:
        return angle * k
    return (360.0 - angle) * k


#   row <out.sql> <zone> <first_id> <end1 "x,y,z"> <end2 "x,y,z"> <heading>
# One row along a wall between two #loc points (GM #loc prints X, Y, Z), all facing <heading>.
if mode in ('sql', 'row'):
    out_path, zone, first_id = sys.argv[3], sys.argv[4], int(sys.argv[5])
    n = len(vendors)
    spots = []
    if mode == 'row':
        (ax, ay, az), (bx, by, bz) = ([float(v) for v in p.split(',')] for p in (sys.argv[6], sys.argv[7]))
        heading = float(sys.argv[8])
        for i in range(n):
            t = i / (n - 1)
            spots.append((ax + (bx - ax) * t, ay + (by - ay) * t, az + (bz - az) * t, heading))
    else:
        (ay, ax, az), (by, bx, bz) = ([float(v) for v in p.split(',')] for p in (sys.argv[6], sys.argv[7]))
        half_width = float(sys.argv[8])

        # Both walls, front to back, vendors in list order alternating sides so a pair faces each other
        per_side = math.ceil(n / 2)
        dx, dy = bx - ax, by - ay
        length = math.hypot(dx, dy)
        ux, uy = dx / length, dy / length          # along the corridor
        px, py = -uy, ux                           # perpendicular
        for i in range(n):
            row, side = divmod(i, 2)
            t = (row + 0.5) / per_side
            cx, cy, cz = ax + dx * t, ay + dy * t, az + (bz - az) * t
            sgn = 1 if side == 0 else -1
            x, y = cx + px * half_width * sgn, cy + py * half_width * sgn
            spots.append((x, y, cz, eq_heading(x, y, cx, cy)))

    npc_cols = [r[0] for r in query("SELECT column_name FROM information_schema.columns WHERE table_schema = "
                                    "DATABASE() AND table_name = 'npc_types' ORDER BY ordinal_position")]
    # The whole reserved block is cleared, so vendors from an older, longer list don't linger
    last_id = first_id + RESERVED_IDS - 1
    sql = ['-- Glamour vendors. GENERATED by gen_glamour_vendors.py', 'START TRANSACTION;', '',
           f'-- Every glamour costs 1,000 platinum (merchants refuse to buy them back: client_packet.cpp)',
           f"UPDATE items SET price = {PRICE_COPPER} WHERE Name LIKE 'Glamour - %';", '',
           f'DELETE FROM merchantlist WHERE merchantid BETWEEN {first_id} AND {last_id};',
           f'DELETE FROM spawn2 WHERE spawngroupID BETWEEN {first_id} AND {last_id};',
           f'DELETE FROM spawnentry WHERE spawngroupID BETWEEN {first_id} AND {last_id};',
           f'DELETE FROM spawngroup WHERE id BETWEEN {first_id} AND {last_id};',
           f'DELETE FROM npc_types WHERE id BETWEEN {first_id} AND {last_id};',
           # Zone:StateSavingOnShutdown restores a zone's saved NPCs on boot, so old vendors would come back
           # beside the new ones. Apply this with the zone shut down, or it saves them again on shutdown.
           f"DELETE FROM zone_state_spawns WHERE instance_id = 0 AND zone_id = "
           f"(SELECT zoneidnumber FROM zone WHERE short_name = '{zone}' AND version = 0 LIMIT 1);", '']
    for i, ((vname, ids), (x, y, z, h)) in enumerate(zip(vendors, spots)):
        vid = first_id + i
        overrides = {'id': str(vid), 'name': "'Glamour_Outfitter'", 'lastname': repr(vname[:32]),
                     'class': '41', 'merchant_id': str(vid), 'loottable_id': '0', 'npc_faction_id': '0'}
        cols = ', '.join(f'`{c}`' for c in npc_cols)
        vals = ', '.join(overrides.get(c, f'`{c}`') for c in npc_cols)
        sql.append(f'-- {vname} ({len(ids)} looks)')
        sql.append(f'INSERT INTO npc_types ({cols}) SELECT {vals} FROM npc_types WHERE id = {TEMPLATE_NPC};')
        for start in range(0, len(ids), 100):
            chunk = ids[start:start + 100]
            sql.append('INSERT INTO merchantlist (merchantid, slot, item) VALUES ' +
                       ', '.join(f'({vid}, {start + k + 1}, {item})' for k, item in enumerate(chunk)) + ';')
        sql.append(f"INSERT INTO spawngroup (id, name) VALUES ({vid}, 'glamour_outfitter_{vid}');")
        sql.append(f'INSERT INTO spawnentry (spawngroupID, npcID, chance) VALUES ({vid}, {vid}, 100);')
        sql.append(f"INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, "
                   f"pathgrid) VALUES ({vid}, '{zone}', 0, {x:.2f}, {y:.2f}, {z:.2f}, {h:.1f}, 640, 0, 0);")
        sql.append('')
    sql.append('COMMIT;')
    open(out_path, 'w', newline='\n').write('\n'.join(sql) + '\n')
    print(f'{n} vendors written to {out_path}')
