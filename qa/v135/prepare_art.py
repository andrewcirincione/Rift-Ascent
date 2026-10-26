from pathlib import Path
from PIL import Image
import sys
q=Path(__file__).parent; dest=Path(sys.argv[1]) if len(sys.argv)>1 else Path('GAME/src')
names=['strength','dexterity','cross','circle','square','start','weak','vulnerable','poison','energy_drain'];sizes=[(22,22),(22,22),(14,14),(14,14),(14,14),(22,14)]+[(22,22)]*4
if '--extract' in sys.argv:
 im=Image.open(q/'icons-original.png').convert('RGBA');w,h=im.size
 # Separations follow the observed transparent gutters in the generated atlas.
 ys=[0,round(h*.397),round(h*.687),h]
 for i,(name,size) in enumerate(zip(names[:6],sizes[:6])):
  cell=im.crop((i%2*w//2,ys[i//2],(i%2+1)*w//2,ys[i//2+1]))
  box=cell.getchannel('A').point(lambda a:255 if a>=16 else 0).getbbox();assert box
  cell=cell.crop(box);cell.thumbnail(size,Image.Resampling.LANCZOS)
  out=Image.new('RGBA',size);out.alpha_composite(cell,((size[0]-cell.width)//2,(size[1]-cell.height)//2));out.save(q/'assets'/(name+'.png'))
s='#pragma once\n#include <stdint.h>\n'
for name,size in zip(names,sizes):
 im=Image.open(q/'assets'/(name+'.png')).convert('RGBA');assert im.size==size
 vals=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()]
 s+='static const uint32_t r135_'+name+'[%d]={\n'%len(vals)+'\n'.join(','.join(map(str,vals[i:i+32]))+',' for i in range(0,len(vals),32))+'\n};\n'
(dest/'rift135_art.h').write_text(s)
print('Packed generated status and controller icons')
