"""Pack original generated 1.6 artwork; stable body anchors and native PSP formats."""
from pathlib import Path
from PIL import Image,ImageDraw
import numpy as np,json
from scipy.ndimage import label,binary_dilation
q=Path(__file__).parent;root=q.parents[1];header='#pragma once\n#include <stdint.h>\ntypedef struct {const uint8_t *pixels;const uint32_t *palette;} Rift160Sprite;\n';facts=[]
def emit(name,im,kind=32):
 global header
 a=np.array(im.convert('RGBA'),dtype=np.uint32)
 if kind==32:v=(a[:,:,0]|a[:,:,1]<<8|a[:,:,2]<<16|a[:,:,3]<<24).flatten().tolist()
 else:v=((a[:,:,0]>>3)<<11|(a[:,:,1]>>2)<<5|(a[:,:,2]>>3)).flatten().tolist()
 header+=f'static const uint{kind}_t {name}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[i:i+64]))+',' for i in range(0,len(v),64))+'\n};\n'
def indexed(name,im,rgba):
 global header
 if rgba:
  qi=im.convert('RGBA').quantize(colors=256,method=Image.Quantize.FASTOCTREE);pal=qi.getpalette('RGBA');pal=(pal+[0]*1024)[:1024];a=np.array(pal,dtype=np.uint32).reshape((256,4));colors=a[:,0]|a[:,1]<<8|a[:,2]<<16|a[:,3]<<24;kind=32
 else:
  qi=im.convert('RGB').quantize(colors=256,method=Image.Quantize.MEDIANCUT);pal=qi.getpalette('RGB');pal=(pal+[0]*768)[:768];a=np.array(pal,dtype=np.uint32).reshape((256,3));colors=(a[:,0]>>3)<<11|(a[:,1]>>2)<<5|(a[:,2]>>3);kind=16
 v=np.array(qi).flatten().tolist()
 header+=f'static const uint8_t {name}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[i:i+96]))+',' for i in range(0,len(v),96))+'\n};\n'
 header+=f'static const uint{kind}_t {name}_pal[256]={{'+','.join(map(str,colors.tolist()))+'};\n'

for char,filename,scale in [('nightshade','nightshade',.275),('regent','regent-clean',.32)]:
 im=Image.open(q/'art'/f'{filename}.png').convert('RGBA');a=np.array(im);labs,n=label(binary_dilation(a[:,:,3]>40,iterations=1));sizes=np.bincount(labs.ravel());frames=[]
 for i in range(20):
  row,col=divmod(i,5);cx=(col+.5)*im.width/5;cy=(row+.6)*im.height/4
  candidates=[k for k in range(1,n+1) if sizes[k]>10000]
  # Death blades can touch across cells; the first 17 frames are independent.
  if char=='regent' and i>=17:
   part=a.copy();part[:row*256,:,3]=0;part[(row+1)*256:,:,3]=0;part[:,:col*1536//5,3]=0;part[:,(col+1)*1536//5:,3]=0
  else:
   k=min(candidates,key=lambda k:((np.nonzero(labs==k)[1].mean()-cx)**2+(np.nonzero(labs==k)[0].mean()-cy)**2));mask=labs==k
   # Keep detached sparks only in their own casting cell, separate from neighboring bodies.
   if char=='regent' and i==13:
    yy,xx=np.indices(mask.shape);mask|=(xx>col*1536/5)&(xx<(col+1)*1536/5)&(yy>row*256)&(yy<(row+1)*256)&(a[:,:,3]>12)
   part=a.copy();part[:,:,3]=np.where(mask,part[:,:,3],0)
  anchorx=col*1536/5+153
  yy,xx=np.indices(part.shape[:2]);ground=(part[:,:,3]>150)&(xx>anchorx-65)&(xx<anchorx+75)&(yy>=row*256)&(yy<(row+1)*256)
  foot=int(np.nonzero(ground)[0].max())+1 if ground.any() else row*256+247
  crop=Image.fromarray(part).crop((round(anchorx-70/scale),round(foot-96/scale),round(anchorx+74/scale),round(foot)))
  frame=crop.resize((144,96),Image.Resampling.LANCZOS);frames.append(frame);frame.save(q/'assets'/f'{char}-{i:02d}.png');indexed(f'r160_{char}_{i}',frame,True)
  facts.append(dict(character=char,frame=i,anchor=[70,96],scale=scale,bounds=frame.getchannel('A').getbbox()))
 header+=f'static const Rift160Sprite r160_{char}[20]={{'+','.join('{'+f'r160_{char}_{i},r160_{char}_{i}_pal'+'}' for i in range(20))+'};\n'
 sheet=Image.new('RGBA',(720,384),(18,18,24,255))
 for i,fr in enumerate(frames):sheet.alpha_composite(fr,((i%5)*144,(i//5)*96))
 sheet.convert('RGB').save(q/'evidence'/f'{char}-runtime.png')
(q/'evidence/sprite-anchors.json').write_text(json.dumps(facts,indent=2)+'\n')
# 96 Regent cards/tokens and 53 solo Colorless cards, in the catalog's fixed order.
art=[]
for group,rows,count in [('regent',4,96),('colorless',3,53)]:
 for i in range(count):
  per=6*rows;sheet=Image.open(q/'art'/f'{group}-cards-{i//per}.png').convert('RGB');w,h=sheet.size;cell=i%per;x=cell%6*w//6;y=cell//6*h//rows;tile=sheet.crop((x+2,y+2,(cell%6+1)*w//6-2,(cell//6+1)*h//rows-2)).resize((160,112),Image.Resampling.LANCZOS)
  name=f'r160_card_{len(art)}';indexed(name,tile,False);art.append(name);tile.save(q/'assets'/f'card-{len(art)+184:03d}.png')
header+='static const uint8_t *const r160_cards[149]={'+','.join(art)+'};\n'
header+='static const uint16_t *const r160_card_palettes[149]={'+','.join(x+'_pal' for x in art)+'};\n'
icons=Image.open(q/'art/icons.png').convert('RGBA');w,h=icons.size
for i,(name,size) in enumerate([('star',(40,40)),('spark',(24,24)),('select',(40,14)),('crown',(80,80))]):
 tile=icons.crop((i%2*w//2,i//2*h//2,(i%2+1)*w//2,(i//2+1)*h//2));tile=tile.crop(tile.getchannel('A').point(lambda a:255 if a>50 else 0).getbbox());tile.thumbnail(size,Image.Resampling.LANCZOS);out=Image.new('RGBA',size);out.alpha_composite(tile,((size[0]-tile.width)//2,(size[1]-tile.height)//2));emit(f'r160_{name}',out);out.save(q/'assets'/f'{name}.png')
for folder in ['GAME/src','portable_build']:(root/folder/'rift160_art.h').write_text(header)
print('Integrated 40 fixed-anchor new character frames, 149 original card illustrations and 4 UI icons')
