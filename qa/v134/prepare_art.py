from pathlib import Path
from PIL import Image,ImageFont,ImageDraw
import json,struct,sys
q=Path(__file__).parent;dest=Path(sys.argv[1]) if len(sys.argv)>1 else Path('GAME/src')
assets=q/'assets';assets.mkdir(exist_ok=True)
# Only conversion, atlas extraction and resampling happen here. The artwork is generated.
assert Image.open(assets/'hand_hud.png').getchannel('A').getextrema()==(255,255)
def array(name,values,ctype='uint32_t'):
 return 'static const '+ctype+' '+name+'[%d]={\n'%len(values)+'\n'.join(','.join(map(str,values[i:i+64]))+',' for i in range(0,len(values),64))+'\n};\n'
head='#pragma once\n#include <stdint.h>\n'
for p in sorted(assets.glob('*.png')):
 im=Image.open(p).convert('RGBA');head+=f'#define R134_{p.stem.upper()}_W {im.width}\n#define R134_{p.stem.upper()}_H {im.height}\n'
 head+=array('r134_'+p.stem,[r|(g<<8)|(b<<16)|(a<<24) for r,g,b,a in im.getdata()])
for name,count in [('card_frames',4),('map_icons',8),('event_icons',12),('utility_icons',6),('attack_fx',6)]:
 head+=f'static const uint32_t *const r134_{name}[{count}]={{'+','.join('r134_'+name+'_'+str(i) for i in range(count))+'};\n'
(dest/'rift134_art.h').write_text(head)
fonthead='#pragma once\n#include <stdint.h>\ntypedef struct {int w,h;const uint8_t *alpha,*advance;} RiftFont;\n'
for size,bold in [(8,0),(9,0),(9,1),(10,0),(18,1)]:
 name=f'f{size}'+('b' if bold else '');font=ImageFont.truetype(str(q/'fonts'/('DejaVuSans'+('-Bold' if bold else '')+'.ttf')),size)
 cellw=size+4;cellh=size+3;values=[];adv=[];top=font.getbbox('Ag')[1]
 for code in range(32,127):
  im=Image.new('L',(cellw,cellh));ImageDraw.Draw(im).text((0,-top),chr(code),font=font,fill=255)
  values.extend(im.getdata());adv.append(max(2,round(font.getlength(chr(code)))))
 fonthead+=array('r134_'+name+'_a',values,'uint8_t')+array('r134_'+name+'_w',adv,'uint8_t')
 fonthead+=f'static const RiftFont r134_{name}={{{cellw},{cellh},r134_{name}_a,r134_{name}_w}};\n'
(dest/'rift134_fonts.h').write_text(fonthead)
print('Packed generated RGBA UI, icon atlases, six attack frames, and native-size antialiased fonts')
