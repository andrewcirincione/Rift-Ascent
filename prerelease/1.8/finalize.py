#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,shutil,zipfile
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1];dist=HERE/'dist';dist.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
audit=json.loads((HERE/'release/PACKAGE_AUDIT.json').read_text());smoke=json.loads((HERE/'evidence/SMOKE_PASSED.json').read_text())
assert audit['sha256']==sha(HERE/'release/EBOOT.PBP')==smoke['sha256'],'Binary not exactly the smoke-tested package'
assert audit['custom_card_images']==audit['unique_custom_card_images']==60
assert audit['music_preserved'] and audit['xmb_audio_preserved']
assert 'ALL 1.8 PRERELEASE NATIVE TESTS PASSED' in (HERE/'evidence/native-tests.txt').read_text()
audit['validation']='PSPDEV compile, native ASAN/UBSAN regression tests, complete media verification, exact-EBOOT PPSSPP assisted progression smoke tests.'
audit['smoke_cases']=smoke['cases'];(HERE/'release/PACKAGE_AUDIT.json').write_text(json.dumps(audit,indent=2))
shutil.copyfile(HERE/'release/EBOOT.PBP',dist/'EBOOT.PBP')
shutil.copyfile(HERE/'release/PACKAGE_AUDIT.json',dist/'PACKAGE_AUDIT.json')
shutil.copyfile(HERE/'RELEASE_NOTES.md',dist/'RELEASE_NOTES.md')
with zipfile.ZipFile(dist/'Rift-Ascent-1.8-Pre-release-PSP.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 z.write(HERE/'release/EBOOT.PBP','PSP/GAME/RIFTASCENT_A18/EBOOT.PBP')
 z.write(HERE/'RELEASE_NOTES.md','README.md');z.write(HERE/'release/PACKAGE_AUDIT.json','PACKAGE_AUDIT.json')
with zipfile.ZipFile(dist/'Rift-Ascent-1.8-Pre-release-Source.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for folder in ('source','original-art','art','recovery','tests'):
  for p in sorted((HERE/folder).rglob('*')):
   if not p.is_file() or p.suffix in ('.o','.elf','.PBP') or p.name in ('fixtures','layout','attacks','progression','PARAM.SFO','game.map'):continue
   z.write(p,p.relative_to(ROOT))
 for name in ('assemble.py','finalize.py','RELEASE_NOTES.md'):z.write(HERE/name,(HERE/name).relative_to(ROOT))
 z.write(ROOT/'tools/fix_psp_elf_metadata.py','tools/fix_psp_elf_metadata.py')
 z.write(ROOT/'versions/1.71-r2/EBOOT.PBP','versions/1.71-r2/EBOOT.PBP')
 # Include mapping sources so the recovery restore command works from the archive.
 import base64,lzma
 text=''.join((HERE/'recovery'/f'source.{i}.b64').read_text().strip() for i in range(4));payload=json.loads(lzma.decompress(base64.b64decode(text)))
 paths=set(payload['mapping'].values())|{p['base'] for p in payload['patches'].values()}
 for rel in sorted(paths):z.write(ROOT/rel,rel)
with zipfile.ZipFile(dist/'Rift-Ascent-1.8-Pre-release-Validation.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted((HERE/'evidence').rglob('*')):
  if p.is_file():z.write(p,p.relative_to(HERE))
manifest={p.name:{'sha256':sha(p),'bytes':p.stat().st_size} for p in sorted(dist.iterdir()) if p.is_file() and p.name not in ('SHA256SUMS.txt','RELEASE_MANIFEST.json')}
(dist/'SHA256SUMS.txt').write_text(''.join(v['sha256']+'  '+k+'\n' for k,v in manifest.items()))
(dist/'RELEASE_MANIFEST.json').write_text(json.dumps(dict(branch='1.8-Pre-release',tag='v1.8-pre.1',prerelease=True,assets=manifest),indent=2))
print(json.dumps(manifest,indent=2))
