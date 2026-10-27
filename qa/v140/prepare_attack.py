"""Extract complete generated poses, then align measured boot anchors.
Connected components avoid clipping the long sword at sprite-grid boundaries.
No poses are redrawn; all pixels come from the original generated sheet.
"""
from pathlib import Path
from PIL import Image
from scipy.ndimage import label,binary_dilation
import numpy as np,json
q=Path(__file__).parent;a=np.array(Image.open(q/'art/attack-sheet.png').convert('RGBA'))
labels,n=label(a[:,:,3]>100);sizes=np.bincount(labels.ravel());ids=sorted(range(1,n+1),key=lambda i:sizes[i],reverse=True)[:6]
# Source boot-center anchors measured independently of capes and sword tips.
anchors=[(296,511),(816,511),(1276,511),(245,956),(801,956),(1330,956)]
ids=sorted(ids,key=lambda i:(int(np.nonzero(labels==i)[0].mean()>550),np.nonzero(labels==i)[1].mean()))
scale=.29;facts=[]
for i,(lab,(ax,ay)) in enumerate(zip(ids,anchors)):
 mask=binary_dilation(labels==lab,iterations=2);part=a.copy();part[:,:,3]=np.where(mask,part[:,:,3],0)
 im=Image.fromarray(part);bounds=im.getchannel('A').point(lambda v:255 if v>8 else 0).getbbox();im=im.crop(bounds)
 im=im.resize((round(im.width*scale),round(im.height*scale)),Image.Resampling.LANCZOS)
 x=round(86-(ax-bounds[0])*scale);y=round(142-(ay-bounds[1])*scale)
 assert x>=2 and y>=2 and x+im.width<=206 and y+im.height<=144,(i,x,y,im.size)
 canvas=Image.new('RGBA',(208,144));canvas.alpha_composite(im,(x,y));canvas.save(q/'assets'/f'attack_{i}.png');facts.append({'frame':i,'source_anchor':[ax,ay],'sprite_bounds':bounds,'canvas_offset':[x,y],'canvas_anchor':[86,142],'scale':scale})
(q/'evidence/attack-anchors.json').write_text(json.dumps(facts,indent=2)+'\n')
