from pathlib import Path
import shutil,subprocess,sys
q=Path(__file__).parent;repo=q.parents[1]
for dst in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and dst=='GAME/src':continue
  shutil.copyfile(p,repo/dst/p.name)
subprocess.run([sys.executable,str(q/'prepare_art.py')],check=True)
p=repo/'portable_build/Makefile';s=p.read_text().replace('RIFT ASCENT 1.3.11','RIFT ASCENT 1.4.0')
if 'PSP_EBOOT_ICON1' not in s:s=s.replace('PSP_EBOOT_ICON = ICON0.PNG','PSP_EBOOT_ICON = ICON0.PNG\nPSP_EBOOT_ICON1 = ICON1.PMF')
s=s.replace('render.o: rift1311_art.h','render.o: rift140_art.h rift1311_art.h');p.write_text(s)
(repo/'portable_build/PARAM.SFO').unlink(missing_ok=True)
