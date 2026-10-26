# Builds lua_modules/rift_appearances.lua: per rift zone, one known-good look per creature race that
# normally spawns in that zone. The client only draws models loaded for the zone it's in (anything
# else shows as the default placeholder model), so a zone's own creatures always render there.
# GLOBAL_RACES (models the client loads in every zone, from its Resources/GlobalLoad.txt) are added
# to every zone's pool.
import json, os, re, subprocess, sys
from collections import Counter, defaultdict

cfg = json.load(open(sys.argv[1]))['server']['database']
out_path = sys.argv[2]
env = dict(os.environ, MYSQL_PWD=cfg['password'])
mysql = r'C:\Program Files\MariaDB 12.3\bin\mysql.exe'

ZONES = ['codecay', 'sebilis', 'chardok',
         'befallen', 'karnor', 'mistmoore', 'fearplane', 'cazicthule', 'charasis', 'permafrost',
         'sleeper', 'velketor', 'hateplaneb', 'akheva', 'griegsend']

# The client's Resources/GlobalLoad.txt loads these zones' whole model files (<zone>_chr) in every
# zone, so every creature that spawns in them renders anywhere
GLOBAL_ZONES = ['poair', 'hohonorb', 'ponightmare', 'poinnovation', 'freportn']

# Rift Guardian looks per zone: newer (.eqg) models no zone here spawns, so the Guardian stands out. Their
# model files are added to the client's GlobalLoad.txt (client patch). Body types: 1 humanoid, 2 lycanthrope,
# 3 undead, 4 giant, 5 construct, 6 extraplanar, 22 insect, 24 summoned, 26 dragon.
# Zones not listed fall back to their own named NPCs.
def boss(race, name, size, bodytype, gender=2, texture=0):
    return dict(race=race, gender=gender, texture=texture, helmtexture=0, size=size, bodytype=bodytype, name=name)

FALLEN_KNIGHT = lambda: boss(719, 'Fallen Knight', 8, 3)
SPECTRE       = lambda: boss(485, 'Spectre', 9, 3)
GHOUL         = lambda: boss(571, 'Ghoul Lord', 8, 3)
BANSHEE       = lambda: boss(487, 'Banshee', 8, 3)
IKSAR_SKEL    = lambda: boss(606, 'Iksar Bonelord', 8, 3)
IKSAR_GHOST   = lambda: boss(605, 'Iksar Revenant', 8, 3)
VAMPIRE       = lambda: boss(497, 'Vampire Lord', 8, 3, gender=0)
GARGOYLE      = lambda: boss(464, 'Gargoyle', 9, 5)
AMYGDALAN     = lambda: boss(663, 'Amygdalan', 9, 6)
CAZIC         = lambda: boss(670, 'Avatar of Fear', 12, 6)
DARK_LORD     = lambda: boss(466, 'Dark Lord', 9, 6)
GELIDRAN      = lambda: boss(417, 'Gelidran', 10, 24)

CURATED_BOSSES = {
    'codecay':    [FALLEN_KNIGHT(), SPECTRE(), GHOUL()],
    'sebilis':    [IKSAR_SKEL(), IKSAR_GHOST(), boss(604, 'Dracolich', 10, 3)],
    'chardok':    [boss(610, 'Sarnak Warlord', 8, 1), boss(413, 'Dragorn', 9, 1), GARGOYLE()],
    'befallen':   [FALLEN_KNIGHT(), BANSHEE(), GHOUL()],
    'karnor':     [VAMPIRE(), FALLEN_KNIGHT(), IKSAR_SKEL()],
    'mistmoore':  [VAMPIRE(), boss(454, 'Werewolf', 9, 2), GARGOYLE()],
    'fearplane':  [CAZIC(), AMYGDALAN(), DARK_LORD()],
    'cazicthule': [CAZIC(), IKSAR_GHOST(), AMYGDALAN()],
    'charasis':   [IKSAR_GHOST(), boss(491, 'Bone Golem', 10, 3), SPECTRE()],
    'permafrost': [boss(523, 'Frost Giant', 14, 4), GELIDRAN(), boss(626, 'Rallosian Giant', 14, 4)],
    'sleeper':    [boss(604, 'Dracolich', 15, 3), boss(530, 'Ancient Dragon', 15, 26), boss(572, 'Clockwork Guardian', 12, 5)],
    'velketor':   [boss(441, 'Spider Queen', 10, 22), GELIDRAN(), SPECTRE()],
    'hateplaneb': [DARK_LORD(), boss(351, 'Knight of Hate', 8, 1, gender=0), boss(352, 'Arcanist of Hate', 8, 1, gender=0)],
    'akheva':     [boss(722, 'Akheva Overlord', 8, 1, gender=0), boss(723, 'Servant of Shadow', 9, 6), boss(708, 'Alaran Ghost', 8, 3)],
    'griegsend':  [boss(588, 'Grieg Phantom', 8, 3, gender=0), boss(659, 'Marionette', 8, 5), BANSHEE()],
}

# Tallest anything may stand in tight zones: regulars get 3/4 of it, Executioners 9/10, the Guardian all of it.
# Zones not listed keep the defaults (Executioners up to 16, the Guardian at least 12).
SIZE_CAP = {
    'befallen': 7,
    'cazicthule': 7,
}
# Models that stand too tall for a capped zone at any size (Treant): left out of those zones' pools
CAP_SKIP_RACES = [244]

PLAYER_RACES = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 128, 130, 330, 522}
SKIP_RACES = {0, 127, 240}           # none / invisible man / untargetable
SKIP_BODYTYPES = {11, 66, 67}        # no target, invisible

# Objects and water creatures look wrong walking around a dungeon
SKIP_WORDS = re.compile(r'\b(trap|vessel|chest|crate|barrel|box|totem|statue|pillar|portal|door|lever|coffin|'
                        r'urn|altar|banner|flag|tent|cauldron|egg|fish|shark|eel|piranha|whale|dolphin|'
                        r'barracuda|seahorse|kraken|jellyfish|squid|crustacean|lobster|crab|clam|'
                        r'trigger|spawner|controller|marker|effect|cloud|light|boat|ship|vase|rack|pawn|'
                        r'sunflower|small mushroom|pile|cluster|model|turtle|anglerfish|minnow|longnose|'
                        r'succulent|seashell|shellfish|pool)\b', re.I)


def query(sql):
    return [line.split('\t') for line in subprocess.run(
        [mysql, '-h', cfg['host'], '-P', str(cfg['port']), '-u', cfg['username'], cfg['db'], '-N', '-B', '-e', sql],
        env=env, capture_output=True, text=True, check=True).stdout.splitlines()]


def clean_name(name):
    # Generic mobs only ("a_rock_golem"); named NPCs like "Trakanon" make odd trash names
    if not re.match(r'^(a|an)_', name):
        return None
    name = re.sub(r'^(a|an)_', '', name)
    name = re.sub(r'\d+$', '', name).replace('_', ' ').strip()
    return name.title() if re.fullmatch(r"[A-Za-z' -]{3,24}", name) else None


skip = ','.join(map(str, SKIP_RACES | PLAYER_RACES))
skip_bt = ','.join(map(str, SKIP_BODYTYPES))
zone_list = ', '.join(repr(z) for z in ZONES)

# Every creature spawning normally (version 0) in a rift zone, any level: these models load there
rows = query(f"""
SELECT s.zone, n.race, n.gender, n.texture, n.helmtexture, n.size, n.name, n.bodytype
FROM npc_types n
JOIN spawnentry se ON se.npcID = n.id
JOIN spawn2 s ON s.spawngroupID = se.spawngroupID
WHERE s.version = 0 AND s.zone IN ({zone_list}) AND n.untargetable = 0 AND n.class < 20
  AND n.race NOT IN ({skip}) AND n.bodytype NOT IN ({skip_bt})""")

looks = defaultdict(lambda: defaultdict(Counter))   # zone -> race -> look
names = defaultdict(lambda: defaultdict(Counter))   # zone -> race -> name
# race -> body type (Undead, Animal, Summoned...), most common across the database so spells that
# only affect one body type treat the look correctly
body_votes = defaultdict(Counter)
for race, bodytype, cnt in query(f"SELECT race, bodytype, COUNT(*) FROM npc_types WHERE bodytype NOT IN ({skip_bt}) "
                                 f"GROUP BY race, bodytype"):
    body_votes[int(race)][int(bodytype)] += int(cnt)
bodytype_of = {race: votes.most_common(1)[0][0] for race, votes in body_votes.items()}

for zone, race, gender, texture, helm, size, name, _bodytype in rows:
    zone, race = zone.lower(), int(race)
    looks[zone][race][(int(gender), int(texture), int(helm), round(float(size)))] += 1
    n = clean_name(name)
    if n:
        names[zone][race][n] += 1


# Fallback names: each race's most common generic ("a_...") name anywhere in the database
any_names = defaultdict(Counter)
for race, name, cnt in query(f"SELECT race, name, COUNT(*) FROM npc_types WHERE name REGEXP '^an?_' "
                             f"AND race NOT IN ({skip}) GROUP BY race, name"):
    n = clean_name(name)
    if n:
        any_names[int(race)][n] += int(cnt)


def pick(zone_looks, zone_names, race):
    # A Guardian look is never a regular or elite mob anywhere, so the Guardian always stands out
    if race in BOSS_RACES:
        return None
    if not zone_names[race]:
        zone_names[race] = any_names[race]
    if not zone_names[race] or any(SKIP_WORDS.search(n) for n, _ in zone_names[race].most_common(3)):
        return None
    (gender, texture, helm, size), _ = zone_looks[race].most_common(1)[0]
    return dict(race=race, gender=gender, texture=texture, helmtexture=helm, size=max(3, min(size, 12)),
                bodytype=bodytype_of.get(race, 1), name=zone_names[race].most_common(1)[0][0])


# Boss looks: each zone's named NPCs (no "a_"/"an_" prefix), highest level first, one per race.
# The Rift Guardian appears as an "Anachronism of <named>" in that zone's own boss models.
boss_rows = query(f"""
SELECT s.zone, n.race, n.gender, n.texture, n.helmtexture, n.size, n.name, n.level
FROM npc_types n
JOIN spawnentry se ON se.npcID = n.id
JOIN spawn2 s ON s.spawngroupID = se.spawngroupID
WHERE s.version = 0 AND s.zone IN ({zone_list}) AND n.untargetable = 0 AND n.class < 20 AND n.level >= 45
  AND n.name NOT REGEXP '^an?_' AND n.race NOT IN ({skip}) AND n.bodytype NOT IN ({skip_bt})
ORDER BY n.level DESC""")

# Custom NPCs spawned in most zones, and things that aren't creatures
BOSS_SKIP = re.compile(r'^echo of the past$|\b(fluid|stones?|remains)\b', re.I)

bosses = defaultdict(list)
for zone, race, gender, texture, helm, size, name, level in boss_rows:
    zone, race = zone.lower(), int(race)
    n = re.sub(r'\d+$', '', name.lstrip('#')).replace('_', ' ').strip()
    if not re.fullmatch(r"[A-Za-z' -]{3,30}", n) or SKIP_WORDS.search(n) or BOSS_SKIP.search(n):
        continue
    n = ' '.join(w if w.lower() in ('of', 'the') and i else w[:1].upper() + w[1:] for i, w in enumerate(n.split()))
    if any(b['race'] == race for b in bosses[zone]) or len(bosses[zone]) >= 6:
        continue
    bosses[zone].append(dict(race=race, gender=int(gender), texture=int(texture), helmtexture=int(helm),
                             size=max(8, min(round(float(size)), 20)), bodytype=bodytype_of.get(race, 1), name=n))

# Curated bosses replace the zone's own named NPCs
for zone, entries in CURATED_BOSSES.items():
    bosses[zone] = entries

BOSS_RACES = {b['race'] for zone in ZONES for b in bosses[zone]}


# Global models: the creatures of the globally loaded zones, with the looks they have there
global_entries = []
if GLOBAL_ZONES:
    g_rows = query(f"""
SELECT n.race, n.gender, n.texture, n.helmtexture, n.size, n.name FROM npc_types n
JOIN spawnentry se ON se.npcID = n.id JOIN spawn2 s ON s.spawngroupID = se.spawngroupID
WHERE s.version = 0 AND s.zone IN ({', '.join(repr(z) for z in GLOBAL_ZONES)})
  AND n.untargetable = 0 AND n.class < 20 AND n.race NOT IN ({skip}) AND n.bodytype NOT IN ({skip_bt})""")
    g_looks, g_names = defaultdict(Counter), defaultdict(Counter)
    for race, gender, texture, helm, size, name in g_rows:
        g_looks[int(race)][(int(gender), int(texture), int(helm), round(float(size)))] += 1
        n = clean_name(name)
        if n:
            g_names[int(race)][n] += 1
    global_entries = [e for e in (pick(g_looks, g_names, r) for r in sorted(g_looks)) if e]


def lua_entry(e):
    return (f'{{ race = {e["race"]}, gender = {e["gender"]}, texture = {e["texture"]}, '
            f'helmtexture = {e["helmtexture"]}, size = {e["size"]}, bodytype = {e["bodytype"]}, '
            f'name = {json.dumps(e["name"])} }}')


out = ['-- ============================================================================',
       '-- RIFT APPEARANCE POOLS (rift_appearances.lua) - GENERATED by gen_rift_appearances.py, do not hand edit',
       '-- Deploy to: lua_modules/rift_appearances.lua',
       '-- zones[short_name]: one look per creature race that normally spawns in that zone. The client',
       '-- only draws models loaded for the current zone, so these always render there.',
       '-- global: models the client loads in every zone (from Resources/GlobalLoad.txt), used everywhere.',
       '-- ============================================================================',
       '',
       'local M = { zones = {}, bosses = {}, global = {}, size_cap = {}, cap_skip_races = {} }',
       '']
counts = {}
for zone in ZONES:
    entries = [e for e in (pick(looks[zone], names[zone], r) for r in sorted(looks[zone])) if e]
    counts[zone] = len(entries)
    out.append(f'M.zones["{zone}"] = {{')
    out += [f'    {lua_entry(e)},' for e in entries]
    out.append('}')
    out.append('')
out.append('-- bosses[short_name]: named NPC models from that zone, for the Rift Guardian')
out.append('M.bosses = {}')
for zone in ZONES:
    out.append(f'M.bosses["{zone}"] = {{')
    out += [f'    {lua_entry(e)},' for e in bosses[zone]]
    out.append('}')
out.append('')
out.append('M.global = {')
out += [f'    {lua_entry(e)},' for e in global_entries]
out.append('}')
out.append('')
out.append('-- size_cap[short_name]: tallest model allowed in tight zones (see SIZE_CAP in the generator)')
for zone, cap in SIZE_CAP.items():
    out.append(f'M.size_cap["{zone}"] = {cap}')
out.append('-- cap_skip_races[race]: never used in a capped zone (too tall at any size)')
for race in CAP_SKIP_RACES:
    out.append(f'M.cap_skip_races[{race}] = true')
out.append('')
out.append('return M')
open(out_path, 'w', newline='\n').write('\n'.join(out) + '\n')

print('global:', len(global_entries))
for zone in ZONES:
    print(f'{zone:12} {counts[zone]:3} models, bosses: ' + ', '.join(b['name'] for b in bosses[zone]))
