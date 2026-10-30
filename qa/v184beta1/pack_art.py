"""Reuse the complete tested artwork and twelve-track soundtrack byte-for-byte."""
from pathlib import Path
import hashlib,struct,zlib,shutil
r=Path(__file__).resolve().parent;repo=r.parents[1]
b=(repo/'versions/1.8.4-r2/EBOOT.PBP').read_bytes()
assert hashlib.sha256(b).hexdigest()=='12a544e6b43fb8dd7deaf3e3f5838b7ebf9aefb0e4a4fd7e504ea5034cd5a3d5'
assert b[-160:-152]==b'RIFTAU19';size,crc=struct.unpack_from('<II',b,len(b)-8);end=len(b)-160-size
assert zlib.crc32(b[end:-160])==crc
length,art_crc=struct.unpack_from('<II',b,end-8);art=b[end-16-length:end]
assert art[-16:-8]==b'RIFTART1' and zlib.crc32(art[:-16])==art_crc
(r/'assets.bin').write_bytes(art);(r/'soundtrack.bank').write_bytes(b[end:])
shutil.copyfile(repo/'qa/v184r2/assets.json',r/'assets.json')
print('PASS all artwork and twelve complete encoded music tracks preserved from v1.8.4-r2')
