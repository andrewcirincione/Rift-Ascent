from pathlib import Path
import shutil,re
q=Path(__file__).parent;root=q.parents[1]
for folder in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and folder=='GAME/src':continue
  shutil.copyfile(p,root/folder/p.name)
p=root/'portable_build/Makefile';t=p.read_text();t=re.sub(r'PSP_EBOOT_TITLE = .*','PSP_EBOOT_TITLE = RIFT ASCENT 1.5b',t);p.write_text(t)
(root/'portable_build/PARAM.SFO').unlink(missing_ok=True)
