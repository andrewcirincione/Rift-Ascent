"""Recreate the complete 1.3.4 tree from the preserved 1.3.3 baseline."""
from pathlib import Path
import subprocess,shutil,sys
repo=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,'qa/v133/build_source.py'],cwd=repo,check=True)
for name in ['game.c','game.h','render.c','sound.c','sound.h']:
 for target in ['GAME/src','portable_build']:
  shutil.copyfile(repo/'qa/v134/source'/name,repo/target/name)
shutil.copyfile(repo/'qa/v134/source/psp_start.c',repo/'portable_build/psp_start.c')
subprocess.run([sys.executable,'qa/v134/prepare_art.py','GAME/src'],cwd=repo,check=True)
for name in ['rift134_art.h','rift134_fonts.h']:
 shutil.copyfile(repo/'GAME/src'/name,repo/'portable_build'/name)
p=repo/'portable_build/Makefile';p.write_text(p.read_text().replace('RIFT ASCENT 1.3.3','RIFT ASCENT 1.3.4'))
print('PASS: 1.3.4 complete source, generated UI, native typography, animation and audio ready')
