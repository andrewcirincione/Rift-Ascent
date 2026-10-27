"""Pack generated pixels; register boots independently of cape/sword extent."""
from pathlib import Path
from PIL import Image
from scipy.ndimage import label,binary_dilation
import numpy as np,json
q=Path(__file__).parent;root=q.parents[1]
header='#pragma once\n';facts=[]
def indexed(name,im):
 global header
 qi=im.quantize(colors=256,method=Image.Quantize.FASTOCTREE)
 pal=np.array((qi.getpalette('RGBA')+[0]*1024)[:1024],dtype=np.uint32).reshape(256,4)
 colors=pal[:,0]|pal[:,1]<<8|pal[:,2]<<16|pal[:,3]<<24
 header+=f'static const uint8_t {name}[13824]={{'+','.join(map(str,np.array(qi).ravel().tolist()))+'};\n'
 header+=f'static const uint32_t {name}_pal[256]={{'+','.join(map(str,colors.tolist()))+'};\n'
def emit_group(name,frames):
 global header
 for i,fr in enumerate(frames):
  fr.save(q/'assets'/f'{name}-{i:02d}.png');indexed(f'r161_{name}_{i}',fr)
 header+=f'static const Rift160Sprite r161_{name}[{len(frames)}]={{'+','.join('{'+f'r161_{name}_{i},r161_{name}_{i}_pal'+'}' for i in range(len(frames)))+'};\n'
 contact=Image.new('RGBA',(144*5,96*((len(frames)+4)//5)),(19,20,26,255))
 for i,fr in enumerate(frames):contact.alpha_composite(fr,(i%5*144,i//5*96))
 contact.save(q/'evidence'/f'{name}-registered.png')
def registered(im,cols,rows,scale,name):
 a=np.array(im);labs,n=label(binary_dilation(a[:,:,3]>80,iterations=1));sizes=np.bincount(labs.ravel())
 components=[k for k in range(1,n+1) if sizes[k]>5000];frames=[]
 for i in range(cols*rows):
  row,col=divmod(i,cols);cw=im.width/cols;ch=im.height/rows;cx=(col+.5)*cw;cy=(row+.6)*ch
  k=min(components,key=lambda k:(np.nonzero(labs==k)[1].mean()-cx)**2+(np.nonzero(labs==k)[0].mean()-cy)**2)
  mask=binary_dilation(labs==k,iterations=1);part=a.copy();part[:,:,3]=np.where(mask,part[:,:,3],0)
  # These windows contain boot soles, excluding trailing cloth and blade tips.
  if name=='warden':bx0,bx1,by0=int(col*cw+100),int(col*cw+455),int(row*ch+ch*.79)
  else:bx0,bx1,by0=int(col*cw+45),int(col*cw+225),int(row*ch+ch*.79)
  boot=np.where((part[:,:,3]>150)&(np.indices(mask.shape)[1]>=bx0)&(np.indices(mask.shape)[1]<bx1)&(np.indices(mask.shape)[0]>=by0)&(np.indices(mask.shape)[0]<(row+1)*ch))
  foot=int(boot[0].max())+1 if len(boot[0]) else round((row+1)*ch-8)
  sole=(part[foot-7:foot,bx0:bx1,3]>150);xs=np.nonzero(sole)[1]
  ax=bx0+(int(xs.min())+int(xs.max()))/2 if len(xs) else cx
  # Action poses keep their measured boot center. Idle registration below also
  # locks lower-body detail, rather than following the cape's bounding box.
  bounds=Image.fromarray(part).getchannel('A').getbbox();piece=Image.fromarray(part).crop(bounds)
  piece=piece.resize((round(piece.width*scale),round(piece.height*scale)),Image.Resampling.LANCZOS)
  dest=(round(70-(ax-bounds[0])*scale),round(96-(foot-bounds[1])*scale))
  frame=Image.new('RGBA',(144,96));frame.alpha_composite(piece,dest);frames.append(frame)
  facts.append(dict(character=name,frame=i,source_boot=[ax,foot],scale=scale,offset=dest))
 return frames
def lock_idle(frames,count,roi):
 ref=np.array(frames[0],dtype=np.int16);x0,y0,x1,y1=roi
 for i in range(1,count):
  a=np.array(frames[i],dtype=np.int16);best=None
  for dx in range(-7,8):
   for dy in range(-2,3):
    region=a[y0-dy:y1-dy,x0-dx:x1-dx];target=ref[y0:y1,x0:x1]
    if region.shape!=target.shape:continue
    mask=(region[:,:,3]>100)|(target[:,:,3]>100)
    score=np.mean(np.abs(region-target)[mask]) if mask.any() else 1e9
    if best is None or score<best[0]:best=(score,dx,dy)
  _,dx,dy=best;out=Image.new('RGBA',(144,96));out.alpha_composite(frames[i],(dx,dy));frames[i]=out
  facts.append(dict(idle_frame=i,registration=[dx,dy],roi=roi))
vesper=registered(Image.open(q/'art/vesper.png').convert('RGBA'),5,4,.275,'vesper')
lock_idle(vesper,5,(47,70,93,93));emit_group('vesper',vesper)
warden=registered(Image.open(q/'art/warden-idle.png').convert('RGBA'),3,2,.235,'warden')
lock_idle(warden,6,(40,70,108,93));emit_group('warden',warden)
proxy=[Image.open(root/'qa/v160/assets'/f'regent-{i:02d}.png').convert('RGBA') for i in range(5)]
lock_idle(proxy,5,(60,70,88,93));emit_group('proxy_idle',proxy)
(q/'evidence/anchors.json').write_text(json.dumps(facts,indent=2)+'\n')
(q/'source/rift161_art.h').write_text(header)
print('Packed 20 Vesper, 6 Warden and 5 registered Proxy frames')
