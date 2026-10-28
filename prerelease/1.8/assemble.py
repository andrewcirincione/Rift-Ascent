#!/usr/bin/env python3
"""Reproducible 1.8 prerelease. Writes only prerelease/1.8 and its new dist files.
Preserves the complete 1.71-r2 original art/audio while appending actual AI images.
"""
from pathlib import Path
import argparse,base64,hashlib,json,lzma,shutil,struct,subprocess,zlib
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).resolve().parent
BASE=ROOT/'versions/1.71-r2/EBOOT.PBP'
BASE_SHA='13e440423f640939d41f271fc94b8cf57adb74fc57db8b030bacb267d0b0ec1c'
PAYLOAD_SHA='c099159f63cbb5c9ae2b6f8b68dbb2617f9b756e4132f60fc2e9d2051cddafd8'
def digest(b):return hashlib.sha256(b).hexdigest()
def base_bytes():
 b=BASE.read_bytes();assert digest(b)==BASE_SHA,'Base package changed';return b

def restore():
 text=''.join((HERE/'recovery'/f'source.{i}.b64').read_text().strip() for i in range(4))
 raw=lzma.decompress(base64.b64decode(text,validate=True));assert digest(raw)==PAYLOAD_SHA,'Recovery payload mismatch'
 j=json.loads(raw);out=HERE/'source';out.mkdir(parents=True,exist_ok=True)
 for name,path in j['mapping'].items():shutil.copyfile(ROOT/path,out/name)
 for name,entry in j['patches'].items():
  shutil.copyfile(ROOT/entry['base'],out/name)
  subprocess.run(['patch','--batch','--fuzz=0','-p1','-d',str(out)],input=entry['diff'].encode(),check=True)
 for name,content in j['new'].items():(out/name).write_text(content)
 for name,sha in j['expected'].items():assert digest((out/name).read_bytes())==sha,name
 b=base_bytes();off=struct.unpack_from('<8I',b,8)
 (out/'ICON0.PNG').write_bytes(b[off[1]:off[2]])
 (out/'PIC1.PNG').write_bytes(b[off[4]:off[5]])
 p=out/'Makefile';p.write_text(p.read_text().replace('RIFT ASCENT A1.8','RIFT ASCENT 1.8 PRE-RELEASE'))
 (HERE/'recovery/source-hashes.json').write_text(json.dumps({p.name:digest(p.read_bytes()) for p in sorted(out.iterdir()) if p.is_file()},indent=2))
 print('PASS: all 83 recovered source hashes match; PSP title set to 1.8 PRE-RELEASE')

def assets():
 from PIL import Image,ImageOps
 import numpy as np
 import cv2
 src=HERE/'original-art';out=HERE/'art';out.mkdir(exist_ok=True)
 b=base_bytes();size,crc=struct.unpack_from('<II',b,len(b)-8);music_start=len(b)-56-size
 assert b[-56:-48]==b'RIFTM311' and zlib.crc32(b[music_start:-56])==crc
 assert b[music_start-16:music_start-8]==b'RIFTART1'
 bank_size,bank_crc=struct.unpack_from('<II',b,music_start-8)
 bank=bytearray(b[music_start-16-bank_size:music_start-16]);assert zlib.crc32(bank)==bank_crc
 (out/'original_music.bin').write_bytes(b[music_start:])
 header=['#ifndef A18_ART_H','#define A18_ART_H','#include <stdint.h>'];records=[]
 def texture(name,path,dimensions,alpha=False,mask=None):
  im=Image.open(path).convert('RGB');sha=digest(path.read_bytes())
  if alpha:
   rgb=np.asarray(im);a=np.full(rgb.shape[:2],255,np.uint8)
   if mask is not None:
    a=np.zeros(rgb.shape[:2],np.uint8);cv2.fillPoly(a,[np.array(mask,np.int32)],255)
    if name=='a18_boss_0':
     r,g,bl=rgb[:,:,0].astype(int),rgb[:,:,1].astype(int),rgb[:,:,2].astype(int)
     a[(r>bl*1.35)&(g>bl*1.2)&(r>65)]=0
    elif name=='a18_boss_1':a[rgb.max(axis=2)<24]=0
   im=Image.fromarray(np.dstack((rgb,a)),'RGBA');box=im.getbbox();im=im.crop(box) if box else im
   im=ImageOps.contain(im,dimensions,Image.Resampling.LANCZOS)
   canvas=Image.new('RGBA',dimensions,(0,0,0,0));canvas.alpha_composite(im,((dimensions[0]-im.width)//2,dimensions[1]-im.height));im=canvas
   raw=im.tobytes();ctype='uint32_t';pb=4
  else:
   im=ImageOps.fit(im,dimensions,Image.Resampling.LANCZOS,centering=(.5,.40));a=np.array(im,dtype=np.uint16)
   raw=(((a[:,:,0]>>3)<<11)|((a[:,:,1]>>2)<<5)|(a[:,:,2]>>3)).astype('<u2').tobytes();ctype='uint16_t';pb=2
  while len(bank)%4:bank.append(0)
  offset=len(bank);bank.extend(raw);c=zlib.crc32(raw);desc=b'RIFTSIMG'+struct.pack('<IIII',offset,len(raw),pb,c)
  vals=struct.unpack('<'+('I' if pb==4 else 'H')*(24//pb),desc)
  header.append('static const %s %s[%d] __attribute__((aligned(4)))={%s};'%(ctype,name,len(vals),','.join(hex(x) for x in vals)))
  records.append(dict(name=name,source=str(path.relative_to(src)),source_sha256=sha,size=list(dimensions),bytes=len(raw),offset=offset,crc32=c,texture_sha256=digest(raw)))
  im.save(out/(name+'.png'));return name
 for c,char in enumerate(['warden','vesper','proxy']):
  names=[texture(f'a18_card_{334+20*c+i}',src/char/f'card_{334+20*c+i:03d}.png',(180,120)) for i in range(20)]
  header.append(f'static const uint16_t *const a18_cards_{c}[20]={{'+','.join(names)+'};')
 header.append('static const uint16_t *const *const a18_card_art[3]={a18_cards_0,a18_cards_1,a18_cards_2};')
 for c,char in enumerate(['warden','vesper','proxy']):
  names=[texture(f'a18_story_{c}_{i}',src/char/f'{char}_{kind}.png',(480,180)) for i,kind in enumerate(['ending','beyond','before','true'])]
  header.append(f'static const uint16_t *const a18_story_{c}[4]={{'+','.join(names)+'};')
 header.append('static const uint16_t *const *const a18_story_art[3]={a18_story_0,a18_story_1,a18_story_2};')
 texture('a18_void_arena',src/'refined/arena_2.png',(480,272))
 for kind,count,start in [('potion',20,6),('relic',10,64)]:
  names=[texture(f'a18_{kind}_{i}',src/'world'/f'{kind}_{i+start:02d}.png',(88,88)) for i in range(count)]
  header.append(f'static const uint16_t *const a18_{kind}_art[{count}]={{'+','.join(names)+'};')
 polygons=[[(100,48),(180,40),(260,50),(300,32),(359,80),(374,208),(398,68),(440,272),(421,473),(116,482),(96,305),(85,186)],[(135,0),(344,0),(397,146),(477,194),(367,327),(365,511),(156,511),(124,339),(20,364),(8,264),(138,201)],[(208,57),(290,64),(321,141),(335,211),(341,421),(300,437),(163,418),(132,359),(151,245),(176,199),(184,109)]]
 for c,char in enumerate(['warden','vesper','proxy']):texture(f'a18_boss_{c}',src/'world'/f'boss_{char}.png',(132,144),True,polygons[c])
 header.append('static const uint32_t *const a18_boss_art[3]={a18_boss_0,a18_boss_1,a18_boss_2};')
 texture('a18_flame_badge',src/'refined/fire_badge.png',(22,22));texture('a18_rift_badge',src/'world/badge_rift.png',(22,22));header.append('#endif')
 (HERE/'source/rift_a18_art.h').write_text('\n'.join(header)+'\n');(out/'resource_bank.bin').write_bytes(bank)
 manifest=dict(base_sha256=digest(b),base_bank_bytes=bank_size,new_bank_bytes=len(bank),bank_crc32=zlib.crc32(bank),music_sha256=digest(b[music_start:]),assets=records)
 (out/'assets.json').write_text(json.dumps(manifest,indent=2))
 cards=[x for x in records if x['name'].startswith('a18_card_')]
 assert len(cards)==60 and len(set(x['texture_sha256'] for x in cards))==60
 assert len(records)==108
 print('PASS: 108 actual AI textures; 60 distinct custom card paintings; old assets and music preserved')

def package():
 import importlib.util
 out=HERE/'release';out.mkdir(exist_ok=True)
 orig=base_bytes();build=(HERE/'source/EBOOT.PBP').read_bytes();oo=struct.unpack_from('<8I',orig,8);bo=struct.unpack_from('<8I',build,8)
 sections=[build[bo[0]:bo[1]]]+[orig[oo[i]:oo[i+1]] for i in range(1,6)]+[build[bo[6]:bo[7]],b'']
 offsets=[];cursor=40
 for block in sections:offsets.append(cursor);cursor+=len(block)
 data=orig[:8]+struct.pack('<8I',*offsets)+b''.join(sections);target=out/'EBOOT.PBP';target.write_bytes(data)
 spec=importlib.util.spec_from_file_location('fix',ROOT/'tools/fix_psp_elf_metadata.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);m.fix(target)
 bank=(HERE/'art/resource_bank.bin').read_bytes();music=(HERE/'art/original_music.bin').read_bytes()
 final=target.read_bytes()+bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank))+music;target.write_bytes(final)
 assert b'RIFT ASCENT 1.8 PRE-RELEASE' in sections[0]
 assert final[offsets[6]:offsets[6]+4]==b'\x7fELF' and final[-56:-48]==b'RIFTM311'
 assert final[offsets[5]:offsets[6]]==orig[oo[5]:oo[6]]
 length,crc=struct.unpack_from('<II',final,len(final)-8);assert zlib.crc32(final[-56-length:-56])==crc
 manifest=json.loads((HERE/'art/assets.json').read_text())
 for image in manifest['assets']:
  raw=bank[image['offset']:image['offset']+image['bytes']];assert digest(raw)==image['texture_sha256']
 sha=digest(final);(out/'EBOOT.PBP.sha256').write_text(sha+'  EBOOT.PBP\n')
 audit=dict(version='1.8 Pre-release',branch='1.8-Pre-release',sha256=sha,eboot_bytes=len(final),base_sha256=BASE_SHA,custom_card_images=60,unique_custom_card_images=60,new_ai_images=108,music_preserved=True,xmb_audio_preserved=True,resource_bank_bytes=len(bank),install_folder='PSP/GAME/RIFTASCENT_A18',validation='Native regression and exact-package PPSSPP evidence are stored alongside this prerelease. Hardware validation is user-reported for earlier builds, not claimed for this new build.')
 (out/'PACKAGE_AUDIT.json').write_text(json.dumps(audit,indent=2));print(json.dumps(audit,indent=2))

if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('phase',choices=['restore','assets','package']);args=parser.parse_args()
 globals()[args.phase]()
