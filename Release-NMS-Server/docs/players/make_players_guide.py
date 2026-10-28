"""Builds Voidforged_Players_Guide.pdf next to this script.

Numbers come from the server's settings as of 2026-10-27 (rift_config.lua, gen_sinks.py, the
dynamic_affix_pool table, docs/dynamic_items). Update them here when those change, then rerun:
    python make_players_guide.py
"""
import os

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import inch
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (BaseDocTemplate, CondPageBreak, Frame, KeepTogether, NextPageTemplate,
                                PageBreak, PageTemplate, Paragraph, Spacer, Table, TableStyle)

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "Voidforged_Players_Guide.pdf")
FONTS = r"C:\Windows\Fonts"

pdfmetrics.registerFont(TTFont("Body", os.path.join(FONTS, "segoeui.ttf")))
pdfmetrics.registerFont(TTFont("Body-Bold", os.path.join(FONTS, "segoeuib.ttf")))
pdfmetrics.registerFont(TTFont("Body-Italic", os.path.join(FONTS, "segoeuii.ttf")))
pdfmetrics.registerFont(TTFont("Body-BoldItalic", os.path.join(FONTS, "segoeuiz.ttf")))
pdfmetrics.registerFont(TTFont("Head", os.path.join(FONTS, "georgiab.ttf")))
pdfmetrics.registerFont(TTFont("Head-Italic", os.path.join(FONTS, "georgiai.ttf")))
pdfmetrics.registerFontFamily("Body", normal="Body", bold="Body-Bold", italic="Body-Italic",
                              boldItalic="Body-BoldItalic")

# Palette: deep void purple with a chronal gold accent
VOID = colors.HexColor("#140b26")
VOID_2 = colors.HexColor("#2a1650")
VIOLET = colors.HexColor("#6b3fb8")
GOLD = colors.HexColor("#d9a441")
INK = colors.HexColor("#1d1a24")
MUTED = colors.HexColor("#5c5668")
ROW = colors.HexColor("#f3effa")
RULE = colors.HexColor("#d8cfea")
TIP_BG = colors.HexColor("#fbf5e6")

PAGE_W, PAGE_H = letter
MARGIN = 0.8 * inch

ss = {
    "h1": ParagraphStyle("h1", fontName="Head", fontSize=24, leading=28, textColor=VOID_2, spaceAfter=4),
    "kicker": ParagraphStyle("kicker", fontName="Body-Bold", fontSize=8.5, leading=11, textColor=VIOLET,
                             spaceAfter=2),
    "lede": ParagraphStyle("lede", fontName="Head-Italic", fontSize=11.5, leading=16, textColor=MUTED,
                           spaceAfter=12),
    "h2": ParagraphStyle("h2", fontName="Head", fontSize=14, leading=18, textColor=VOID_2, spaceBefore=12,
                         spaceAfter=5),
    "h3": ParagraphStyle("h3", fontName="Body-Bold", fontSize=10.5, leading=14, textColor=VIOLET,
                         spaceBefore=8, spaceAfter=3),
    "body": ParagraphStyle("body", fontName="Body", fontSize=9.6, leading=13.6, textColor=INK, spaceAfter=6),
    "bullet": ParagraphStyle("bullet", fontName="Body", fontSize=9.6, leading=13.4, textColor=INK,
                             leftIndent=14, bulletIndent=3, spaceAfter=3),
    "cell": ParagraphStyle("cell", fontName="Body", fontSize=8.4, leading=11, textColor=INK),
    "cellb": ParagraphStyle("cellb", fontName="Body-Bold", fontSize=8.4, leading=11, textColor=INK),
    "th": ParagraphStyle("th", fontName="Body-Bold", fontSize=8.4, leading=11, textColor=colors.white),
    "tip": ParagraphStyle("tip", fontName="Body", fontSize=9.2, leading=13, textColor=INK),
    "small": ParagraphStyle("small", fontName="Body-Italic", fontSize=8, leading=11, textColor=MUTED,
                            spaceAfter=6),
    "quote": ParagraphStyle("quote", fontName="Head-Italic", fontSize=10.5, leading=15, textColor=VOID_2,
                            leftIndent=12, rightIndent=12, spaceBefore=4, spaceAfter=10),
}


def P(text, style="body"):
    return Paragraph(text, ss[style])


def bullets(items):
    return [Paragraph(t, ss["bullet"], bulletText="\u2022") for t in items]


def section(kicker, title, lede=None):
    out = [P(kicker.upper(), "kicker"), P(title, "h1")]
    if lede:
        out.append(P(lede, "lede"))
    return out


def table(rows, widths, header=True, bold_first_col=False):
    data = []
    for r, row in enumerate(rows):
        cells = []
        for c, value in enumerate(row):
            if r == 0 and header:
                style = "th"
            elif bold_first_col and c == 0:
                style = "cellb"
            else:
                style = "cell"
            cells.append(Paragraph(str(value), ss[style]))
        data.append(cells)
    t = Table(data, colWidths=widths, repeatRows=1 if header else 0, hAlign="LEFT")
    cmds = [
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("TOPPADDING", (0, 0), (-1, -1), 3.5),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 3.5),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("LINEBELOW", (0, 0), (-1, -1), 0.4, RULE),
    ]
    if header:
        cmds.append(("BACKGROUND", (0, 0), (-1, 0), VOID_2))
    for r in range(1 if header else 0, len(rows)):
        if (r % 2 == 0) == header:
            cmds.append(("BACKGROUND", (0, r), (-1, r), ROW))
    t.setStyle(TableStyle(cmds))
    return t


def tip(text, label="Tip"):
    t = Table([[Paragraph(f"<b>{label}.</b> {text}", ss["tip"])]], colWidths=[PAGE_W - 2 * MARGIN])
    t.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), TIP_BG),
        ("LINEBEFORE", (0, 0), (0, -1), 3, GOLD),
        ("LEFTPADDING", (0, 0), (-1, -1), 10),
        ("RIGHTPADDING", (0, 0), (-1, -1), 10),
        ("TOPPADDING", (0, 0), (-1, -1), 6),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
    ]))
    return KeepTogether([Spacer(1, 4), t, Spacer(1, 8)])


# ---------------------------------------------------------------------------------------------
# Page decoration


def draw_cover(canvas, doc):
    canvas.saveState()
    canvas.setFillColor(VOID)
    canvas.rect(0, 0, PAGE_W, PAGE_H, stroke=0, fill=1)

    # Rift: concentric rings fading out from a point above centre
    cx, cy = PAGE_W / 2, PAGE_H * 0.63
    for i in range(22, 0, -1):
        alpha = 0.03 + 0.014 * (22 - i)
        canvas.setFillColor(colors.Color(0.42, 0.25, 0.72, alpha=min(alpha, 0.35)))
        canvas.circle(cx, cy, i * 11, stroke=0, fill=1)
    canvas.setStrokeColor(colors.Color(0.85, 0.64, 0.25, alpha=0.55))
    for r, w in ((130, 0.6), (165, 0.4), (200, 0.3)):
        canvas.setLineWidth(w)
        canvas.circle(cx, cy, r, stroke=1, fill=0)
    # The tear itself
    canvas.setFillColor(colors.Color(1, 0.93, 0.75, alpha=0.95))
    p = canvas.beginPath()
    p.moveTo(cx, cy + 105)
    p.curveTo(cx + 15, cy + 35, cx + 9, cy - 35, cx, cy - 105)
    p.curveTo(cx - 9, cy - 35, cx - 15, cy + 35, cx, cy + 105)
    canvas.drawPath(p, stroke=0, fill=1)

    canvas.setFillColor(GOLD)
    canvas.setFont("Body-Bold", 10)
    canvas.drawCentredString(PAGE_W / 2, PAGE_H * 0.30 + 70, "A  P L A Y E R ' S   G U I D E")
    canvas.setFillColor(colors.white)
    canvas.setFont("Head", 54)
    canvas.drawCentredString(PAGE_W / 2, PAGE_H * 0.30, "VOIDFORGED")
    canvas.setFillColor(colors.Color(0.85, 0.80, 0.95))
    canvas.setFont("Head-Italic", 15)
    canvas.drawCentredString(PAGE_W / 2, PAGE_H * 0.30 - 32, "Every rift goes deeper.")

    canvas.setStrokeColor(GOLD)
    canvas.setLineWidth(0.8)
    canvas.line(PAGE_W / 2 - 60, PAGE_H * 0.30 - 52, PAGE_W / 2 + 60, PAGE_H * 0.30 - 52)

    canvas.setFillColor(colors.Color(0.7, 0.66, 0.8))
    canvas.setFont("Body", 9.5)
    canvas.drawCentredString(PAGE_W / 2, 0.9 * inch,
                             "Greater Rifts  \u00b7  Rolled gear  \u00b7  Class sets  \u00b7  Paragon  \u00b7  Chronal Tributes")
    canvas.restoreState()


def draw_body(canvas, doc):
    canvas.saveState()
    canvas.setFillColor(VOID)
    canvas.rect(0, PAGE_H - 0.42 * inch, PAGE_W, 0.42 * inch, stroke=0, fill=1)
    canvas.setFillColor(GOLD)
    canvas.rect(0, PAGE_H - 0.42 * inch - 2, PAGE_W, 2, stroke=0, fill=1)
    canvas.setFillColor(colors.white)
    canvas.setFont("Head", 10)
    canvas.drawString(MARGIN, PAGE_H - 0.27 * inch, "VOIDFORGED")
    canvas.setFont("Body", 8.5)
    canvas.setFillColor(colors.Color(0.8, 0.76, 0.9))
    canvas.drawRightString(PAGE_W - MARGIN, PAGE_H - 0.27 * inch, "Player's Guide")
    canvas.setFillColor(MUTED)
    canvas.setFont("Body", 8.5)
    canvas.drawCentredString(PAGE_W / 2, 0.45 * inch, str(doc.page))
    canvas.restoreState()


# ---------------------------------------------------------------------------------------------
# Data

CW = PAGE_W - 2 * MARGIN  # content width

RIFT_ZONES = [
    "The Crypt of Decay", "The Ruins of Sebilis", "Chardok", "Befallen", "Karnor's Castle",
    "The Castle of Mistmoore", "The Plane of Fear", "Accursed Temple of Cazic-Thule", "The Howling Stones",
    "The Permafrost Caverns", "The Sleeper's Tomb", "The Plane of Hate",
    "The Akheva Ruins", "Grieg's End", "Dreadspire Keep", "Solusek's Eye", "Nagafen's Lair", "Kael Drakkel",
    "Kaesora Library", "Anguish, the Fallen Palace", "Frostcrypt", "The Convorteum", "Kaesora",
    "Sanctum Somnium", "Riftseekers' Sanctum", "Chapterhouse of the Fallen", "Ashengate",
]

# Guaranteed rolls on a tier 1 helm (multiplier 1.0) from dynamic_affix_pool, 2026-10-27
SAMPLE_CLASSES = [
    # name, HP, (resource label, lo, hi), AC, (main stat, lo, hi), (heroic, lo, hi), (power label, lo, hi)
    ("Warrior", (424, 518), ("End", 290, 350), (124, 161), ("STR", 49, 61), ("hSTR", 19, 25), ("ATK", 26, 44)),
    ("Rogue", (424, 518), ("End", 290, 350), (108, 142), ("DEX", 40, 52), ("hDEX", 12, 15), ("ATK", 26, 44)),
    ("Paladin", (406, 497), ("Mana", 265, 320), (111, 145), ("STR", 48, 59), ("hSTR", 17, 23), ("ATK", 23, 38)),
    ("Cleric", (353, 436), ("Mana", 360, 440), (80, 103), ("WIS", 52, 71), ("hWIS", 19, 25), ("Heal Amt", 47, 76)),
    ("Wizard", (353, 436), ("Mana", 360, 440), (44, 63), ("INT", 52, 71), ("hINT", 19, 25), ("Spell Dmg", 47, 76)),
]
TIERS = [(1, "Chronal", 1.0), (2, "Tempered", 1.5), (3, "Exalted", 2.0), (4, "Eternal", 2.5), (5, "Timeless", 3.0)]
REGULAR_STAT_CAP = 127  # per item, regular stats are stored in a small field


def rng(lo, hi, mult, cap=None):
    a, b = int(lo * mult), int(hi * mult)
    if cap:
        a, b = min(a, cap), min(b, cap)
    return f"{a:,}" if a == b else f"{a:,}\u2013{b:,}"


def shards(tier, fast=False):
    base = 5 + 2 * tier
    return int(base * 1.25) if fast else base


# ---------------------------------------------------------------------------------------------
# Content

story = [NextPageTemplate("body"), PageBreak()]

# --- Welcome ---------------------------------------------------------------------------------
story += section("Welcome", "Welcome to Voidforged",
                 "Norrath as you remember it, with a tear running through it. Every character plays three "
                 "classes, the endgame is an endless ladder of Greater Rifts, and the gear you earn there is "
                 "rolled just for you.")
story.append(P("This guide covers what is different here: how to reach The Void, how Greater Rifts work, "
               "the rolled rift gear and its set bonuses, and the long-term systems that turn platinum and "
               "AA points into power. A command reference is on the last page."))
story.append(P("At a glance", "h2"))
story.append(table([
    ["Feature", "What it means for you"],
    ["Three classes", "Every character combines three classes. Gear, spells and set bonuses for any of them count."],
    ["Level 65", "The level cap is 65. Alternate Advancement opens at level 51."],
    ["Greater Rifts", "Timed, randomized dungeons in 27 zones with tiers from 1 to 500. Opened by Zebuxoruk in The Void."],
    ["Rolled gear", "Rift armor and jewelry roll their own stats when you buy them, weighted for your class. No two pieces are alike."],
    ["Class sets", "Armor sets give bonuses at 2, 4 and 8 pieces; jewelry sets at 3, 6 and 9."],
    ["Glamours", "Change how your armor and weapons look without losing stats."],
    ["Chronal Tributes", "Rift-only tributes that boost you inside Fractures and pay for themselves in shards."],
    ["Paragon", "Twelve deep AA lines for raw HP, mana, AC and heroic stats once you have the rest."],
    ["Reforging", "Swap one random line on a rolled piece for a better one."],
    ["Quality of life", "The Nautilus Vault, personal loot offers (/nmsloot), melee while casting, gear sets, and more."],
], [1.35 * inch, CW - 1.35 * inch], bold_first_col=True))

story.append(P("Quality of life", "h2"))
story += bullets([
    "<b>Nautilus Vault.</b> Every character receives a Nautilus Vault on login. It gives you extra storage, lets "
    "you use clickies stored in it, and opens your bank and a merchant from anywhere.",
    "<b>Personal loot (/nmsloot).</b> When a mob dies, every group or raid member in the zone gets their own "
    "loot roll, offered in the /nmsloot window: keep it, sell it for platinum on the spot, or pass it to someone. "
    "Coin is split automatically. Grouped, the corpse's normal loot is replaced by these personal rolls; solo, "
    "you keep the corpse loot and your personal roll.",
    "<b>Melee while casting.</b> Auto-attack and ranged attacks keep going while you cast.",
    "<b>Gear sets.</b> Saving a set in the Bandolier window also remembers every worn slot. Loading it, or "
    "<b>#gear load &lt;n&gt;</b>, swaps your whole outfit from your bags, even in combat. <b>#gear list</b> shows your sets.",
    "<b>Bags and curios.</b> The Collector of Curios in the Bazaar sells Hero's Backpacks, Rucksacks and Boxes "
    "of Holding, Combinerators, the Unattuner and delivery vouchers for platinum.",
    "<b>Blessings.</b> Sneed Galliway (Bazaar and Qeynos) sells Heroes' and Legends' Blessings for 1 copper each.",
    "<b>Lockouts.</b> Non-respawning instances lock you out for 6 hours.",
])

# --- The Void --------------------------------------------------------------------------------
story.append(PageBreak())
story += section("Chapter 1", "The Void and Zebuxoruk",
                 "Freed from a prison woven through time, Zebuxoruk left the Chronal Fabric torn behind him. "
                 "The tears are Fractures, and The Void is where you step into them.")
story.append(P("Getting there", "h2"))
story += bullets([
    "<b>Requirement:</b> your account needs Plane of Time access (the Maelin flag, earned by defeating the "
    "four elemental gods). Without it, entering The Void returns you to the Bazaar.",
    "<b>Travel:</b> open the Bazaar map and pick The Void from the <b>Special</b> tab.",
])
story.append(P("Who you'll find there", "h2"))
story.append(table([
    ["NPC", "What they do"],
    ["Zebuxoruk", "Storyteller and keeper of the rifts. Hail him, ask about the Fabric, then click a tier to open a rift."],
    ["Chronal Quartermaster", "Tier 1 class armor, for Chronal Shards."],
    ["Tier vendors", "Armor, jewelry and augments for each gear tier, three vendors per tier, each titled with what it sells (for example \"Tier 2 Jewelry\")."],
    ["Chronal Artificer", "Reforges rolled gear (see Chapter 5)."],
], [1.6 * inch, CW - 1.6 * inch], bold_first_col=True))
story.append(P("When you hail Zebuxoruk:", "h3"))
story.append(P("\u201cAh, another mind drawn to the edge of everything. I am Zebuxoruk, and the Chronal Fabric "
               "is unraveling because of me.\u201d", "quote"))

story.append(P("The 27 rift zones", "h2"))
story.append(P(", ".join(RIFT_ZONES) + "."))

# --- Greater Rifts ---------------------------------------------------------------------------
story.append(PageBreak())
story += section("Chapter 2", "Greater Rifts",
                 "A Greater Rift (a Fracture) is a private copy of a classic dungeon, filled with creatures "
                 "pulled out of time. Fill the progress bar, kill the Guardian before the clock runs out, and the "
                 "Fabric pays you.")
story.append(P("Opening a rift", "h2"))
story += bullets([
    "Hail Zebuxoruk and say <b>rifts</b>. Click a tier, or say <b>tier &lt;number&gt;</b>.",
    "You can open any tier up to <b>one above your deepest clear</b>, to a maximum of 500. Progress is per "
    "character: grouping with someone deeper doesn't unlock tiers for you.",
    "Your group comes with you, but only members <b>standing in The Void</b> when the rift opens.",
    "The zone is picked at random from 27 dungeons. Rifts are free to open.",
    "One open rift per character. Say <b>re-enter</b> to go back to yours, or <b>close</b> to abandon it.",
])
story.append(P("Inside the rift", "h2"))
story.append(table([
    ["", ""],
    ["Time limit", "15 minutes to kill the Guardian. Warnings count down at 12, 9, 6, 3 and 1 minutes."],
    ["Progress", "Normal creatures add 1%, elites 4%. At 100% the Rift Guardian appears."],
    ["Pylons", "At 25%, 50% and 75% a pylon spawns. Click it for a burst of power: <b>Conduit</b> (fire around "
               "you), <b>Cleansing</b> (group heal and cure), <b>Shielding</b> (big absorb shield) or "
               "<b>Speed</b> (haste, run speed, faster casting)."],
    ["Champions", "About 15% of creatures are champions, shown by their surname. <b>Molten</b> pulses fire "
                  "and explodes when it dies. <b>Arcane</b> pulses magic damage. <b>Vortex</b> drags players "
                  "who keep their distance into melee range; the tank is never pulled."],
    ["The Guardian", "Damage pulses around it, calls adds at half health, wipes its hate list during phase "
                     "shifts, and enrages two minutes after the pull."],
    ["Scaling", "Creatures are level 65 to 70 and get tougher every tier: more health and damage, higher "
                "resists and AC, and their spells cut through resists from tier 1, more each tier, so stacking resists won't make you immune."],
    ["Respawns", "Creatures respawn about every 3 minutes, so don't dawdle."],
], [1.2 * inch, CW - 1.2 * inch], header=False, bold_first_col=True))

story.append(P("Rewards", "h2"))
story.append(P("Each player in the rift gets <b>Chronal Shards</b> (5 + 2 per tier, +25% if you finish in under "
               "10 minutes) and <b>1,000 platinum per tier</b>. A new deepest clear also unlocks the next "
               "tier and, every 100 tiers, new gear."))
tiers = [1, 10, 50, 100, 250, 500]
story.append(table(
    [["Rift tier", "Shards", "Shards (fast)", "Platinum"]] +
    [[t, shards(t), shards(t, True), f"{1000 * t:,}"] for t in tiers],
    [CW / 4] * 4))

# --- Rift gear -------------------------------------------------------------------------------
story.append(PageBreak())
story += section("Chapter 3", "Rift Gear",
                 "Rift gear is bought with Chronal Shards in The Void, comes in five tiers, and rolls its stats "
                 "the moment you buy it.")
story.append(P("Gear tiers and unlocks", "h2"))
story.append(P("Every gear tier unlocks from your own deepest clear. Vendors hide what you haven't unlocked, "
               "so an empty vendor means you need a deeper clear. Armor and jewelry vendors only list pieces "
               "for your own classes."))
story.append(table([
    ["Gear tier", "Armor", "Jewelry", "Augments", "Stats", "Name prefix"],
    ["1", "always", "clear 25", "clear 75", "1x", "\u2014"],
    ["2", "clear 100", "clear 125", "clear 175", "1.5x", "Tempered"],
    ["3", "clear 200", "clear 225", "clear 275", "2x", "Exalted"],
    ["4", "clear 300", "clear 325", "clear 375", "2.5x", "Eternal"],
    ["5", "clear 400", "clear 425", "clear 475", "3x", "Timeless"],
], [0.8 * inch, 0.95 * inch, 0.95 * inch, 0.95 * inch, 0.7 * inch, CW - 4.35 * inch]))
story.append(Spacer(1, 6))
story.append(table([
    ["Price (Chronal Shards)", "Tier 1", "Tier 2", "Tier 3", "Tier 4", "Tier 5"],
    ["Armor piece", "25", "50", "100", "200", "400"],
    ["Jewelry piece", "15", "30", "60", "120", "240"],
    ["Augment", "40", "80", "160", "320", "640"],
], [1.7 * inch] + [(CW - 1.7 * inch) / 5] * 5, bold_first_col=True))
story.append(Spacer(1, 6))
story += bullets([
    "<b>Armor sets</b> have 7 pieces (Helm, Chestguard, Sleeves, Bracer, Gloves, Leggings, Boots); wear two "
    "Bracers for 8. <b>Jewelry sets</b> have 8 (Charm, Earring, Mask, Pendant, Mantle, Cloak, Girdle, Ring); "
    "wear two Earrings and two Rings for 10.",
    "Every rift <b>Girdle</b> carries 68% haste, so a belt covers your worn haste.",
    "Everything is <b>No Trade</b>, and you can only buy pieces for classes you have.",
    "Selling a piece back refunds a fifth of its price.",
])

story.append(P("How a piece rolls", "h2"))
story += bullets([
    "<b>Always:</b> HP, AC, Mana and/or Endurance (casters Mana, melee Endurance, hybrids both), all regular "
    "stats weighted to the class, all six resists, Spell Damage, Heal Amount and Attack.",
    "<b>Sometimes:</b> regen, Shielding, Avoidance, Accuracy, Stun Resist, Strike Through, Damage Shield, "
    "Spell Shield, and physical, spell, DoT and heal crits. These can roll 0, and then the piece doesn't have them.",
    "<b>Random affixes:</b> 2 to 3 extra lines from your class's list, such as a weapon or defense skill %, "
    "Backstab or Flying Kick damage %, an element's spell damage %, Healing % or an instrument %.",
    "<b>Name:</b> your best random affix names the piece, for example <i>Shadowblade Boots of the Assassin</i>.",
    "Rolls stack with all your other gear, set bonuses, buffs, AAs and focus effects. Skill % raises the skill "
    "itself, which here drives both to-hit and damage. Percent bonuses are capped at 100%, crit chance at 50%.",
])
story.append(P("Some slots roll stronger: Chest 125%, Legs 115%, Head and Feet 100%, Arms and Hands 90%, "
               "each Wrist 80%."))

story.append(PageBreak())
story.append(P("What a helm rolls, tier by tier", "h2"))
story.append(P("The guaranteed lines on a Helm (the 100% slot) for five sample classes. Random affixes and "
               "0-able stats come on top. A single item's regular stat stops at 127."))
rows = [["Class", "Tier", "HP", "Mana / End", "AC", "Main stat", "Heroic", "Power"]]
for name, hp, res, ac, stat, hero, power in SAMPLE_CLASSES:
    for tier, prefix, mult in TIERS:
        rows.append([
            name if tier == 1 else "",
            f"{tier} {prefix}",
            rng(*hp, mult),
            f"{res[0]} {rng(res[1], res[2], mult)}",
            rng(*ac, mult),
            f"{stat[0]} {rng(stat[1], stat[2], mult, REGULAR_STAT_CAP)}",
            f"{hero[0]} {rng(hero[1], hero[2], mult)}",
            f"{power[0]} {rng(power[1], power[2], mult)}",
        ])
t = table(rows, [0.62 * inch, 0.8 * inch, 0.78 * inch, 1.1 * inch, 0.66 * inch, 0.95 * inch, 0.8 * inch,
                 CW - 5.71 * inch], bold_first_col=True)
t.setStyle(TableStyle([("LINEBELOW", (0, r), (-1, r), 1.1, VIOLET) for r in range(5, len(rows) - 1, 5)]))
story.append(t)
story.append(P("Rogue, Monk and Berserker get the standout DEX rolls. Hybrids blend 75% of a melee class with "
               "25% of a caster (Paladin = Warrior + Cleric), with 90% of the AC.", "small"))

# --- Sets ------------------------------------------------------------------------------------
story.append(PageBreak())
story += section("Chapter 4", "Set Bonuses, Augments and Glamours",
                 "Wear enough pieces of your class's set and it starts paying you back.")
story.append(P("Armor sets: 2, 4 and 8 pieces", "h2"))
story.append(P("Bonuses add up: at 8 pieces you have all three. Tier 2 to 5 sets give the tier 1 bonuses times "
               "the tier's multiplier, and each tier is its own set. With three classes, every set of each class "
               "counts separately, so 4 + 4 pieces of two sets give both 4-piece bonuses."))
story.append(table([
    ["Set", "Class", "2 pieces", "4 pieces", "8 pieces"],
    ["Juggernaut", "Warrior", "+250 HP, +15 AC", "+40 AC, +10 HP Regen", "+750 HP, +5 Shielding, +10 Stun Resist"],
    ["Divine Conduit", "Cleric", "+5 Mana Regen, +250 Mana", "+30 WIS, +8% healing", "+10 Mana Regen, +5% Heal Crit"],
    ["Bulwark", "Paladin", "+150 Mana, +200 HP", "+30 AC, +5% healing", "+500 HP, +3 Shielding, +10 Stun Resist"],
    ["Wildstalker", "Ranger", "+25 Attack, +20 DEX", "+10% Archery", "+5% Physical Crit, +4% Archery Crit"],
    ["Dread Harbinger", "Shadowknight", "+250 HP, +150 Mana", "+10% disease and poison spells, +30 AC", "+500 HP, +5% DoT Crit"],
    ["Verdant Storm", "Druid", "+250 Mana, +5 Mana Regen", "+5% healing, +8% cold and fire spells", "+25 Damage Shield, +5% DoT Crit"],
    ["Iron Fist", "Monk", "+20 AGI, +25 Attack", "+10% Flying Kick", "+5 Avoidance, +8% Physical Crit"],
    ["Maestro", "Bard", "+25 CHA, +150 Mana", "+10% all instruments", "+500 HP, +5% Physical Crit, +5 End Regen"],
    ["Shadowblade", "Rogue", "+25 Attack, +20 DEX", "+10% Backstab", "+9% Physical Crit, +10% Backstab Dmg"],
    ["Spiritcaller", "Shaman", "+200 HP, +250 Mana", "+20 STA, +10% cold, poison, disease spells", "+5% DoT Crit, +5% healing, +10 Mana Regen"],
    ["Lich Lord", "Necromancer", "+300 Mana, +5 Mana Regen", "+10% poison and disease spells", "+10% DoT Crit Dmg, +5% DoT Crit, +10 Mana Regen"],
    ["Archmage", "Wizard", "+300 Mana, +30 INT", "+10% fire, cold, magic spells", "+15% Spell Crit Dmg, +5% Spell Crit"],
    ["Elementalist", "Magician", "+300 Mana, +20 STA", "+10% fire and magic spells", "+5% Spell Crit, +25 Damage Shield"],
    ["Mesmerist", "Enchanter", "+25 CHA, +300 Mana", "+10 Mana Regen, +10% magic spells", "+5 Spell Shield, +5% Spell Crit"],
    ["Feral Bond", "Beastlord", "+150 Mana, +20 AGI, +20 STR", "+10% Hand to Hand, +10% cold and poison spells", "+8% Physical Crit"],
    ["Bloodrage", "Berserker", "+25 STR, +250 HP", "+10% Frenzy, +30 Attack", "+10% Physical Crit, +15% Physical Crit Dmg"],
], [1.05 * inch, 0.9 * inch, 1.35 * inch, 1.6 * inch, CW - 4.9 * inch], bold_first_col=True))
story.append(P("Physical Crit counts for every weapon attack: melee, archery, throwing, kicks, Backstab and the "
               "rest. Archery Crit adds to it for arrows only.", "small"))

story.append(CondPageBreak(2.5 * inch))
story.append(P("Jewelry sets: 3, 6 and 9 pieces", "h2"))
story.append(P("3 pieces give a percent boost (tier 1 value, tier 5 in brackets), 6 pieces a worn effect and 9 "
               "a proc, both one rank higher per tier. The worn effect starts a rank above the best that drops, "
               "so it always wins. Melee procs fire from weapon hits; spell procs fire when you cast damage spells."))
story.append(table([
    ["Class", "3 pieces", "6 pieces (T1 to T5)", "9 pieces (T1 to T5)"],
    ["Warrior", "+4% Defense, +3% Parry (12%, 9%)", "Improved Parry / Block IV\u2013VIII", "Blessed Rune VI\u2013X"],
    ["Cleric", "+4% healing, +3% heal crit (12%, 9%)", "Divine Insight I\u2013V", "Sympathetic Healing Light I\u2013V"],
    ["Paladin", "+4% healing, +3% heal crit (12%, 9%)", "Improved Parry / Block IV\u2013VIII", "Remote Healing Surge IX\u2013XIII"],
    ["Ranger", "+4% Archery, +3% archery crit (12%, 9%)", "Sharpshooting VII\u2013XI", "Hail of Arrows I\u2013V"],
    ["Shadowknight", "+4% disease and poison spells (12%)", "Ferocity VII\u2013XI", "Vampiric Strike IX\u2013XIII"],
    ["Druid", "+4% fire and cold spells (12%)", "Divine Insight I\u2013V", "Sympathetic Healing Light I\u2013V"],
    ["Monk", "+4% Flying Kick dmg, +3% Hand to Hand (12%, 9%)", "Improved Dodge V\u2013IX", "Force of Magic IX\u2013XIII"],
    ["Bard", "+4% all instruments (12%)", "Accuracy I\u2013V", "Prismatic Strike IX\u2013XIII"],
    ["Rogue", "+4% Backstab dmg, +3% Piercing (12%, 9%)", "Cleave VII\u2013XI", "Strike of Venom IX\u2013XIII"],
    ["Shaman", "+4% cold, poison and disease spells (12%)", "Divine Insight I\u2013V", "Sympathetic Healing Light I\u2013V"],
    ["Necromancer", "+4% disease and poison spells (12%)", "Void Conduit I\u2013V (pet focus)", "Sympathetic Force of Corruption X\u2013XIV"],
    ["Wizard", "+4% fire and cold spells (12%)", "Arcane Insight I\u2013V", "Sympathetic Burst of Ice IX\u2013XIII"],
    ["Magician", "+4% magic and fire spells (12%)", "Void Conduit I\u2013V (pet focus)", "Sympathetic Force of Magic IX\u2013XIII"],
    ["Enchanter", "+4% magic spells, +3% spell crit (12%, 9%)", "Void Conduit I\u2013V (pet focus)", "Sympathetic Force of Magic IX\u2013XIII"],
    ["Beastlord", "+4% Hand to Hand, +3% cold spells (12%, 9%)", "Void Conduit I\u2013V (pet focus)", "Strike of Venom IX\u2013XIII"],
    ["Berserker", "+4% Frenzy dmg, +3% 2H Slash and Blunt (12%, 9%)", "Ferocity VII\u2013XI", "Burst of Ice IX\u2013XIII"],
], [1.0 * inch, 2.35 * inch, 1.65 * inch, CW - 5.0 * inch], bold_first_col=True))
story.append(P("Divine Insight adds heal crit and mana regen; Arcane Insight adds spell crit damage and mana regen. "
               "Void Conduit is a pet focus (pet power 30 at tier 1 up to 70 at tier 5); it does not stack with a pet focus item, the stronger one counts. "
               "Hail of Arrows is an archery strike that grows from tier 1 to tier 5. "
               "Sympathetic Healing Light heals you each time it fires, so healers heal themselves while they nuke.",
               "small"))

aug = []
aug.append(P("Augments", "h2"))
aug.append(P("Rift augments have fixed stats, any class can use them, and they fit slot 1 or 2 of any rift "
               "armor or jewelry piece. Higher tiers multiply the stats; the focus effect stays the same."))
aug.append(table([
    ["Augment", "Tier 1 stats", "Focus"],
    ["Bulwark Shard", "+150 HP, +15 AC, +5 heroic STA, +10 Magic, Fire and Cold resist", "\u2014"],
    ["Ferocity Shard", "+15 Attack, +10 STR, +10 DEX, +4 heroic STR, +4 heroic DEX", "\u2014"],
    ["Arcana Shard", "+150 Mana, +20 Spell Damage, +10 INT", "Improved Damage V"],
    ["Mending Shard", "+150 Mana, +20 Heal Amount, +10 WIS", "Improved Healing V"],
    ["Alacrity Shard", "+200 Mana, +8 INT, +8 WIS", "Spell Haste V"],
    ["Valor Shard", "+100 HP, +10 Attack, +10 Heal Amount", "Mana Preservation VII"],
], [1.2 * inch, CW - 2.75 * inch, 1.55 * inch], bold_first_col=True))

aug.append(P("Glamours", "h2"))
aug.append(P("Rift armor has a glamour slot (type 21). Eighteen <b>Glamour Outfitters</b> stand in the East "
               "Commonlands tunnel and sell glamours for 1,000 platinum each: Plate, Chain and Leather (every "
               "slot, 8 matching colors), seven Exotic vendors by slot, Robes, and seven weapon vendors by type. "
               "Your new look stays after zoning and everyone around you sees it."))
story.append(KeepTogether(aug))

# --- Endgame ---------------------------------------------------------------------------------
story.append(PageBreak())
story += section("Chapter 5", "The Long Climb",
                 "Three systems turn rift income into permanent power. Their costs grow with you, so there is "
                 "always something to spend on.")

story.append(P("Chronal Tributes", "h2"))
story.append(P("Four tributes that only work inside Greater Rifts. Pick them at any tribute master like other "
               "personal tributes and turn tribute on."))
story += bullets([
    "<b>Tiers stack.</b> Each tier covers 100 rift tiers: Tier I works in rifts 1\u2013100, Tier II in 1\u2013200, "
    "Tier III in 1\u2013300, and so on. You get the bonus of the band you're in, up to your tribute's tier: "
    "Tier III gives its Tier I bonus in a tier 50 rift and its Tier III bonus in a tier 250 rift.",
    "<b>Cost:</b> 100 favor \u00d7 the rift tier, per tribute, once per rift, taken when you enter. In a tier 120 "
    "rift that's 12,000 favor each. The normal 10-minute tribute charge doesn't apply to them.",
    "<b>Above your tier, or outside rifts,</b> a Chronal Tribute lies dormant and costs nothing. If you're short "
    "on favor, they stay dormant for that rift.",
])
story.append(table([
    ["Tribute", "Each tier gives"],
    ["Chronal Might", "+5 Attack, +3 heroic STR, +3 heroic DEX, +50 HP"],
    ["Chronal Arcana", "+10 Spell Damage, +10 Heal Amount, +50 Mana, +3 heroic INT, +3 heroic WIS"],
    ["Chronal Bulwark", "+5 AC, +75 HP, +3 heroic STA, +2 to each heroic resist"],
    ["Chronal Fortune", "+5% Chronal Shards from the rift's clear reward (never platinum)"],
], [1.5 * inch, CW - 1.5 * inch], bold_first_col=True))
story.append(P("Values are per band: Chronal Might in the 201\u2013300 band gives +15 Attack and +150 HP. "
               "Starting values are modest and may be raised.", "small"))

story.append(P("Paragon AAs", "h2"))
story.append(P("Twelve deep lines on the General tab, open from level 51. They cost more with every rank."))
story.append(table([
    ["Line", "Per rank", "Ranks", "Cost", "Total"],
    ["Paragon Vitality / Mind / Stamina", "+25 HP / Mana / Endurance", "50", "5 AA, +1 per rank", "1,475 AA each"],
    ["Paragon Fortitude / Might", "+1 AC / Attack", "50", "5 AA, +1 per rank", "1,475 AA each"],
    ["Paragon Ferocity", "+1% melee damage", "10", "25 AA, +10 per rank", "700 AA"],
    ["Paragon heroic STR, STA, AGI, DEX, INT, WIS", "+1 heroic stat", "50", "15 AA, +3 per rank", "4,425 AA each"],
], [2.2 * inch, 1.45 * inch, 0.5 * inch, 1.3 * inch, CW - 5.45 * inch], bold_first_col=True))
story.append(P("Heroic stats from Paragon count like heroic stats on items. Use the Voidforged client files, "
               "or the Paragon lines show without names.", "small"))

ref = [P("Reforging", "h2")]
ref.append(P("Don't like a random line on a rolled piece? The <b>Chronal Artificer</b> in The Void can "
               "replace it."))
ref += bullets([
    "<b>#reforge</b> lists the rolled items you carry. <b>#reforge &lt;slot&gt;</b> shows that item's lines "
    "and the price.",
    "Target the Artificer, then <b>#reforge &lt;slot&gt; &lt;line&gt;</b> to pay. You're offered two new lines; "
    "<b>#reforge &lt;slot&gt; take 1</b> or <b>take 2</b> swaps one in, <b>keep</b> leaves the current line. "
    "The offer is saved on the item, so you can zone and decide later.",
    "Only random lines (marked *) can be reforged, and <b>only one line per item, ever</b>: the first line you "
    "reforge is the one you can keep rerolling.",
    "<b>Cost:</b> 25,000 platinum and 50 Chronal Shards \u00d7 (2 \u00d7 item tier \u2212 1), +25% for each "
    "earlier reforge of the same item, up to 5\u00d7.",
])
ref.append(table([
    ["Item tier", "First reforge", "Fifth reforge", "Cap (17th on)"],
    ["1", "25,000 pp + 50 shards", "50,000 pp + 100 shards", "125,000 pp + 250 shards"],
    ["3", "125,000 pp + 250 shards", "250,000 pp + 500 shards", "625,000 pp + 1,250 shards"],
    ["5", "225,000 pp + 450 shards", "450,000 pp + 900 shards", "1,125,000 pp + 2,250 shards"],
], [0.9 * inch, (CW - 0.9 * inch) / 3, (CW - 0.9 * inch) / 3, (CW - 0.9 * inch) / 3], bold_first_col=True))
story.append(KeepTogether(ref))

# --- Reference -------------------------------------------------------------------------------
story.append(PageBreak())
story += section("Reference", "Commands and Tips")
story.append(table([
    ["Command", "What it does"],
    ["Hail Zebuxoruk, say <b>rifts</b>", "Open a Greater Rift. <b>tier &lt;n&gt;</b> picks a tier, <b>re-enter</b> returns you to yours, <b>close</b> abandons it."],
    ["#dyninfo", "Compare all your rolled gear. Add a number, a slot, <b>compare a b</b> or <b>worn</b>."],
    ["#reforge", "List, price and reforge rolled gear (pay and choose with the Chronal Artificer targeted)."],
    ["#gear list / show &lt;n&gt; / load &lt;n&gt;", "Your saved gear sets; save them in the Bandolier window."],
    ["/nmsloot", "Your personal loot offers: keep, sell or pass each one."],
], [1.9 * inch, CW - 1.9 * inch], bold_first_col=True))
story.append(P("Command replies go to the Other chat window by default; scroll it if nothing seems to happen.",
               "small"))

story.append(KeepTogether([P("Comparing gear with #dyninfo", "h2"), table([
    ["Command", "What it shows"],
    ["#dyninfo", "Every rolled piece you own (worn, bags and bank) by slot: what you wear, and each spare as \"X better, Y worse\". Every piece gets a clickable number."],
    ["#dyninfo &lt;number&gt;", "That piece against what you wear in its slot: Stat, Worn, This item, Change, with gains in green and losses in red."],
    ["#dyninfo &lt;slot&gt;", "Every piece for one slot side by side, for example feet, wrist or weapon."],
    ["#dyninfo compare &lt;a&gt; &lt;b&gt;", "Any two pieces against each other."],
    ["#dyninfo worn", "Totals for everything you wear: rolled gear, other items, set bonuses, active procs and worn effects."],
], [1.7 * inch, CW - 1.7 * inch], bold_first_col=True)]))
story.append(P("The item window shows flat stats and your biggest skill bonus; #dyninfo shows everything, "
               "including percent bonuses and crits. Random affixes are marked *.", "small"))

story.append(P("Tips for climbing", "h2"))
story += bullets([
    "Push your deepest clear early: it gates every gear tier and the next rift tier.",
    "A fast clear (under 10 minutes) pays a quarter more shards. If a tier takes longer, dropping a few tiers "
    "often earns shards faster.",
    "Spread out your three classes' set pieces with care. A full 8-piece set beats two half sets at the top, "
    "but two 4-piece bonuses are strong while you gear up.",
    "Read the champion's surname before you pull. Bring Vortex champions to the tank; step away from Molten "
    "ones before they die.",
    "Save pylons for the Guardian when you can: Shielding and Speed shine in the boss fight.",
    "Use <b>#dyninfo &lt;number&gt;</b> before you sell anything. The better/worse count tells you at a glance.",
    "Turn on Chronal Fortune before a long session of fast clears; it pays for itself in shards.",
])
story.append(Spacer(1, 18))
story.append(P("See you in the Fractures.", "quote"))


# ---------------------------------------------------------------------------------------------

doc = BaseDocTemplate(OUT, pagesize=letter, leftMargin=MARGIN, rightMargin=MARGIN, topMargin=0.75 * inch,
                      bottomMargin=0.75 * inch, title="Voidforged Player's Guide", author="Voidforged",
                      subject="Player's guide and introduction")
cover_frame = Frame(0, 0, PAGE_W, PAGE_H, id="cover")
body_frame = Frame(MARGIN, 0.7 * inch, CW, PAGE_H - 0.7 * inch - 0.75 * inch, id="body")
doc.addPageTemplates([
    PageTemplate(id="cover", frames=[cover_frame], onPage=draw_cover),
    PageTemplate(id="body", frames=[body_frame], onPage=draw_body),
])
doc.build([Spacer(1, 1)] + story)
print(OUT)
