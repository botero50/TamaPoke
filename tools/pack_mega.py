#!/usr/bin/env python3
"""Packs the Mega forms (PMD SpriteCollab, CC BY-NC) for the board.

Each species in mega_forms.py becomes /mons/mNNN.bin: the same TPK2 multi-action sprite as pNNN.bin
(pack_pmd.py writes it). They are NOT written into tools/sdcard/mons: pack_bundle.py sorts that
folder into the region packs by the number in the file name. They go to build/mega/ and into one
small pack, web/sprites-mega.pak, committed next to the installer like the scenery pack.

    python tools/pack_mega.py                       # download, build/mega/*.bin and web/sprites-mega.pak
    python tools/pack_mega.py --copy-to some/mons   # also copy the .bin files there (emulator previews)
"""
import argparse
import glob
import os
import shutil
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import pack_pmd                      # noqa: E402
from mega_forms import MEGA_FORMS    # noqa: E402

BUILD = os.path.join(HERE, "..", "build", "mega")
WEB = os.path.join(HERE, "..", "web")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--copy-to")
    args = ap.parse_args()
    os.makedirs(BUILD, exist_ok=True)
    failed = []
    for dex, form in sorted(MEGA_FORMS.items()):
        print("#%03d mega %s" % (dex, form))
        try:
            pack_pmd.pack(dex, False, form=form, out_dir=BUILD)
        except Exception as e:           # noqa: BLE001
            print("  FAILED:", e)
            failed.append(dex)
    files = []
    for p in sorted(glob.glob(os.path.join(BUILD, "m*.bin"))):
        with open(p, "rb") as f:
            files.append((os.path.basename(p), f.read()))
    if not files:
        raise SystemExit("no mega sprites built")
    pak = os.path.join(WEB, "sprites-mega.pak")
    with open(pak, "wb") as o:
        o.write(b"TPAK")
        o.write(struct.pack("<H", len(files)))
        for name, data in files:
            nb = ("mons/" + name).encode()
            o.write(struct.pack("<B", len(nb)) + nb + struct.pack("<I", len(data)))
        for _, data in files:
            o.write(data)
    print("%d megas, %d bytes in %s%s" % (len(files), os.path.getsize(pak), os.path.relpath(pak, HERE),
                                          ("; FAILED: %s" % failed) if failed else ""))
    if args.copy_to:
        os.makedirs(args.copy_to, exist_ok=True)
        for name, _ in files:
            shutil.copy(os.path.join(BUILD, name), os.path.join(args.copy_to, name))


if __name__ == "__main__":
    main()
