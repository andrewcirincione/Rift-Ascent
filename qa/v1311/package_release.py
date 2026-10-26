from pathlib import Path
import hashlib,json,shutil,zipfile
root=Path(__file__).resolve().parents[2];workspace=root.parent;ev=workspace/'evidence1311';out=workspace/'deliverables1311';out.mkdir(exist_ok=True);dest=root/'qa/v1311/evidence';dest.mkdir(exist_ok=True)
required={'native-tests.log':'PASS: block mixes with attack','migration-tests.log':'PASS v16/v17/v18','relic-tests.log':'PASS all 28','ppsspp-results.txt':'PASS PPSSPP 1.3.11','ppsspp-final.txt':'PASS final release EBOOT','playtests.log':'stalled=0'}
for name,needle in required.items():assert needle in (ev/name).read_text(),name
for source in (root/'qa/v1311/source').iterdir():
 for folder in ['GAME/src','portable_build']:
  if folder=='GAME/src' and source.name=='psp_start.c':continue
  assert source.read_bytes()==(root/folder/source.name).read_bytes(),source
pbp=ev/'EBOOT.PBP';digest=hashlib.sha256(pbp.read_bytes()).hexdigest()
assert digest in (ev/'ppsspp-final.txt').read_text()
assert hashlib.sha256((workspace/'runtime/qa1311finalconfig/ppsspp/PSP/GAME/RIFTASCENT/EBOOT.PBP').read_bytes()).hexdigest()==digest
assert json.loads((ev/'media-audit.json').read_text())['eboot_sha256']==digest
for pattern in ['ppsspp-*.png','state-*.dat','*-tests.log','*-audit.*','*-build.log','ppsspp-*.txt','playtests.log','audio-before.txt','audio-after.txt','grounding-contact.png']:
 for p in ev.glob(pattern):
  if p.is_file():shutil.copyfile(p,dest/p.name)
followup=workspace/'evidence1311-splash'
assert 'PASS 1.3.11 follow-up:' in (followup/'native-tests.log').read_text()
assert 'PASS final release EBOOT 1.3.11 follow-up' in (followup/'ppsspp-final.txt').read_text()
fdir=dest/'followup';fdir.mkdir(exist_ok=True)
for pattern in ['ppsspp-*.png','power-combat-contact.png','native-tests.log','native-build.log','psp-build.log','*-audit.txt','ppsspp-final.txt','launcher-test.txt']:
 for p in followup.glob(pattern):shutil.copyfile(p,fdir/p.name)
music=workspace/'evidence1311-music'
assert 'PASS replacement RiftFight3 sample count' in (music/'ppsspp-final.txt').read_text()
mdir=dest/'music-replacement';mdir.mkdir(exist_ok=True)
for pattern in ['ppsspp-*.png','ppsspp-final.txt','replacement.log','*-audit.*']:
 for p in music.glob(pattern):shutil.copyfile(p,mdir/p.name)
victory=workspace/'evidence1311-victory'
assert 'PASS shorter victory completes before rewards' in (victory/'ppsspp-final.txt').read_text()
assert 'PASS 1.3.11 follow-up:' in (victory/'native-tests.log').read_text()
vdir=dest/'shorter-victory';vdir.mkdir(exist_ok=True)
for pattern in ['ppsspp-*.png','ppsspp-final.txt','native-tests.log','build.log','*-audit.*']:
 for p in victory.glob(pattern):shutil.copyfile(p,vdir/p.name)
manifest={'version':'1.3.11','eboot_bytes':pbp.stat().st_size,'eboot_sha256':digest,'save_version':19,'game_bytes':3196,'emulator':'PPSSPP 1.20.4','physical_psp_tested':False,'followup':['RiftBoss XMB audio metadata','final-hit slow motion/music pause/victory','matching Triangle','bright hand arrows','anchored Power poses'],'victory_banner_frames':12,'victory_fade_frames':3,'riftfight3_source':'RiftFight3(1).wav','music_tracks':['Checkers','RiftMenu','RiftFight','RiftFight2','RiftFight3'],'standard_simulation':{'runs':1000,'wins':372,'stalls':0},'endless_transition_tests':1000,'attack_modifier_cases':81312}
(dest/'release.json').write_text(json.dumps(manifest,indent=2)+'\n')
prefix='Rift-Ascent-1.3.11';shutil.copyfile(pbp,out/(prefix+'-EBOOT.PBP'));shutil.copyfile(ev/'ppsspp-final-release-combat.png',out/(prefix+'-Combat.png'))
install='''Rift Ascent 1.3.11

Extract the PSP folder to your Memory Stick root, replacing
PSP/GAME/RIFTASCENT/EBOOT.PBP. Keep SAVE.DAT and PROFILE.DAT.
Old v16-v18 saves migrate automatically to v19. Keep a backup if you want
to return to an older build, which cannot read the expanded v19 format.

PPSSPP: open PSP/GAME/RIFTASCENT/EBOOT.PBP.
Choose BEGIN ASCENT for three acts or ENDLESS MODE for continuing acts.

Tested in PPSSPP 1.20.4. No physical PSP was available.
The EBOOT includes all five music tracks, the title icon and splash audio.
Changes, tests and rebuilding instructions: qa/v1311/README.md in Source.zip.
'''
with zipfile.ZipFile(out/(prefix+'-PSP.zip'),'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 z.write(pbp,'PSP/GAME/RIFTASCENT/EBOOT.PBP');z.writestr('README.txt',install)
exclude={'.o','.elf','.pbp','.ppm','.pyc','.wav','.mp3','.zip'}
with zipfile.ZipFile(out/(prefix+'-Source.zip'),'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  if not p.is_file():continue
  rel=p.relative_to(root)
  if any(part in {'.git','__pycache__'} for part in rel.parts):continue
  if p.suffix.lower() in exclude or p.name in {'game.map','PARAM.SFO'} or p.name.startswith('core.'):continue
  z.write(p,'rift-ascent/'+str(rel))
for p in out.glob('*.zip'):
 with zipfile.ZipFile(p) as z:
  assert z.testzip() is None
  if p.name.endswith('-PSP.zip'):assert hashlib.sha256(z.read('PSP/GAME/RIFTASCENT/EBOOT.PBP')).hexdigest()==digest
  else:
   assert z.read('rift-ascent/qa/v1311/source/game.c')==(root/'qa/v1311/source/game.c').read_bytes()
   assert z.read('rift-ascent/portable_build/SND0.AT3')==(root/'portable_build/SND0.AT3').read_bytes()
print(json.dumps({'release':manifest,'files':[{'name':p.name,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(out.iterdir()) if p.is_file()]},indent=2))
