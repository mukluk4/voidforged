# Adds the custom spells the client has to know (set bonus procs it sees cast, e.g. Hail of Arrows I-V) to a
# player's EverQuest spells_us.txt.
#   python add_spells.py <EverQuest folder>/spells_us.txt [tier_gear.json]
# Each spell is a copy of its "copy" spell's line with the new id and name (damage is server-side only).
# Lines with the same id are replaced; the original is kept as spells_us.txt.bak.
import json, os, shutil, sys

if len(sys.argv) < 2:
    sys.exit('usage: add_spells.py <spells_us.txt> [tier_gear.json]')

target = sys.argv[1]
config = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '..', '..', 'dynamic_items', 'tier_gear.json')

spells = [s for s in json.load(open(config, encoding='utf-8'))['custom_spells'] if 'id' in s]
raw = open(target, 'rb').read()
newline = '\r\n' if b'\r\n' in raw else '\n'
lines = [l for l in raw.decode('latin-1').splitlines() if l]
by_id = {int(l.split('^', 1)[0]): l for l in lines}

added = replaced = 0
for s in spells:
    parts = by_id[s['copy']].split('^')
    parts[0], parts[1] = str(s['id']), s['name']
    if s['id'] in by_id:
        replaced += 1
    else:
        added += 1
    by_id[s['id']] = '^'.join(parts)

if not os.path.exists(target + '.bak'):
    shutil.copy2(target, target + '.bak')

ending = newline if raw.endswith(newline.encode()) else ''  # stock file has no final newline; keep it that way
open(target, 'w', encoding='latin-1', newline='').write(newline.join(by_id[i] for i in sorted(by_id)) + ending)
print(f'{target}: {replaced} replaced, {added} added')
