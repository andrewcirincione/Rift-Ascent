"""Preserve user-confirmed original media byte-for-byte. No visual/audio audit.
Bootstrap once from the pinned prior release; thereafter builds are self contained.
"""
from pathlib import Path
import hashlib,json,struct,urllib.request,zlib
R=Path(__file__).resolve().parents[2];D=R/'distribution/resources'
BASE_SHA='d144cea99ae30e0e7c571e06515454ff253b3a284450d69296472c00e9a847eb'
URL='https://github.com/andrewcirincione/Rift-Ascent/releases/download/v1.8.6-r4/RiftAscent-1.8.6-r4-EBOOT.PBP'
def mapped(elf,addr,size):
 phoff=struct.unpack_from('<I',elf,28)[0];ents,n=struct.unpack_from('<HH',elf,42)
 for i in range(n):
  typ,off,vaddr,phys,filesz,memsz,flags,align=struct.unpack_from('<8I',elf,phoff+ents*i)
  if typ==1 and vaddr<=addr and addr+size<=vaddr+filesz:return off+addr-vaddr
 raise AssertionError(('Unmapped asset',hex(addr),size))
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 D.mkdir(exist_ok=True);manifest=D/'manifest.json'
 if manifest.exists():
  m=json.loads(manifest.read_text())
  if all((D/n).exists() and digest(D/n)==v['sha256'] for n,v in m['files'].items()):print('PASS preserved media checksums');return
 p=R/'versions/1.8.6-r4/EBOOT.PBP'
 if p.exists():b=p.read_bytes()
 else:
  # Resolve the asset URL from the immutable published tag, then require the pinned digest.
  meta=json.load(urllib.request.urlopen('https://api.github.com/repos/andrewcirincione/Rift-Ascent/releases/tags/v1.8.6-r4'))
  url=next(x['browser_download_url'] for x in meta['assets'] if x['name'].endswith('EBOOT.PBP'))
  b=urllib.request.urlopen(url).read()
 assert hashlib.sha256(b).hexdigest()==BASE_SHA
 assert b[-160:-152]==b'RIFTAU19';music=struct.unpack_from('<I',b,len(b)-8)[0];end=len(b)-160-music
 banklen,crc=struct.unpack_from('<II',b,end-8);assert b[end-16:end-8]==b'RIFTART1'
 bank=b[end-16-banklen:end-16];assert zlib.crc32(bank)==crc
 (D/'art.bank').write_bytes(b[end-16-banklen:end]);(D/'soundtrack.bank').write_bytes(b[end:])
 offsets=struct.unpack_from('<8I',b,8);elf=b[offsets[6]:offsets[7]]
 for name,addr,size in [('title565.bin',0x08d474d0,261120),('hero565.bin',0x08d8ded0,53340)]:
  at=mapped(elf,addr,size);(D/name).write_bytes(elf[at:at+size])
 for name,i in [('ICON0.PNG',1),('ICON1.PMF',2),('PIC1.PNG',4),('SND0.AT3',5)]: (D/name).write_bytes(b[offsets[i]:offsets[i+1]])
 m={'provenance':'Artwork and music confirmed distinct by the owner; unchanged bytes from prior release.','baseline_sha256':BASE_SHA,'files':{p.name:{'bytes':p.stat().st_size,'sha256':digest(p)} for p in D.iterdir() if p.name!='manifest.json'}}
 manifest.write_text(json.dumps(m,indent=2)+'\n');print('PASS extracted unchanged media; discarded baseline executable')
if __name__=='__main__':main()
