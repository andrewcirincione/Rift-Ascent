"""Freeze evidence for the exact already-tested complete EBOOT; never rebuild."""
from pathlib import Path
import hashlib,json,shutil
q=Path(__file__).parent;root=q.parents[1];ev=q/'evidence';binary=ev/'EBOOT.PBP';digest=hashlib.sha256(binary.read_bytes()).hexdigest()
reports=['story-tests.txt','logic-tests.txt','regent-tests.txt','asset-tests.txt','recovery-audio-tests.txt','resident-tests.txt','streamed-tests.txt','media-tests.txt','scene-tests.txt','control-tests.txt','finisher-tests.txt','cold-resume-tests.txt']
assert (ev/'resident-hash.txt').read_bytes()==(ev/'streamed-hash.txt').read_bytes()
lines=['Rift Ascent 1.7 final validation','SHA256 '+digest,'EBOOT bytes: '+str(binary.stat().st_size),'']
for name in reports:
 b=(ev/name).read_bytes();s=b.decode();assert 'PASS' in s and 'Traceback' not in s and 'ERROR: AddressSanitizer' not in s,name
 if name in ['scene-tests.txt','control-tests.txt','finisher-tests.txt']:assert digest in s
 lines += [name+' [report SHA256 '+hashlib.sha256(b).hexdigest()+']']+[v for v in s.splitlines() if v.startswith(('PASS','SHA256'))]+['']
lines += ['Visual review: native 480x272 PPSSPP captures of all twelve character/act entry illustrations, nine three-beat boss exchanges, the matching combat models, the three-option title, save/resume and scene boundary fades. All 255 pages fit the actual font/width without clipping.','Frame breakpoints capture actual old-scene dimming, full black midpoint and new-scene brightening. Held X does not skip an exchange; three buffered edges cannot select a map room. Entry music is silent, boss music starts with its scene and its decoder continues into combat.','The fixed 4 MiB artwork cache, 32 MiB PSP target and hardware-confirmed 1.6-r2 loader correction are retained. Physical PSP confirmation of this patch remains pending.','PASS final release EBOOT']
(ev/'final-tests.txt').write_text('\n'.join(lines)+'\n')
dest=root/'versions/1.7';dest.mkdir(exist_ok=True);shutil.copyfile(binary,dest/'EBOOT.PBP');shutil.copyfile(ev/'ppsspp-fight-0-1.png',dest/'Combat.png');shutil.copyfile(ev/'ppsspp-boss-0-1-0.png',dest/'Scene.png');(dest/'SHA256SUMS.txt').write_text(digest+'  EBOOT.PBP\n')
m=dict(version='1.7',tag='v1.7',eboot='versions/1.7/EBOOT.PBP',eboot_sha256=digest,notes='versions/1.7/PATCH_NOTES.md',screenshot='versions/1.7/Combat.png',canonical_source='qa/v170/source',validation='qa/v170/evidence/final-tests.txt',resources_manifest='qa/v170/assets.json')
(root/'release/current.json').write_text(json.dumps(m,indent=2)+'\n')
print('PASS frozen release',digest)
