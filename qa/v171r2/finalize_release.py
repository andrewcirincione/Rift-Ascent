from pathlib import Path
import json,hashlib,shutil,re
q=Path(__file__).parent;root=q.parents[1];ev=q/'evidence';pbp=ev/'EBOOT.PBP';digest=hashlib.sha256(pbp.read_bytes()).hexdigest()
render=[]
for name in ['resident','streamed']:
 text=(ev/(name+'-tests.txt')).read_text();render.append(re.search(r'combined-frame-hash=([0-9a-f]+)',text)[1])
assert render[0]==render[1]
reports=['flow-tests.txt','resident-tests.txt','streamed-tests.txt','asset-tests.txt','recovery-audio-tests.txt','media-tests.txt','ppsspp-tests.txt','cold-resume-tests.txt','fade-tests.txt']
lines=['Rift Ascent 1.71-r2 final validation','SHA256 '+digest,'EBOOT bytes: '+str(pbp.stat().st_size),'']
for n in reports:
 b=(ev/n).read_bytes();s=b.decode();assert 'PASS' in s and 'Traceback' not in s and 'ERROR:' not in s,n
 assert (ev/n).stat().st_mtime>pbp.stat().st_mtime,n
 if n in ['ppsspp-tests.txt','fade-tests.txt']:assert digest in s
 lines += [n+' [report SHA256 '+hashlib.sha256(b).hexdigest()+']']+[x for x in s.splitlines() if x.startswith(('PASS','SHA256','cases='))]+['']
lines+=['Visual review: native PPSSPP captures show the opaque act-title bar, readable stories before relic choices, clear pause focus on both actions, complete Tome/Armory cards inside the panel, no unintended Select icon on the main menu and grounded combat sprites. Faster idle sequences retain stable feet and character position. No new clipping, missing art or ghosting observed.', '32 MiB PSP mode, bounded 4 MiB cache and prior hardware-confirmed loader correction preserved. All five soundtrack payloads and the home-screen RiftBoss audio are intact. Music is 15% louder, with sound-effect levels unchanged. Silent entry scenes and music handoff checked in the live PSP audio buffer.', 'Physical PSP testing of this correction remains pending.', 'PASS final release EBOOT']
(ev/'final-tests.txt').write_text('\n'.join(lines)+'\n')
d=root/'versions/1.71-r2';d.mkdir(exist_ok=True);shutil.copyfile(pbp,d/'EBOOT.PBP');shutil.copyfile(ev/'ppsspp-idle-0-2.png',d/'Combat.png');shutil.copyfile(ev/'ppsspp-intro-0-1.png',d/'ActIntro.png');(d/'SHA256SUMS.txt').write_text(digest+'  EBOOT.PBP\n')
m=dict(version='1.71',tag='v1.71-r2',eboot='versions/1.71-r2/EBOOT.PBP',eboot_sha256=digest,notes='versions/1.71-r2/PATCH_NOTES.md',screenshot='versions/1.71-r2/Combat.png',canonical_source='qa/v171r2/source',validation='qa/v171r2/evidence/final-tests.txt',resources_manifest='qa/v171r2/assets.json');(root/'release/current.json').write_text(json.dumps(m,indent=2)+'\n')
print('PASS frozen release',digest)
