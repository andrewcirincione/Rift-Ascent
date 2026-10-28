"""Keep generated paintings at 3-4x PSP texture resolution for source control."""
from pathlib import Path
from PIL import Image,ImageOps
import json,hashlib
r=Path(__file__).parent/'art';old={}
if (r/'processing.json').exists():old={x['file']:x for x in json.loads((r/'processing.json').read_text())}
records=[]
for p in sorted(r.glob('*.png')):
 source=p.read_bytes();sha=hashlib.sha256(source).hexdigest()
 if p.name in old and sha==old[p.name]['optimized_sha256']:records.append(old[p.name]);continue
 im=Image.open(p);original=im.size;name=p.stem
 if name.startswith('card-'):size=(720,480)
 elif name.startswith(('potion-','relic-')):size=(240,240)
 elif name.startswith('badge-'):size=(120,120)
 elif name.startswith('enemy-'):size=(432,480)
 elif name=='unmaker':size=(480,576)
 elif name.startswith('act4-') or name=='unmaker-arena':size=(960,544)
 else:size=(1008,378)
 if name.startswith(('potion-','relic-','badge-','enemy-')) or name=='unmaker':
  im=im.convert('RGBA');box=im.getbbox()
  if box:im=im.crop(box)
  im.thumbnail((size[0]-8,size[1]-8),Image.Resampling.LANCZOS);out=Image.new('RGBA',size);out.alpha_composite(im,((size[0]-im.width)//2,size[1]-im.height));im=out
 else:im=ImageOps.fit(im.convert('RGB'),size,Image.Resampling.LANCZOS)
 im.save(p,optimize=True)
 records.append(dict(file=p.name,original_size=original,original_sha256=sha,optimized_size=size,optimized_sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
(r/'processing.json').write_text(json.dumps(records,indent=2)+'\n');print('Optimized',len(records),'generated assets')
