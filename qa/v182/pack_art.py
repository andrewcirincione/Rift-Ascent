"""Preserve the stable bank byte-for-byte, append original rebuild art.
--draft permits explicit transparent/black placeholders solely to compile QA.
The release path refuses any missing asset.
"""
from pathlib import Path
from PIL import Image,ImageOps
import struct,zlib,hashlib,json,argparse
root=Path(__file__).resolve().parent;repo=root.parents[1]
a=argparse.ArgumentParser();a.add_argument('--draft',action='store_true');args=a.parse_args()
base=(repo/'versions/1.8.1/EBOOT.PBP').read_bytes();assert hashlib.sha256(base).hexdigest()=='b0d016a4eadaca721f5e639b27f6b3e02e2629d7ca012a5c3bedcaa1736065e3'
assert base[-136:-128]==b'RIFTAU18';music=struct.unpack_from('<I',base,len(base)-8)[0];end=len(base)-136-music
assert base[end-16:end-8]==b'RIFTART1';length=struct.unpack_from('<I',base,end-8)[0];bank=bytearray(base[end-16-length:end-16]);initial=bytes(bank)
(root/'soundtrack.bank').write_bytes(base[end:])
header=['#ifndef AESTHETIC_ART_H','#define AESTHETIC_ART_H','#include <stdint.h>'];records=[];missing=[]
def add(name,file,size,alpha=False):
 p=root/'art'/file
 if p.exists():
  im=Image.open(p).convert('RGBA' if alpha else 'RGB')
  if alpha:
   # Retain all opaque detail with generous transparent margins.
   box=im.getchannel('A').getbbox();im=im.crop(box) if box else im
   im.thumbnail((size[0]-4,size[1]-4),Image.Resampling.LANCZOS)
   canvas=Image.new('RGBA',size);canvas.alpha_composite(im,((size[0]-im.width)//2,size[1]-im.height));im=canvas
  else:im=ImageOps.fit(im,size,Image.Resampling.LANCZOS,centering=(.5,.5))
 else:
  missing.append(file)
  if not args.draft:raise FileNotFoundError(file)
  im=Image.new('RGBA' if alpha else 'RGB',size)
 import numpy as np
 pix=np.array(im,dtype=np.uint16)
 data=im.tobytes() if alpha else (((pix[:,:,0]>>3)<<11)|((pix[:,:,1]>>2)<<5)|(pix[:,:,2]>>3)).astype('<u2').tobytes()
 offset=len(bank);bank.extend(data);crc=zlib.crc32(data);desc=b'RIFTSIMG'+struct.pack('<IIII',offset,len(data),4 if alpha else 2,crc)
 symbol='aesthetic_'+name
 header.append('static const unsigned char '+symbol+'_data[24] __attribute__((aligned(4)))={'+','.join(str(v) for v in desc)+'};')
 header.append('#define '+symbol+' ((const '+('uint32_t' if alpha else 'uint16_t')+'*)'+symbol+'_data)')
 records.append(dict(name=symbol,file=file,width=size[0],height=size[1],bytes=len(data),offset=offset,crc32=f'{crc:08x}',sha256=hashlib.sha256(data).hexdigest(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None))
 return symbol
specs=json.loads((root/'art-specs.json').read_text())
dimensions={}
for spec in specs:
 size=(spec['width'],spec['height'])
 if spec['key']!='target' and (root/'art'/spec['file']).exists():
  im=Image.open(root/'art'/spec['file']);box=im.getchannel('A').getbbox();w,h=(box[2]-box[0],box[3]-box[1]) if box else im.size
  size=(224,max(96,round(224*h/w))) if w>h else (max(96,round(224*w/h)),224)
 dimensions[spec.get('id',-1)]=size
 add(spec['key'],spec['file'],size,spec['alpha'])
lookup={s['id']:'aesthetic_'+s['key'] for s in specs if 'id' in s}
header.append('static const uint32_t*const aesthetic_enemies[47]={'+','.join(lookup.get(i,'0') for i in range(47))+'};')
header.append('static const unsigned short aesthetic_enemy_size[47][2]={'+','.join('{%d,%d}'%dimensions.get(i,(0,0)) for i in range(47))+'};')
header.append('#endif');(root/'source/aesthetic_art.h').write_text('\n'.join(header)+'\n')
assert bank[:length]==initial
(root/'assets.bin').write_bytes(bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank)))
(root/'assets.json').write_text(json.dumps({'bytes':len(bank),'sha256':hashlib.sha256(bank).hexdigest(),'assets':json.loads((repo/'qa/v181/assets.json').read_text())['assets']+records,'base':'v1.8.1','base_bank_bytes':length,'base_bank_sha256':hashlib.sha256(initial).hexdigest(),'new_assets':records,'draft_missing':missing},indent=2)+'\n');print(len(bank),'bank bytes;',len(missing),'missing')
