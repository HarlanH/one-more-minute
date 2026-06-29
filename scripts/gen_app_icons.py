#!/usr/bin/env python3
"""
Generate app launcher icons for One More Minute.

Design: Roman numeral 'I' with vibration arcs on each side —
"one more minute, felt on your wrist."

Renders at two sizes with 4x supersampling for anti-aliasing:
  - icon_80.png   (80x80)   Pebble watch launcher
  - icon_144.png  (144x144)  phone app list

No external deps — output is hand-encoded 8-bit grayscale PNG
(0 = black, 255 = white on black background).
"""
import math, zlib, struct, os

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "resources")
SCALE = 4   # supersample factor


# ── raster helpers ─────────────────────────────────────────────────────────────

def make_render(size):
    R = size * SCALE
    return [[0.0] * R for _ in range(R)], R


def fill_rect(buf, R, x, y, w, h, v=1.0):
    x0, y0 = int(round(x)), int(round(y))
    x1, y1 = int(round(x + w)), int(round(y + h))
    for row in range(max(0, y0), min(R, y1)):
        for col in range(max(0, x0), min(R, x1)):
            buf[row][col] = max(buf[row][col], v)


def _stamp_circle(buf, R, cx, cy, r):
    for dy in range(-int(r) - 1, int(r) + 2):
        for dx in range(-int(r) - 1, int(r) + 2):
            if dx * dx + dy * dy <= r * r:
                col = int(round(cx)) + dx
                row = int(round(cy)) + dy
                if 0 <= row < R and 0 <= col < R:
                    buf[row][col] = max(buf[row][col], 1.0)


def stroke_arc(buf, R, cx, cy, r, start_deg, end_deg, thickness, s):
    r_px = r * s
    cx_px, cy_px = cx * s, cy * s
    stamp_r = (thickness / 2.0) * s
    a0 = start_deg % 360
    a1 = end_deg % 360
    if a1 <= a0:
        a1 += 360
    for deg in range(int(a0), int(a1) + 1):
        rad = math.radians(deg)
        px = cx_px + r_px * math.cos(rad)
        py = cy_px - r_px * math.sin(rad)
        _stamp_circle(buf, R, px, py, stamp_r)


# ── supersample → 8-bit grayscale PNG ─────────────────────────────────────────

def downsample(buf, size, R):
    out = []
    for ry in range(size):
        row = []
        for rx in range(size):
            total = 0.0
            for dy in range(SCALE):
                for dx in range(SCALE):
                    total += buf[ry * SCALE + dy][rx * SCALE + dx]
            row.append(total / (SCALE * SCALE))
        out.append(row)
    return out


def encode_png_8bit(pixels, size):
    W = H = size
    rows_bytes = []
    for row in pixels:
        rows_bytes.append(b'\x00' + bytes(int(round(v * 255)) for v in row))
    raw = b''.join(rows_bytes)
    compressed = zlib.compress(raw, 9)

    def chunk(typ, data):
        crc_val = zlib.crc32(typ + data) & 0xFFFFFFFF
        return struct.pack('>I', len(data)) + typ + data + struct.pack('>I', crc_val)

    ihdr_data = struct.pack('>IIBBBBB', W, H, 8, 0, 0, 0, 0)
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', ihdr_data)
            + chunk(b'IDAT', compressed)
            + chunk(b'IEND', b''))


def write_icon(name, buf, size):
    path = os.path.join(OUT_DIR, name + '.png')
    img = downsample(buf, size, size * SCALE)
    with open(path, 'wb') as f:
        f.write(encode_png_8bit(img, size))
    print(f"  wrote {path}")
    ascii_preview(img, name)


def ascii_preview(img, name):
    print(f"\n  === {name} ===")
    step = max(1, len(img) // 32)
    for ry in range(0, len(img), step):
        line = ''
        for rx in range(0, len(img[ry]), step):
            v = img[ry][rx]
            ch = ' ' if v < 0.15 else '·' if v < 0.4 else '░' if v < 0.65 else '▒' if v < 0.85 else '█'
            line += ch
        print(f"  {line}")


# ── icon design ────────────────────────────────────────────────────────────────

def make_app_icon(size):
    """Roman numeral 'I' with vibration arcs — design in fractions of *size*."""
    buf, R = make_render(size)
    s = float(SCALE)
    cx = size / 2.0
    cy = size / 2.0

    # ── Roman numeral 'I' ──
    i_height   = 0.52 * size
    serif_w    = 0.18 * size
    serif_t    = 0.065 * size
    stem_w     = 0.10 * size

    i_top = cy - i_height / 2
    i_bot = cy + i_height / 2
    serif_l = cx - serif_w / 2
    serif_r = cx + serif_w / 2
    stem_l  = cx - stem_w  / 2
    stem_r  = cx + stem_w  / 2

    # top serif
    fill_rect(buf, R, serif_l * s, i_top * s, serif_w * s, serif_t * s)
    # bottom serif
    fill_rect(buf, R, serif_l * s, (i_bot - serif_t) * s, serif_w * s, serif_t * s)
    # stem
    fill_rect(buf, R, stem_l * s, (i_top + serif_t) * s, stem_w * s, (i_height - 2 * serif_t) * s)

    # ── vibration arcs ──
    n_arcs       = 2
    inner_r      = 0.27 * size
    radius_step  = 0.09 * size
    arc_thick    = 0.032 * size
    arc_half_deg = 52

    arc_cx, arc_cy = cx, cy

    for i in range(n_arcs):
        r = inner_r + i * radius_step
        # left arcs: opening left (center at 180°)
        stroke_arc(buf, R, arc_cx, arc_cy, r,
                   180 - arc_half_deg, 180 + arc_half_deg,
                   arc_thick, s)
        # right arcs: opening right (center at 0°/360°)
        stroke_arc(buf, R, arc_cx, arc_cy, r,
                   360 - arc_half_deg, 360 + arc_half_deg,
                   arc_thick, s)

    return buf


# ── main ───────────────────────────────────────────────────────────────────────

if __name__ == '__main__':
    os.makedirs(OUT_DIR, exist_ok=True)
    print("Generating app icons…")
    write_icon('icon_80',  make_app_icon(80),  80)
    write_icon('icon_144', make_app_icon(144), 144)
    print(f"\nDone. Icons written to {OUT_DIR}")
