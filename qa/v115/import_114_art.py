#!/usr/bin/env python3
"""Preserve tested v1.1.4 title, hero and XMB artwork across v1.1.5 recompile.
Use symbol signature matching, NEVER hardcode offsets for newly compiled ELF.
"""
from pathlib import Path
import base64,lzma,re,struct,sys
root=Path(sys.argv[1]); pbp_path=Path(sys.argv[2])
repo=Path('qa/v114')
def chunks(pattern):
 return lzma.decompress(base64.b64decode(''.join(p.read_text().strip() for p in sorted(repo.glob(pattern)))))
original=bytearray(chunks('parts/part_*.txt'))
patch=chunks('fixes/fixpart_*.txt')
assert patch[:8]==b'RIFT114P'
n=struct.unpack_from('<I',patch,8)[0]; at=12
for _ in range(n):
 off,sz=struct.unpack_from('<II',patch,at);at+=8
 original[off:off+sz]=patch[at:at+sz];at+=sz
assert at==len(patch)
logo=chunks('titlefix/titlefix_*.txt')
assert logo[:8]==b'RIFT114T'
off,sz=struct.unpack_from('<II',logo,8)
assert sz==480*272*2
original[off:off+sz]=logo[16:]
orig_offsets=struct.unpack_from('<8I',original,8)
assert original[:4]==b'\\x00PBP'
oldelf=original[orig_offsets[6]:]
title=oldelf[0x1b6890:0x1b6890+480*272*2]
hero=oldelf[2085520:2085520+210*127*2]
assert len(title)==261120 and len(hero)==53340
assert hero.count(b'\\x1f\\xf8')>100
pic1=original[orig_offsets[4]:orig_offsets[5]]

# Use the arrays compiled into C source as an exact locator for binary blobs.
# The compiler may shift these assets by thousands of bytes between versions.
def find_source_data(name):
 for fname in ('artwork.c','motion_art.c','artwork.h','motion_art.h'):
  p=root/fname
  if not p.exists():continue
  s=p.read_text()
  m=re.search(r'\\b'+re.escape(name)+r'\\s*\\[[^]]*\\]\\s*=\\s*\\{(.*?)\\};',s,re.S)
  if m:
   vals=re.findall(r'0x([0-9a-fA-F]{4})',m.group(1))
   if len(vals)>250:
    return b''.join(struct.pack('<H',int(x,16)) for x in vals)
 raise RuntimeError('Cannot find original source texture '+name)
current=pbp_path.read_bytes()
o=list(struct.unpack_from('<8I',current,8))
segments=[bytearray(current[o[i]:(o[i+1] if i<7 else len(current))]) for i in range(8)]
elf=segments[6]
for name,data in [('ra_title_bg',title),('rift_hero',hero)]:
 original_bytes=find_source_data(name)
 assert len(original_bytes)==len(data),(name,len(original_bytes),len(data))
 i=elf.find(original_bytes[:512])
 assert i>=0 and elf.count(original_bytes[:512])==1,('missing/non-unique',name,i)
 assert elf[i:i+len(original_bytes)]==original_bytes,('array not contiguous',name)
 elf[i:i+len(data)]=data
 print('ART REUSE:',name,'offset=',i,'length=',len(data))
segments[4]=bytearray(pic1)
header=bytearray(current[:40]);body=bytearray();new_offsets=[]
for section in segments:
 new_offsets.append(40+len(body));body.extend(section)
struct.pack_into('<8I',header,8,*new_offsets)
new=bytes(header)+body
assert new[:4]==b'\\x00PBP'
pbp_path.write_bytes(new)
print('PASS: new PSP executable uses exact v1.1.4 title, hero and XMB assets')
