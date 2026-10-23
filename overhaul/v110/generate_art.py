#!/usr/bin/env python3
"""Build genuine (non-line-overlay) claymore hero and antialiased room icon art.

Uses original motion_art.c sprite; removes its disconnected old weapon,
composites a modeled longsword with guard, fuller, hilt and pommel behind glove.
Outputs C source in GAME/src; user art, no binary external asset dependencies.
"""
from pathlib import Path
import re,sys
from PIL import Image,ImageDraw
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
hero=Image.new("RGBA",(210*S,127*S));d=ImageDraw.Draw(hero)
# Real steel blade: broad base, central fuller, tapered distal blade and tip.
polygon(d,[(124,74),(134,64),(194,11),(205,6),(201,20),(146,72),(134,82)],(24,33,44,255))
polygon(d,[(133,66),(197,11),(201,13),(195,24),(144,74)],(111,126,146,255))
polygon(d,[(139,65),(199,10),(194,20),(146,72)],(199,207,208,255))
polygon(d,[(142,65),(198,11),(188,25),(146,69)],(235,230,210,255))
polygon(d,[(149,65),(194,18),(188,24),(150,67)],(93,116,135,255))
stroke(d,[(136,68),(197,13)],(246,242,226,255),1.4)
stroke(d,[(153,62),(188,27)],(58,81,104,255),1.4)
# Two quillons, wrapped grip, and counterweighted pommel.
stroke(d,[(132,69),(108,92)],(29,27,31,255),8)
stroke(d,[(132,69),(108,92)],(93,61,40,255),5)
for a in range(0,21,4):
 x,y=113+a*.85,86-a*.85
 stroke(d,[(x-1,y-3),(x+3,y+1)],(219,160,74,255),1.5)
polygon(d,[(122,60),(127,56),(145,77),(140,82)],(31,33,37,255))
polygon(d,[(120,58),(125,56),(143,77),(140,79)],(203,157,77,255))
polygon(d,[(119,58),(123,56),(127,64),(125,65)],(237,211,124,255))
polygon(d,[(138,77),(141,79),(147,81),(145,74)],(237,202,114,255))
ellipse(d,(129,65,139,75),(217,170,81,255),(59,54,53,255))
ellipse(d,(101,89,110,98),(31,29,37,255))
ellipse(d,(103,91,108,95),(205,165,88,255))
hero.alpha_composite(old.resize((164*S,127*S),Image.Resampling.NEAREST))
d=ImageDraw.Draw(hero)
polygon(d,[(112,75),(119,71),(125,78),(120,83),(113,82)],(37,40,46,255))
polygon(d,[(113,75),(119,72),(122,78),(115,79)],(120,117,106,255))
hero=hero.resize((210,127),Image.Resampling.LANCZOS)
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
 elif n==2:
  ellipse(d,(9,7,21,19),None,(136,225,239,255))
  stroke(d,[(17,17),(17,20),(13,20)],(136,225,239,255),3)
  ellipse(d,(12,23,18,27),(136,225,239,255))
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
hero_body="const uint16_t rift_hero[26670] = {\n"+",".join("0x%04x"%x for x in heroval)+"\n};"
source=source[:match.start()]+hero_body+source[match.end():]
(root/"motion_art.c").write_text(source)
h=(root/"motion_art.h").read_text()
h=h.replace("#define RIFT_HERO_W 164","#define RIFT_HERO_W 210")
h=h.replace("extern const uint16_t rift_hero[20828]","extern const uint16_t rift_hero[26670]")
(root/"motion_art.h").write_text(h)
out="#ifndef RIFT110_MAP_ICONS_H\n#define RIFT110_MAP_ICONS_H\n#include <stdint.h>\n"
for i in range(7):out+=declaration("rift110_map_icon_"+str(i),rgb565(icon(i)))
out+="static const uint16_t *const rift110_map_icons[7]={"+",".join("rift110_map_icon_"+str(i) for i in range(7))+"};\n#endif\n"
(root/"rift110_map_icons.h").write_text(out)
print("PASS 1.1.0: replaced disconnected old weapon with painted claymore; built seven smooth pictograms")
