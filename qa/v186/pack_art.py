"""Append the six 1.8.6 assets and shared finale; preserve existing pixels/music."""
from pathlib import Path
import hashlib,json,struct,zlib,re,shutil
from PIL import Image

q=Path(__file__).resolve().parent;repo=q.parents[1]
b=(repo/'versions/1.8.4-r2/EBOOT.PBP').read_bytes()
assert hashlib.sha256(b).hexdigest()=='12a544e6b43fb8dd7deaf3e3f5838b7ebf9aefb0e4a4fd7e504ea5034cd5a3d5'
assert b[-160:-152]==b'RIFTAU19'
music_size=struct.unpack_from('<I',b,len(b)-8)[0];end=len(b)-160-music_size
old_length,old_crc=struct.unpack_from('<II',b,end-8)
bank=bytearray(b[end-16-old_length:end-16]);assert zlib.crc32(bank)==old_crc
(q/'soundtrack.bank').write_bytes(b[end:])
manifest=json.loads((repo/'qa/v184r2/assets.json').read_text())
assert manifest['bytes']==len(bank)
header=['#ifndef REVISION186_ART_H','#define REVISION186_ART_H','#include <stdint.h>']
names=['censer_bailiff','rime_pallbearer','vowbreaker_seraph','crown_tether','unborn_witness','primed']
sizes=[]
for i,name in enumerate(names):
    path=q/'art'/f'{name}.png';im=Image.open(path).convert('RGBA')
    assert im.getchannel('A').getextrema()[0]==0, 'Sprite must have transparency: '+name
    bounds=im.getchannel('A').getbbox();assert bounds
    im=im.crop(bounds)
    if name=='primed':im=im.resize((40,40),Image.Resampling.LANCZOS)
    else:im.thumbnail((224,224),Image.Resampling.LANCZOS)
    pixels=im.tobytes();offset=len(bank);bank.extend(pixels);crc=zlib.crc32(pixels)
    symbol='r186_primed' if name=='primed' else f'aesthetic_enemy_{59+i}'
    descriptor=b'RIFTSIMG'+struct.pack('<4I',offset,len(pixels),4,crc)
    header += [f'static const unsigned char {symbol}_data[24] __attribute__((aligned(4)))={{'+','.join(map(str,descriptor))+'};',f'#define {symbol} ((const uint32_t*){symbol}_data)']
    manifest['assets'].append({'name':symbol,'file':str(path.relative_to(q)),'width':im.width,'height':im.height,'bytes':len(pixels),'offset':offset,'crc32':f'{crc:08x}','sha256':hashlib.sha256(pixels).hexdigest(),'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
    if i<5:sizes.append(im.size)
header+=['#endif'];(q/'source/revision186_art.h').write_text('\n'.join(header)+'\n')
# One new shared ending image, appended without changing any existing pixels.
path=repo/'qa/v186r4/art/three-as-one.png'
im=Image.open(path).convert('RGB').resize((336,126),Image.Resampling.LANCZOS)
pixels=b''.join(struct.pack('<H',((r>>3)<<11)|((g>>2)<<5)|(b>>3)) for r,g,b in im.get_flattened_data())
offset=len(bank);bank.extend(pixels);crc=zlib.crc32(pixels)
descriptor=b'RIFTSIMG'+struct.pack('<4I',offset,len(pixels),2,crc)
(q/'source/finale_art.h').write_text('#ifndef FINALE_ART_H\n#define FINALE_ART_H\n#include <stdint.h>\nstatic const unsigned char ascent_finale_art_data[24] __attribute__((aligned(4)))={'+','.join(map(str,descriptor))+'};\n#define ascent_finale_art ((const uint16_t*)ascent_finale_art_data)\n#endif\n')
manifest['assets'].append({'name':'ascent_finale_art','file':'../v186r4/art/three-as-one.png','width':336,'height':126,'bytes':len(pixels),'offset':offset,'crc32':f'{crc:08x}','sha256':hashlib.sha256(pixels).hexdigest(),'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
art=(repo/'qa/v185/source/aesthetic_art.h').read_text().replace('#include <stdint.h>','#include <stdint.h>\n#include "revision186_art.h"')
art=re.sub(r'(aesthetic_enemies)\[59\]=\{([^}]+)\};',lambda m:m[1]+'[64]={'+m[2]+','+','.join(f'aesthetic_enemy_{i}' for i in range(59,64))+'};',art)
art=re.sub(r'(aesthetic_enemy_size)\[59\]\[2\]=\{(.+?)\};',lambda m:m[1]+'[64][2]={'+m[2]+','+','.join('{%d,%d}'%s for s in sizes)+'};',art)
(q/'source/aesthetic_art.h').write_text(art)
manifest['bytes']=len(bank);manifest['sha256']=hashlib.sha256(bank).hexdigest();manifest['draft_missing']=[]
manifest['revision186']={'base_bytes':old_length,'new_assets':6,'generator':'Built-in image generation','pillow':'12.3.0'}
manifest['revision186r4']={'new_assets':1,'generator':'Built-in image generation','pillow':'12.3.0'}
(q/'assets.json').write_text(json.dumps(manifest,indent=2)+'\n')
(q/'assets.bin').write_bytes(bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank)))
print('PASS preserved base artwork and twelve music tracks; appended five enemies, Primed seal and Three as One finale:',len(bank)-old_length,'bytes')
