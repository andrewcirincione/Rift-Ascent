from pathlib import Path
from PIL import Image
import numpy as np
from scipy.ndimage import label,binary_dilation
q=Path(__file__).parent;assets=q/'assets';assets.mkdir(exist_ok=True)
im=Image.open(q/'originals/ui-atlas.png').convert('RGBA')
for name,box,size in [('select',(0,40,960,365),(208,26)),('end',(970,90,1536,365),(66,31)),('shield',(100,380,760,1024),(28,30)),('status',(770,370,1536,1024),(88,88))]:
 cell=im.crop(box);bounds=cell.getchannel('A').point(lambda a:255 if a>10 else 0).getbbox();cell=cell.crop(bounds);cell.resize(size,Image.Resampling.LANCZOS).save(assets/(name+'.png'))
im=Image.open(q/'originals/hound.png').convert('RGBA');im=im.crop(im.getchannel('A').getbbox());im.thumbnail((132,104),Image.Resampling.LANCZOS);canvas=Image.new('RGBA',(132,104));canvas.alpha_composite(im,((132-im.width)//2,104-im.height));canvas.save(assets/'hound.png')
# Preserve generated poses; extract one actor and align foot stances. Render x=47 anchors these to the idle hip position.
for i,shift in [(0,0),(1,-4),(2,2),(3,2),(4,4),(5,4)]:
 im=Image.open(q.parent/'v137/assets'/f'hero_{i}.png').convert('RGBA');a=np.array(im);lab,n=label(a[:,:,3]>4);sizes=np.bincount(lab.ravel());sizes[0]=0;mask=binary_dilation(lab==sizes.argmax(),iterations=2);a[:,:,3]=np.where(mask,a[:,:,3],0)
 im=Image.fromarray(a);canvas=Image.new('RGBA',(208,144));canvas.alpha_composite(im,(8+shift,0));canvas.save(assets/f'attack_{i}.png')
header='#pragma once\n#include <stdint.h>\n'
for p in sorted(assets.glob('*.png')):
 im=Image.open(p).convert('RGBA');vals=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()]
 header+=f'#define R139_{p.stem.upper()}_W {im.width}\n#define R139_{p.stem.upper()}_H {im.height}\nstatic const uint32_t r139_{p.stem}[{len(vals)}]={{\n'+'\n'.join(','.join(map(str,vals[i:i+64]))+',' for i in range(0,len(vals),64))+'\n};\n'
header+='static const uint32_t *const r139_attack[6]={'+','.join('r139_attack_'+str(i) for i in range(6))+'};\n'
for dst in ['GAME/src','portable_build']:(Path(dst)/'rift139_art.h').write_text(header)
