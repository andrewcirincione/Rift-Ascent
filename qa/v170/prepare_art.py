"""Pack the generated paintings and matching boss cutouts; no replacement drawings."""
from pathlib import Path
from PIL import Image
import numpy as np,json
q=Path(__file__).parent;lines=['#pragma once\n'];sizes=[]
def emit(name,im,bits):
 a=np.asarray(im.convert('RGBA'),dtype=np.uint32)
 data=(a[:,:,0]>>3)<<11|(a[:,:,1]>>2)<<5|(a[:,:,2]>>3) if bits==16 else a[:,:,0]|a[:,:,1]<<8|a[:,:,2]<<16|a[:,:,3]<<24
 lines.append(f'static const uint{bits}_t {name}[{data.size}]={{'+','.join(map(str,data.ravel().tolist()))+'};\n')
for c,name in enumerate(['warden','vesper','proxy']):
 for kind,rows in [('entries',4),('bosses',3)]:
  im=Image.open(q/'art'/f'{name}-{kind}.png').convert('RGB')
  for a in range(rows):
   tile=im.crop((0,round(a*im.height/rows),im.width,round((a+1)*im.height/rows))).resize((336,126),Image.Resampling.LANCZOS)
   tile.save(q/'assets'/f'{name}-{kind}-{a}.png');emit(f'r170_{kind}_{c}_{a}',tile,16)
lines.append('static const uint16_t*const r170_entry_images[3][4]={'+','.join('{'+','.join(f'r170_entries_{c}_{a}' for a in range(4))+'}' for c in range(3))+'};\n')
lines.append('static const uint16_t*const r170_boss_images[3][3]={'+','.join('{'+','.join(f'r170_bosses_{c}_{a}' for a in range(3))+'}' for c in range(3))+'};\n')
for i,name in enumerate(['furnace-maw','crimson-judge','hollow-king']):
 im=Image.open(q/'art'/f'{name}.png').convert('RGBA');a=np.asarray(im.getchannel('A'));assert a.min()==0 and (a==0).mean()>.1
 box=im.getchannel('A').point(lambda x:255 if x>12 else 0).getbbox();im=im.crop(box)
 h=196;w=round(im.width*h/im.height);im=im.resize((w,h),Image.Resampling.LANCZOS);im.save(q/'assets'/f'{name}.png');sizes.append([w,h]);emit(f'r170_boss_sprite_{i}',im,32)
lines.append('static const uint32_t*const r170_boss_sprites[3]={r170_boss_sprite_0,r170_boss_sprite_1,r170_boss_sprite_2};\n')
lines.append('static const int r170_boss_size[3][2]={'+','.join('{'+str(w)+','+str(h)+'}' for w,h in sizes)+'};\n')
(q/'source/rift170_art.h').write_text(''.join(lines));(q/'evidence/boss-sizes.json').write_text(json.dumps(sizes)+'\n')
print('PASS packed 21 scene paintings and 3 matching transparent boss sprites')
