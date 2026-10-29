"""Arrange unmodified native-render screenshots for visual review."""
from pathlib import Path
from PIL import Image,ImageDraw
p=Path(__file__).resolve().parent/'evidence'
for act in range(1,4):
 files=sorted(p.glob(f'native-enemy-*-act-{act}.ppm'));out=Image.new('RGB',(1120,680),'#121212');d=ImageDraw.Draw(out)
 for i,f in enumerate(files):
  x=i%4*280;y=i//4*170;im=Image.open(f);out.paste(im.crop((200,34,480,174)),(x,y+20));d.text((x+8,y+3),f.stem.replace('native-',''),fill='white')
 out.save(p/f'enemy-review-act-{act}.png')
for f in p.glob('native-*.ppm'):Image.open(f).save(f.with_suffix('.png'))
