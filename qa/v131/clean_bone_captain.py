#!/usr/bin/env python3
"""Remove the three white extraction-guide lines from Bone Captain sprite."""
from pathlib import Path
import re,sys
p=Path(sys.argv[1]);s=p.read_text()
m=re.search(r'(const uint16_t rift_enemy_14\[[^]]+\]\s*=\s*\{\s*)(.*?)(\s*\};)',s,re.S)
if not m:raise RuntimeError("Bone Captain texture not found")
vals=[int(x,16) for x in re.findall(r'0x[0-9a-fA-F]+',m.group(2))]
if len(vals)!=104*110:raise RuntimeError(f"unexpected sprite dimensions {len(vals)}")
removed=0
for i,c in enumerate(vals):
 x,y=i%104,i//104
 if (x<18 or x>86 or y>99) and (c>>11&31)>=23 and (c>>5&63)>=45 and (c&31)>=23:
  vals[i]=0xf81f;removed+=1
if not 20<removed<1200:raise RuntimeError(f"unexpected guide line count {removed}")
body='\n'+''.join(''.join('0x%04x,'%x for x in vals[j:j+32])+'\n'
                  for j in range(0,len(vals),32))
s=s[:m.start()]+m.group(1)+body+m.group(3)+s[m.end():]
p.write_text(s)
print("PASS Bone Captain: removed",removed,"white guide-line pixels, preserved remaining sprite")
