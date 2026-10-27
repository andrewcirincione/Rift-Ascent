"""Crop and resize generated bronze variants; retain original colors and alpha."""
from pathlib import Path
from PIL import Image
import numpy as np
q=Path(__file__).parent;source=q.parent/'source';im=Image.open(q/'art/bronze-stars.png').convert('RGBA')
assert im.size==(1774,887),im.size
header='#pragma once\n#include <stdint.h>\n'
for name,x in [('lit',22),('empty',872)]:
 tile=im.crop((x,0,x+880,887)).resize((40,40),Image.Resampling.LANCZOS)
 tile.save(q/'art'/f'star-{name}.png')
 a=np.asarray(tile,dtype=np.uint32);v=(a[:,:,0]|a[:,:,1]<<8|a[:,:,2]<<16|a[:,:,3]<<24).ravel()
 header+=f'static const uint32_t r160_star_{name}[1600]={{\n'
 header+='\n'.join(','.join(map(str,v[i:i+40]))+',' for i in range(0,1600,40))+'\n};\n'
(source/'rift160_star_contrast.h').write_text(header)
print('Packed generated bronze star, glow and depleted variants')
