# Adds the Paragon AA names and descriptions to a player's EverQuest client file.
#   python add_dbstr.py <EverQuest folder>/Resources/dbstr_us.txt [dbstr_paragon.txt]
# Lines with the same id and type are replaced; the original is kept as dbstr_us.txt.bak.
import os, shutil, sys

if len(sys.argv) < 2:
    sys.exit(__doc__ or 'usage: add_dbstr.py <dbstr_us.txt> [lines file]')

target = sys.argv[1]
source = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dbstr_paragon.txt')


def key(line):
    parts = line.split('^')
    return (parts[0], parts[1]) if len(parts) > 2 else None


new = [l.rstrip('\r\n') for l in open(source, encoding='latin-1') if l.strip()]
new_keys = {key(l) for l in new}
old = [l.rstrip('\r\n') for l in open(target, encoding='latin-1')]

if not os.path.exists(target + '.bak'):
    shutil.copy2(target, target + '.bak')

kept = [l for l in old if key(l) not in new_keys]
open(target, 'w', encoding='latin-1', newline='\r\n').write('\n'.join(kept + new) + '\n')
print(f'{target}: {len(old) - len(kept)} replaced, {len(new) - (len(old) - len(kept))} added')
