from pathlib import Path
import shutil,subprocess,sys
q=Path(__file__).parent;repo=q.parents[1]
for dst in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and dst=='GAME/src':continue
  shutil.copyfile(p,repo/dst/p.name)
subprocess.run([sys.executable,str(q/'prepare_art.py')],check=True)
p=repo/'portable_build/Makefile';s=p.read_text().replace('RIFT ASCENT 1.3.10','RIFT ASCENT 1.3.11')
s=s.replace('PSP_EBOOT_PIC1 = PIC1.PNG','PSP_EBOOT_PIC1 = PIC1.PNG\nPSP_EBOOT_SND0 = SND0.AT3') if 'PSP_EBOOT_SND0' not in s else s
s=s.replace('render.o: combat_fx.inc','render.o: rift1311_art.h combat_fx.inc');p.write_text(s)
(repo/'portable_build/PARAM.SFO').unlink(missing_ok=True)
