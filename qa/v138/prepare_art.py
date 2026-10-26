"""Convert generated raster assets and licensed serif glyphs for PSP."""
from pathlib import Path
from PIL import Image,ImageFont,ImageDraw
import sys
q=Path(__file__).parent;assets=q/'assets';assets.mkdir(exist_ok=True)
im=Image.open(q/'originals/ui-atlas.png').convert('RGBA')
for name,box,size in [('player_hp',(0,0,1536,282),(140,30)),('button_on',(0,282,1536,548),(176,32))]:
 cell=im.crop(box);cell=cell.crop(cell.getchannel('A').getbbox());cell.resize(size,Image.Resampling.LANCZOS).save(assets/(name+'.png'))
im=Image.open(q/'originals/markers-atlas.png').convert('RGBA')
for name,box,size in [('pawn',(0,0,im.width//2,im.height),(34,30)),('choice',(im.width//2,0,im.width,im.height),(42,28))]:
 cell=im.crop(box);cell=cell.crop(cell.getchannel('A').getbbox());cell.resize(size,Image.Resampling.LANCZOS).save(assets/(name+'.png'))
Image.open(q/'originals/menu-scene.png').convert('RGB').resize((480,272),Image.Resampling.LANCZOS).save(assets/'menu.png')
def array(name,values,ctype):
 return 'static const '+ctype+' '+name+'[%d]={\n'%len(values)+'\n'.join(','.join(map(str,values[i:i+64]))+',' for i in range(0,len(values),64))+'\n};\n'
header='#pragma once\n#include <stdint.h>\n'
for p in sorted(assets.glob('*.png')):
 im=Image.open(p).convert('RGBA');header+=f'#define R138_{p.stem.upper()}_W {im.width}\n#define R138_{p.stem.upper()}_H {im.height}\n'
 if(p.stem=='menu'):values=[(r>>3)<<11|(g>>2)<<5|(b>>3) for r,g,b,a in im.getdata()];ctype='uint16_t'
 else:values=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()];ctype='uint32_t'
 header+=array('r138_'+p.stem,values,ctype)
if (assets/'death_5.png').exists():header+='static const uint32_t *const r138_death[6]={'+','.join('r138_death_'+str(i) for i in range(6))+'};\n'
for size in [7,8,9,10]:
 f=ImageFont.truetype(str(q/'fonts/DejaVuSerif-Bold.ttf'),size);w,h=size+5,size+3;values=[];adv=[];top=f.getbbox('Ag')[1]
 for code in range(32,127):
  cell=Image.new('L',(w,h));ImageDraw.Draw(cell).text((0,-top),chr(code),font=f,fill=255);values.extend(cell.getdata());adv.append(max(2,round(f.getlength(chr(code)))))
 header+=array(f'r138_title{size}_a',values,'uint8_t')+array(f'r138_title{size}_w',adv,'uint8_t')+f'static const RiftFont r138_title{size}={{{w},{h},r138_title{size}_a,r138_title{size}_w}};\n'
for dst in sys.argv[1:] or ['GAME/src','portable_build']:(Path(dst)/'rift138_art.h').write_text(header)
print('Packed new generated menu, player HP, button plates, selection markers, death poses and serif titles')
