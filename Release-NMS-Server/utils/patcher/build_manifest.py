"""Build the VoidPatcher manifest and object store from a tested reference client folder.

    python build_manifest.py --client <client dir> --out <publish dir> --version 2026.10.28.1
        [--eqhost host:port] [--patcher VoidPatcher.exe] [--message "text"] [--rules patch_rules.txt]

Writes <out>/manifest.txt and <out>/objects/<md5[:2]>/<md5>. Objects already in <out> are kept, so
re-running after a client change only adds the new files. Upload objects/ first and manifest.txt last,
so players never see a manifest that names files the host does not have yet.

LEGO brick models never ship (user rule, 2026-11-12). The brick install keeps each file it replaced as
brick_orig/<path>.orig; when that folder exists, the original is shipped in place of the brick file.
"""
import argparse
import fnmatch
import hashlib
import json
import os
import shutil
import sys


def load_rules(path):
    rules = []
    for n, line in enumerate(open(path, encoding='utf-8'), 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        kind, _, pattern = line.partition(' ')
        if kind not in ('exclude', 'ifmissing', 'delete', 'stray') or not pattern:
            sys.exit(f'{path}:{n}: bad rule: {line}')
        if kind == 'stray' and '/' in pattern:
            sys.exit(f'{path}:{n}: stray patterns cover the top folder only: {line}')
        rules.append((kind, pattern.strip().lower()))
    return rules


def match(rules, rel):
    low = rel.lower()
    for kind, pattern in rules:
        if kind in ('delete', 'stray'):
            continue
        if fnmatch.fnmatchcase(low, pattern):
            return kind
        # "dir/**" also covers the files directly in dir
        if pattern.endswith('/**') and fnmatch.fnmatchcase(low, pattern[:-3] + '/*'):
            return kind
    return 'ship'


def md5_file(path):
    h = hashlib.md5()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def store(out, md5, src=None, data=None):
    dst = os.path.join(out, 'objects', md5[:2], md5)
    if os.path.exists(dst):
        return False
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    tmp = dst + '.tmp'
    if data is not None:
        with open(tmp, 'wb') as f:
            f.write(data)
    else:
        shutil.copyfile(src, tmp)
    os.replace(tmp, dst)
    return True


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser()
    ap.add_argument('--client', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--version', required=True)
    ap.add_argument('--rules', default=os.path.join(here, 'patch_rules.txt'))
    ap.add_argument('--eqhost', help='login server host:port written into eqhost.txt')
    ap.add_argument('--patcher', help='VoidPatcher.exe to offer as a self-update')
    ap.add_argument('--message', action='append', default=[], help='line shown in the patcher window')
    args = ap.parse_args()

    client = os.path.abspath(args.client)
    out = os.path.abspath(args.out)
    if out.lower().startswith(client.lower() + os.sep):
        sys.exit('--out must not be inside the client folder')
    rules = load_rules(args.rules)
    os.makedirs(out, exist_ok=True)

    # Brick files installed in the reference client: path -> the original it replaced
    originals = {}
    brick_dir = os.path.join(client, 'brick_orig')
    if os.path.isdir(brick_dir):
        for root, _, files in os.walk(brick_dir):
            for name in files:
                if name.lower().endswith('.orig'):
                    full = os.path.join(root, name)
                    rel = os.path.relpath(full, brick_dir).replace(os.sep, '/')[:-len('.orig')]
                    originals[rel.lower()] = (rel, full)

    # Hashes are cached by size and write time so a rebuild only reads what changed
    cache_path = os.path.join(out, 'build_cache.json')
    try:
        cache = json.load(open(cache_path, encoding='utf-8'))
    except (OSError, ValueError):
        cache = {}

    entries, skipped = [], {'exclude': 0}
    new_objects = new_bytes = 0
    swapped = set()

    def add(rel, full, kind):
        nonlocal new_objects, new_bytes
        key = rel + ('|orig' if rel.lower() in originals else '')
        st = os.stat(full)
        c = cache.get(key)
        if c and c[0] == st.st_size and c[1] == st.st_mtime_ns:
            md5 = c[2]
        else:
            md5 = md5_file(full)
            cache[key] = [st.st_size, st.st_mtime_ns, md5]
        if store(out, md5, src=full):
            new_objects += 1
            new_bytes += st.st_size
        entries.append(('I' if kind == 'ifmissing' else 'F', md5, st.st_size, rel))

    for root, dirs, files in os.walk(client):
        dirs.sort()
        for name in sorted(files):
            full = os.path.join(root, name)
            rel = os.path.relpath(full, client).replace(os.sep, '/')
            if '\t' in rel or '\n' in rel:
                sys.exit(f'unsupported character in file name: {rel!r}')
            if rel.lower().startswith('brick_orig/'):
                skipped['exclude'] += 1
                continue
            kind = match(rules, rel)
            if kind == 'exclude':
                skipped['exclude'] += 1
                continue
            if args.eqhost and rel.lower() == 'eqhost.txt':
                continue
            if rel.lower() in originals:
                swapped.add(rel.lower())
                full = originals[rel.lower()][1]
            add(rel, full, kind)

    # An original whose brick replacement isn't in the client any more still ships
    for low, (rel, full) in sorted(originals.items()):
        kind = match(rules, rel)
        if low not in swapped and kind != 'exclude':
            swapped.add(low)
            add(rel, full, kind)

    if args.eqhost:
        data = f'[LoginServer]\r\nHost={args.eqhost}\r\n'.encode('ascii')
        md5 = hashlib.md5(data).hexdigest()
        store(out, md5, data=data)
        entries.append(('F', md5, len(data), 'eqhost.txt'))

    lines = ['# VoidPatcher manifest', f'version\t{args.version}']
    lines += [f'message\t{m}' for m in args.message]
    if args.patcher:
        md5 = md5_file(args.patcher)
        if store(out, md5, src=args.patcher):
            new_objects += 1
        lines.append(f'patcher\t{md5}\t{os.path.getsize(args.patcher)}')
    lines += [f'D\t{p}' for k, p in rules if k == 'delete']
    lines += [f'S\t{p}' for k, p in rules if k == 'stray']
    lines += [f'{k}\t{m}\t{s}\t{p}' for k, m, s, p in sorted(entries, key=lambda e: e[3].lower())]

    tmp = os.path.join(out, 'manifest.txt.tmp')
    with open(tmp, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    os.replace(tmp, os.path.join(out, 'manifest.txt'))
    json.dump(cache, open(cache_path, 'w', encoding='utf-8'))

    total = sum(e[2] for e in entries)
    print(f'files {len(entries)} ({total / (1 << 30):.2f} GB), install-if-missing {sum(e[0] == "I" for e in entries)}, '
          f'excluded {skipped["exclude"]}, new objects {new_objects} ({new_bytes / (1 << 20):.1f} MB)')
    if swapped:
        print(f'brick files left out: shipped the {len(swapped)} originals from brick_orig/ instead')


if __name__ == '__main__':
    main()
