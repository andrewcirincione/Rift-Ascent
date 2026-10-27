from pathlib import Path
import shutil,subprocess,sys,re
q=Path(__file__).parent;repo=q.parents[1]
for dst in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and dst=='GAME/src':continue
  shutil.copyfile(p,repo/dst/p.name)
subprocess.run([sys.executable,str(q/'prepare_art.py')],check=True)
p=repo/'portable_build/Makefile';s=p.read_text();s=re.sub(r'RIFT ASCENT [0-9.]+','RIFT ASCENT 1.5',s)
s=s.replace('render.o: rift140_art.h','render.o: rift150_art.h rift140_art.h');p.write_text(s)
(repo/'portable_build/PARAM.SFO').unlink(missing_ok=True)
