# Builds the SQL that sets a zone up for Greater Rifts (instance version 1), plus rift_zones.lua.
# Per zone:
#   - a rift archetype spawn (spawngroup rift_archetypes) at EVERY normal spawn location, 180s +-30s
#     respawn, keeping each location's heading and path grid
#   - the rift controller at the entry point
#   - doors (unlocked), zone points and ground spawns copied from version 0
#   - a version-1 zone row: no flag / level / status requirement, entry point as the safe spot
#
#   python gen_rift_zones.py <eqemu_config.json> <out.sql> <out rift_zones.lua>
#          [--zones a,b,c] [--entry zone:x,y,z ...] [--mysql <path to mysql.exe>]
# Without --zones the default list below is used. --entry overrides an entry point where the zone's
# safe point is unusable (0,0,0); repeat it per zone.
#
# Instance version: each zone uses the first version (from ZONE_VERSION, else 1) that holds no other content
# (no zone row or spawns that aren't the rift's own), so a rift never mixes into an existing version of a zone.
import argparse, json, os, subprocess, sys

ARCHETYPE_GROUP = 5003834      # spawngroup "rift_archetypes" (the four archetype NPC types)
CONTROLLER_GROUP = 5003835     # spawngroup holding the rift controller NPC
VERSION = 1                    # instance version the rift copies live in (rift_config.lua instance_version)
# Zones whose version 1 is already other content (Dreadspire: Demi-Plane of Blood; Nagafen's Lair: The Fabled
# Lord Nagafen) use another version; rift_zones.lua carries it and the Rift Keeper creates the instance at it
ZONE_VERSION = {'dreadspire': 2, 'soldungb': 2}
ENTRY_CLEAR = 40               # no archetype spawns within this many units of the entry point (same floor) ...
ENTRY_CLEAR_Z = 30             # ... where same floor = within this much height
# zones whose entry room is bigger than ENTRY_CLEAR: clear out to the gap before the next spawns
ENTRY_CLEAR_BY_ZONE = {'sleeper': 150, 'dreadspire': 100, 'soldunga': 80, 'soldungb': 90, 'convorteum': 60,
                       'somnium': 140, 'riftseekers': 120, 'chapterhouse': 100}

DEFAULT_ZONES = ['codecay', 'sebilis', 'chardok',
                 'befallen', 'karnor', 'mistmoore', 'fearplane', 'cazicthule', 'charasis', 'permafrost',
                 'sleeper', 'hateplaneb', 'akheva', 'griegsend',
                 'dreadspire', 'soldunga', 'soldungb', 'kael', 'oldkaesoraa', 'anguish', 'frostcrypt',
                 'convorteum', 'kaesora', 'somnium', 'riftseekers', 'chapterhouse', 'ashengate']
# Chapterhouse's safe point is a cramped corner; the arrival point from Shard's Landing opens into the zone
DEFAULT_ENTRY = {'sleeper': (1, -40, 3), 'chapterhouse': (13, -13, 0)}
# Heading on arrival (0 north, 128 east, 256 south, 384 west), facing into the zone and away from the zone
# out. Picked from map walk tests out of the entry point, the zone lines nearby and the game's own arrival
# headings from neighboring zones.
ENTRY_HEADING = {'codecay': 128, 'sebilis': 256, 'chardok': 384, 'befallen': 384, 'karnor': 384,
                 'mistmoore': 0, 'fearplane': 0, 'cazicthule': 128, 'charasis': 128, 'permafrost': 256,
                 'sleeper': 128, 'velketor': 384, 'hateplaneb': 448, 'akheva': 0, 'griegsend': 256,
                 'dreadspire': 336, 'soldunga': 384, 'soldungb': 128, 'kael': 240, 'oldkaesoraa': 128,
                 'anguish': 0, 'frostcrypt': 0, 'convorteum': 0, 'kaesora': 256, 'somnium': 256,
                 'riftseekers': 0, 'chapterhouse': 128, 'ashengate': 0}

ap = argparse.ArgumentParser()
ap.add_argument('config')
ap.add_argument('sql_out')
ap.add_argument('lua_out')
ap.add_argument('--zones', help='comma separated zone short names (default: this server\'s 15)')
ap.add_argument('--entry', action='append', default=[], help='zone:x,y,z entry point override')
ap.add_argument('--mysql', default=r'C:\Program Files\MariaDB 12.3\bin\mysql.exe')
args = ap.parse_args()

cfg = json.load(open(args.config))['server']['database']
sql_out, lua_out = args.sql_out, args.lua_out
env = dict(os.environ, MYSQL_PWD=cfg['password'])
mysql = args.mysql

ZONES = [z.strip() for z in args.zones.split(',')] if args.zones else DEFAULT_ZONES
ENTRY_OVERRIDE = dict(DEFAULT_ENTRY)
for e in args.entry:
    zone, xyz = e.split(':')
    ENTRY_OVERRIDE[zone] = tuple(float(v) for v in xyz.split(','))


def query(sql):
    out = subprocess.run([mysql, '-h', cfg['host'], '-P', str(cfg['port']), '-u', cfg['username'], cfg['db'],
                          '-N', '-B', '-e', sql], env=env, capture_output=True, text=True, check=True).stdout
    return [line.split('\t') for line in out.splitlines()]


def columns(table, skip):
    rows = query(f"SELECT column_name FROM information_schema.columns WHERE table_schema = DATABASE() "
                 f"AND table_name = '{table}' ORDER BY ordinal_position")
    return [r[0] for r in rows if r[0] not in skip]


def copy_version(table, zone_col, zone_val, cols, version, overrides=None):
    overrides = dict(overrides or {}, version=str(version))
    names = ', '.join(f'`{c}`' for c in cols)
    values = ', '.join(overrides.get(c, f'`{c}`') for c in cols)
    return (f"DELETE FROM `{table}` WHERE `{zone_col}` = {zone_val} AND `version` = {version};\n"
            f"INSERT INTO `{table}` ({names}) SELECT {values} FROM `{table}` "
            f"WHERE `{zone_col}` = {zone_val} AND `version` = 0;\n")


door_cols = columns('doors', {'id'})
zp_cols = columns('zone_points', {'id'})
gs_cols = columns('ground_spawns', {'id'})
zone_cols = columns('zone', {'id'})

info = {}
for zid, sn, ln, sx, sy, sz, sh in query(
        "SELECT zoneidnumber, short_name, long_name, safe_x, safe_y, safe_z, safe_heading FROM zone "
        f"WHERE version = 0 AND short_name IN ({', '.join(repr(z) for z in ZONES)})"):
    x, y, z = ENTRY_OVERRIDE.get(sn, (round(float(sx)), round(float(sy)), round(float(sz))))
    info[sn] = dict(id=int(zid), long=ln, x=x, y=y, z=z, h=ENTRY_HEADING.get(sn, 0), v=ZONE_VERSION.get(sn, VERSION))

missing = [z for z in ZONES if z not in info]
if missing:
    sys.exit(f'zones not found: {missing}')

# Versions already holding other content: a zone row without the rift controller, or spawns that aren't the rift's
zone_list = ', '.join(repr(z) for z in ZONES)
ours, theirs, rows_at = {}, {}, set()
for zone, ver, is_rift, cnt in query(
        f"SELECT zone, version, spawngroupID IN ({ARCHETYPE_GROUP}, {CONTROLLER_GROUP}) AS is_rift, COUNT(*) "
        f"FROM spawn2 WHERE version > 0 AND zone IN ({zone_list}) GROUP BY zone, version, is_rift"):
    (ours if int(is_rift) else theirs)[(zone.lower(), int(ver))] = int(cnt)
for zone, ver in query(f"SELECT short_name, version FROM zone WHERE version > 0 AND short_name IN ({zone_list})"):
    rows_at.add((zone.lower(), int(ver)))
for sn, z in info.items():
    v = z['v']
    while (sn, v) in theirs or ((sn, v) in rows_at and (sn, v) not in ours):
        v += 1
    if v != z['v']:
        print(f'{sn}: version {z["v"]} already holds other content, using version {v}')
        z['v'] = v


sql = [f'-- Greater Rift zone setup (instance version {VERSION}, or its ZONE_VERSION). GENERATED by gen_rift_zones.py',
       'START TRANSACTION;', '']
for sn in ZONES:
    z = info[sn]
    q = f"'{sn}'"
    sql.append(f'-- {z["long"]} ({sn}, zone {z["id"]})')
    # Spawns: one rift archetype per distinct normal spawn location (expansion variants share spots)
    v = z['v']
    sql.append(f"DELETE FROM spawn2 WHERE zone = {q} AND version = {v} "
               f"AND spawngroupID IN ({ARCHETYPE_GROUP}, {CONTROLLER_GROUP});")
    sql.append(f"INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, "
               f"path_when_zone_idle, _condition, cond_value, animation, min_expansion, max_expansion) "
               f"SELECT {ARCHETYPE_GROUP}, {q}, {v}, MIN(x), MIN(y), MIN(z), MIN(heading), 180, 30, MAX(pathgrid), "
               f"0, 0, 1, 0, -1, -1 FROM spawn2 WHERE zone = {q} AND version = 0 "
               f"GROUP BY ROUND(x), ROUND(y), ROUND(z) "
               # players zone in on the entry point: keep it clear of spawns on the same floor
               f"HAVING POW(MIN(x) - ({z['x']}), 2) + POW(MIN(y) - ({z['y']}), 2) > {ENTRY_CLEAR_BY_ZONE.get(sn, ENTRY_CLEAR) ** 2} "
               f"OR ABS(MIN(z) - ({z['z']})) > {ENTRY_CLEAR_Z};")
    sql.append(f"INSERT INTO spawn2 (spawngroupID, zone, version, x, y, z, heading, respawntime, variance, pathgrid, "
               f"path_when_zone_idle, _condition, cond_value, animation, min_expansion, max_expansion) "
               f"VALUES ({CONTROLLER_GROUP}, {q}, {v}, {z['x']}, {z['y']}, {z['z']}, {z['h']}, 30, 0, 0, 0, 0, 1, 0, -1, -1);")
    # Rifts can't expect players to carry zone keys: copied doors are unlocked
    sql.append(copy_version('doors', 'zone', q, door_cols, v, {'keyitem': '0', 'lockpick': '0'}).rstrip())
    sql.append(copy_version('zone_points', 'zone', q, zp_cols, v).rstrip())
    sql.append(copy_version('ground_spawns', 'zoneid', z['id'], gs_cols, v).rstrip())
    # Zone row for the rift version: no entry requirements, rift entry point as the safe spot
    overrides = {'version': str(v), 'flag_needed': "''", 'min_level': '0', 'min_status': '0',
                 'safe_x': str(z['x']), 'safe_y': str(z['y']), 'safe_z': str(z['z']), 'safe_heading': str(z['h'])}
    names = ', '.join(f'`{c}`' for c in zone_cols)
    values = ', '.join(overrides.get(c, f'`{c}`') for c in zone_cols)
    sql.append(f"DELETE FROM zone WHERE short_name = {q} AND version = {v};")
    sql.append(f"INSERT INTO zone ({names}) SELECT {values} FROM zone WHERE short_name = {q} AND version = 0;")
    sql.append('')
sql.append('COMMIT;')
open(sql_out, 'w', newline='\n').write('\n'.join(sql) + '\n')

lua = ['-- ============================================================================',
       '-- MASTER RIFT ZONE POOL - GENERATED by gen_rift_zones.py (regenerate rather than hand edit)',
       '-- Deploy to: lua_modules/rift_zones.lua',
       '-- Usage:     local RIFT_ZONE_POOL = require("rift_zones")',
       '--',
       '-- x/y/z/h are the instance entry point and facing (also the zone safe point of the rift version). Every',
       '-- zone here has a rift spawn set at instance version 1 (or `version` where given): one archetype at',
       '-- each normal spawn location, the controller, and copies of the doors, zone points and ground spawns.',
       '-- ============================================================================',
       '',
       'return {']
for sn in ZONES:
    z = info[sn]
    lua.append(f'    {{ zone_id = {z["id"]:<3}, short_name = {json.dumps(sn) + ",":<14} name = {json.dumps(z["long"]) + ",":<38} '
               f'x = {z["x"]}, y = {z["y"]}, z = {z["z"]}, h = {z["h"]}' + (f', version = {z["v"]}' if z['v'] != VERSION else '') + ' },')
lua.append('}')
open(lua_out, 'w', newline='\n').write('\n'.join(lua) + '\n')

print(len(ZONES), 'zones')
