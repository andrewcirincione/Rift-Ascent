from pathlib import Path
import hashlib,struct,zlib
root=Path(__file__).resolve().parents[2];new=(root/'qa/v185/evidence/EBOOT.PBP').read_bytes();old=(root/'versions/1.8.4-r2/EBOOT.PBP').read_bytes()
def media(b):
 assert b[-160:-152]==b'RIFTAU19';size,crc=struct.unpack_from('<II',b,len(b)-8);end=len(b)-160-size;assert zlib.crc32(b[end:-160])==crc
 bank_size,bank_crc=struct.unpack_from('<II',b,end-8);start=end-16-bank_size;assert b[end-16:end-8]==b'RIFTART1' and zlib.crc32(b[start:end-16])==bank_crc
 return b[start:]
assert media(new)==media(old)
a=struct.unpack_from('<8I',new,8);b=struct.unpack_from('<8I',old,8)
for i in range(1,6):assert new[a[i]:a[i+1]]==old[b[i]:b[i+1]],i
print('PASS every artwork byte, all twelve complete encoded music streams, footer and splash assets remain identical to v1.8.4-r2.')
print('Bytes',len(new));print('SHA256',hashlib.sha256(new).hexdigest())
