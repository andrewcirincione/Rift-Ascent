from pathlib import Path
import shutil
q=Path(__file__).parent;root=q.parents[1]
for folder in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and folder=='GAME/src':continue
  shutil.copyfile(p,root/folder/p.name)
(root/'portable_build/PARAM.SFO').unlink(missing_ok=True)
