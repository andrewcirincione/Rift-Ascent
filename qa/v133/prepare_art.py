from pathlib import Path
from PIL import Image
import re,struct,sys,hashlib,json
root=Path(sys.argv[1]);assets=Path(__file__).parent/'assets'
def declaration(name,data):
 vals=struct.unpack('<%dH'%(len(data)//2),data)
 return 'static const uint16_t '+name+'[%d]={\n'%len(vals)+'\n'.join(','.join(str(v) for v in vals[i:i+80])+',' for i in range(0,len(vals),80))+'\n};\n'
# Compile the exact approved paintings into source; no post-link placeholders.
(root/'rift130_slots.h').write_text('#pragma once\n#include <stdint.h>\n'+''.join(declaration('rift130_'+n,(assets/(n+'.rgb565')).read_bytes()) for n in ['map1','map2','map3','boss1','boss2','boss3','normal1','normal2','elite1','elite2','camp']))
p=root/'rift120_cards.h';s=p.read_text()
for i in range(92):
 pattern=r'(static const uint16_t rift120_card_'+str(i)+r'\[[^]]+\]\s*=\s*\{)(.*?)(\};)'
 m=re.search(pattern,s,re.S)
 if not m:
  pattern=r'(static const uint16_t rift120_c'+str(i)+r'\[[^]]+\]\s*=\s*\{)(.*?)(\};)';m=re.search(pattern,s,re.S)
 if not m:raise RuntimeError('Missing card '+str(i))
 data=(assets/('card_'+str(i)+'.rgb565')).read_bytes();body=','.join(map(str,struct.unpack('<2400H',data)))
 s=s[:m.start(2)]+body+s[m.end(2):]
p.write_text(s)
header='#pragma once\n#include <stdint.h>\n'
for name in ['button','card_frame','hand_tray','energy']:
 im=Image.open(assets/(name+'.png')).convert('RGBA');w,h=im.size
 values=[]
 for r,g,b,a in im.getdata():
  v=0xf81f if a<96 else (r>>3)<<11|(g>>2)<<5|(b>>3)
  if a>=96 and v==0xf81f:v=0xf81e
  values.append(v)
 header+=f'#define RIFT133_{name.upper()}_W {w}\n#define RIFT133_{name.upper()}_H {h}\n'+declaration('rift133_'+name,struct.pack('<%dH'%len(values),*values))
(root/'rift133_art.h').write_text(header)
print('PASS: 103 preserved paintings + four generated Gothic UI assets compiled into source')
