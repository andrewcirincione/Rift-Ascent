from pathlib import Path
from PIL import Image, ImageDraw
import re, json, shutil
q=Path(__file__).parent;repo=q.parents[1];assets=q/'assets';assets.mkdir(exist_ok=True)
files={'act':'dc6a9e18-2647-4813-992a-5e2cfbb3c537','plating':'6af9f448-5f14-4112-a217-383050f8a31b','shop':'d1f8be5e-24b9-41d5-a808-d780e6025dfd','triangle':'1a554ab9-7734-4033-82c5-3d6ef3e3cb91'}
for name,size in [('act',(480,272)),('plating',(22,22)),('shop',(204,22)),('triangle',(14,14)),('arrow_glow',(36,28))]:
 source=q/'art'/f'{name}.png'
 if not source.exists():shutil.copyfile(repo.parent/'generated_images'/f'exec-{files[name]}.png',source)
 im=Image.open(source).convert('RGBA')
 if name!='act':im=im.crop(im.getchannel('A').point(lambda a:255 if a>12 else 0).getbbox())
 im=im.resize(size,Image.Resampling.LANCZOS);im.save(assets/f'{name}.png')
header='#pragma once\n#include <stdint.h>\n'
for p in sorted(assets.glob('*.png')):
 im=Image.open(p).convert('RGBA');vals=[r|g<<8|b<<16|a<<24 for r,g,b,a in im.getdata()]
 header+=f'static const uint32_t r1311_{p.stem}[{len(vals)}]={{\n'+'\n'.join(','.join(map(str,vals[i:i+64]))+',' for i in range(0,len(vals),64))+'\n};\n'
# Ground sprites at their connected silhouette, ignoring detached stray pixels.
s=(repo/'GAME/src/enemy_art.c').read_text();bounds=[];sheet=Image.new('RGB',(832,600),(32,32,37));draw=ImageDraw.Draw(sheet)
for i in range(36):
 m=re.search(r'const uint16_t rift_enemy_'+str(i)+r'\[.*?\] = \{(.*?)\};',s,re.S)
 v=[int(x,16) for x in re.findall(r'0x[0-9a-fA-F]+',m[1])]
 live={(j%104,j//104) for j,c in enumerate(v) if c!=0xf81f};remaining=set(live);groups=[]
 while remaining:
  stack=[remaining.pop()];group=[]
  while stack:
   x,y=stack.pop();group.append((x,y))
   for dx,dy in [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(1,1),(1,-1),(-1,1)]:
    xy=(x+dx,y+dy)
    if xy in remaining:remaining.remove(xy);stack.append(xy)
  groups.append(group)
 body=max(groups,key=len);bottom=max(y for x,y in body)
 # Keep all substantial silhouette pieces, but discard detached dust below feet.
 useful=[p for group in groups if len(group)>=12 for p in group if p[1]<=bottom]
 left=min(x for x,y in useful);right=max(x for x,y in useful);top=min(y for x,y in useful)
 bounds.append((left,top,right-left+1,bottom-top+1))
 im=Image.new('RGBA',(104,110));im.putdata([((c>>11)*255//31,((c>>5)&63)*255//63,(c&31)*255//31,255) if c!=0xf81f else (0,0,0,0) for c in v])
 x=i%8*104;y=i//8*120;sheet.paste(im,(x,y),im);draw.text((x+4,y+108),str(i),fill='white')
header+='static const unsigned char r1311_enemy_bounds[36][4]={'+','.join('{'+','.join(map(str,b))+'}' for b in bounds)+'};\n'
for dst in ['GAME/src','portable_build']:(repo/dst/'rift1311_art.h').write_text(header)
sheet.save(repo.parent/'evidence1311/enemy-atlas.png')
# XMB icon uses exactly the same painted logo as the main menu.
logo=Image.open(repo/'qa/v134/assets/logo.png').convert('RGBA');logo.thumbnail((140,72),Image.Resampling.LANCZOS)
icon=Image.new('RGBA',(144,80));icon.alpha_composite(logo,((144-logo.width)//2,(80-logo.height)//2))
for dst in ['portable_build','GAME']:icon.save(repo/dst/'ICON0.PNG')
print('Packed act gate, Plating, restrained shop plate, Triangle and matching XMB logo')
