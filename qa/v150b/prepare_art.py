from pathlib import Path
from PIL import Image
import numpy as np
from scipy.ndimage import label,binary_dilation
import json,sys
q=Path(__file__).parent;root=q.parents[1];partial='--partial' in sys.argv
(q/'assets').mkdir(exist_ok=True)
header='#pragma once\n#include <stdint.h>\n'
def emit(name,im,bits=32):
 global header
 rgba=np.array(im.convert('RGBA'),dtype=np.uint32)
 if bits==16:v=((rgba[:,:,0]>>3)<<11)|((rgba[:,:,1]>>2)<<5)|(rgba[:,:,2]>>3)
 else:v=rgba[:,:,0]|rgba[:,:,1]<<8|rgba[:,:,2]<<16|rgba[:,:,3]<<24
 v=v.flatten().tolist();header+=f'static const uint{bits}_t {name}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[j:j+64]))+',' for j in range(0,len(v),64))+'\n};\n'
# Each action has a separate generated 2x2 atlas. Measured hip/boot anchors
# prevent the apparent motion caused by recentering every bounding box.
facts=[];frames=[]
anchors={
 'idle':[(390,601),(1000,601),(395,1218),(1003,1218)],
 'attack':[(385,617),(1010,618),(420,1191),(1030,1194)],
 'block':[(380,619),(1000,619),(370,1193),(1000,1193)],
 'power':[(390,617),(1000,617),(390,1217),(1000,1217)],
 'death':[(390,715),(1030,759),(400,1155),(1010,1172)]}
for action in ['idle','attack','block','power','death']:
 im=Image.open(q/'art'/('ninja-'+action+'.png')).convert('RGBA');rgba=np.array(im)
 labels,n=label(binary_dilation(rgba[:,:,3]>30,iterations=2));sizes=np.bincount(labels.ravel());ids=sorted(range(1,n+1),key=lambda i:sizes[i],reverse=True)[:4]
 ids.sort(key=lambda i:np.nonzero(labels==i)[0].mean());ordered=[]
 for row in range(2):ordered+=sorted(ids[row*2:row*2+2],key=lambda i:np.nonzero(labels==i)[1].mean())
 for local,lab in enumerate(ordered):
  i=len(frames);mask=labels==lab;ys,xs=np.nonzero(mask);bounds=(int(xs.min()),int(ys.min()),int(xs.max()+1),int(ys.max()+1))
  part=rgba.copy();part[:,:,3]=np.where(mask,part[:,:,3],0);tile=Image.fromarray(part)
  ax,foot=anchors[action][local];box=(ax-427,foot-667,ax+467,foot)
  frame=tile.crop(box).resize((134,100),Image.Resampling.LANCZOS);frame.save(q/'assets'/f'ninja_{i:02d}.png');frames.append(frame);emit(f'r150b_ninja_{i}',frame)
  facts.append({'frame':i,'action':action,'source_bounds':bounds,'source_anchor':[ax,foot],'runtime_anchor':[64,100],'runtime_bounds':frame.getchannel('A').getbbox()})
header+='static const uint32_t *const r150b_ninja[20]={'+','.join(f'r150b_ninja_{i}' for i in range(20))+'};\n'
(q/'evidence/ninja-anchors.json').write_text(json.dumps(facts,indent=2)+'\n')
canvas=Image.new('RGBA',(4*134,5*100),(28,27,30,255))
for i,im in enumerate(frames):canvas.alpha_composite(im,(i%4*134,i//4*100))
canvas.convert('RGB').save(q/'evidence/ninja-frames.png')
# Generated inventory/status emblems and six dagger-contact frames.
for stem,cols,rows,size in [('icons',4,4,80),('dagger-fx',3,2,112)]:
 im=Image.open(q/'art'/(stem+'.png')).convert('RGBA');w,h=im.size;names=[]
 for i in range(cols*rows):
  cell=im.crop((i%cols*w//cols,i//cols*h//rows,(i%cols+1)*w//cols,(i//cols+1)*h//rows))
  if stem=='icons':
   box=cell.getchannel('A').point(lambda a:255 if a>24 else 0).getbbox();cell=cell.crop(box)
   tile=Image.new('RGBA',(size,size));cell.thumbnail((size-4,size-4),Image.Resampling.LANCZOS);tile.alpha_composite(cell,((size-cell.width)//2,(size-cell.height)//2))
  else:tile=cell.resize((size,size),Image.Resampling.LANCZOS)
  name=f'r150b_{stem.replace("-","_")}_{i}';names.append(name);emit(name,tile);tile.save(q/'assets'/(name+'.png'))
 header+=f'static const uint32_t *const r150b_{stem.replace("-","_")}[{len(names)}]={{'+','.join(names)+'};\n'
missing=[];thumbs=[]
for id in range(96,185):
 path=q/'art'/f'card_{id}.jpg'
 if not path.exists():
  missing.append(id)
  if not partial:raise SystemExit(f'Missing required generated card art {id}')
  path=q/'art/card_96.jpg'
 im=Image.open(path).convert('RGB');w,h=im.size
 if w/h>1.5:delta=(w-h*1.5)/2;im=im.crop((int(delta),0,int(w-delta),h))
 else:im=im.crop((0,0,w,int(w/1.5)))
 im=im.resize((180,120),Image.Resampling.LANCZOS);im.save(q/'assets'/f'card_{id}.png');emit(f'r150b_card_{id}',im,16)
 thumbs.append(im.resize((90,60),Image.Resampling.LANCZOS))
header+='static const uint16_t *const r150b_cards[89]={'+','.join(f'r150b_card_{id}' for id in range(96,185))+'};\n'
for folder in ['GAME/src','portable_build']:(root/folder/'rift150b_art.h').write_text(header)
canvas=Image.new('RGB',(90*9,78*10),(20,20,25));from PIL import ImageDraw
draw=ImageDraw.Draw(canvas)
for i,im in enumerate(thumbs):x=i%9*90;y=i//9*78;canvas.paste(im,(x,y));draw.text((x+3,y+61),str(96+i),fill='white')
canvas.save(q/'evidence/cards-contact.png')
print('Generated runtime art; missing originals:',missing)
