#!/usr/bin/env python3
"""v1.3.0 binary-texture slots; replaced post-compilation with human-reviewed AI art.
Sentinels are not final game assets. Strict package audit rejects unpatched slots.
"""
from pathlib import Path
import re,sys
p=Path(sys.argv[1])/"rift130_slots.h"
names=["map1","map2","map3","boss1","boss2","boss3","normal1","normal2","elite1","elite2","camp"]
# Reference texture just establishes the final array length; no generated shapes.
source=(Path(sys.argv[1])/"rift120_scenes.h").read_text()
m=re.search(r'static const uint16_t rift120_fight_bg\[(\d+)\]=\{\s*(.*?)\s*\};',source,re.S)
if not m:raise RuntimeError("1.2.0 source image unavailable")
src=[int(v) for v in re.findall(r'\d+',m.group(2))]
assert len(src)==int(m.group(1))==480*272
with p.open("w") as f:
 f.write("#pragma once\n#include <stdint.h>\n")
 for i,name in enumerate(names):
  # 64 unique identifying pixel values allow an exact binary patch after PSPDEV.
  vals=src.copy()
  vals[:64]=[0x6111+i]*64
  f.write(f"static const uint16_t rift130_{name}[130560]={{\n")
  for pos in range(0,len(vals),64):
   f.write(",".join(str(n) for n in vals[pos:pos+64])+",\n")
  f.write("};\n")
print("PASS 1.3.0: 11 unique linker-visible art slots prepared")
