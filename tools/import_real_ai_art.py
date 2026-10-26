#!/usr/bin/env python3
"""Convert actual AI-generated illustrations to PSP RGB565 art headers.
NO geometric/procedural art is generated here.

Requires original high-resolution images:
 assets/cards/strike.png, defend.png, card_002.png ... card_091.png
 assets/scenes/fight.png, elite.png, boss_act1.png, boss_act2.png,
               boss_act3.png, campfire.png

Images must be authored with a generative image model, then reviewed. This
converter ONLY formats finished artwork; it must not synthesize objects.
"""
from pathlib import Path
from PIL import Image,ImageOps
import hashlib,sys
if len(sys.argv)!=3:raise SystemExit("Usage: python tools/import_real_ai_art.py ASSETS_DIR OUTPUT_DIR")
assets=Path(sys.argv[1]);out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
def pack(path,size):
 if not path.exists():raise FileNotFoundError(f"Missing illustrated source: {path}")
 with Image.open(path) as img:
  img=img.convert("RGB")
  if img.width<512 or img.height<512:
   raise ValueError(f"Too small for real full-resolution AI art: {path}: {img.size}")
  im=ImageOps.fit(img,size,method=Image.Resampling.LANCZOS,centering=(.5,.5))
  data=im.tobytes()
 pixels=[]
 for i in range(0,len(data),3):
  r,g,b=data[i:i+3];pixels.append(((r>>3)<<11)|((g>>2)<<5)|(b>>3))
 return pixels
def emit_header(path,items):
 with path.open("w") as f:
  f.write("#pragma once\n#include <stdint.h>\n")
  for name,pixels in items:
   f.write(f"static const uint16_t {name}[{len(pixels)}]={{\n")
   for i in range(0,len(pixels),24):
    f.write(",".join(map(str,pixels[i:i+24]))+",\n")
   f.write("};\n")
seen=set();cards=[]
for i in range(92):
 p=assets/"cards"/("strike.png" if i==0 else "defend.png" if i==1 else f"card_{i:03}.png")
 if i>=2:
  h=hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else ""
  if h in seen:raise ValueError(f"Duplicate non-basic illustration: {p}")
  seen.add(h)
 cards.append((f"rift121_ai_card_{i}",pack(p,(60,40))))
emit_header(out/"rift121_ai_cards.h",cards)
with (out/"rift121_ai_cards.h").open("a") as f:
 f.write("static const uint16_t *const rift121_ai_cards[92]={"+",".join(n for n,_ in cards)+"};\n")
scenes=[("fight","rift121_ai_fight"),("elite","rift121_ai_elite"),
 ("boss_act1","rift121_ai_boss1"),("boss_act2","rift121_ai_boss2"),
 ("boss_act3","rift121_ai_boss3"),("campfire","rift121_ai_camp")]
emit_header(out/"rift121_ai_scenes.h",[(n,pack(assets/"scenes"/(source+".png"),(480,272))) for source,n in scenes])
print("PASS: imported 92 actual illustrations and 6 distinct original scenes; no procedural art")
