from pathlib import Path
import json,hashlib,shutil
q=Path(__file__).parent;root=q.parents[1];ev=q/'evidence';pbp=ev/'EBOOT.PBP';digest=hashlib.sha256(pbp.read_bytes()).hexdigest()
assert (ev/'resident-hash.txt').read_bytes()==(ev/'streamed-hash.txt').read_bytes()
reports=['maw-tests.txt','resident-tests.txt','streamed-tests.txt','asset-tests.txt','scope-tests.txt','media-tests.txt','ppsspp-tests.txt','animation-tests.txt','cold-resume-tests.txt']
lines=['Rift Ascent 1.71 final validation','SHA256 '+digest,'EBOOT bytes: '+str(pbp.stat().st_size),'']
for n in reports:
 b=(ev/n).read_bytes();s=b.decode();assert 'PASS' in s and 'Traceback' not in s and 'ERROR:' not in s,n
 if n in ['ppsspp-tests.txt','animation-tests.txt']:assert digest in s
 lines += [n+' [report SHA256 '+hashlib.sha256(b).hexdigest()+']']+[x for x in s.splitlines() if x.startswith(('PASS','SHA256'))]+['']
lines+=['Visual review: final native PPSSPP captures show distinct silver-gray armor planes against the red/orange arena, complete feet/limbs and unchanged character alignment. Idle, hit, strike, guard and death frames checked. No new clipping, missing art or ghosting observed.','32 MiB PSP mode, bounded 4 MiB cache and prior hardware-confirmed loader correction preserved. Physical PSP testing of 1.71 remains pending.','PASS final release EBOOT']
(ev/'final-tests.txt').write_text('\n'.join(lines)+'\n')
d=root/'versions/1.71';d.mkdir(exist_ok=True);shutil.copyfile(pbp,d/'EBOOT.PBP');shutil.copyfile(ev/'ppsspp-combat-0.png',d/'Combat.png');(d/'SHA256SUMS.txt').write_text(digest+'  EBOOT.PBP\n')
m=dict(version='1.71',tag='v1.71',eboot='versions/1.71/EBOOT.PBP',eboot_sha256=digest,notes='versions/1.71/PATCH_NOTES.md',screenshot='versions/1.71/Combat.png',canonical_source='qa/v171/source',validation='qa/v171/evidence/final-tests.txt',resources_manifest='qa/v171/assets.json');(root/'release/current.json').write_text(json.dumps(m,indent=2)+'\n')
print('PASS frozen release',digest)
