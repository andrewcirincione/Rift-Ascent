"""Preserve the stable bank byte-for-byte, append original rebuild art.
--draft permits explicit transparent/black placeholders solely to compile QA.
The release path refuses any missing asset.
"""
from pathlib import Path
from PIL import Image,ImageOps
import struct,zlib,hashlib,json,argparse
root=Path(__file__).resolve().parent;repo=root.parents[1]
a=argparse.ArgumentParser();a.add_argument('--draft',action='store_true');args=a.parse_args()
base=(repo/'versions/1.71-r2/EBOOT.PBP').read_bytes();assert hashlib.sha256(base).hexdigest()=='13e440423f640939d41f271fc94b8cf57adb74fc57db8b030bacb267d0b0ec1c'
assert base[-56:-48]==b'RIFTM311';music=struct.unpack_from('<I',base,len(base)-8)[0];end=len(base)-56-music
assert base[end-16:end-8]==b'RIFTART1';length=struct.unpack_from('<I',base,end-8)[0];bank=bytearray(base[end-16-length:end-16]);initial=bytes(bank)
header=['#ifndef ASCENT_ART_H','#define ASCENT_ART_H','#include <stdint.h>'];records=[];missing=[]
def add(name,file,size,alpha=False):
 p=root/'art'/file
 if p.exists():
  im=Image.open(p).convert('RGBA' if alpha else 'RGB')
  if alpha:
   # Retain all opaque detail with generous transparent margins.
   box=im.getbbox();im=im.crop(box) if box else im
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
 symbol='ascent_'+name
 header.append('static const unsigned char '+symbol+'_data[24] __attribute__((aligned(4)))={'+','.join(str(v) for v in desc)+'};')
 header.append('#define '+symbol+' ((const '+('uint32_t' if alpha else 'uint16_t')+'*)'+symbol+'_data)')
 records.append(dict(name=symbol,file=file,width=size[0],height=size[1],bytes=len(data),offset=offset,crc32=f'{crc:08x}',sha256=hashlib.sha256(data).hexdigest(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None))
 return symbol
cards=[add('card_'+str(i),'card-'+str(i)+'.png',(180,120)) for i in range(334,394)]
header.append('static const uint16_t*const ascent_card_art[60]={'+','.join(cards)+'};')
for kind,start,n in [('potion',6,20),('relic',64,10)]:
 names=[add(kind+'_'+str(i),kind+'-'+str(i)+'.png',(80,80),True) for i in range(start,start+n)]
 header.append('static const uint32_t*const ascent_'+kind+'_art['+str(n)+']={'+','.join(names)+'};')
scenes=[]
for c in ['warden','vesper','proxy']:
 scenes.append([add(c+'_'+k.replace('-','_'),c+'-'+k+'.png',(336,126)) for k in ['ending','revelation','preboss','true-ending']])
header.append('static const uint16_t*const ascent_scene_art[3][4]={'+','.join('{'+','.join(row)+'}' for row in scenes)+'};')
for k in ['act4-title','act4-map','act4-fight','unmaker-arena']:add(k.replace('-','_'),k+'.png',(480,272))
add('unmaker','unmaker.png',(160,192),True)
for eid in [2,18,25]:add('enemy_'+str(eid),'enemy-'+str(eid)+'.png',(144,160),True)
for k in ['fire','rift']:add('badge_'+k,'badge-'+k+'.png',(40,40),True)
header.append('#endif');(root/'source/ascent_art.h').write_text('\n'.join(header)+'\n')
assert bank[:length]==initial
(root/'assets.bin').write_bytes(bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank)))
(root/'assets.json').write_text(json.dumps({'bytes':len(bank),'sha256':hashlib.sha256(bank).hexdigest(),'assets':records,'base':'v1.71-r2','base_bank_bytes':length,'base_bank_sha256':hashlib.sha256(initial).hexdigest(),'new_assets':records,'draft_missing':missing},indent=2)+'\n');print(len(bank),'bank bytes;',len(missing),'missing')
