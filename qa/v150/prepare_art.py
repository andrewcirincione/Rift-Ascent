from pathlib import Path
from PIL import Image
q=Path(__file__).parent;repo=q.parents[1]
im=Image.open(q/'art/event-relic-atlas.png').convert('RGBA')
# The generated atlas uses 3 equal cells per axis. Retain alpha, crop empty margin,
# then fit each complete object inside a 76px region with a transparent 2px guard.
names=['crypt','anvil','scriptorium','auction','bargain','hourglass','bell','needle','reliquary']
header='#pragma once\n#include <stdint.h>\n'
for i,name in enumerate(names):
 tile=im.crop((i%3*im.width//3,i//3*im.height//3,(i%3+1)*im.width//3,(i//3+1)*im.height//3))
 tile=tile.crop(tile.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox());tile.thumbnail((76,76),Image.Resampling.LANCZOS)
 canvas=Image.new('RGBA',(80,80));canvas.alpha_composite(tile,((80-tile.width)//2,(80-tile.height)//2));canvas.save(q/'assets'/f'{name}.png')
 v=[r|g<<8|b<<16|a<<24 for r,g,b,a in canvas.get_flattened_data()]
 header+=f'static const uint32_t r150_{name}[6400]={{\n'+'\n'.join(','.join(map(str,v[j:j+64]))+',' for j in range(0,len(v),64))+'\n};\n'
header+='static const uint32_t *const r150_events[5]={r150_crypt,r150_anvil,r150_scriptorium,r150_auction,r150_bargain};\n'
header+='static const uint32_t *const r150_relics[4]={r150_hourglass,r150_bell,r150_needle,r150_reliquary};\n'
for dst in ['GAME/src','portable_build']:(repo/dst/'rift150_art.h').write_text(header)
seals=Image.open(q/'art/modification-seals.png').convert('RGBA')
for i,name in enumerate(['echo','bound','keen','ward','fleeting','kindled']):
 tile=seals.crop((i%3*seals.width//3,i//3*seals.height//2,(i%3+1)*seals.width//3,(i//3+1)*seals.height//2))
 tile=tile.crop(tile.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox());tile.thumbnail((38,38),Image.Resampling.LANCZOS)
 canvas=Image.new('RGBA',(40,40));canvas.alpha_composite(tile,((40-tile.width)//2,(40-tile.height)//2));canvas.save(q/'assets'/f'seal_{name}.png')
 v=[r|g<<8|b<<16|a<<24 for r,g,b,a in canvas.get_flattened_data()]
 header+=f'static const uint32_t r150_seal_{name}[1600]={{\n'+'\n'.join(','.join(map(str,v[j:j+64]))+',' for j in range(0,len(v),64))+'\n};\n'
header+='static const uint32_t *const r150_seals[6]={'+','.join('r150_seal_'+n for n in ['echo','bound','keen','ward','fleeting','kindled'])+'};\n'
for dst in ['GAME/src','portable_build']:(repo/dst/'rift150_art.h').write_text(header)
# Connected components retain long weapons that cross nominal atlas cells.
import numpy as np,json
from scipy.ndimage import label,binary_dilation
rgba=np.array(Image.open(q/'art/enemies.png').convert('RGBA'))
labels,n=label(rgba[:,:,3]>100);sizes=np.bincount(labels.ravel());ids=sorted(range(1,n+1),key=lambda i:sizes[i],reverse=True)[:8]
ids=sorted(ids,key=lambda i:(np.nonzero(labels==i)[0].mean()>rgba.shape[0]/2,np.nonzero(labels==i)[1].mean()))
facts=[];dims=[]
for i,lab in enumerate(ids):
 part=rgba.copy();part[:,:,3]=np.where(binary_dilation(labels==lab,iterations=2),part[:,:,3],0)
 tile=Image.fromarray(part);box=tile.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox();tile=tile.crop(box);tile.thumbnail((160,160),Image.Resampling.LANCZOS)
 tile.save(q/'assets'/f'enemy_{i+36}.png');dims.append(tile.size);facts.append({'id':i+36,'bounds':box,'size':tile.size})
 v=[r|g<<8|b<<16|a<<24 for r,g,b,a in tile.get_flattened_data()]
 header+=f'static const uint32_t r150_enemy_{i+36}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[j:j+64]))+',' for j in range(0,len(v),64))+'\n};\n'
header+='static const uint32_t *const r150_enemies[8]={'+','.join(f'r150_enemy_{i}' for i in range(36,44))+'};\n'
header+='static const int r150_enemy_size[8][2]={'+','.join('{%d,%d}'%xy for xy in dims)+'};\n'
(q/'evidence/enemy-art-bounds.json').write_text(json.dumps(facts,indent=2)+'\n')
cards=Image.open(q/'art/cards.png').convert('RGBA')
for i in range(4):
 tile=cards.crop((i%2*cards.width//2,i//2*cards.height//2,(i%2+1)*cards.width//2,(i//2+1)*cards.height//2)).resize((240,160),Image.Resampling.LANCZOS)
 tile.save(q/'assets'/f'card_{i+92}.png');v=[r|g<<8|b<<16|a<<24 for r,g,b,a in tile.get_flattened_data()]
 header+=f'static const uint32_t r150_card_{i+92}[{len(v)}]={{\n'+'\n'.join(','.join(map(str,v[j:j+64]))+',' for j in range(0,len(v),64))+'\n};\n'
header+='static const uint32_t *const r150_cards[4]={'+','.join(f'r150_card_{i}' for i in range(92,96))+'};\n'
for dst in ['GAME/src','portable_build']:(repo/dst/'rift150_art.h').write_text(header)
