"""Move large legacy textures out of DATA.PSP, retaining byte-identical pixels.

The originals remain in each header's non-streaming branch for portable tests.
This script is idempotent; descriptor order and bank offsets are deterministic.
"""
from pathlib import Path
import re,struct,zlib,json,hashlib
q=Path(__file__).parent;root=q.parents[1];bank=bytearray();records=[]
pattern=re.compile(r'static const uint(16|32)_t\s+(\w+)\[(\d+)\]\s*=\s*\{(.*?)\};',re.S)
wrapped=re.compile(r'/\* RIFT_STREAM_BEGIN \*/.*?#else\n(.*?)\n#endif /\* RIFT_STREAM_END \*/',re.S)
for p in sorted((root/'portable_build').glob('rift*.h')):
 if p.name in ('rift160_star_contrast.h',):continue
 text=wrapped.sub(lambda m:m[1],p.read_text())
 def replace(m):
  bits,name,count,body=m.groups();bits=int(bits);count=int(count)
  if count*bits//8<16384:return m[0]
  vals=[int(x,0) for x in re.findall(r'0x[0-9a-fA-F]+|\b\d+\b',body)]
  assert len(vals)==count,(p,name,count,len(vals))
  raw=struct.pack('<'+('H' if bits==16 else 'I')*count,*vals)
  assert not raw.startswith(b'RIFTSIMG')
  offset=len(bank);bank.extend(raw);crc=zlib.crc32(raw)
  descriptor=b'RIFTSIMG'+struct.pack('<4I',offset,len(raw),bits//8,crc)
  dv=struct.unpack('<'+('H' if bits==16 else 'I')*(len(descriptor)*8//bits),descriptor)
  records.append(dict(name=name,header=p.name,offset=offset,bytes=len(raw),crc32=crc,sha256=hashlib.sha256(raw).hexdigest(),pixel_bytes=bits//8))
  definition=f'static const uint{bits}_t {name}[{len(dv)}] __attribute__((aligned(4)))={{'+','.join(map(str,dv))+'};'
  return '/* RIFT_STREAM_BEGIN */\n#ifdef RIFT_STREAMED_ART\n'+definition+'\n#else\n'+m[0]+'\n#endif /* RIFT_STREAM_END */'
 text=pattern.sub(replace,text)

 for dst in [p,root/'GAME/src'/p.name]+([q/'source'/p.name] if p.name=='rift162_art.h' else []):
  if dst.read_text()!=text:dst.write_text(text)
(q/'assets.bin').write_bytes(bank)
(q/'assets.json').write_text(json.dumps(dict(bytes=len(bank),sha256=hashlib.sha256(bank).hexdigest(),assets=records),indent=2)+'\n')
print('Packed',len(records),'byte-identical textures;',len(bank),'bytes outside the executable')
