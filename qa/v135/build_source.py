"""Apply 1.3.5 presentation changes to the complete 1.3.4 sources."""
from pathlib import Path
import subprocess,shutil,sys
repo=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,'qa/v134/build_source.py'],cwd=repo,check=True)
for target in ['GAME/src','portable_build']:shutil.copyfile(repo/'qa/v135/source/render.c',repo/target/'render.c')
subprocess.run([sys.executable,'qa/v135/prepare_art.py','GAME/src'],cwd=repo,check=True)
shutil.copyfile(repo/'GAME/src/rift135_art.h',repo/'portable_build/rift135_art.h')
p=repo/'portable_build/Makefile';p.write_text(p.read_text().replace('RIFT ASCENT 1.3.4','RIFT ASCENT 1.3.5'))
print('PASS: 1.3.5 ready to build')
