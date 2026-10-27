"""Freeze evidence for the exact already-tested complete EBOOT; never rebuild."""
from pathlib import Path
import hashlib,json,shutil
q=Path(__file__).parent;root=q.parents[1];ev=q/'evidence';binary=ev/'EBOOT.PBP';digest=hashlib.sha256(binary.read_bytes()).hexdigest()
reports=['logic-tests.txt','ui-tests.txt','regent-tests.txt','asset-tests.txt','recovery-audio-tests.txt','resident-tests.txt','streamed-tests.txt','media-tests.txt','ppsspp-final-tests.txt','animation-final-tests.txt','cold-resume-tests.txt']
assert (ev/'resident-frames.txt').read_bytes()==(ev/'streamed-frames.txt').read_bytes()
lines=['Rift Ascent 1.6.2 final validation','SHA256 '+digest,'EBOOT bytes: '+str(binary.stat().st_size),'']
for name in reports:
 b=(ev/name).read_bytes();s=b.decode();assert 'PASS' in s and 'Traceback' not in s and 'ERROR: AddressSanitizer' not in s,name
 if name in ['ppsspp-final-tests.txt','animation-final-tests.txt']:assert digest in s
 lines += [name+' [report SHA256 '+hashlib.sha256(b).hexdigest()+']']+[v for v in s.splitlines() if v.startswith(('PASS','SHA256'))]+['']
lines += ['Visual review: actual native 480x272 PPSSPP captures of all three act palettes, both normal/elite variants, each actual boss, maps, campfires, shops, introductions, main menu, long previews, modification and inventory grids, status icons, arrow, reward/potion selections, pause and fades. No clipped reward labels or off-center controls observed.','All three character idle/action loops and the final-hit music route were captured on the final EBOOT. The fixed 4 MiB artwork cache and hardware-confirmed 1.6-r2 loader correction are retained. Tests use 32 MiB PSP mode; physical PSP confirmation of this patch remains pending.','PASS final release EBOOT']
(ev/'final-tests.txt').write_text('\n'.join(lines)+'\n')
dest=root/'versions/1.6.2';dest.mkdir(exist_ok=True);shutil.copyfile(binary,dest/'EBOOT.PBP');shutil.copyfile(ev/'ppsspp-status-arrow.png',dest/'Combat.png');(dest/'SHA256SUMS.txt').write_text(digest+'  EBOOT.PBP\n')
m=json.loads((root/'release/current.json').read_text());assert m['eboot_sha256']==digest and m['version']=='1.6.2'
print('PASS frozen release',digest)
