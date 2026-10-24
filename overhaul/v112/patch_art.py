#!/usr/bin/env python3
"""1.1.2: install original generated hero, event/shop art, and PSP XMB splash.

All assets are compact base64-encoded PNG/JPEG files in overhaul/v112/assets.
Build-time only: PIL decodes images, emits RGB565 in static read-only arrays.
No PSP runtime image decoder, heap allocation, or network dependency.
"""
from pathlib import Path
from PIL import Image
import base64
import io
import re
import sys

game = Path(sys.argv[1])
assets = Path(__file__).parent / "assets"

def load(name):
    raw = base64.b64decode((assets / (name + ".b64")).read_text(), validate=False)
    im = Image.open(io.BytesIO(raw))
    im.load()
    return im

def pack565(im):
    out = []
    for r,g,b,a in im.convert("RGBA").getdata():
        if a < 110:
            out.append(0xf81f)
        else:
            color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            # 0xf81f is the engine's transparent color key.
            out.append(0xf81e if color == 0xf81f else color)
    return out

def definition(name, values):
    rows = [",".join(f"0x{x:04x}" for x in values[i:i + 24])
            for i in range(0, len(values), 24)]
    return f"static const uint16_t {name}[{len(values)}] = {{\n" + ",\n".join(rows) + "\n};\n"

hero = load("hero").convert("RGBA")
assert hero.size == (210, 127), f"Unexpected hero sprite size {hero.size}"
hero_src = (game / "motion_art.c").read_text()
pattern = r"const uint16_t rift_hero\\[26670\\]\\s*=\\s*\\{.*?\\};"
new_hero = definition("rift_hero", pack565(hero)).replace("static const uint16_t", "const uint16_t", 1)
hero_src, count = re.subn(pattern, lambda m: new_hero, hero_src, count=1, flags=re.S)
if count != 1:
    raise RuntimeError("1.1.2 hero replacement anchor missing; check preceding 1.1.0 art generator")
(game / "motion_art.c").write_text(hero_src)

scenes = ["#ifndef RIFT112_SCENE_ART_H", "#define RIFT112_SCENE_ART_H", "#include <stdint.h>"]
for name in ("event", "shop"):
    im = load(name).convert("RGB")
    assert im.size == (240, 136), f"{name}: expected 240x136, got {im.size}"
    scenes.append(definition("rift112_" + name, pack565(im)))
scenes.append("#endif")
(game / "rift112_scene_art.h").write_text("\n".join(scenes))

render = game / "render.c"
s = render.read_text()
if '#include "motion_art.h"' not in s:
    raise RuntimeError("Renderer's motion_art.h include missing")
s = s.replace('#include "motion_art.h"',
              '#include "motion_art.h"\n#include "rift112_scene_art.h"', 1)
for func, name in (("shop_screen","shop"), ("event_screen","event")):
    # Draw scenery under existing UI. All controls/panels retain their layout
    # and a dark overlay maintains text contrast on the 480x272 PSP LCD.
    target = "static void " + func + "(const Game*g){"
    if target not in s:
        raise RuntimeError("1.1.2 renderer anchor missing: " + target)
    replacement = (target + "\n"
                   " sprite_scaled(0,28,480,224,rift112_" + name + ",240,136);\n"
                   " dark_rect(0,28,480,224,165);\n")
    s = s.replace(target, replacement, 1)
render.write_text(s)

pic = load("splash").convert("RGB").resize((480,272), Image.Resampling.LANCZOS)
pic.save(game / "rift112_pic1.png", format="PNG", optimize=True)
print("PASS 1.1.2: new 210x127 hero, 240x136 shop/event scenery, 480x272 XMB art")
