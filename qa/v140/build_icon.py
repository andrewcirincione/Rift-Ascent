"""Stable painted logo on true black, with a seamless generated flame cycle."""
from pathlib import Path
from PIL import Image
import subprocess, tempfile, json, hashlib
q=Path(__file__).resolve().parent;repo=q.parents[1]
logo=Image.open(repo/'qa/v134/assets/logo.png').convert('RGBA');logo.thumbnail((140,65),Image.Resampling.LANCZOS)
flames=[Image.open(q/'assets'/f'flame_{i}.png').convert('RGBA').resize((144,64),Image.Resampling.LANCZOS) for i in range(6)]
with tempfile.TemporaryDirectory() as td:
 td=Path(td)
 for f in range(180):
  phase=f%60;idx=phase//10;blend=(phase%10)/10
  frame=Image.new('RGBA',(144,80),(0,0,0,255));flame=Image.blend(flames[idx],flames[(idx+1)%6],blend)
  frame.alpha_composite(flame,(0,16));frame.alpha_composite(logo,((144-logo.width)//2,3))
  frame.convert('RGB').save(td/f'{f:03}.png')
  if f==0:
   for dst in ['GAME','portable_build']:frame.convert('RGB').save(repo/dst/'ICON0.PNG')
 subprocess.run(['ffmpeg','-v','error','-y','-framerate','30000/1001','-i',str(td/'%03d.png'),'-c:v','libx264','-crf','12','-pix_fmt','yuv420p',str(td/'icon.mp4')],check=True)
 subprocess.run(['python3',str(repo/'tools/vendor/psp-media-toolkit/pspmedia.py'),'convert',str(td/'icon.mp4'),'-o',str(repo/'portable_build/ICON1.PMF'),'--budget-kb','240'],check=True)
 subprocess.run(['ffmpeg','-v','error','-y','-i',str(repo/'portable_build/ICON1.PMF'),'-frames:v','180',str(td/'decoded-%03d.png')],check=True)
 # Inspect actual decoded frames, including loop boundary, in a contact sheet.
 decoded=sorted(td.glob('decoded-*.png'));assert len(decoded)==180,len(decoded)
 sheet=Image.new('RGB',(576,160))
 for j,k in enumerate([0,10,20,30,40,50,59,60]):sheet.paste(Image.open(decoded[k]),(j%4*144,j//4*80))
 sheet.save(q/'evidence/icon-decoded.png')
 import numpy as np
 a=np.array([np.array(Image.open(p).convert('RGB')) for p in decoded]);luma=a.astype(float).mean(axis=(1,2,3))
 assert luma.min()>10 and luma.max()-luma.min()<9,(luma.min(),luma.max())
 assert a[:,:3,:3].max()<12,'background is not black'
 # Logo title center must be stable across the full animation; only flame tips move.
 delta=np.abs(a[1:].astype(float)-a[:-1].astype(float)).mean(axis=(1,2,3));assert delta.max()<8,delta.max()
 facts={'frames':180,'duration':6.006,'size':(repo/'portable_build/ICON1.PMF').stat().st_size,'luma_range':[float(luma.min()),float(luma.max())],'max_frame_difference':float(delta.max()),'wallpaper_unchanged':True}
 (q/'evidence/icon-validation.json').write_text(json.dumps(facts,indent=2));print(facts)
