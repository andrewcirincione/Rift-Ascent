from pathlib import Path
from PIL import Image
import shutil
q=Path(__file__).parent
src=Path('../generated_images/exec-192619e3-6def-43cf-ba23-5f2289d84e4b.png')
if src.exists():shutil.copyfile(src,q/'originals/death-atlas.png')
im=Image.open(q/'originals/death-atlas.png').convert('RGBA')
cols=[(0,538),(538,1008),(1008,1536)]
for i in range(6):
 row,col=divmod(i,3);left,right=cols[col];top=0 if row==0 else 530;bottom=530 if row==0 else 1024
 cell=im.crop((left,top,right,bottom));bbox=cell.getchannel('A').point(lambda x:255 if x>8 else 0).getbbox();assert bbox
 # Uniform scale and a common floor anchor keep the actor in place.
 scale=.30;scaled=cell.resize((round(cell.width*scale),round(cell.height*scale)),Image.Resampling.LANCZOS)
 pivot=[300,790,1265][col]-left;ground=bbox[3]
 canvas=Image.new('RGBA',(192,144));canvas.alpha_composite(scaled,(round(84-pivot*scale),round(141-ground*scale)))
 canvas.save(q/'assets'/f'death_{i}.png')
 print(i,bbox,canvas.getchannel('A').getbbox())
