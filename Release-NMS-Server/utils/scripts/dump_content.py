"""Dump Voidforged's world content (no accounts, characters or other player data) to a zip.

    python utils/scripts/dump_content.py [--config Build/bin/Release/eqemu_config.json] [--out database/voidforged-content.zip]

Tables: EQEmu's content and server tables (common/database_schema.h) plus the custom content tables
below. Player, login, state, query-server and *_backup_* tables are never included. The database
password is read from eqemu_config.json and handed to mysqldump in a temporary options file.
The zip also holds spells_us.txt, written by export_client_files from the folder holding the config.
Needs mysqldump (MariaDB's bin folder) on PATH.
"""
import argparse
import json
import os
import re
import subprocess
import tempfile
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))

# Custom content tables EQEmu's schema lists don't know about
CUSTOM_CONTENT = [
    'armor_set_bonuses', 'armor_set_items',                              # set bonuses (zone/client.cpp)
    'dynamic_items', 'dynamic_affix_pool', 'dynamic_affix_suffixes',     # dynamic items (zone/dynamic_items.cpp)
    'progression_atlas', 'progression_flags', 'progression_stages',      # NMS progression
    'rank_ability_mapping',
]

CLIENT_SPELL_ID_LIMIT = 45000


def schema_tables(func):
    src = open(os.path.join(ROOT, 'common', 'database_schema.h'), encoding='utf-8', errors='ignore').read()
    body = re.search(r'std::vector<std::string> ' + func + r'\(\)\s*\{(.*?)\n\t\}', src, re.S).group(1)
    return re.findall(r'"([a-z0-9_]+)"', body)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--config', default=os.path.join(ROOT, 'Build', 'bin', 'Release', 'eqemu_config.json'))
    ap.add_argument('--out', default=os.path.join(ROOT, 'database', 'voidforged-content.zip'))
    args = ap.parse_args()

    db = json.load(open(args.config))['server']['database']
    wanted = schema_tables('GetContentTables') + schema_tables('GetServerTables') + CUSTOM_CONTENT

    with tempfile.TemporaryDirectory() as tmp:
        opts = os.path.join(tmp, 'my.cnf')
        with open(opts, 'w') as f:
            f.write(f"[client]\nhost={db['host']}\nport={db['port']}\nuser={db['username']}\npassword={db['password']}\n")

        present = subprocess.run(['mysql', f'--defaults-extra-file={opts}', '-N', '-e', 'SHOW TABLES', db['db']],
                                 capture_output=True, text=True, check=True).stdout.split()
        tables = [t for t in dict.fromkeys(wanted) if t in present]
        missing = [t for t in dict.fromkeys(wanted) if t not in present]
        if missing:
            print('not in this database, skipped:', ' '.join(missing))

        sql = os.path.join(tmp, 'voidforged-content.sql')
        with open(sql, 'wb') as f:
            subprocess.run(['mysqldump', f'--defaults-extra-file={opts}', '--single-transaction', '--skip-comments',
                            '--no-tablespaces', '--default-character-set=utf8mb4', db['db']] + tables,
                           stdout=f, check=True)

        os.makedirs(os.path.dirname(args.out), exist_ok=True)
        # The client spell file, from the server's own exporter (it rewrites rows; a raw spells_new dump won't match)
        bin_dir = os.path.dirname(os.path.abspath(args.config))
        subprocess.run([os.path.join(bin_dir, 'export_client_files'), 'spells'], cwd=bin_dir, check=True,
                       stdout=subprocess.DEVNULL)

        with zipfile.ZipFile(args.out, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
            z.write(sql, 'voidforged-content.sql')
            # IDs in the 50000s stop spell animations on the client (common/repositories/spells_new_repository.h);
            # filtered here too so an exporter built before that limit can't put them in the snapshot
            with open(os.path.join(bin_dir, 'export', 'spells_us.txt'), 'rb') as f:
                z.writestr('spells_us.txt', b''.join(line for line in f
                                                     if int(line.split(b'^', 1)[0]) < CLIENT_SPELL_ID_LIMIT))
        print(f'{len(tables)} tables, {os.path.getsize(sql) / 1e6:.1f} MB SQL -> {args.out} '
              f'({os.path.getsize(args.out) / 1e6:.1f} MB)')


if __name__ == '__main__':
    main()
