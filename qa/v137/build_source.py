"""Apply 1.3.7 combat presentation to the tested 1.3.6 game."""
from pathlib import Path
import subprocess,sys,shutil
repo=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,'qa/v136/build_source.py'],cwd=repo,check=True)
for dst in ['GAME/src','portable_build']:
 for name in ['game.c','game.h','render.c','sound.c','sound.h','card_preview.inc','combat_fx.inc']:
  shutil.copyfile(repo/'qa/v137/source'/name,repo/dst/name)
shutil.copyfile(repo/'qa/v137/source/psp_start.c',repo/'portable_build/psp_start.c')
subprocess.run([sys.executable,'qa/v137/prepare_art.py','GAME/src'],cwd=repo,check=True)
shutil.copyfile(repo/'GAME/src/rift137_art.h',repo/'portable_build/rift137_art.h')
p=repo/'portable_build/Makefile'
text=p.read_text().replace('RIFT ASCENT 1.3.6','RIFT ASCENT 1.3.7')
text+='\nrender.o: combat_fx.inc rift137_art.h game.h sound.h\ngame.o: card_preview.inc game.h\nsound.o psp_start.o: sound.h game.h\n'
p.write_text(text)
(repo/'portable_build/PARAM.SFO').unlink(missing_ok=True)
print('PASS: 1.3.7 ready to compile')
