from pathlib import Path
import shutil,subprocess,sys
repo=Path(__file__).resolve().parents[2];q=Path(__file__).parent
for dst in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and dst=='GAME/src':continue
  shutil.copyfile(p,repo/dst/p.name)
subprocess.run([sys.executable,str(q/'prepare_art.py')],cwd=repo,check=True)
p=repo/'portable_build/Makefile';s=p.read_text().replace('RIFT ASCENT 1.3.9','RIFT ASCENT 1.3.10')
s=s.split('\nrender.o:')[0]
s+='\nrender.o: combat_fx.inc rift1310_art.h rift139_art.h rift138_art.h rift137_art.h game.h sound.h\ngame.o: card_preview.inc game.h\nsound.o psp_start.o: sound.h game.h\n';p.write_text(s)
(repo/'portable_build/PARAM.SFO').unlink(missing_ok=True)
print('1.3.10 sources and generated assets ready')
