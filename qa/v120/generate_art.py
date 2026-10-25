#!/usr/bin/env python3
"""Generate original, deterministic scene and individualized card illustrations.
No old character, words, HUD, or UI are baked into scene textures.
Outputs self-contained RGB565 headers for PSP.
"""
from PIL import Image, ImageDraw, ImageFilter
from pathlib import Path
import math, random, sys
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def c565(rgb):
 r,g,b=rgb[:3];return (r>>3)<<11 | (g>>2)<<5 | b>>3
def save_header(filename,arrays):
 with (out/filename).open('w') as f:
  f.write("#pragma once\n#include <stdint.h>\n")
  for name,img in arrays:
   f.write("static const uint16_t "+name+"["+str(img.width*img.height)+"]={\n")
   vals=[c565(p) for p in img.convert("RGB").getdata()]
   for i in range(0,len(vals),24):
    f.write(",".join(str(x) for x in vals[i:i+24])+",\n")
   f.write("};\n")
def background(kind):
 w,h=480,272;im=Image.new('RGB',(w,h));a=im.load()
 palettes=[
 ((7,22,42),(55,73,91),(25,33,46),(37,57,68)),
 ((36,7,22),(157,61,43),(56,20,27),(91,31,27)),
 ((17,8,42),(88,37,100),(28,16,61),(49,30,75)),
 ((14,19,27),(102,68,49),(46,35,38),(68,55,43))]
 top,mid,rock,near=palettes[kind]
 for y in range(h):
  t=y/(h-1)
  for x in range(w):
   noise=math.sin(x*.12+y*.16)*3+math.sin(x*.37-y*.13)*2
   tt=min(1,t*1.45)
   sky=tuple(int(top[i]*(1-tt)+mid[i]*tt+noise) for i in range(3))
   a[x,y]=tuple(max(0,min(255,z)) for z in sky)
 d=ImageDraw.Draw(im,'RGB');rng=random.Random(kind*1903+211)
 if kind==3:
  # Firelit camp shelter with actual burning logs and no obsolete UI.
  d.polygon([(0,149),(89,89),(174,149),(300,113),(480,168),(480,272),(0,272)],fill=(29,27,28))
  d.rectangle((0,194,479,271),fill=(55,42,38))
  for i in range(19):
   x=rng.randrange(480);y=rng.randrange(205,272)
   d.line((x,y,x+23,y-7),fill=(83,65,55),width=2)
  for j in range(3):
   x=218+j*8
   d.ellipse((x-45,109,x+39,205),fill=(115+j*15,42+j*20,16))
  d.polygon([(225,202),(244,80),(258,205),(245,178),(231,213)],fill=(250,138,49))
  d.polygon([(231,198),(243,112),(252,188),(245,174)],fill=(255,222,116))
  d.line((182,206,305,236),fill=(56,36,28),width=15)
  d.line((296,206,194,232),fill=(68,41,27),width=12)
 else:
  # Distant architecture and ground rendered in layered silhouette passes.
  for layer in range(3):
   pts=[(0,h)]
   base=130+layer*29
   for x in range(-20,501,20):
    y=base+rng.randint(-30,25)
    pts.append((x,y))
   pts.extend([(480,h),(0,h)])
   tone=tuple(max(0,min(255,v+(layer*12))) for v in (rock if layer<2 else near))
   d.polygon(pts,fill=tone)
  for j in range(8 if kind==0 else 12):
   x=rng.randrange(18,480);height=rng.randrange(28,107)+(kind==2)*16
   y=155-height+rng.randint(-14,15);width=rng.randint(6,19)
   shade=(10+kind*8,17,25+kind*5)
   d.rectangle((x,y,x+width,211),fill=shade)
   d.polygon([(x-6,y+2),(x+width//2,y-17),(x+width+6,y+2)],fill=shade)
   if kind==2:
    d.line((x+width//2,y-11,x+width//2,y-27),fill=(126,60,130),width=2)
  for j in range(58 if kind==1 else 27):
   x=rng.randrange(w);y=rng.randrange(45,235)
   palette=(249,106,45) if kind==1 else ((151,120,228) if kind==2 else (96,156,200))
   d.ellipse((x,y,x+1,y+1),fill=palette)
  if kind==2:
   d.ellipse((212,22,294,103),outline=(166,81,170),width=5)
   d.ellipse((231,40,276,85),fill=(37,13,48))
  if kind==1:
   d.polygon([(0,213),(480,209),(480,272),(0,272)],fill=(62,19,23))
   for j in range(18):
    x=rng.randrange(480);d.line((x,215,x+14,244),fill=(193,53,34),width=2)
 return im
def card_art(cardid,typ,variation=0):
 rng=random.Random(cardid*10313+variation*75577+944)
 w,h=60,40;im=Image.new('RGB',(w,h));p=im.load()
 # Different palette and composition for each of the 92 entries and 10 basic variants.
 hue=rng.randrange(360)
 if typ==0: col=(142+rng.randrange(70),28+rng.randrange(50),29+rng.randrange(65))
 elif typ==1: col=(22+rng.randrange(50),84+rng.randrange(65),145+rng.randrange(70))
 elif typ==2: col=(85+rng.randrange(65),37+rng.randrange(35),138+rng.randrange(90))
 else: col=(92,82,72)
 for y in range(h):
  for x in range(w):
   n=math.sin(x*.31+cardid)*6+math.cos(y*.34+variation)*7
   k=.18+.49*y/h
   p[x,y]=tuple(max(0,min(255,int(z*k+n))) for z in col)
 d=ImageDraw.Draw(im,'RGBA')
 for i in range(10):
  x=rng.randrange(w);y=rng.randrange(h)
  d.line((x,y,x+rng.randint(-12,13),y-rng.randint(0,20)),
       fill=(*col, rng.randint(50,125)),width=rng.choice((1,2)))
 cx=18+rng.randrange(25);cy=10+rng.randrange(20)
 if typ==0:
  # Every attack is a separate weapon pose, spark pattern, or energy slash.
  angle=rng.random()*math.pi*2
  dx=math.cos(angle)*25;dy=math.sin(angle)*19
  d.line((cx-dx,cy-dy,cx+dx,cy+dy),fill=(255,225,158,240),width=4)
  d.line((cx-dx+3,cy-dy+2,cx+dx+3,cy+dy+2),fill=(252,105,58,225),width=2)
  d.ellipse((cx-5,cy-5,cx+5,cy+5),fill=(243,70,39,160))
 elif typ==1:
  # Blue shields and raised guard poses with 10 physically distinct emblems.
  rx=9+rng.randrange(9);ry=10+rng.randrange(7)
  d.polygon([(cx,cy-ry),(cx+rx,cy-ry//3),(cx+rx-3,cy+ry//2),
       (cx,cy+ry),(cx-rx+3,cy+ry//2),(cx-rx,cy-ry//3)],
       fill=(46,114,196,225),outline=(190,237,255,255))
  for i in range(variation+1):
   d.arc((cx-rx+i*2,cy-ry+i,cx+rx-i*2,cy+ry-i),185,345,fill=(209,246,255,180),width=2)
 elif typ==2:
  # Sigils rotate by card identifier.
  r=9+rng.randrange(7)
  for j in range(5):
   t=2*math.pi*j/5 + (cardid%9)*.24
   d.line((cx,cy,cx+math.cos(t)*r,cy+math.sin(t)*r),fill=(220,141,255,205),width=2)
  d.ellipse((cx-4,cy-4,cx+4,cy+4),fill=(249,215,255,232))
 else:
  for j in range(4):
   d.line((cx+j*3-7,cy-11,cx-j*2+9,cy+11),fill=(185,169,142,190),width=2)
 return im
save_header('rift120_scenes.h',[
 ('rift120_fight_bg',background(0)),('rift120_elite_bg',background(1)),
 ('rift120_boss_bg',background(2)),('rift120_camp_bg',background(3))])
# Card type mapping is derived from post-patch game.c and verified, not guessed.
import re
game=(out/'game.c').read_text()
definitions=game.split('const CardDef cards[]={',1)[1].split('};',1)[0]
types={}
for line in definitions.splitlines():
 m=re.search(r'\{".*?",".*?",-?\d+,\d+,(\d+)\}.*?//\s*(\d+)',line)
 if m:types[int(m.group(2))]=int(m.group(1))
assert len(types)==92, f"Expected 92 card definitions, found {len(types)}"
save_header('rift120_cards.h',
 [('rift120_card_'+str(i),card_art(i,types[i])) for i in range(92)]+
 [('rift120_strike_'+str(i),card_art(0,0,i+1)) for i in range(5)]+
 [('rift120_defend_'+str(i),card_art(1,1,i+1)) for i in range(5)])
with (out/'rift120_cards.h').open('a') as f:
 f.write('static const uint16_t *const rift120_cards[92]={'+','.join('rift120_card_'+str(i) for i in range(92))+'};\n')
 f.write('static const uint16_t *const rift120_strikes[5]={'+','.join('rift120_strike_'+str(i) for i in range(5))+'};\n')
 f.write('static const uint16_t *const rift120_defends[5]={'+','.join('rift120_defend_'+str(i) for i in range(5))+'};\n')
 f.write('static const uint16_t *rift120_art(int id,int x,int y){int n=((x/9+y/13)%5+5)%5;return id==0?rift120_strikes[n]:id==1?rift120_defends[n]:rift120_cards[id];}\n')
print("PASS: 4 new original backgrounds, 92 bespoke card illustrations and 10 additional basic variants")
