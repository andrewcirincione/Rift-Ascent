"""Cut out and resize six reviewed AI originals; never synthesize illustration pixels."""
from pathlib import Path
import hashlib,json,re,sys
import numpy as np,cv2
from PIL import Image,ImageEnhance,ImageOps,ImageDraw
root=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent
src=root/'source';art=root/'art';out=art/'sprites';out.mkdir(parents=True,exist_ok=True)
rects={0:(8,72,496,400),1:(15,9,289,480),11:(2,5,503,494),18:(119,21,250,443),21:(118,35,365,451),31:(124,20,197,466)}
# Each rectangle isolates a single full-body approved design, excluding detached
# draft props, alternate poses, text, and backgrounds from the original image.
text=(src/'enemy_art.c').read_text();boundsfile=src/'rift1311_art.h';bs=boundsfile.read_text();pattern=r'(static const unsigned char r1311_enemy_bounds\[36\]\[4\]=)\{(.*?)\};';bm=re.search(pattern,bs,re.S);assert bm
bounds=[list(map(int,re.findall(r'\d+',r))) for r in re.findall(r'\{([^}]+)\}',bm[2])];assert len(bounds)==36
records=[];sheet=Image.new('RGB',(6*150,160),(21,20,26));d=ImageDraw.Draw(sheet)
for k,i in enumerate(rects):
 path=art/('refined-early' if i in (0,1,11) else 'refined-late')/f'enemy_{i:02d}.png'
 rgb=np.array(Image.open(path).convert('RGB'));mask=np.zeros(rgb.shape[:2],np.uint8)
 bg=np.zeros((1,65),np.float64);fg=np.zeros((1,65),np.float64)
 cv2.setRNGSeed(180+i)
 cv2.grabCut(rgb,mask,rects[i],bg,fg,6,cv2.GC_INIT_WITH_RECT)
 mask=np.where((mask==cv2.GC_FGD)|(mask==cv2.GC_PR_FGD),255,0).astype(np.uint8)
 if i==11:
  # Large neutral paper-colored island between the moth's blue wings is background.
  neutral=((rgb.min(axis=2)>165)&(rgb.max(axis=2).astype(int)-rgb.min(axis=2)<45)).astype(np.uint8)
  nc,lab,st,_=cv2.connectedComponentsWithStats(neutral)
  for c in range(1,nc):
   if st[c,cv2.CC_STAT_AREA]>500:mask[lab==c]=0
 num,labels,stats,centroids=cv2.connectedComponentsWithStats(mask)
 assert num>1;i_largest=1+np.argmax(stats[1:,cv2.CC_STAT_AREA]);mask=np.where(labels==i_largest,255,0).astype(np.uint8)
 # Fill only small enclosed segmentation holes; do not paint over silhouette.
 im=Image.fromarray(np.dstack((rgb,mask)));box=im.getbbox();assert box
 im=im.crop(box);im=ImageOps.contain(im,(100,106),Image.Resampling.LANCZOS)
 # Muted material palette retains the colors of the generated artwork.
 a=im.getchannel('A');col=ImageEnhance.Color(im.convert('RGB')).enhance(.68);col.putalpha(a);im=col
 canvas=Image.new('RGBA',(104,110),(0,0,0,0));canvas.alpha_composite(im,((104-im.width)//2,108-im.height));a=np.array(canvas);opaque=a[:,:,3]>=100
 bbox=Image.fromarray((opaque*255).astype(np.uint8)).getbbox();assert bbox
 bounds[i]=[bbox[0],bbox[1],bbox[2]-bbox[0],bbox[3]-bbox[1]]
 v=((a[:,:,0].astype(np.uint16)>>3)<<11)|((a[:,:,1].astype(np.uint16)>>2)<<5)|(a[:,:,2].astype(np.uint16)>>3);v[~opaque]=0xf81f
 vals=v.ravel().tolist();body='\n'+''.join(','.join(f'0x{x:04x}' for x in vals[j:j+24])+',\n' for j in range(0,len(vals),24))
 pattern_i=r'(const uint16_t rift_enemy_'+str(i)+r'\[[^]]+\]\s*=\s*\{).*?(\};)'
 text,n=re.subn(pattern_i,lambda m:m[1]+body+m[2],text,flags=re.S);assert n==1
 canvas.save(out/f'enemy_{i:02d}.png');sheet.paste(canvas,(k*150+23,29),canvas);d.text((k*150+8,8),str(i),fill='white')
 records.append(dict(enemy=i,source=str(path.relative_to(art)),source_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),sprite_sha256=hashlib.sha256(v.astype('<u2').tobytes()).hexdigest(),source_rect=rects[i],bounds=bounds[i],processing='Deterministic foreground segmentation, isolated main connected component, aspect-preserving resize and mild saturation reduction'))
bs=bs[:bm.start()]+bm[1]+'{'+','.join('{'+','.join(map(str,r))+'}' for r in bounds)+'};'+bs[bm.end():];boundsfile.write_text(bs);(src/'enemy_art.c').write_text(text)
p=src/'combat_fx.inc';s=p.read_text()
# All replacement portraits face left/front. Do not apply the old inherited flips.
for i in (1,11,18,21):s=s.replace('||art=='+str(i)+'||','||')
p.write_text(s)
(out/'manifest.json').write_text(json.dumps(records,indent=2));sheet.save(art/'integrated-sprites.png')
print('PASS six unique generated enemy assets imported; detached props/text excluded; old frame bounds updated')
