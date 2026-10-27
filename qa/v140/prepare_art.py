"""Pack generated 1.4.0 raster art, retaining alpha and text-safe interiors."""
from pathlib import Path
from PIL import Image
import subprocess,sys
q=Path(__file__).parent;repo=q.parents[1]
for name,size in [('victory',(324,100)),('selection',(208,28)),('energy_empty',(46,46))]:
 im=Image.open(q/'art'/f'{name}.png').convert('RGBA');im=im.crop(im.getchannel('A').point(lambda x:255 if x>12 else 0).getbbox());im.resize(size,Image.Resampling.LANCZOS).save(q/'assets'/f'{name}.png')
im=Image.open(q/'art/chest.png').convert('RGBA');im=im.crop(im.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox());im.thumbnail((32,32),Image.Resampling.LANCZOS)
canvas=Image.new('RGBA',(36,36));canvas.alpha_composite(im,((36-im.width)//2,(36-im.height)//2));canvas.save(q/'assets/chest.png')
# Fixed-cell extraction avoids re-centering the flame sequence between frames.
im=Image.open(q/'art/flames.png').convert('RGBA')
for i in range(6):
 x=i%3*512;y=i//3*512
 im.crop((x,y,x+512,y+512)).resize((64,48),Image.Resampling.LANCZOS).save(q/'assets'/f'flame_{i}.png')
subprocess.run([sys.executable,str(q/'prepare_attack.py')],check=True)
header='#pragma once\n#include <stdint.h>\n'
for p in sorted((q/'assets').glob('*.png')):
 im=Image.open(p).convert('RGBA');v=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()]
 header+=f'static const uint32_t r140_{p.stem}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[i:i+64]))+',' for i in range(0,len(v),64))+'\n};\n'
header+='static const uint32_t *const r140_attack[6]={'+','.join('r140_attack_'+str(i) for i in range(6))+'};\n'
for dst in ['GAME/src','portable_build']:(repo/dst/'rift140_art.h').write_text(header)
