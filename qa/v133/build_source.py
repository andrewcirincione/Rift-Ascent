"""Reconstruct 1.3.2, then apply the complete 1.3.3 source and artwork."""
from pathlib import Path
import subprocess,shutil,sys
repo=Path(__file__).resolve().parents[2]
# Both directories are generated from the archived base, never source of truth.
for name in ['GAME','portable_build']:
 if (repo/name).exists():shutil.rmtree(repo/name)
workflow=(repo/'.github/workflows/overhaul-psp.yml').read_text()
script=workflow.split('        run: |\n',1)[1].split('\n      - name:',1)[0]
script='\n'.join(line[10:] for line in script.splitlines() if not line.lstrip().startswith('sudo ') and 'clean_bone_captain.py' not in line and 'qa/v132/clean_sprites.py' not in line)
subprocess.run(['bash','-euo','pipefail','-c',script],cwd=repo,check=True)
for name in ['game.c','game.h','render.c']:
 shutil.copyfile(repo/'qa/v133/source'/name,repo/'GAME/src'/name)
subprocess.run([sys.executable,'qa/v133/prepare_art.py','GAME/src'],cwd=repo,check=True)
subprocess.run([sys.executable,'qa/v133/regenerate_enemies.py','GAME'],cwd=repo,check=True)
for p in (repo/'GAME/src').iterdir():
 if p.is_file():shutil.copyfile(p,repo/'portable_build'/p.name)
for name in ['psp_start.c','sound.c']:
 shutil.copyfile(repo/'qa/v133/source'/name,repo/'portable_build'/name)
p=repo/'portable_build/Makefile';p.write_text(p.read_text().replace('RIFT ASCENT 1.3.2','RIFT ASCENT 1.3.3'))
(repo.parent/'evidence').mkdir(exist_ok=True)
for p in (repo/'qa/v133/fixtures').iterdir():shutil.copyfile(p,repo.parent/'evidence'/p.name)
print('PASS: complete 1.3.3 sources and embedded artwork ready')
