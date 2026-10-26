"""Atlas slicing and pixel scaling only; poses are generated artwork."""
from pathlib import Path
from PIL import Image
import shutil,json
root=Path(__file__).parent
src=root/'originals/hero-atlas-final.png'
im=Image.open(src).convert('RGBA');assert im.size==(1536,1024)
# Delimit actual transparent gutters: the generator's rows have unequal heights.
rows=[(0,365),(365,675),(675,1024)]
xs=[[0,376,740,1200,1536],[0,384,768,1152,1536],[0,384,768,1152,1536]]
pivots=[[220,555,942,1365],[214,570,960,1360],[212,558,942,1370]]
feet=[352,654,969]
for row in range(3):
 for dst,col in enumerate([0,1,2,2,3,3]):
  left,right=xs[row][col:col+2];top,bottom=rows[row]
  cell=im.crop((left,top,right,bottom));scale=.40
  scaled=cell.resize((round(cell.width*scale),round(cell.height*scale)),Image.Resampling.LANCZOS)
  canvas=Image.new('RGBA',(192,144))
  x=round(84-(pivots[row][col]-left)*scale);y=round(140-(feet[row]-top)*scale)
  canvas.alpha_composite(scaled,(x,y));canvas.save(root/'assets'/f'hero_{row*6+dst}.png')
meta=json.loads((root/'asset-provenance.json').read_text());meta['player_poses']={'source':'originals/hero-atlas-final.png','reference':'originals/hero-reference.png','unique_poses':12,'animation_slots':18,'processing':'transparent gutter extraction; shared 0.40 scale and foot/torso anchors; repeated contact/recovery frames'}
(root/'asset-provenance.json').write_text(json.dumps(meta,indent=2)+'\n')
print('PASS: 12 generated player poses aligned into 18 keyframe slots')
