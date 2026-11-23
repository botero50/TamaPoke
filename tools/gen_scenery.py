#!/usr/bin/env python3
"""Generates the region landscapes for the main screen with a local ComfyUI.

Original art, made here: DreamShaper 8 (Stable Diffusion 1.5, CreativeML Open RAIL-M) through the
ComfyUI HTTP API. The prompts name no Pokemon, no characters and no places from the games -- only the
mood of each region. Every image has a fixed seed so a run can be repeated.

For each of the seven regions there are three moods (d = day, t = golden hour, n = night). Each is
rendered at 768x512, cropped to the screen's 466:312, shrunk to 233x156 and quantized to a small
palette; the firmware doubles it with nearest-neighbour, which gives the pixel-art look the sprites
have. The small PNGs go to tools/assets/scenery/ (committed); the raw renders stay in tools/cache/.

    python tools/gen_scenery.py --only kanto_d       # one image, to judge the style
    python tools/gen_scenery.py --region kanto       # the three moods of one region
    python tools/gen_scenery.py --all                # all 21
    python tools/gen_scenery.py --all --skip-existing

ComfyUI must be running (default http://127.0.0.1:8188, or --server URL).
"""
import argparse
import json
import os
import random
import sys
import time
import urllib.parse
import urllib.request

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
RAW = os.path.join(HERE, "cache", "scenery_raw")
OUT = os.path.join(HERE, "assets", "scenery")
CKPT = "DreamShaper_8_pruned.safetensors"
W, H = 768, 512
SCREEN_W, SCREEN_H = 466, 312        # the landscape area of the main screen
OUT_W, OUT_H = 233, 156              # half size; the firmware doubles it
COLORS = 48

STYLE = ("16-bit pixel art video game background, wide landscape scene, clean shapes, limited palette, "
         "soft calm upper sky area, no characters, no people, no animals, no text")
NEGATIVE = ("people, person, character, animal, creature, text, letters, watermark, logo, signature, frame, border, "
            "blurry, photo, realistic, ugly, deformed, cropped, noise")

# region index -> (key, scene, {mood: lighting words})
MOODS = {
    "d": "bright sunny day, blue sky, soft white clouds",
    "t": "golden hour, warm orange and pink sky, long soft shadows, glowing light",
    "n": "night, deep blue sky full of stars, a big moon, glowing lit windows and lanterns, calm and quiet",
}
REGIONS = [
    ("kanto",  "a peaceful small hillside town with red roofs and white houses, green meadows with tall grass, "
               "a winding path, round trees and a tall snow-capped mountain far away"),
    ("johto",  "traditional countryside, autumn maple trees with red and orange leaves, an old wooden pagoda tower on a hill, "
               "a calm lake with stone lanterns, a curved wooden bridge"),
    ("hoenn",  "tropical islands, turquoise sea, white sand beach, palm trees, lush green rainforest hills "
               "and a smoking volcano in the distance"),
    ("sinnoh", "snowy mountain slopes with dark pine forest, a calm frozen blue lake, ancient stone ruins, "
               "misty peaks, cold air"),
    ("unova",  "a big modern city with tall skyscrapers, a long suspension bridge over a wide river, "
               "green parks along the water, busy but clean"),
    ("kalos",  "an elegant european countryside, a grand palace with a garden, purple lavender fields, "
               "a tall iron lattice tower in the distance, a cobblestone village square"),
    ("alola",  "a tropical island paradise, curved palm trees, thatched wooden huts on a sandy beach, "
               "colourful flowers, a green volcano mountain, calm turquoise water"),
]


def prompt_for(region_idx, mood):
    key, scene = REGIONS[region_idx]
    return "%s, %s, %s" % (scene, MOODS[mood], STYLE)


# images re-rolled after a look at the first batch: name -> seed added (kanto_n came out as a snowy alpine village)
RE_ROLL = {"kanto_n": 1}


def seed_for(region_idx, mood):
    return 7000 + region_idx * 10 + "dtn".index(mood) + RE_ROLL.get("%s_%s" % (REGIONS[region_idx][0], mood), 0)


def workflow(pos, seed):
    return {
        "1": {"class_type": "CheckpointLoaderSimple", "inputs": {"ckpt_name": CKPT}},
        "2": {"class_type": "CLIPTextEncode", "inputs": {"text": pos, "clip": ["1", 1]}},
        "3": {"class_type": "CLIPTextEncode", "inputs": {"text": NEGATIVE, "clip": ["1", 1]}},
        "4": {"class_type": "EmptyLatentImage", "inputs": {"width": W, "height": H, "batch_size": 1}},
        "5": {"class_type": "KSampler", "inputs": {
            "seed": seed, "steps": 28, "cfg": 7.0, "sampler_name": "dpmpp_2m", "scheduler": "karras",
            "denoise": 1.0, "model": ["1", 0], "positive": ["2", 0], "negative": ["3", 0], "latent_image": ["4", 0]}},
        "6": {"class_type": "VAEDecode", "inputs": {"samples": ["5", 0], "vae": ["1", 2]}},
        "7": {"class_type": "SaveImage", "inputs": {"filename_prefix": "tamapoke_scenery", "images": ["6", 0]}},
    }


def http_json(url, data=None):
    req = urllib.request.Request(url, data=None if data is None else json.dumps(data).encode("utf-8"),
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return json.loads(r.read().decode("utf-8"))


def render(server, pos, seed):
    """Queues one image and returns its PNG bytes."""
    q = http_json(server + "/prompt", {"prompt": workflow(pos, seed), "client_id": "tamapoke-scenery"})
    pid = q["prompt_id"]
    for _ in range(600):
        time.sleep(1.0)
        hist = http_json(server + "/history/" + pid)
        if pid in hist and hist[pid].get("outputs"):
            for node in hist[pid]["outputs"].values():
                for img in node.get("images", []):
                    qs = urllib.parse.urlencode({"filename": img["filename"], "subfolder": img.get("subfolder", ""),
                                                 "type": img.get("type", "output")})
                    with urllib.request.urlopen(server + "/view?" + qs, timeout=60) as r:
                        return r.read()
    raise RuntimeError("ComfyUI did not finish in time")


FADE_ROWS = 46          # the top rows (of 156) carry the name and the clock: 90 px on the screen
FADE_MAX = 0.34         # how far they are pulled toward white (day, golden hour) or black (night)


def fade_top(im, mood):
    """The header text is dark on day art and white on night art: lift or sink the top so it reads."""
    target = 0 if mood == "n" else 255
    px = im.load()
    for y in range(min(FADE_ROWS, im.height)):
        k = FADE_MAX * (1.0 - y / float(FADE_ROWS)) ** 1.5
        for x in range(im.width):
            r, g, b = px[x, y]
            px[x, y] = (int(r + (target - r) * k), int(g + (target - g) * k), int(b + (target - b) * k))
    return im


def to_small(raw_path, out_path, mood="d"):
    """Crop to the screen's aspect, shrink to half size, fade the top, quantize, save a small palette PNG."""
    im = Image.open(raw_path).convert("RGB")
    cw = int(round(im.height * SCREEN_W / SCREEN_H))          # 765 of 768
    left = (im.width - cw) // 2
    im = im.crop((left, 0, left + cw, im.height)).resize((OUT_W, OUT_H), Image.LANCZOS)
    im = fade_top(im, mood)
    im = im.quantize(colors=COLORS, method=Image.MEDIANCUT, dither=Image.NONE)
    im.save(out_path, optimize=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--server", default="http://127.0.0.1:8188")
    ap.add_argument("--only", help="one image, e.g. kanto_d")
    ap.add_argument("--region", help="the three moods of one region, e.g. kanto")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--skip-existing", action="store_true")
    ap.add_argument("--seed-offset", type=int, default=0, help="re-roll: added to every seed")
    ap.add_argument("--reprocess", action="store_true", help="rebuild the small PNGs from the raw renders, no ComfyUI")
    a = ap.parse_args()
    jobs = []
    for i, (key, _) in enumerate(REGIONS):
        for m in "dtn":
            name = "%s_%s" % (key, m)
            if a.all or a.only == name or a.region == key:
                jobs.append((i, key, m, name))
    if not jobs:
        print("nothing selected: use --only, --region or --all")
        return 1
    os.makedirs(RAW, exist_ok=True)
    os.makedirs(OUT, exist_ok=True)
    for i, key, m, name in jobs:
        out = os.path.join(OUT, name + ".png")
        if a.skip_existing and os.path.exists(out):
            print("skip", name)
            continue
        raw = os.path.join(RAW, name + ".png")
        if not a.reprocess:
            seed = seed_for(i, m) + a.seed_offset
            print("render %-10s seed %d ..." % (name, seed), flush=True)
            png = render(a.server, prompt_for(i, m), seed)
            with open(raw, "wb") as f:
                f.write(png)
        to_small(raw, out, m)
        print("  ->", os.path.relpath(out, HERE), os.path.getsize(out), "bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
