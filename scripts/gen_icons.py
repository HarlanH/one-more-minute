#!/usr/bin/env python3
"""
Generate 1-bit 24x24 PNG icons for One More Minute.

Draws into a float coverage buffer at 4x (96x96), supersamples down to
24x24 by averaging 4x4 blocks, then thresholds at 50%.  No Pillow or
other external deps needed — output is hand-encoded PNG (zlib + struct).

Icons produced in resources/:
  icon_play.png       filled right-pointing triangle
  icon_pause.png      two filled vertical bars
  icon_vibe_on.png    phone rectangle + two arcs each side (vibrating)
  icon_vibe_off.png   phone rectangle only
"""
import math, zlib, struct, os

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "resources")
DESIGN  = 24    # design-space units
SCALE   = 4     # supersample factor (render at 96x96)
RENDER  = DESIGN * SCALE  # 96


# ── low-level raster helpers ──────────────────────────────────────────────────

def make_buf():
    return [[0.0] * RENDER for _ in range(RENDER)]


def _clamp(v, lo, hi):
    return max(lo, min(hi, v))


def fill_rect(buf, x, y, w, h, v=1.0):
    """Fill a rectangle in render-pixel coords.  Accepts floats; converts to int."""
    x0, y0 = int(round(x)), int(round(y))
    x1, y1 = int(round(x + w)), int(round(y + h))
    for row in range(max(0, y0), min(RENDER, y1)):
        for col in range(max(0, x0), min(RENDER, x1)):
            buf[row][col] = max(buf[row][col], v)


def fill_triangle(buf, ax, ay, bx, by, cx, cy):
    """Scanline-fill a triangle in render-pixel coords."""
    verts = sorted([(ax, ay), (bx, by), (cx, cy)], key=lambda p: p[1])
    (x0, y0), (x1, y1), (x2, y2) = verts

    def interp(ya, xa, yb, xb, y):
        if yb == ya:
            return xa
        return xa + (xb - xa) * (y - ya) / (yb - ya)

    for row in range(max(0, int(y0)), min(RENDER, int(y2) + 1)):
        if row < y1:
            lx = interp(y0, x0, y1, x1, row)
            rx = interp(y0, x0, y2, x2, row)
        else:
            lx = interp(y1, x1, y2, x2, row)
            rx = interp(y0, x0, y2, x2, row)
        if lx > rx:
            lx, rx = rx, lx
        for col in range(max(0, int(lx)), min(RENDER, int(rx) + 1)):
            buf[row][col] = 1.0


def _stamp_circle(buf, cx, cy, r):
    """Fill a small circle (used for fat-stroke primitives)."""
    for dy in range(-int(r) - 1, int(r) + 2):
        for dx in range(-int(r) - 1, int(r) + 2):
            if dx * dx + dy * dy <= r * r:
                col = int(round(cx)) + dx
                row = int(round(cy)) + dy
                if 0 <= row < RENDER and 0 <= col < RENDER:
                    buf[row][col] = 1.0


def stroke_arc(buf, cx, cy, r, start_deg, end_deg, thickness):
    """Draw a thick arc (stroke only).  Angles in degrees, 0=right, CCW."""
    r_px = r * SCALE
    cx_px, cy_px = cx * SCALE, cy * SCALE
    stamp_r = (thickness / 2.0) * SCALE

    # Normalise angles so we always step from start to end going CCW
    a0 = start_deg % 360
    a1 = end_deg   % 360
    if a1 <= a0:
        a1 += 360

    # Step every 1° across the arc
    for deg in range(int(a0), int(a1) + 1):
        rad = math.radians(deg)
        px = cx_px + r_px * math.cos(rad)
        py = cy_px - r_px * math.sin(rad)   # y-down screen
        _stamp_circle(buf, px, py, stamp_r)


# ── supersample → 1-bit PNG ───────────────────────────────────────────────────

def downsample(buf):
    """Average SCALE×SCALE blocks -> 24×24 float image 0..1 (1=black)."""
    out = []
    for ry in range(DESIGN):
        row = []
        for rx in range(DESIGN):
            total = 0.0
            for dy in range(SCALE):
                for dx in range(SCALE):
                    total += buf[ry * SCALE + dy][rx * SCALE + dx]
            row.append(total / (SCALE * SCALE))
        out.append(row)
    return out


def threshold(img, level=0.5):
    return [[1 if v >= level else 0 for v in row] for row in img]


def encode_png_1bit(pixels):
    """Encode a 24×24 list-of-lists (1=black) as a 1-bit grayscale PNG."""
    W = H = DESIGN
    rows_bytes = []
    for row in pixels:
        row_byte = bytearray()
        for i in range(0, W, 8):
            byte = 0
            for j in range(8):
                if i + j < W:
                    # PNG 1-bit grayscale: 0=black, 1=white  → invert our 1=black
                    if row[i + j] == 0:
                        byte |= (1 << (7 - j))
            row_byte.append(byte)
        rows_bytes.append(b'\x00' + bytes(row_byte))   # filter byte 0

    raw = b''.join(rows_bytes)
    compressed = zlib.compress(raw, 9)

    def chunk(typ, data):
        crc_val = zlib.crc32(typ + data) & 0xFFFFFFFF
        return struct.pack('>I', len(data)) + typ + data + struct.pack('>I', crc_val)

    ihdr_data = struct.pack('>IIBBBBB', W, H, 1, 0, 0, 0, 0)
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', ihdr_data)
            + chunk(b'IDAT', compressed)
            + chunk(b'IEND', b''))


def write_icon(name, buf):
    img = threshold(downsample(buf))
    path = os.path.join(OUT_DIR, name + '.png')
    with open(path, 'wb') as f:
        f.write(encode_png_1bit(img))
    # ASCII preview
    print(f"\n=== {name} ===")
    for row in img:
        print(''.join('█' if p else ' ' for p in row))


# ── icon definitions (design-space units, render-space = units * SCALE) ───────

def make_play():
    buf = make_buf()
    #  Right-pointing triangle:  left edge x=4, y=2..22   tip at x=21, y=12
    ax, ay = 4 * SCALE,  2 * SCALE
    bx, by = 4 * SCALE, 22 * SCALE
    cx, cy = 21 * SCALE, 12 * SCALE
    fill_triangle(buf, ax, ay, bx, by, cx, cy)
    return buf


def make_pause():
    buf = make_buf()
    # Two bars, each 5 wide × 18 tall, at x=3 and x=16
    fill_rect(buf, 3 * SCALE, 3 * SCALE, 5 * SCALE, 18 * SCALE)
    fill_rect(buf, 16 * SCALE, 3 * SCALE, 5 * SCALE, 18 * SCALE)
    return buf


# Phone rectangle shared by both vibe icons
PHONE_X, PHONE_Y = 9, 4      # top-left corner in design units
PHONE_W, PHONE_H = 6, 16     # size in design units
PHONE_CX = PHONE_X + PHONE_W / 2   # 12.0
PHONE_CY = PHONE_Y + PHONE_H / 2   #  12.0

# Arc geometry (design units)
ARC_INNER_R  = 7
ARC_OUTER_R  = 10
ARC_THICKNESS = 1.8   # design-unit stroke width
ARC_HALF_SPAN = 50    # degrees each side of the axis


def draw_phone(buf):
    fill_rect(buf,
              PHONE_X * SCALE, PHONE_Y * SCALE,
              PHONE_W * SCALE, PHONE_H * SCALE)


def make_vibe_off():
    buf = make_buf()
    draw_phone(buf)
    return buf


def make_vibe_on():
    buf = make_buf()
    draw_phone(buf)

    # Left side arcs: centred at phone centre, opening left (180° ± half_span)
    for r in (ARC_INNER_R, ARC_OUTER_R):
        stroke_arc(buf, PHONE_CX, PHONE_CY, r,
                   180 - ARC_HALF_SPAN, 180 + ARC_HALF_SPAN,
                   ARC_THICKNESS)

    # Right side arcs: opening right (0° ± half_span, CCW so 360-span..360+span)
    for r in (ARC_INNER_R, ARC_OUTER_R):
        stroke_arc(buf, PHONE_CX, PHONE_CY, r,
                   360 - ARC_HALF_SPAN, 360 + ARC_HALF_SPAN,
                   ARC_THICKNESS)

    return buf


# ── main ──────────────────────────────────────────────────────────────────────

if __name__ == '__main__':
    os.makedirs(OUT_DIR, exist_ok=True)
    write_icon('icon_play',     make_play())
    write_icon('icon_pause',    make_pause())
    write_icon('icon_vibe_off', make_vibe_off())
    write_icon('icon_vibe_on',  make_vibe_on())
    print('\nDone. PNGs written to', OUT_DIR)
