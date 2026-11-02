from pathlib import Path
import hashlib,struct,zlib,json
root=Path(__file__).resolve().parents[2];q=root/'qa/v186';new=(q/'evidence/EBOOT.PBP').read_bytes();old=(root/'versions/1.8.4-r2/EBOOT.PBP').read_bytes()
def media(b):
 assert b[-160:-152]==b'RIFTAU19';size,crc=struct.unpack_from('<II',b,len(b)-8);end=len(b)-160-size;assert zlib.crc32(b[end:-160])==crc
 bank_size,bank_crc=struct.unpack_from('<II',b,end-8);start=end-16-bank_size;assert b[end-16:end-8]==b'RIFTART1' and zlib.crc32(b[start:end-16])==bank_crc
 return b[start:end-16],b[end:]
a,am=media(new);b,bm=media(old);assert a[:len(b)]==b and am==bm
m=json.loads((q/'assets.json').read_text());assert len(a)==m['bytes'] and hashlib.sha256(a).hexdigest()==m['sha256'];assert len(new)<100*1024*1024
for item in m['assets'][-7:]:
 data=a[item['offset']:item['offset']+item['bytes']];assert zlib.crc32(data)==int(item['crc32'],16) and hashlib.sha256(data).hexdigest()==item['sha256']
x=struct.unpack_from('<8I',new,8);y=struct.unpack_from('<8I',old,8)
for i in range(1,6):assert new[x[i]:x[i+1]]==old[y[i]:y[i+1]],i
assert b'RIFT ASCENT 1.8.6' in new[x[0]:x[1]]
print('PASS every old artwork byte, all twelve full music streams, and splash assets unchanged; seven appended assets checksum verified; 1.8.6 title; package below 100 MiB.')
print('Bytes',len(new));print('SHA256',hashlib.sha256(new).hexdigest())
