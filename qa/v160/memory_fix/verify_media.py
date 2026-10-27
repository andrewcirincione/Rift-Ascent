from pathlib import Path
import struct,zlib,hashlib,json,sys
old,new=map(lambda p:Path(p).read_bytes(),sys.argv[1:3])
def sections(data):
 o=struct.unpack_from('<8I',data,8);return [data[o[i]:o[i+1]] for i in range(6)]
a,b=sections(old),sections(new)
assert a==b,'SFO/icon/animation/wallpaper/RiftBoss audio changed'
assert old[-56:-48]==new[-56:-48]==b'RIFTM311'
size,crc=struct.unpack('<II',new[-8:]);payload=new[-size-56:-56]
assert payload==old[-size-56:-56] and zlib.crc32(payload)==crc
report=dict(sha256=hashlib.sha256(new).hexdigest(),bytes=len(new),all_xmb_media_unchanged=True,all_five_soundtrack_streams_unchanged=True,music_crc=crc)
print('PASS',json.dumps(report,sort_keys=True))
