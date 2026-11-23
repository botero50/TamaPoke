#!/usr/bin/env python3
"""Turns the landscape PNGs (tools/assets/scenery/<region>_<d|t|n>.png, made by gen_scenery.py)
into the board's TPBG files and the web installer's pack.

TPBG (see scenery.h): "TPBG", u16 width, u16 height, u8 colours, u8 0, the palette as RGB565
(little endian), then one byte per pixel. 233 x 156, shown at twice the size.

The files are NOT written into tools/sdcard/mons: pack_bundle.py sorts that folder into the region
packs by the number in each file name, and bg1_d.bin would end up in the Kanto sprite pack.
They go to build/scenery/ and into one small pack, web/sprites-scenery.pak (about 0.8 MB, so it is
committed next to the installer, unlike the 40 MB sprite packs).

    python tools/pack_scenery.py                       # build/scenery/*.bin and web/sprites-scenery.pak
    python tools/pack_scenery.py --copy-to some/mons   # also copy the .bin files there (emulator previews)
"""
import argparse
import os
import random
import shutil
import struct
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from gen_scenery import REGIONS, OUT_W, OUT_H   # noqa: E402

SRC = os.path.join(HERE, "assets", "scenery")
BUILD = os.path.join(HERE, "..", "build", "scenery")
WEB = os.path.join(HERE, "..", "web")


def rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


MAX_SPOTS = 32


def find_spots(im, mood, seed):
    """Where the art can sparkle: water glints by day, sun reflections at golden hour, lit windows,
    lanterns and stars at night. Up to MAX_SPOTS points, spread out, picked from the picture's own
    pixels; a picture with too few of the kind falls back to its brightest pixels."""
    rgb = im.convert("RGB")
    w, h = rgb.size
    px = rgb.load()
    cand, bright = [], []
    for y in range(h):
        for x in range(w):
            r, g, b = px[x, y]
            lum = (r * 299 + g * 587 + b * 114) // 1000
            if y > 12:
                bright.append((lum, x, y))
            if mood == "n":
                warm = r > 175 and g > 115 and b < 140 and r >= g and y > 30      # a lit window or a lantern
                star = y < 75 and lum > 185 and abs(r - b) < 45                   # a bright dot in the sky
                if warm or star:
                    cand.append((x, y))
            elif mood == "t":
                if y > 60 and lum > 185 and r >= b:                               # the sun on the water, warm and bright
                    cand.append((x, y))
            else:
                if y > 60 and b > r + 28 and g >= r and b > 130 and lum > 110:   # bright bluish water, not the sky
                    cand.append((x, y))
    rnd = random.Random(seed)

    def pick(points, want):
        points = list(points)
        rnd.shuffle(points)
        out = []
        for (x, y) in points:
            if all((x - a) ** 2 + (y - b) ** 2 >= 36 for (a, b) in out):         # at least 6 px apart
                out.append((x, y))
                if len(out) >= want:
                    break
        return out

    spots = pick(cand, MAX_SPOTS)
    if len(spots) < 12:                                                           # too few of the kind: the brightest pixels
        bright.sort(reverse=True)
        extra = pick([(x, y) for (_, x, y) in bright[:600]], MAX_SPOTS - len(spots))
        spots += [pt for pt in extra if all((pt[0] - a) ** 2 + (pt[1] - b) ** 2 >= 36 for (a, b) in spots)]
    return spots[:MAX_SPOTS]


def tpbg(path, mood="d", seed=1):
    im = Image.open(path)
    if im.size != (OUT_W, OUT_H):
        raise SystemExit("%s is %dx%d, expected %dx%d" % (path, im.size[0], im.size[1], OUT_W, OUT_H))
    if im.mode != "P":
        im = im.quantize(colors=48, dither=Image.NONE)
    pal = im.getpalette()
    used = sorted(set(im.getdata()))
    remap = {old: new for new, old in enumerate(used)}
    if len(used) > 64:
        raise SystemExit("%s uses %d colours (max 64)" % (path, len(used)))
    out = bytearray(b"TPBG")
    out += struct.pack("<HHBB", OUT_W, OUT_H, len(used), 0)
    for c in used:
        out += struct.pack("<H", rgb565(pal[c * 3], pal[c * 3 + 1], pal[c * 3 + 2]))
    out += bytes(remap[p] for p in im.getdata())
    spots = find_spots(im, mood, seed)
    if spots:                                   # the optional FX block: where to twinkle
        out += b"FX" + bytes([len(spots)])
        for (x, y) in spots:
            out += bytes([x, y])
    return bytes(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--copy-to")
    a = ap.parse_args()
    os.makedirs(BUILD, exist_ok=True)
    files = []
    for i, (key, _) in enumerate(REGIONS):
        for m in "dtn":
            src = os.path.join(SRC, "%s_%s.png" % (key, m))
            if not os.path.exists(src):
                print("missing", src)
                continue
            name = "bg%d_%s.bin" % (i + 1, m)
            data = tpbg(src, m, 100 + i * 3 + "dtn".index(m))
            with open(os.path.join(BUILD, name), "wb") as f:
                f.write(data)
            files.append((name, data))
            if a.copy_to:
                os.makedirs(a.copy_to, exist_ok=True)
                shutil.copy(os.path.join(BUILD, name), os.path.join(a.copy_to, name))
    if not files:
        raise SystemExit("no landscapes found in " + SRC)
    pak = os.path.join(WEB, "sprites-scenery.pak")
    with open(pak, "wb") as o:
        o.write(b"TPAK")
        o.write(struct.pack("<H", len(files)))
        for name, data in files:
            nb = ("mons/" + name).encode()
            o.write(struct.pack("<B", len(nb)) + nb + struct.pack("<I", len(data)))
        for _, data in files:
            o.write(data)
    print("%d landscapes, %d bytes in %s" % (len(files), os.path.getsize(pak), os.path.relpath(pak, HERE)))


if __name__ == "__main__":
    main()
