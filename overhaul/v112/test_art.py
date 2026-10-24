#!/usr/bin/env python3
"""Fail the native PSP build if v1.1.2 art isn't compiled into its source."""
from pathlib import Path
from PIL import Image
import sys, re, hashlib, base64, io

p=Path(sys.argv[1]); source=(p/"motion_art.c").read_text()
render=(p/"render.c").read_text(); scenes=(p/"rift112_scene_art.h").read_text()
assert len(re.findall(r"const uint16_t rift_hero\[26670\]",source))==1, "hero not embedded"
hero_body=re.search(r"const uint16_t rift_hero\[26670\]\s*=\s*\{([^}]+)\}",source,re.S)
assert hero_body and len(re.findall(r"0x[0-9a-fA-F]{4}",hero_body.group(1)))==26670
assert source.count("rift_hero[26670]")==1
assert '#include "rift112_scene_art.h"' in render
for name in ("event","shop"):
    assert f"const uint16_t rift112_{name}[32640]" in scenes
    assert f"sprite_scaled(0,28,480,224,rift112_{name},240,136);" in render
    assert f"static void {name}_screen(const Game*g){{" in render
assert len(re.findall(r"0x[0-9a-fA-F]{4}",scenes))==2*240*136, "scene pixel count wrong"

pic=p/"rift112_pic1.png"
with Image.open(pic) as im:
    assert im.size==(480,272) and im.mode=="RGB"
    im.verify()
old=p.parent/"assets"/"PIC1.PNG"
if old.exists():
    assert hashlib.sha256(pic.read_bytes()).digest()!=hashlib.sha256(old.read_bytes()).digest(), "XMB art unchanged"
print("PASS 1.1.2 art: actual hero + both scenes in compiled C, separate 480x272 PSP splash")
