"""Five indexed, independently looping 11025 Hz IMA streams; original tracks unchanged."""
from pathlib import Path
import sys,struct,zlib,runpy,json,hashlib,array,wave
q=Path(__file__).parent;repo=q.parents[1];codec=runpy.run_path(str(repo/'tools/embed_churn.py'))
base,compiled,out=map(Path,sys.argv[1:4]);old=base.read_bytes()
parts=[];counts=[]
if old[-56:-48]==b'RIFTM311':ntracks,footer_size=5,56
else:assert old[-40:-32]==b'RIFTM119';ntracks,footer_size=3,40
size,crc=struct.unpack('<II',old[-8:]);payload=old[-footer_size-size:-footer_size];assert zlib.crc32(payload)==crc
for i in range(ntracks):
 off,count=struct.unpack_from('<II',old,len(old)-footer_size+8+i*8);parts.append(payload[off:off+(count+1)//2]);counts.append(count)
if ntracks==3:
 assert len(sys.argv)==6, 'A 1.3.10 baseline also requires RiftFight2.wav and RiftFight3.wav'
 for path in map(Path,sys.argv[4:6]):
  samples=codec['decode_wav'](path);parts.append(codec['encode'](samples));counts.append(len(samples))
entries=[];off=0
for part,n in zip(parts,counts):entries.append((off,n));off+=len(part)
payload=b''.join(parts);footer=b'RIFTM311'+b''.join(struct.pack('<II',*v) for v in entries)+struct.pack('<II',len(payload),zlib.crc32(payload));assert len(footer)==56
pbp=compiled.read_bytes();assert pbp[:4]==b'\x00PBP';out.write_bytes(pbp+payload+footer)
report={'tracks':[{'name':name,'samples':n,'seconds':n/11025,'sha256':hashlib.sha256(part).hexdigest()} for name,n,part in zip(['Checkers','RiftMenu','RiftFight','RiftFight2','RiftFight3'],counts,parts)],'eboot_sha256':hashlib.sha256(out.read_bytes()).hexdigest()}
(out.parent/'music-audit.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS: 5 indexed tracks; 3 original tracks preserved; CRC verified')
