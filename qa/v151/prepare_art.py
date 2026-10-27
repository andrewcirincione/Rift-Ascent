"""Integrate newly generated 1.5.1 atlases with fixed body anchors."""
from pathlib import Path
from PIL import Image,ImageDraw
import numpy as np
from scipy.ndimage import label,binary_dilation
import json
q=Path(__file__).parent;root=q.parents[1];(q/'assets').mkdir(exist_ok=True)
header='#pragma once\n#include <stdint.h>\n'
def emit(name,im):
 global header
 a=np.array(im.convert('RGBA'),dtype=np.uint32);v=(a[:,:,0]|a[:,:,1]<<8|a[:,:,2]<<16|a[:,:,3]<<24).flatten().tolist()
 header+=f'static const uint32_t {name}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[i:i+64]))+',' for i in range(0,len(v),64))+'\n};\n'
anchors={'idle':[(449,511),(1174,511),(449,1055),(1174,1055)],'attack':[(425,548),(1110,548),(425,1046),(1163,1046)],'block':[(449,523),(1174,523),(449,1060),(1174,1060)],'power':[(449,523),(1080,529),(449,1066),(1174,1066)],'death':[(449,548),(1140,548),(449,1003),(1140,1003)]}
frames=[];facts=[]
for action in anchors:
 im=Image.open(q/'art'/f'nightshade-{action}.png').convert('RGBA');a=np.array(im)
 labs,n=label(binary_dilation(a[:,:,3]>30,iterations=2));sizes=np.bincount(labs.ravel());ids=sorted(range(1,n+1),key=lambda i:sizes[i],reverse=True)[:4];ids.sort(key=lambda i:np.nonzero(labs==i)[0].mean());ordered=[]
 for row in range(2):ordered+=sorted(ids[row*2:row*2+2],key=lambda i:np.nonzero(labs==i)[1].mean())
 for i,lab in enumerate(ordered):
  mask=labs==lab;part=a.copy();part[:,:,3]=np.where(mask,part[:,:,3],0);ax,foot=anchors[action][i]
  # A single scale for every pose; only measured hip/ground anchors move.
  frame=Image.fromarray(part).crop((ax-444,foot-556,ax+390,foot)).resize((150,100),Image.Resampling.LANCZOS)
  idx=len(frames);frame.save(q/'assets'/f'nightshade-{idx:02d}.png');emit(f'r151_ninja_{idx}',frame);frames.append(frame)
  facts.append(dict(action=action,frame=idx,source_anchor=[ax,foot],runtime_anchor=[80,100],bounds=frame.getchannel('A').getbbox()))
header+='static const uint32_t *const r151_ninja[20]={'+','.join(f'r151_ninja_{i}' for i in range(20))+'};\n'
canvas=Image.new('RGBA',(600,500),(28,27,30,255))
for i,im in enumerate(frames):canvas.alpha_composite(im,(i%4*150,i//4*100))
canvas.convert('RGB').save(q/'evidence/runtime-sprites.png');(q/'evidence/sprite-anchors.json').write_text(json.dumps(facts,indent=2)+'\n')
im=Image.open(q/'art/relics.png').convert('RGBA');w,h=im.size
for i in range(10):
 cell=im.crop((i%5*w//5,i//5*h//2,(i%5+1)*w//5,(i//5+1)*h//2));cell=cell.crop(cell.getchannel('A').point(lambda a:255 if a>24 else 0).getbbox());cell.thumbnail((76,76),Image.Resampling.LANCZOS);tile=Image.new('RGBA',(80,80));tile.alpha_composite(cell,((80-cell.width)//2,(80-cell.height)//2));emit(f'r151_relic_{i}',tile);tile.save(q/'assets'/f'relic-{53+i}.png')
header+='static const uint32_t *const r151_relics[10]={'+','.join(f'r151_relic_{i}' for i in range(10))+'};\n'
im=Image.open(q/'art/selection.png').convert('RGBA');a=np.array(im);labs,n=label(a[:,:,3]>32);sizes=np.bincount(labs.ravel());ys,xs=np.nonzero(labs==sizes[1:].argmax()+1);im=im.crop((xs.min(),ys.min(),xs.max()+1,ys.max()+1)).resize((420,60),Image.Resampling.LANCZOS);emit('r151_selection',im);im.save(q/'assets/selection.png')
for folder in ['GAME/src','portable_build']:(root/folder/'rift151_art.h').write_text(header)
print('Integrated 20 new Nightshade frames, ten relic emblems, selection tablet')
