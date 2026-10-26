from pathlib import Path
from PIL import Image
import sys
root=Path(__file__).parent;dst=Path(sys.argv[1]);header='#pragma once\n#include <stdint.h>\n'
def array(name,values):
 return 'static const uint32_t '+name+'[%d]={\n'%len(values)+'\n'.join(','.join(map(str,values[i:i+64]))+',' for i in range(0,len(values),64))+'\n};\n'
for name,count,size in [('impact',6,(96,96)),('shield',6,(96,96)),('power',6,(96,96)),('fire',6,(32,32)),('hero',18,(192,144))]:
 for i in range(count):
  im=Image.open(root/'assets'/f'{name}_{i}.png').convert('RGBA');assert im.size==size
  assert im.getchannel('A').getextrema()[0]==0
  header+=array('r137_'+name+'_'+str(i),[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()])
 header+=f'static const uint32_t *const r137_{name}_frames[{count}]={{'+','.join('r137_'+name+'_'+str(i) for i in range(count))+'};\n'
(dst/'rift137_art.h').write_text(header)
print('PASS: transparent generated combat FX and player keyframes embedded')
