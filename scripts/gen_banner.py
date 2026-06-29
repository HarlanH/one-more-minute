#!/usr/bin/env python3
"""
Generate a 720×320 app-store banner for One More Minute.

Approach: build a single ImageMagick command.
  - Watch face and icons  → -draw  (MVG shapes)
  - All text              → -annotate  (system font, proper kerning)

No external Python deps beyond subprocess/math.

Layout
------
  Left  (~60%): app title + 3-bullet feature summary
  Right (~40%): 1.78× upscale of the 144×168 Pebble watch face

    Top zone    — 4 m 40 s · running · vibe assigned   (▶  4  ⟨|⟩)
    Bottom zone — 2 m 20 s · paused  · no vibe         (⏸  2   | )
"""

import math, subprocess, os

OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'resources')
OUT     = os.path.join(OUT_DIR, 'banner.png')

FONT_BOLD = '/System/Library/Fonts/HelveticaNeue.ttc'
FONT_REG  = '/System/Library/Fonts/HelveticaNeue.ttc'

# ── Banner canvas ────────────────────────────────────────────────────────────
BW, BH = 720, 320
BG     = '#1c1c1c'

# ── Native watch layout (mirrors src/c/ui.c) ─────────────────────────────────
NW, NH    = 144, 168          # display size
ZONE_H    = NH // 2           # 84
BAR_H     = 16
ICON_S    = 24
ICON_I    = 3                 # ICON_INSET
SPM       = 60

# Scale + placement: watch centred in right panel, 10 px margin top/bottom
WSCALE    = 1.78
WW        = int(round(NW * WSCALE))   # 256
WH        = int(round(NH * WSCALE))   # 299
WX        = BW - 20 - WW              # right-anchored (right margin 20 px)  → 444
WY        = (BH - WH) // 2            # vertical centre ≈ 10 px

# ── Coordinate helpers ───────────────────────────────────────────────────────

def bx(nx):  return WX + nx * WSCALE
def by(ny):  return WY + ny * WSCALE


# ── MVG draw-string helpers ──────────────────────────────────────────────────
# Each returns a string suitable for inclusion in a single -draw argument.
# Coordinates are in banner pixels.

def filled_rect(nx, ny, nw, nh):
    x0, y0 = bx(nx),       by(ny)
    x1, y1 = bx(nx + nw),  by(ny + nh)
    return f"rectangle {x0:.1f},{y0:.1f} {x1:.1f},{y1:.1f}"


def filled_polygon(native_pts):
    pairs = ' '.join(f'{bx(x):.1f},{by(y):.1f}' for x, y in native_pts)
    return f"polygon {pairs}"


def arc_segments(cx_n, cy_n, r_n, a0_deg, a1_deg, n=48):
    """Approximate an arc as n short line segments (avoids SVG sweep ambiguity)."""
    segs = []
    for i in range(n):
        d0 = a0_deg + (a1_deg - a0_deg) *  i      / n
        d1 = a0_deg + (a1_deg - a0_deg) * (i + 1) / n
        for d in (d0, d1):
            r  = math.radians(d)
            bx_ = bx(cx_n) + r_n * WSCALE * math.cos(r)
            by_ = by(cy_n) - r_n * WSCALE * math.sin(r)   # y-down
        # regenerate properly
        r0 = math.radians(d0)
        r1 = math.radians(d1)
        x0 = bx(cx_n) + r_n * WSCALE * math.cos(r0)
        y0 = by(cy_n) - r_n * WSCALE * math.sin(r0)
        x1 = bx(cx_n) + r_n * WSCALE * math.cos(r1)
        y1 = by(cy_n) - r_n * WSCALE * math.sin(r1)
        segs.append(f"line {x0:.2f},{y0:.2f} {x1:.2f},{y1:.2f}")
    return segs


# ── Icon drawing (MVG, in banner coords) ─────────────────────────────────────

# Phone rectangle dimensions (from gen_icons.py, design space = 24 units)
_PX, _PY, _PW, _PH = 9, 4, 6, 16
_PSW = 1.5 * WSCALE                   # phone stroke width in banner px
_ASW = 1.8 * WSCALE                   # arc   stroke width in banner px
_ARC_HALF  = 50                        # degrees each side of horizontal axis
_ARC_RADII = (7, 10)


def vibe_icon_mvg(ox, oy, vibe_on):
    """
    Returns a list of MVG draw strings for the vibe icon whose
    top-left corner is at native coords (ox, oy).
    """
    out = []
    phone_segs = [
        # top
        filled_rect(ox + _PX,              oy + _PY,              _PW,  _PSW / WSCALE),
        # bottom
        filled_rect(ox + _PX,              oy + _PY + _PH - _PSW / WSCALE, _PW, _PSW / WSCALE),
        # left
        filled_rect(ox + _PX,              oy + _PY,              _PSW / WSCALE, _PH),
        # right
        filled_rect(ox + _PX + _PW - _PSW / WSCALE, oy + _PY,   _PSW / WSCALE, _PH),
    ]
    out.append(f"fill black stroke none {'  '.join(phone_segs)}")

    if vibe_on:
        cx = ox + _PX + _PW / 2   # native: 12 within icon cell
        cy = oy + _PY + _PH / 2   # native: 12 within icon cell
        arc_lines = []
        for r in _ARC_RADII:
            arc_lines += arc_segments(cx, cy, r, 180 - _ARC_HALF, 180 + _ARC_HALF)
            arc_lines += arc_segments(cx, cy, r, 360 - _ARC_HALF, 360 + _ARC_HALF)
        out.append(f"fill none stroke black stroke-width {_ASW:.2f}  {'  '.join(arc_lines)}")

    return out


# ── Watch face ───────────────────────────────────────────────────────────────

def watch_draw_cmds():
    """Return list of MVG draw strings (each a -draw argument) for the watch."""
    cmds = []

    # White screen with visible outline (cleaner than a dark bezel on dark bg)
    cmds.append(
        f"fill white stroke '#505050' stroke-width 2  "
        f"rectangle {bx(0):.1f},{by(0):.1f} {bx(NW):.1f},{by(NH):.1f}"
    )

    # Zone divider
    div_y = by(ZONE_H)
    cmds.append(
        f"fill none stroke black stroke-width 1  "
        f"line {bx(0):.1f},{div_y:.1f} {bx(NW):.1f},{div_y:.1f}"
    )

    # ── Zone 0: 4 m 40 s · running · vibe assigned ──
    elapsed0  = 4 * SPM + 40
    bar_w0    = elapsed0 % SPM * NW // SPM    # 96 px native

    cmds.append(f"fill black stroke none  {filled_rect(0, ZONE_H - BAR_H, bar_w0, BAR_H)}")

    # Play icon ▶  at (ICON_I, ICON_I)
    ix, iy = ICON_I, ICON_I
    cmds.append(f"fill black stroke none  {filled_polygon([(ix+4, iy+2), (ix+4, iy+22), (ix+21, iy+12)])}")

    # Vibe-on icon (top-right)
    vx = NW - ICON_I - ICON_S   # 117
    cmds.extend(vibe_icon_mvg(vx, ICON_I, vibe_on=True))

    # ── Zone 1: 2 m 20 s · paused · no vibe ──
    elapsed1  = 2 * SPM + 20
    bar_w1    = elapsed1 % SPM * NW // SPM    # 48 px native

    cmds.append(f"fill black stroke none  {filled_rect(0, NH - BAR_H, bar_w1, BAR_H)}")

    # Pause icon ⏸  at (ICON_I, ZONE_H + ICON_I)
    ix, iy = ICON_I, ZONE_H + ICON_I
    cmds.append(
        f"fill black stroke none  "
        f"{filled_rect(ix+3,  iy+3, 5, 18)}  "
        f"{filled_rect(ix+16, iy+3, 5, 18)}"
    )

    # Vibe-off icon (top-right zone 1)
    cmds.extend(vibe_icon_mvg(vx, ZONE_H + ICON_I, vibe_on=False))

    return cmds


# ── Annotate helpers (text placed in banner coords) ──────────────────────────

def annotate(cmd_list, text, x, y, size, weight='Normal', color='white'):
    """Append -annotate args for text at (x, baseline_y)."""
    cmd_list += [
        '-font',     FONT_BOLD if weight == 'Bold' else FONT_REG,
        '-weight',   weight,
        '-pointsize', str(size),
        '-fill',     color,
        '-annotate', f'+{int(x)}+{int(y)}', text,
    ]


def watch_digit(cmd_list, digit_str, zone_index):
    """
    Place a large bold digit centred in the given zone's text area.
    Text layer native:  y = zone_y + 8,  height = ZONE_H - BAR_H - 16 = 52 px
    """
    zone_y   = zone_index * ZONE_H
    # Centre of text area in banner pixels
    cx_b = bx(NW / 2)
    cy_b = by(zone_y + 8 + 26)            # 8 top-inset + half of 52

    fsize = int(round(44 * WSCALE))        # ≈ 78 px
    # Baseline ≈ centre_y + cap_height/2; Helvetica cap≈72% of pt size
    baseline_y = cy_b + fsize * 0.72 / 2

    # Rough single-digit half-width for Helvetica Neue Bold ≈ 0.42 × pt
    x = cx_b - fsize * 0.42 / 2

    annotate(cmd_list, digit_str, x, baseline_y, fsize, weight='Bold', color='black')


# ── Main ─────────────────────────────────────────────────────────────────────

def build_command():
    cmd = ['magick', '-size', f'{BW}x{BH}', f'xc:{BG}']

    # Watch face geometry
    for d in watch_draw_cmds():
        cmd += ['-draw', d]

    # Watch digits
    watch_digit(cmd, '4', zone_index=0)
    watch_digit(cmd, '2', zone_index=1)

    # ── Left panel text ──
    # Title
    annotate(cmd, 'One More Minute', x=45, y=100, size=44, weight='Bold')

    # Bullets
    BULLETS = [
        'Two minute counters + second bar.',
        'Roman-numeral vibe per minute.',
        'For cooking and more!',
    ]
    bx_dot  = 55
    bx_text = 76
    by0     = 168
    bdy     = 46

    for i, text in enumerate(BULLETS):
        baseline_y = by0 + i * bdy

        # Bullet dot (small filled circle)
        dot_y = baseline_y - 7    # roughly mid-cap of 22 pt text
        cmd += ['-draw',
                f"fill '#888888' stroke none  "
                f"circle {bx_dot},{dot_y} {bx_dot + 4},{dot_y}"]

        annotate(cmd, text,
                 x=bx_text, y=baseline_y,
                 size=22, weight='Normal', color='#c8c8c8')

    cmd.append(OUT)
    return cmd


if __name__ == '__main__':
    os.makedirs(OUT_DIR, exist_ok=True)
    cmd = build_command()
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print('Error:', r.stderr)
    else:
        import struct
        with open(OUT, 'rb') as f:
            d = f.read()
        w, h = struct.unpack('>II', d[16:24])
        print(f'wrote {OUT}  ({w}×{h} px, {len(d)//1024} KB)')
