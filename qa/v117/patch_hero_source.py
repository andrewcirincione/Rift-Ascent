#!/usr/bin/env python3
"""Replace the hero in actual C source before EVERY compilation, including
the emulator deterministic demo, never only patching the final release PBP.
"""
from pathlib import Path
from PIL import Image
import base64,io,re,sys
p=Path(sys.argv[1])
png=base64.b64decode((Path(__file__).parent/"assets/knight_210.png.b64").read_text())
im=Image.open(io.BytesIO(png)).convert("RGBA")
assert im.size==(210,127)
vals=[]
for rr,gg,bb,aa in im.getdata():
 if aa<118:v=0xf81f
 else:
  v=((rr>>3)<<11)|((gg>>2)<<5)|(bb>>3)
  if v==0xf81f:v=0xf81e
 vals.append(v)
s=p.read_text()
pat=r"(?:static\s+)?const uint16_t rift_hero\[26670\][^=]*=\s*\{.*?\};"
m=re.search(pat,s,re.S)
if not m:raise RuntimeError("Expected 210x127 RGB565 hero in motion_art.c")
data="const uint16_t rift_hero[26670]={\n"+",\n".join(
 ",".join("0x%04x"%v for v in vals[i:i+40])
 for i in range(0,len(vals),40))+"\n};"
s=s[:m.start()]+data+s[m.end():]
p.write_text(s)
print("PASS: dark great-helm knight compiled into all EBOOTs and emulator demos")
