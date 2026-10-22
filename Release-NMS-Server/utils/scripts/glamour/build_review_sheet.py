# Builds glamour_review.xlsx from the vendor lists (gen_glamour_vendors.py ... json <file>): one tab per
# vendor, a Keep column (Y/N) to edit, a first-pass flag for likely junk, a Patterns tab for name rules,
# and a Summary tab counting what each vendor keeps. gen_glamour_vendors.py reads it back with --review.
#   python build_review_sheet.py <vendors.json> <out.xlsx>
import json, re, sys
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill
from openpyxl.worksheet.datavalidation import DataValidation

vendors = json.load(open(sys.argv[1]))
out = sys.argv[2]

FONT = Font(name='Arial', size=10)
BOLD = Font(name='Arial', size=10, bold=True)
HEAD_FILL = PatternFill('solid', start_color='D9D9D9')
EDIT_FILL = PatternFill('solid', start_color='FFFF00')       # cells to edit
DROP_FILL = PatternFill('solid', start_color='F4CCCC')       # pre-set to drop

# First pass: strong = default N, review = flagged but kept
STRONG = [
    (re.compile(r'\b(test|placeholder|unused|deprecated|do not use|prototype)\b', re.I), 'test/placeholder'),
    (re.compile(r'\b(potion|dose|scroll|tome|spell:|elixir|draught)\b', re.I), 'not gear'),
    (re.compile(r'\b(u|ur|h8|4)\b|<3|!', re.I), 'joke name'),
    (re.compile(r'[A-Z]{2,}\d+|\d{3,}'), 'model/part number'),
    # Basic merchant metals; Steel is only dropped as the leading word (Singing/Blued/Frosted Steel stay)
    (re.compile(r'\bBronze\b', re.I), 'basic metal: bronze'),
    (re.compile(r'^(Fine\s+)?Steel\b', re.I), 'basic metal: steel'),
]
REVIEW = re.compile(r'\b(rusty|tattered|cracked|bent|broken|dented|moldy|mouldy|splintered|flawed|worn|damaged|'
                    r'crude|rough|warped|chipped|tarnished|ragged|frayed|patchwork|makeshift)\b', re.I)


# Tiered gear families: one look per piece is enough, keep only the top tier on each vendor.
# Tiers listed lowest to highest.
TIER_FAMILIES = {
    'Defiant': ['Crude', 'Rough', 'Flawed', 'Simple', 'Ornate', 'Intricate', 'Elaborate', 'Elegant'],
}


def tier_of(name):
    """(family, base piece name, tier rank) for a tiered family item, else None"""
    for family, tiers in TIER_FAMILIES.items():
        for rank, tier in enumerate(tiers):
            m = re.match(r'(?:' + tier + r')\s+(' + family + r'(?![A-Za-z]).*)$', name)
            if m:
                return family, m.group(1), rank
    return None


def lower_tiers(looks):
    """Names in one vendor's list that a higher tier of the same piece outranks"""
    best = {}
    for name in looks:
        t = tier_of(name)
        if t:
            key = (t[0], t[1])
            best[key] = max(best.get(key, -1), t[2])
    return {name for name in looks if (t := tier_of(name)) and t[2] < best[(t[0], t[1])]}


# Size variants ("Small Cloth Cap", "Medium Cloth Cap", "Large Cloth Cap", "Cloth Cap"): keep Medium and
# the plain name; drop Small/Large when either exists. If only Small/Large exist, keep the largest.
SIZE_RE = re.compile(r'^(Small|Medium|Large)\s+(.+)$')
SIZE_RANK = {'Small': 0, 'Large': 1}


def size_drops(looks):
    groups = {}
    for name in looks:
        m = SIZE_RE.match(name)
        size, base = (m.group(1), m.group(2)) if m else ('', name)
        groups.setdefault(base, []).append((size, name))
    drops = {}
    for base, members in groups.items():
        sizes = {sz for sz, _ in members}
        if not (sizes & {'Small', 'Large'}):
            continue
        if sizes & {'Medium', ''}:
            for sz, name in members:
                if sz in ('Small', 'Large'):
                    drops[name] = f'{sz.lower()} size (keeping medium/plain)'
        else:
            best = max(sizes, key=SIZE_RANK.get)
            for sz, name in members:
                if sz != best:
                    drops[name] = f'{sz.lower()} size (keeping {best.lower()})'
    return drops


def first_pass(name, lower=frozenset(), sized=None):
    if name in lower:
        return f'lower {tier_of(name)[0]} tier', 'N'
    if sized and name in sized:
        return sized[name], 'N'
    for rx, why in STRONG:
        if rx.search(name):
            return why, 'N'
    m = REVIEW.search(name)
    if m:
        return f'low-tier word: "{m.group(0).lower()}"', 'N'
    return '', 'Y'


wb = Workbook()
guide = wb.active
guide.title = 'How to use'
lines = [
    ('Glamour vendor review', BOLD),
    ('', FONT),
    ('Each vendor has a tab. Set Keep (yellow column) to N for any glamour you want off that vendor.', FONT),
    ('Rows shaded red were pre-set to N by a first pass (reason in the Flag column); set them back to Y to keep.', FONT),
    ('First-pass rules: test/joke/non-gear names, low-tier words (Rusty, Cracked...), lower Defiant tiers, Small/Large when a Medium/plain version exists.', FONT),
    ('Patterns tab: name rules that drop matches on every vendor (e.g. "Practice" drops anything with Practice).', FONT),
    ('Summary tab: how many each vendor keeps. Lists must stay at 500 or fewer.', FONT),
    ('', FONT),
    ('When done: save this file and say so. The vendor lists are rebuilt from it; NPCs, prices and spots stay.', FONT),
    ('Example: on Plate Helms, set "Rusty Iron Helm" to N to remove it.', FONT),
]
for r, (text, font) in enumerate(lines, 1):
    c = guide.cell(row=r, column=1, value=text)
    c.font = font
guide.column_dimensions['A'].width = 110

summary = wb.create_sheet('Summary')
patterns = wb.create_sheet('Patterns')

dv_yn = lambda: DataValidation(type='list', formula1='"Y,N"', allow_blank=False)

vendor_tabs = []
for v in vendors:
    title = v['vendor'][:31]
    ws = wb.create_sheet(title)
    vendor_tabs.append((v['vendor'], title, len(v['items'])))
    headers = ['Item ID', 'Glamour', 'Look of', 'Flag', 'Keep']
    for col, h in enumerate(headers, 1):
        c = ws.cell(row=1, column=col, value=h)
        c.font, c.fill = BOLD, HEAD_FILL
    dv = dv_yn()
    ws.add_data_validation(dv)
    looks = [n[11:-1] if n.startswith("Glamour - '") and n.endswith("'") else n for _, n in v['items']]
    lower = lower_tiers(looks)
    sized = size_drops(looks)
    for r, (item_id, name) in enumerate(v['items'], 2):
        look = looks[r - 2]
        flag, keep = first_pass(look, lower, sized)
        row = [item_id, name, look, flag, keep]
        for col, val in enumerate(row, 1):
            c = ws.cell(row=r, column=col, value=val)
            c.font = FONT
        k = ws.cell(row=r, column=5)
        k.fill = DROP_FILL if keep == 'N' else EDIT_FILL
        k.alignment = Alignment(horizontal='center')
        dv.add(k)
    ws.freeze_panes = 'A2'
    ws.auto_filter.ref = f'A1:E{len(v["items"]) + 1}'
    for col, w in zip('ABCDE', (10, 52, 44, 22, 7)):
        ws.column_dimensions[col].width = w

# Summary: formulas so it updates as Keep values change
for col, h in enumerate(['Vendor', 'Looks', 'Kept', 'Dropped', 'Pattern drops not included'], 1):
    c = summary.cell(row=1, column=col, value=h)
    c.font, c.fill = BOLD, HEAD_FILL
for r, (name, title, count) in enumerate(vendor_tabs, 2):
    ref = f"'{title}'"
    summary.cell(row=r, column=1, value=name).font = FONT
    summary.cell(row=r, column=2, value=f'=COUNTA({ref}!A:A)-1').font = FONT
    summary.cell(row=r, column=3, value=f'=COUNTIF({ref}!E:E,"Y")').font = FONT
    summary.cell(row=r, column=4, value=f'=COUNTIF({ref}!E:E,"N")').font = FONT
total = len(vendor_tabs) + 2
summary.cell(row=total, column=1, value='Total').font = BOLD
for col in 'BCD':
    summary[f'{col}{total}'] = f'=SUM({col}2:{col}{total - 1})'
    summary[f'{col}{total}'].font = BOLD
summary.cell(row=2, column=5, value='Patterns are applied when the lists are rebuilt').font = FONT
summary.column_dimensions['A'].width = 22
summary.column_dimensions['E'].width = 44
summary.freeze_panes = 'A2'

# Patterns: one rule per row, matched against the "Look of" name, case-insensitive substring
for col, h in enumerate(['Drop names containing', 'Applies to (vendor, or blank = all)', 'Note'], 1):
    c = patterns.cell(row=1, column=col, value=h)
    c.font, c.fill = BOLD, HEAD_FILL
examples = [('Practice', '', 'Example rule: edit or delete'), ('', '', ''), ('', '', '')]
for r, row in enumerate(examples, 2):
    for col, val in enumerate(row, 1):
        c = patterns.cell(row=r, column=col, value=val or None)
        c.font = FONT
        if col < 3:
            c.fill = EDIT_FILL
for col, w in zip('ABC', (30, 34, 40)):
    patterns.column_dimensions[col].width = w

wb.save(out)
print('saved', out)
