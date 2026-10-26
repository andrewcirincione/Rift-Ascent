"""Build the complete 1.3.6 source, preserving the existing generated artwork."""
from pathlib import Path
import subprocess, shutil, sys
repo=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,'qa/v135/build_source.py'],cwd=repo,check=True)
for target in ['GAME/src','portable_build']:
    for name in ['game.c','game.h','render.c','card_preview.inc']:
        shutil.copyfile(repo/'qa/v136/source'/name,repo/target/name)
shutil.copyfile(repo/'qa/v136/source/psp_start.c',repo/'portable_build/psp_start.c')
p=repo/'portable_build/Makefile'
p.write_text(p.read_text().replace('RIFT ASCENT 1.3.5','RIFT ASCENT 1.3.6'))
print('PASS: 1.3.6 ready to build')
