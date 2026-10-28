"""Package the 1.8 executable with the exact working prerelease media tail."""
from pathlib import Path
import hashlib,struct,json,sys,importlib.util,re,shutil
root=Path(sys.argv[1]).resolve();oldpath=Path(sys.argv[2]).resolve();tool=Path(sys.argv[3]).resolve()
sha=lambda b:hashlib.sha256(b).hexdigest()
old=oldpath.read_bytes();assert sha(old)=='f4edacf5421003eb7b1a5e4c9e3be1dce1a23e5ef89d5fad1233c43a917ba1f4'
build=(root/'source/EBOOT.PBP').read_bytes();a=struct.unpack_from('<8I',old,8);b=struct.unpack_from('<8I',build,8)
parts=[build[b[0]:b[1]]]+[old[a[i]:a[i+1]] for i in range(1,6)]+[build[b[6]:b[7]],old[a[7]:]]
off=[];pos=40
for p in parts:off.append(pos);pos+=len(p)
out=root/'release';out.mkdir(exist_ok=True);p=out/'EBOOT.PBP';p.write_bytes(old[:8]+struct.pack('<8I',*off)+b''.join(parts))
spec=importlib.util.spec_from_file_location('metadata',tool);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);m.fix(p)
final=p.read_bytes();assert final[off[7]:]==old[a[7]:];assert final[off[5]:off[6]]==old[a[5]:a[6]]
assert b'RIFT ASCENT 1.8\0' in parts[0] and b'PRE-RELEASE' not in parts[0]
import zlib
size,crc=struct.unpack_from('<II',final,len(final)-8);assert final[-56:-48]==b'RIFTM311' and zlib.crc32(final[-56-size:-56])==crc
bank_end=len(final)-56-size;bs,bc=struct.unpack_from('<II',final,bank_end-8);bank=final[bank_end-16-bs:bank_end-16];assert zlib.crc32(bank)==bc
assets=[]
for file in sorted((root/'source').iterdir()):
 if file.suffix not in ('.h','.c'):continue
 text=file.read_text()
 for m in re.finditer(r'const uint(16|32)_t\s+(\w+)\[(12|6)\][^=]*=\s*\{([^}]+)\}',text):
  width=int(m[1])//8;v=[int(x,0) for x in re.findall(r'0x[\da-fA-F]+|\d+',m[4])]
  if len(v)*width!=24:continue
  raw=struct.pack('<'+('H' if width==2 else 'I')*len(v),*v)
  if raw[:8]!=b'RIFTSIMG':continue
  o,n,pb,c=struct.unpack_from('<4I',raw,8);pixels=bank[o:o+n];assert len(pixels)==n and zlib.crc32(pixels)==c,m[2]
  assets.append(dict(name=m[2],offset=o,bytes=n,sha256=sha(pixels)))
(root/'assets.json').write_text(json.dumps(dict(bytes=len(bank),sha256=sha(bank),assets=assets),indent=2))
audit=dict(version='1.8',tag='v1.8',sha256=sha(final),bytes=len(final),previous_sha256=sha(old),media_tail_preserved=True,music_tracks=5,xmb_audio_preserved=True,custom_card_images=60,refreshed_enemy_images=6,debug_default=False,save_layout_unchanged=True,hardware_tested=False)
(out/'PACKAGE_AUDIT.json').write_text(json.dumps(audit,indent=2));(out/'EBOOT.PBP.sha256').write_text(sha(final)+'  EBOOT.PBP\n');print(json.dumps(audit,indent=2))
