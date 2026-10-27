"""Pack generated illustrations at PSP resolution; no substitute drawings."""
from pathlib import Path
from PIL import Image,ImageOps
import numpy as np,json
q=Path(__file__).parent;header=['#pragma once\n']
def emit(name,im,bits):
 a=np.array(im.convert('RGBA'),dtype=np.uint32)
 values=(a[:,:,0]>>3)<<11|(a[:,:,1]>>2)<<5|(a[:,:,2]>>3) if bits==16 else a[:,:,0]|a[:,:,1]<<8|a[:,:,2]<<16|a[:,:,3]<<24
 v=values.ravel().tolist();header.append(f'static const uint{bits}_t {name}[{len(v)}]={{'+','.join(map(str,v))+'};\n')
for b in range(6):
 im=Image.open(q/'art'/f'warden-cards-{b}.png').convert('RGB')
 for i in range(16):
  x,y=i%4,i//4;card=im.crop((round(x*im.width/4),round(y*im.height/4),round((x+1)*im.width/4),round((y+1)*im.height/4))).resize((240,160),Image.Resampling.LANCZOS)
  card.save(q/'assets'/f'warden-card-{b*16+i:02d}.png');emit(f'r162_card_{b*16+i}',card,16)
header.append('static const uint16_t*const r162_cards[96]={'+','.join(f'r162_card_{i}' for i in range(96))+'};\n')
# Nine complete places per region, all derived from the same rift reference.
for act in range(3):
 im=Image.open(q/'art'/f'rift-act-{act+1}-world.png').convert('RGB')
 for i in range(9):
  x,y=i%3,i//3;tile=im.crop((round(x*im.width/3),round(y*im.height/3),round((x+1)*im.width/3),round((y+1)*im.height/3)))
  tile=ImageOps.fit(tile,(480,272),method=Image.Resampling.LANCZOS)
  tile.save(q/'assets'/f'world-{act+1}-{i}.png');emit(f'r162_world_{act}_{i}',tile,16)
header.append('static const uint16_t*const r162_world[3][9]={'+','.join('{'+','.join(f'r162_world_{a}_{i}' for i in range(9))+'}' for a in range(3))+'};\n')
im=ImageOps.fit(Image.open(q/'art/act-endless.png').convert('RGB'),(480,272),method=Image.Resampling.LANCZOS);im.save(q/'assets/act-endless.png');emit('r162_act_endless',im,16)
header.append('static const uint16_t*const r162_acts[4]={r162_world_0_0,r162_world_1_0,r162_world_2_0,r162_act_endless};\n')
im=ImageOps.fit(Image.open(q/'art/rift-menu-world.png').convert('RGB'),(480,272),method=Image.Resampling.LANCZOS);im.save(q/'assets/rift-menu-world.png');emit('r162_menu',im,16)
for name,size,ident in [('hand-arrow',(36,28),'r162_arrow'),('pause-panel',(360,180),'r162_pause')]:
 im=Image.open(q/'art'/f'{name}.png').convert('RGBA');im=im.crop(im.getchannel('A').getbbox()).resize(size,Image.Resampling.LANCZOS)
 im.save(q/'assets'/f'{name}.png');emit(ident,im,32)
p=q/'source/rift162_art.h';new=''.join(header)
if not p.exists() or p.read_text()!=new:p.write_text(new)
print('Packed 96 new 240x160 Warden paintings, 27 act environments, rift menu, endless intro, arrow and pause panel')
