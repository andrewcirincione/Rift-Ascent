"""Package only the current distribution snapshot, never historical catalogs."""
from pathlib import Path
import hashlib,json,subprocess,zipfile,shutil
R=Path(__file__).resolve().parents[2];meta=json.loads((R/'release/current.json').read_text());D=R/'dist';D.mkdir(exist_ok=True)
assert meta['tag']=='v1.8.6-dist2' and meta['install_folder']=='RIFTASCENT_DIST' and meta['prerelease'] is True
binary=R/meta['eboot'];data=binary.read_bytes();assert hashlib.sha256(data).hexdigest()==meta['eboot_sha256'];assert len(data)==meta['eboot_bytes'];assert data[-160:-152]==b'RIFTAU19'
for p in D.iterdir():
 assert p.is_file();p.unlink()
prefix='RiftAscent-1.8.6-dist2';shutil.copyfile(binary,D/(prefix+'-EBOOT.PBP'))
docs={'README.md':R/'README.md','PATCH_NOTES.md':R/meta['notes'],'BALANCE.md':R/'distribution/BALANCE.md','VALIDATION.md':R/'distribution/VALIDATION.md','REVIEW.md':R/'distribution/REVIEW.md','LEGAL_REVIEW.md':R/'distribution/LEGAL_REVIEW.md','NOTICE.txt':R/'distribution/NOTICE.txt'}
for n,p in docs.items():shutil.copyfile(p,D/n)
shutil.copyfile(R/meta['screenshot'],D/'Combat.png')
install='PSP/GAME/'+meta['install_folder']
with zipfile.ZipFile(D/(prefix+'-PSP.zip'),'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 z.writestr(install+'/EBOOT.PBP',data)
 for n,p in docs.items():z.write(p,install+'/'+n)
 for p in sorted((R/'distribution/licenses').iterdir()):z.write(p,install+'/licenses/'+p.name)
# This tree is itself allowlisted at commit creation. Apply another release boundary.
tracked=subprocess.check_output(['git','ls-files'],cwd=R,text=True).splitlines()
allowed_prefixes=('qa/v186/source/','distribution/','tools/','release/','versions/1.8.6-dist2/','.github/workflows/distribution.yml')
allowed_roots={'README.md','AGENTS.md','.gitignore','qa/v186/assets.json'}
with zipfile.ZipFile(D/(prefix+'-Source.zip'),'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for n in tracked:
  if n not in allowed_roots and not n.startswith(allowed_prefixes):continue
  if n.endswith(('.o','.elf','.su','.map','.PBP')) or n.endswith('/PARAM.SFO'):continue
  assert 'prior-tuning' not in n and '/candidate/' not in n and '/__pycache__/' not in n,n
  z.write(R/n,'rift-ascent/'+n)
 for file in (R/'distribution/resources').iterdir():
  name='rift-ascent/'+file.relative_to(R).as_posix()
  if name not in z.namelist():z.write(file,name)
for p in D.glob('*.zip'):
 with zipfile.ZipFile(p) as z:
  assert z.testzip() is None
  if p.name.endswith('-PSP.zip'):assert hashlib.sha256(z.read(install+'/EBOOT.PBP')).hexdigest()==meta['eboot_sha256']
  else:
   names=set(z.namelist());assert 'rift-ascent/distribution/resources/art.bank' in names and 'rift-ascent/distribution/resources/soundtrack.bank' in names
   assert not any('/GAME/src/' in n or '/portable_build/' in n or 'ninja_cards.inc' in n or 'regent_cards.inc' in n or n.endswith('.PBP') for n in names)
(D/'SHA256SUMS.txt').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in sorted(D.iterdir())))
print('PASS complete PSP ZIP, preserved media, current-only source ZIP, notes and all checksums')
