#!/usr/bin/env python3
"""Remove only disconnected drawing guides from Oathbreaker/Cinder Hound."""
from pathlib import Path
import re,sys
p=Path(sys.argv[1]);s=p.read_text()
for index in (9,12):
 pat=rf'(const uint16_t rift_enemy_{index}\[[^]]+\]\s*=\s*\{{)(.*?)(\}};)'
 m=re.search(pat,s,re.S)
 if not m:raise RuntimeError(f"Enemy sprite {index} not found")
 vals=[int(v,16) for v in re.findall(r'0x[0-9a-fA-F]+',m.group(2))]
 if len(vals)!=104*110:raise RuntimeError(f"Unexpected sprite pixels: {index}")
 n=0
 for j,v in enumerate(vals):
  x,y=j%104,j//104
  r=((v>>11)&31)*255//31;g=((v>>5)&63)*255//63;b=(v&31)*255//31
  if index==9:
   # Cyan unconnected triangular arrow above Cinder Hound's head.
   bad=y<=29 and 38<=x<=68 and r<120 and g>125 and b>110
  else:
   # Oathbreaker's bright orange halo and detached diagonal guide strokes.
   bright=r>170 and 50<=g<190 and b<130 and v!=0xf81f
   bad=bright and ((y<34 and 20<=x<=84) or
                   (55<y<95 and (x<30 or x>76)))
  if bad:vals[j]=0xf81f;n+=1
 if index==9 and not 40<=n<=140:raise RuntimeError(f"Arrow mask unexpectedly removed {n}")
 if index==12 and not 100<=n<=270:raise RuntimeError(f"Oathbreaker mask unexpectedly removed {n}")
 body='\n'+''.join(''.join(f'0x{v:04x},' for v in vals[q:q+32])+'\n' for q in range(0,len(vals),32))
 s=s[:m.start()]+m.group(1)+body+m.group(3)+s[m.end():]
 print(f"PASS enemy {index}: removed {n} annotation pixels")
p.write_text(s)
