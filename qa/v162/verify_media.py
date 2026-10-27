"""Check the media bytes in the final complete PBP, not just input files."""
from pathlib import Path
import struct,hashlib,json,zlib
q=Path(__file__).resolve().parent;root=q.parents[1]
old=(root/'versions/1.6-r2/EBOOT.PBP').read_bytes();new=(q/'evidence/EBOOT.PBP').read_bytes()
def sections(data):
 assert data[:4]==b'\0PBP';o=struct.unpack_from('<8I',data,8);return [data[o[i]:o[i+1]] for i in range(7)]
a,b=sections(old),sections(new)
assert b'1.6.2' in b[0]
assert b[2][:4]==b'PSMF' and b[2]==(root/'portable_build/ICON1.PMF').read_bytes()
assert b[1]==(root/'portable_build/ICON0.PNG').read_bytes()
assert b[4]==a[4] and b[4][:8]==bytes.fromhex('89504e470d0a1a0a'),'wallpaper changed'
assert b[5]==a[5] and b[5][:4]==b'RIFF','RiftBoss home-screen audio changed'
assert len(b[2])+len(b[5])<500*1024
assert new[-56:-48]==old[-56:-48]==b'RIFTM311'
size,crc=struct.unpack('<II',new[-8:]);assert new[-size-56:-56]==old[-size-56:-56] and zlib.crc32(new[-size-56:-56])==crc
facts={'sha256':hashlib.sha256(new).hexdigest(),'bytes':len(new),'icon1_bytes':len(b[2]),'snd0_bytes':len(b[5]),'pic1_unchanged':True,'riftboss_snd0_unchanged':True,'all_five_soundtrack_payloads_unchanged':True,'music_crc':crc}
(q/'evidence/media-validation.json').write_text(json.dumps(facts,indent=2)+'\n');print('PASS',facts)
