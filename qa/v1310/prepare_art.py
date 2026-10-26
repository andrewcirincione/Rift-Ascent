from pathlib import Path
from PIL import Image
import re,json,hashlib
q=Path(__file__).parent;assets=q/'assets';assets.mkdir(exist_ok=True)
for name,size in [('hound',(144,96)),('end',(68,29)),('arrow',(36,28))]:
 im=Image.open(q/'originals'/f'{name}.png').convert('RGBA')
 bounds=im.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox();im=im.crop(bounds)
 if name=='hound':
  im.thumbnail(size,Image.Resampling.LANCZOS);canvas=Image.new('RGBA',size);canvas.alpha_composite(im,((size[0]-im.width)//2,size[1]-im.height));im=canvas
 else:im=im.resize(size,Image.Resampling.LANCZOS)
 im.save(assets/f'{name}.png')
# Keep more detail than the legacy 60x40 card atlas; renderer crops its vertical center.
im=Image.open(q/'originals/setup.png').convert('RGBA');im=im.resize((240,135),Image.Resampling.LANCZOS);im.save(assets/'setup.png')
header='#pragma once\n#include <stdint.h>\n'
for p in sorted(assets.glob('*.png')):
 im=Image.open(p).convert('RGBA');vals=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()]
 header+=f'#define R1310_{p.stem.upper()}_W {im.width}\n#define R1310_{p.stem.upper()}_H {im.height}\nstatic const uint32_t r1310_{p.stem}[{len(vals)}]={{\n'+'\n'.join(','.join(map(str,vals[i:i+64]))+',' for i in range(0,len(vals),64))+'\n};\n'
# Crop existing sprite sampling bounds to remove unused atlas margins, without altering the art.
s=Path('GAME/src/enemy_art.c').read_text();bounds=[]
for i in range(36):
 match=re.search(r'const uint16_t rift_enemy_'+str(i)+r'\[.*?\] = \{(.*?)\};',s,re.S)
 values=[int(v,16) for v in re.findall(r'0x[0-9a-fA-F]+',match[1])];assert len(values)==104*110
 xy=[(j%104,j//104) for j,v in enumerate(values) if v!=0xf81f]
 left=min(x for x,y in xy);right=max(x for x,y in xy);top=min(y for x,y in xy);bottom=max(y for x,y in xy)
 bounds.append((left,top,right-left+1,bottom-top+1))
header+='static const unsigned char r1310_enemy_bounds[36][4]={'+','.join('{'+','.join(map(str,b))+'}' for b in bounds)+'};\n'
for dst in ['GAME/src','portable_build']:Path(dst,'rift1310_art.h').write_text(header)
(q/'asset-provenance.json').write_text(json.dumps({'version':'1.3.10','tool':'Built-in OpenAI image generation','assets':[{'file':str(p.relative_to(q)),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted((q/'originals').glob('*.png'))],'processing':'Alpha-bounds crop and size conversion; Setup Strike keeps 240x135 source. Existing enemy sprite bounds recorded for larger in-game sampling. No decorative line substitutes.'},indent=2)+'\n')
print('Generated art embedded; all 36 enemy bounds recorded')
