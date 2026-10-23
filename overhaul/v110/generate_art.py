#!/usr/bin/env python3
"""Build genuine (non-line-overlay) claymore hero and antialiased room icon art.

Uses original motion_art.c sprite; removes its disconnected old weapon,
composites a modeled longsword with guard, fuller, hilt and pommel behind glove.
Outputs C source in GAME/src; user art, no binary external asset dependencies.
"""
from pathlib import Path
import re,sys
from PIL import Image,ImageDraw,ImageFont
root=Path(sys.argv[1]);S=4
def P(v):return int(round(v*S))
def polygon(d,points,color):d.polygon([(P(x),P(y)) for x,y in points],fill=color)
def stroke(d,points,color,w):d.line([(P(x),P(y)) for x,y in points],fill=color,width=P(w),joint="curve")
def ellipse(d,rect,color,outline=None):d.ellipse(tuple(P(q) for q in rect),fill=color,outline=outline,width=S)
source=(root/"motion_art.c").read_text()
match=re.search(r"const uint16_t rift_hero\[20828\] = \{(.*?)\};",source,re.S)
if not match:raise RuntimeError("Missing original 164x127 hero, cannot rebuild sword")
colors=[int(h,16) for h in re.findall(r"0x([0-9a-fA-F]{4})",match.group(1))]
assert len(colors)==20828
old=Image.new("RGBA",(164,127))
for i,c in enumerate(colors):
 if c==0xf81f:continue
 r=(c>>11)&31;g=(c>>5)&63;b=c&31
 old.putpixel((i%164,i//164),(r*255//31,g*255//63,b*255//31,255))
# The separate old weapon is the ONLY component in this rectangle.
for y in range(36,69):
 for x in range(135,163):old.putpixel((x,y),(0,0,0,0))
# Hand-matched to the original gritty 164x127 sprite. Paint the weapon at
# final PSP pixel resolution with the same subdued steel/bronze palette:
# no vector-smooth glow, gold starburst, or skyward detached diagonal.
# Length is proportional to the knight; the grip follows the existing glove.
hero=Image.new("RGBA",(202,127))
d=ImageDraw.Draw(hero)
offset=-7  # a slightly lowered forward guard, never above the knight's head
# Worn, double-edged 2-handed sword pointing forward and down.
d.polygon([(127,73+offset),(133,71+offset),(191,91+offset),
           (201,98+offset),(193,102+offset),(133,86+offset),
           (127,81+offset)],fill=(27,30,34,255))
d.polygon([(133,74+offset),(191,94+offset),(197,98+offset),
           (192,100+offset),(132,82+offset)],fill=(77,80,78,255))
d.polygon([(134,75+offset),(191,95+offset),(194,97+offset),
           (132,79+offset)],fill=(134,132,120,255))
d.polygon([(133,83+offset),(193,100+offset),(188,100+offset),
           (132,81+offset)],fill=(53,60,62,255))
d.line([(137,76+offset),(191,95+offset)],fill=(158,153,136,255),width=1)
d.line([(142,81+offset),(187,96+offset)],fill=(47,49,49,255),width=1)
# Fixed-seed pixels weather the steel, like the original's gritty pixels.
import random
rng=random.Random(5542)
for j in range(60):
 t=rng.random()
 x=int(140+48*t)
 y=int(78+18*t+offset+rng.uniform(-2,4))
 d.point((x,y),fill=rng.choice([
   (107,101,88,255),(57,60,55,255),
   (80,69,58,255),(118,112,100,255)]))
# Narrow leather-wrapped two-handed grip aligned to the armored arm.
d.line([(111,72+offset),(133,79+offset)],fill=(29,24,24,255),width=6)
d.line([(113,73+offset),(131,79+offset)],fill=(77,49,40,255),width=4)
for t in range(4):
 x=116+4*t
 y=74+offset+t
 d.line([(x,y-2),(x+2,y+2)],fill=(110,83,53,255),width=1)
# Restrained, weathered iron crossguard and slightly flared quillons.
d.line([(134,68+offset),(126,88+offset)],fill=(29,28,27,255),width=5)
d.line([(135,69+offset),(127,87+offset)],fill=(90,79,65,255),width=3)
d.polygon([(134,67+offset),(137,69+offset),
           (135,73+offset)],fill=(145,119,82,255))
d.polygon([(125,86+offset),(127,91+offset),
           (130,86+offset)],fill=(113,88,66,255))
d.ellipse((130,75+offset,135,82+offset),
          fill=(109,96,76,255),outline=(30,30,30,255))
# Original helmet, cape, plate and hand retain exactly their native pixels.
hero.alpha_composite(old)
d=ImageDraw.Draw(hero)
d.polygon([(110,69),(117,69),(124,75),(121,80),(114,77)],
          fill=(35,36,36,255))
d.line([(114,70),(121,77)],fill=(102,96,87,255),width=2)
# Do not resize or apply LANCZOS to the weapon: the original is low-res.
def icon(n):
 im=Image.new("RGBA",(30*S,30*S));d=ImageDraw.Draw(im)
 ellipse(d,(1,1,29,29),(16,26,39,248),(166,127,79,255))
 ellipse(d,(3,3,27,27),None,(57,76,96,255))
 if n==0: # Crossing steel swords
  polygon(d,[(8,7),(11,6),(23,20),(24,24),(20,23),(7,10)],(180,205,222,255))
  polygon(d,[(22,7),(19,6),(7,20),(6,24),(10,23),(23,10)],(133,169,201,255))
  stroke(d,[(9,8),(22,22)],(245,247,239,255),1)
  stroke(d,[(21,8),(8,22)],(250,243,226,255),1)
  stroke(d,[(6,12),(12,7)],(232,178,91,255),2)
  stroke(d,[(24,12),(18,7)],(232,178,91,255),2)
 elif n==1: # Elite skull, deliberately not a pixellated four-line glyph
  ellipse(d,(7,6,23,21),(220,216,205,255))
  polygon(d,[(10,19),(20,19),(19,25),(11,25)],(220,216,205,255))
  polygon(d,[(8,10),(5,5),(11,8)],(188,82,80,255))
  polygon(d,[(22,10),(25,5),(19,8)],(188,82,80,255))
  ellipse(d,(9,12,13,16),(26,33,46,255))
  ellipse(d,(17,12,21,16),(26,33,46,255))
  polygon(d,[(15,16),(13,19),(17,19)],(26,33,46,255))
  for x in (12,15,18):stroke(d,[(x,21),(x,24)],(36,38,41,255),1)
 elif n==2: # unmistakable question mark for a mystery event
  font=ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",P(23))
  d.text((P(15),P(14)),"?",font=font,anchor="mm",
         fill=(150,231,243,255),stroke_width=P(.3),
         stroke_fill=(24,56,76,255))
 elif n==3:
  polygon(d,[(11,6),(19,6),(17,11),(21,13),(23,22),(19,25),(10,25),(7,22),(9,14),(13,11)],(215,169,70,255))
  stroke(d,[(12,11),(18,11)],(80,64,45,255),2)
  ellipse(d,(12,14,18,22),(241,214,120,255),(97,83,48,255))
 elif n==4:
  stroke(d,[(7,24),(23,18)],(147,100,68,255),3)
  stroke(d,[(7,18),(23,24)],(183,122,68,255),3)
  polygon(d,[(14,6),(10,14),(11,18),(7,16),(9,23),(15,25),(22,20),(22,14),(18,17)],(235,99,48,255))
  polygon(d,[(15,11),(13,17),(12,22),(17,23),(20,18)],(255,214,99,255))
 elif n==5:
  polygon(d,[(7,14),(23,14),(23,24),(7,24)],(161,96,50,255))
  polygon(d,[(7,14),(10,9),(21,9),(23,14)],(234,186,92,255))
  stroke(d,[(7,14),(23,14)],(235,214,136,255),2)
  stroke(d,[(14,14),(14,24)],(251,216,106,255),2)
  ellipse(d,(13,17,17,21),(45,45,50,255))
 else:
  polygon(d,[(6,11),(10,16),(15,6),(20,16),(24,11),(21,24),(9,24)],(198,65,77,255))
  stroke(d,[(9,23),(21,23)],(244,181,94,255),2)
  for x,y in ((6,11),(15,6),(24,11)):ellipse(d,(x-1.5,y-1.5,x+1.5,y+1.5),(244,197,100,255))
 return im.resize((30,30),Image.Resampling.LANCZOS)
def rgb565(im):
 result=[]
 for r,g,b,a in im.convert("RGBA").getdata():
  if a<40:result.append(0xf81f);continue
  r=(r*a+16*(255-a))//255
  g=(g*a+26*(255-a))//255
  b=(b*a+39*(255-a))//255
  result.append((r>>3)<<11 | (g>>2)<<5 | (b>>3))
 return result
def declaration(name,v):
 chunks=[",".join("0x%04x"%x for x in v[i:i+16]) for i in range(0,len(v),16)]
 return "static const uint16_t "+name+"["+str(len(v))+"]={\n"+",\n".join(chunks)+"\n};\n"
heroval=rgb565(hero)
hero_body="const uint16_t rift_hero[25654] = {\n"+",".join("0x%04x"%x for x in heroval)+"\n};"
source=source[:match.start()]+hero_body+source[match.end():]
(root/"motion_art.c").write_text(source)
h=(root/"motion_art.h").read_text()
h=h.replace("#define RIFT_HERO_W 164","#define RIFT_HERO_W 202")
h=h.replace("extern const uint16_t rift_hero[20828]","extern const uint16_t rift_hero[25654]")
(root/"motion_art.h").write_text(h)
out="#ifndef RIFT110_MAP_ICONS_H\n#define RIFT110_MAP_ICONS_H\n#include <stdint.h>\n"
for i in range(7):out+=declaration("rift110_map_icon_"+str(i),rgb565(icon(i)))
out+="static const uint16_t *const rift110_map_icons[7]={"+",".join("rift110_map_icon_"+str(i) for i in range(7))+"};\n#endif\n"
(root/"rift110_map_icons.h").write_text(out)
print("PASS 1.1.0: replaced disconnected old weapon with painted claymore; built seven smooth pictograms")
