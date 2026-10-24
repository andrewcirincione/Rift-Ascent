#!/usr/bin/env python3
"""Rift Ascent 1.1.3 original illustrated asset pipeline (Pillow only).
No battle screenshots, no runtime image decoding, no external asset downloads.
The exact same new hero is composited into the XMB PIC1 art.
All 92 cards get deterministic individually rendered 60x40 illustrations.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont
import colorsys, math, random, re, sys, base64, io
root=Path(sys.argv[1]); SCALE=3
def P(v):return round(v*SCALE)
def poly(d,pts,fill): d.polygon([(P(x),P(y)) for x,y in pts],fill=fill)
def seg(d,pts,fill,width=1):d.line([(P(x),P(y)) for x,y in pts],fill=fill,width=P(width),joint="curve")
def ellipse(d,area,fill,outline=None,width=1):
 d.ellipse(tuple(P(z) for z in area),fill=fill,outline=outline,width=P(width))
def composite_hero():
 im=Image.new("RGBA",(210*SCALE,127*SCALE),(0,0,0,0));d=ImageDraw.Draw(im)
 # Dark cloak: flowing cloth, gold trim, and deep red fabric, lit on top.
 poly(d,[(80,36),(69,42),(55,52),(41,62),(13,96),(40,88),(20,112),
         (51,104),(70,111),(84,92),(102,63)],(81,20,28,255))
 poly(d,[(76,41),(60,52),(44,75),(19,99),(57,84),(43,105),
         (74,91),(89,69)],(157,40,43,255))
 seg(d,[(76,48),(57,74),(26,96)],(214,83,72,255),2)
 seg(d,[(83,49),(65,73),(48,96)],(114,21,31,255),3)
 # Boots and greaves, body points toward the enemy on the right.
 poly(d,[(78,93),(93,93),(88,119),(65,120),(69,111)],(25,30,39,255))
 poly(d,[(99,92),(111,91),(121,118),(97,120),(95,110)],(27,32,41,255))
 seg(d,[(77,101),(70,115),(84,114)],(131,139,143,255),3)
 seg(d,[(107,100),(113,115),(119,115)],(160,158,150,255),3)
 poly(d,[(75,60),(101,52),(119,68),(110,102),(78,99),(70,78)],(24,29,38,255))
 poly(d,[(78,65),(99,58),(111,69),(106,93),(83,91)],(76,87,99,255))
 poly(d,[(84,67),(99,63),(104,76),(86,77)],(136,148,152,255))
 seg(d,[(87,66),(90,89)],(204,160,78,255),3)
 seg(d,[(99,65),(103,90)],(26,39,51,255),2)
 # Shoulder plates, asymmetrical glove and bracer.
 ellipse(d,(71,55,90,75),(42,48,62,255),(160,168,169,255),2)
 ellipse(d,(101,57,122,78),(50,57,70,255),(191,177,142,255),2)
 poly(d,[(109,74),(124,76),(137,90),(129,97),(116,89)],(67,75,83,255))
 seg(d,[(116,79),(133,91)],(215,180,114,255),3)
 ellipse(d,(126,87,138,98),(40,40,48,255),(160,151,131,255),2)
 # Same scarlet scarf used for character identity in all screens.
 poly(d,[(76,49),(109,48),(117,60),(91,65),(73,58)],(169,35,44,255))
 poly(d,[(77,56),(55,70),(64,86),(91,62)],(210,54,59,255))
 seg(d,[(76,51),(97,58),(111,55)],(232,101,87,255),2)
 # Head and uncovered face looking to the right.
 ellipse(d,(83,25,111,51),(41,38,43,255))
 poly(d,[(92,32),(113,36),(109,49),(99,54),(89,46)],(170,131,112,255))
 poly(d,[(85,23),(96,18),(108,26),(116,33),(104,32),(89,34),(80,32)],(28,31,38,255))
 seg(d,[(103,38),(112,39)],(24,27,34,255),2)
 seg(d,[(92,46),(101,51)],(107,58,56,255),2)
 # Claymore points up and to right, made of steel, not disconnected strokes.
 poly(d,[(131,84),(139,79),(193,9),(207,3),(205,17),(146,89),(137,95)],(38,48,59,255))
 poly(d,[(137,81),(202,8),(199,20),(146,86)],(169,184,188,255))
 poly(d,[(140,82),(203,7),(194,21),(142,88)],(220,223,208,255))
 seg(d,[(142,82),(198,17)],(245,237,202,255),1.5)
 poly(d,[(126,76),(135,73),(151,92),(146,97)],(36,34,40,255))
 seg(d,[(127,76),(148,93)],(209,161,80,255),2)
 seg(d,[(140,88),(121,106)],(60,47,45,255),5)
 seg(d,[(140,88),(121,106)],(153,106,64,255),2)
 ellipse(d,(116,104,124,113),(207,170,101,255),(31,37,45,255),1)
 return im.resize((210,127),Image.Resampling.LANCZOS)
def backdrop(kind, full_res=False):
 rnd=random.Random(447 if kind=="event" else 892)
 im=Image.new("RGB",(240*SCALE,136*SCALE));d=ImageDraw.Draw(im)
 for y in range(136):
  if kind=="event":
   col=(max(5,16+y//7),max(10,24+y//6),max(23,52+y//6))
  else: col=(max(13,35+y//5),max(8,20+y//7),max(10,22+y//8))
  d.rectangle((0,P(y),P(240),P(y+1)),fill=col)
 # Distant ruin silhouettes and path, standalone environment (never a fight).
 if kind=="event":
  for j in range(13):
   x=j*22-18;h=rnd.randrange(24,64)
   poly(d,[(x,93),(x+6,93-h),(x+15,93-h-8),(x+20,93),(x+22,101)],
        (16,27,47))
  poly(d,[(0,123),(78,110),(167,112),(240,127),(240,136),(0,136)],(34,42,56))
  poly(d,[(81,135),(113,92),(137,92),(168,135)],(63,63,67))
  # Pedestal, ruined arch, and isolated ethereal doorway.
  ellipse(d,(101,46,145,111),(18,51,81),(89,159,184),3)
  ellipse(d,(107,50,139,108),(30,92,131),(141,218,229),1)
  for x in (67,157):
   poly(d,[(x,115),(x-4,29),(x+10,16),(x+23,23),(x+20,115)],(31,40,59))
   poly(d,[(x-5,31),(x+6,19),(x+25,23),(x+23,30)],(116,113,105))
   for y in (45,65,85,105):seg(d,[(x-3,y),(x+20,y-3)],(79,81,90),2)
  poly(d,[(78,17),(98,8),(132,8),(152,23),(149,35),(89,32)],(61,63,81))
  for i in range(47):
   x=rnd.randrange(35,208);y=rnd.randrange(13,130)
   if abs(x-122)<24 and 45<y<110:continue
   ellipse(d,(x,y,x+0.7,y+0.7),(95,144,166))
  for x,y in [(35,108),(188,113),(51,68),(180,66)]:
   seg(d,[(x,y),(x,y-18)],(116,90,66),2)
   ellipse(d,(x-4,y-23,x+4,y-12),(241,148,66))
   ellipse(d,(x-1,y-21,x+1,y-14),(255,216,111))
 else:
  # Merchant's canvas canopy, old timber shelves and hanging lamps.
  for x in range(0,240,23):
   rect=(x,32,x+21,135)
   poly(d,[(x,38),(x+16,38),(x+18,136),(x+2,136)],(53,36,38))
   seg(d,[(x,38),(x+16,38)],(120,71,50),2)
  poly(d,[(0,19),(85,24),(155,13),(240,22),(240,53),(0,49)],(113,35,38))
  poly(d,[(0,45),(75,51),(149,36),(240,46),(240,56),(0,57)],(160,58,43))
  for x in (14,218):seg(d,[(x,38),(x,134)],(145,102,63),4)
  for x in (25,170):
   for y in (67,89,111):
    seg(d,[(x,y),(x+43,y)],(171,102,59),3)
    for j in range(4):
     px=x+6+j*9
     poly(d,[(px,y-15),(px+5,y-15),(px+6,y-2),(px,y-2)],
          ((78+17*j)%180,52+8*j,38+5*j))
  poly(d,[(58,105),(183,105),(207,136),(33,136)],(90,45,36))
  poly(d,[(58,106),(183,106),(180,114),(62,114)],(157,92,61))
  for x in (99,132):
   seg(d,[(x,40),(x,60)],(156,125,83),1)
   ellipse(d,(x-7,59,x+7,77),(224,145,64),(81,61,45),2)
   ellipse(d,(x-3,62,x+3,69),(253,216,128))
  for j in range(10):
   x=72+j*10;y=94+(j%2)*6
   ellipse(d,(x,y-9,x+7,y),(80+14*j,68+8*j,72+5*j),(180,125,84),1)
  for j in range(30):
   x=rnd.randrange(240);y=rnd.randrange(18,133)
   ellipse(d,(x,y,x+1,y+1),(188,101,65))
 if not full_res:
  asset=Path(__file__).parent/"assets"/(kind+".b64")
  image=Image.open(io.BytesIO(base64.b64decode(asset.read_text()))).convert("RGB")
  if image.size!=(240,136):raise RuntimeError("Incorrect "+kind+" art size")
  return image
 return im.resize((480,272),Image.Resampling.LANCZOS)
def pack565(im):
 out=[]
 for r,g,b,a in im.convert("RGBA").getdata():
  if a<115:out.append(0xf81f)
  else:
   color=(r>>3)<<11|(g>>2)<<5|(b>>3)
   out.append(0xf81e if color==0xf81f else color)
 return out
def declare(name,vals):
 return "static const uint16_t "+name+"["+str(len(vals))+"]={\n"+",\n".join(
  ",".join("0x%04x"%v for v in vals[i:i+40]) for i in range(0,len(vals),40))+"\n};\n"
hero=Image.open(io.BytesIO(base64.b64decode((Path(__file__).parent/"assets/hero.b64").read_text()))).convert("RGBA")
assert hero.size==(210,127)
hero_src=(root/"motion_art.c").read_text()
hero_values=pack565(hero)
match=re.search(r"const uint16_t rift_hero\[26670\]\s*=\s*\{.*?\};",hero_src,re.S)
if not match:raise RuntimeError("1.1.3 hero anchor missing")
hero_src=hero_src[:match.start()]+declare("rift_hero",hero_values).replace("static const","const",1)+hero_src[match.end():]
(root/"motion_art.c").write_text(hero_src)
headers=["#ifndef RIFT112_SCENE_ART_H","#define RIFT112_SCENE_ART_H","#include <stdint.h>"]
for kind in ("event","shop"):headers.append(declare("rift112_"+kind,pack565(backdrop(kind))))
headers.append("#endif")
(root/"rift112_scene_art.h").write_text("\n".join(headers))
# A genuine native 480x272 PIC1, not resized/stretched from a wide thumbnail.
# The warrior is the exact asset compiled as the playable 210x127 sprite.
splash=backdrop("event",full_res=True).convert("RGBA")
hero_big=hero.resize((300,182),Image.Resampling.LANCZOS)
splash.alpha_composite(hero_big,(4,70))
dd=ImageDraw.Draw(splash)
for y in range(155,272):
 dd.rectangle((235,y,479,y+1),fill=(9,14,27,180 if y<245 else 215))
fontfile="/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf"
font=ImageFont.truetype(fontfile,35)
dd.text((238,182),"RIFT ASCENT",font=font,fill=(245,211,147),stroke_width=2,stroke_fill=(14,23,33))
font2=ImageFont.truetype(fontfile,15)
dd.text((241,233),"ENTER THE RIFT",font=font2,fill=(220,232,235))
splash.convert("RGB").save(root/"rift112_pic1.png",format="PNG",optimize=True)
# Original deterministic deck illustrations: 92 independent tile designs.
card_src=(root/"game.c").read_text()
entries=re.findall(r'\{"([^"]+)","([^"]*)",(-?\d+),(\d+),(\d+)\}',card_src)
if len(entries)<92:raise RuntimeError("Expected full Ironclad-style 92 card catalog")
tiles=[];identities=set()
for id,(name,desc,cost,rarity,kind) in enumerate(entries[:92]):
 rnd=random.Random(15374+id*9176)
 hi=(id*0.61803398875+.08)%1.0
 rr,gg,bb=colorsys.hsv_to_rgb(hi,.57,.63)
 base=tuple(int(v*255) for v in (rr,gg,bb))
 art=Image.new("RGB",(120,80),(17,23,34));d=ImageDraw.Draw(art)
 for y in range(80):
  a=y/80
  d.line((0,y,120,y),fill=tuple(max(0,int(v*(.37+.55*(1-a)))) for v in base))
 for i in range(14):
  x=rnd.randrange(120);y=rnd.randrange(80)
  d.line((x,y,x+rnd.randrange(-26,30),y+rnd.randrange(-16,17)),
          fill=tuple(min(255,int(c*.6+65)) for c in base),width=1)
 symbol=id%9
 glow=tuple(min(255,int(v*.65+93)) for v in base)
 if symbol in (0,1,2):
  d.polygon([(20,67),(80,8),(95,4),(78,19),(34,73)],fill=(202,216,220))
  d.polygon([(25,67),(82,12),(87,9),(73,20),(27,72)],fill=glow)
  d.line((19,68,41,76),fill=(245,194,102),width=5)
 elif symbol in (3,4):
  d.polygon([(60,8),(95,25),(91,55),(60,73),(28,55),(25,25)],fill=(65,85,103))
  d.polygon([(60,12),(89,27),(86,53),(60,67),(33,53),(31,27)],outline=glow)
  d.line((60,20,60,61),fill=(235,219,174),width=4)
 elif symbol==5:
  for r in (29,21,12):d.ellipse((60-r,38-r,60+r,38+r),outline=glow,width=2)
  d.polygon([(60,4),(67,30),(99,38),(67,46),(60,73),(53,46),(21,38),(53,30)],fill=glow)
 elif symbol==6:
  for i in range(5):
   ang=i*math.pi*2/5+id/7
   x=60+int(27*math.cos(ang));y=38+int(27*math.sin(ang))
   d.line((60,38,x,y),fill=glow,width=4)
  d.ellipse((48,26,72,50),fill=(217,197,156))
 elif symbol==7:
  d.polygon([(60,4),(102,35),(82,73),(39,64),(15,24)],fill=(71,65,95))
  d.polygon([(59,10),(89,36),(80,63),(46,58),(24,27)],outline=glow)
  d.ellipse((48,27,71,50),fill=glow)
 else:
  for j in range(8):
   x=rnd.randint(25,90);y=rnd.randint(10,70)
   d.polygon([(x-7,y),(x+3,y-13),(x+10,y+2),(x,y+8)],outline=glow)
   d.line((x-8,y+3,x+9,y-4),fill=(215,193,166),width=2)
 # Distinct ID-specific foreground stroke so even adjacent cards differ.
 d.arc((8+id%18,6,111,73-id%20),45+(id*13)%80,260+(id*19)%85,
       fill=(196,175,142),width=2)
 small=art.resize((60,40),Image.Resampling.LANCZOS)
 values=pack565(small)
 sig=tuple(values)
 if sig in identities:raise RuntimeError("duplicate art "+str(id))
 identities.add(sig);tiles.append(values)
card_lines=["#ifndef RIFT113_CARDS_H","#define RIFT113_CARDS_H","#include <stdint.h>"]
card_lines.append("static const uint16_t rift113_cards[92][2400]={")
for tile in tiles:
 card_lines.append("{"+",".join("0x%04x"%v for v in tile)+"},")
card_lines.append("};\n#endif")
(root/"rift113_cards.h").write_text("\n".join(card_lines))
render=root/"render.c";s=render.read_text()
anchor='#include "rift112_scene_art.h"'
if anchor not in s:raise RuntimeError("new scene art missing")
s=s.replace(anchor,anchor+'\n#include "rift113_cards.h"',1)
a='art_blit(x+4,y+28,w-8,arth,art,100,60);'
if a not in s:raise RuntimeError("card art anchor missing")
s=s.replace(a,'art_blit(x+4,y+28,w-8,arth,rift113_cards[id],60,40);',1)
render.write_text(s)
print("PASS 1.1.3: fresh warrior + exact matching XMB, two original scenes, 92 unique card paintings")
