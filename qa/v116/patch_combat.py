#!/usr/bin/env python3
"""Rift Ascent 1.1.6: balanced combat silhouettes and illustrated compact intent HUD.
This patch runs after v1.1.5 card-art patch. No existing art/music is replaced.
"""
from pathlib import Path
from PIL import Image,ImageDraw
import sys,re
root=Path(sys.argv[1]); r=root/"render.c"; s=r.read_text()
def once(old,new,label):
 global s
 n=s.count(old)
 if n!=1:raise RuntimeError(f"v116: {label} expected once, got {n}")
 s=s.replace(old,new,1)

# Generate distinct anti-aliased, transparent art for the four enemy intent classes.
scale=5
def p(v):return int(round(v*scale))
def polygon(d,coords,fill,outline=None,width=1):
 pts=[(p(x),p(y)) for x,y in coords]
 d.polygon(pts,fill=fill)
 if outline:d.line(pts+[pts[0]],fill=outline,width=p(width),joint="curve")
def line(d,coords,fill,width):
 d.line([(p(x),p(y)) for x,y in coords],fill=fill,width=p(width),joint="curve")
def oval(d,b,fill,outline=None,width=1):
 d.ellipse(tuple(p(x) for x in b),fill=fill,outline=outline,width=p(width))
def create_icon(kind):
 img=Image.new("RGBA",(24*scale,24*scale),(0,0,0,0));d=ImageDraw.Draw(img)
 if kind=="swords":
  # Two steel blades, warm bronze hilts. Distinct silhouettes, not red slash lines.
  for mirrored in (False,True):
   def q(x,y):return (24-x,y) if mirrored else (x,y)
   polygon(d,[q(4.0,3),q(7.5,3.5),q(19.5,17.0),q(17.0,19.0),q(4.0,6.7)],
           (30,44,61,255))
   polygon(d,[q(4.7,3.6),q(7.5,4.5),q(18.8,17.2),q(17.3,18.3),q(5.0,6.3)],
           (212,222,224,255))
   line(d,[q(5.5,5),q(18,18)],(252,251,231,255),0.65)
   line(d,[q(15.1,17.3),q(20.4,21.4)],(89,47,32,255),2.5)
   line(d,[q(13.6,18.0),q(18.1,14.1)],(235,181,93,255),2)
   oval(d,(19.1,20.0,22.0,23.0),(227,177,85,255))
 elif kind=="shield":
  polygon(d,[(12,1.3),(20.2,4.2),(19.1,13.3),(16.5,18.3),(12,22.7),
             (7.5,18.3),(4.9,13.3),(3.8,4.2)],
          (18,46,66,255),(230,187,102,255),1.7)
  polygon(d,[(12,4),(17.6,6),(17,13),(14.9,17),(12,19.5),(9,17),
             (7,13),(6.4,6)],(65,139,194,255),(158,216,240,255),.65)
  line(d,[(12,5),(12,19)],(209,232,237,255),1.1)
  polygon(d,[(9,9),(12,6.8),(15,9),(12,11.5)],(229,227,193,255))
 elif kind=="up":
  polygon(d,[(12,1.5),(22.3,12),(16.5,12),(16.5,22),(7.5,22),(7.5,12),(1.7,12)],
          (24,63,106,255),(154,217,245,255),1.3)
  polygon(d,[(12,4),(19,11),(14.2,11),(14.2,19),(9.8,19),(9.8,11),(5,11)],
          (71,174,241,255))
  line(d,[(12,5),(12,16.5)],(226,243,254,255),1.4)
 elif kind=="down":
  polygon(d,[(7.5,2),(16.5,2),(16.5,12),(22.3,12),(12,22.5),(1.7,12),(7.5,12)],
          (105,28,42,255),(247,156,149,255),1.3)
  polygon(d,[(9.8,5),(14.2,5),(14.2,13),(19,13),(12,20),(5,13),(9.8,13)],
          (222,61,73,255))
  line(d,[(12,6),(12,17.4)],(255,220,205,255),1.3)
 else:raise ValueError(kind)
 return img.resize((24,24),Image.Resampling.LANCZOS)

def colors(im):
 values=[]
 for rr,gg,bb,aa in im.getdata():
  if aa<100:values.append(0xf81f);continue
  # Composite fringes against dark surrounding night sky.
  a=aa/255
  rr=round(rr*a+15*(1-a));gg=round(gg*a+22*(1-a));bb=round(bb*a+32*(1-a))
  value=((rr>>3)<<11)|((gg>>2)<<5)|(bb>>3)
  values.append(0xf81e if value==0xf81f else value)
 return values

defs=["#ifndef RIFT116_INTENT_ICONS_H","#define RIFT116_INTENT_ICONS_H","#include <stdint.h>"]
for name in ("swords","shield","up","down"):
 vals=colors(create_icon(name))
 defs.append("static const uint16_t rift116_icon_"+name+"[576]={")
 for i in range(0,len(vals),24):
  defs.append(",".join(f"0x{v:04x}" for v in vals[i:i+24])+(", " if i+24<len(vals) else ""))
 defs.append("};")
defs.append("#endif")
(root/"rift116_intent_icons.h").write_text("\n".join(defs)+"\n")
once('#include "rift113_cards.h"',
     '#include "rift113_cards.h"\n#include "rift116_intent_icons.h"',"include icons")
once('sprite(RIFT_HERO_X+recoil,RIFT_HERO_Y+hero_bob,RIFT_HERO_W,RIFT_HERO_H,rift_hero);',
     'sprite_scaled(RIFT_HERO_X+24+recoil,RIFT_HERO_Y+28+hero_bob,160,99,rift_hero,RIFT_HERO_W,RIFT_HERO_H);',
     "smaller hero")
once('int dw=(g->enemy_count==1?106:82)+wave(animation_tick+i*9u,140,1);',
     'int dw=(g->enemy_count==1?128:96)+wave(animation_tick+i*9u,140,1);',
     "larger enemy width")
once('int dh=(g->enemy_count==1?111:96)+wave(animation_tick+i*13u,130,1);',
     'int dh=(g->enemy_count==1?124:108)+wave(animation_tick+i*13u,130,1);',
     "larger enemy height")
once('dark_rect(270,29,145,56,245);',
     '/* No giant backdrop above enemies: distinct icon silhouettes render on scenery. */',
     "remove large dark patch")
# Keep enemy name in inspect/inventory, not in combat's icon rail.
start=s.index('  dark_rect(ex-48,30,96,17,184);',s.index("static void combat("))
end=s.index('  dark_rect(ex-42,142,84,28,144);',start)
s=s[:start]+'  rift116_draw_enemy_intent(g,i,ex);\n'+s[end:]
# Render intent icons strictly from live engine, not static text.
func=r'''
/* Compact, shadowed, ICON-ONLY enemy telegraphs.
 * The enemy's full intention remains accessible through Start / Info.
 * Attack damage uses game's actual calculated post-modifier value.
 */
static void rift116_draw_enemy_intent(const Game*g,int index,int cx){
 const Enemy *e=&g->enemies[index];
 const uint16_t *glyph[3];
 char value[3][16];
 int count=0;
 int hits=game_enemy_intent_hits(g,index);
 int damage=game_enemy_intent_damage(g,index);
 if(hits>0){
  glyph[count]=rift116_icon_swords;
  if(hits>1)snprintf(value[count],sizeof(value[count]),"%dx%d",hits,damage/hits);
  else snprintf(value[count],sizeof(value[count]),"%d",damage);
  count++;
 }
 int block=-1,buff=-1,debuff=-1;
 switch(e->intent){
  case 1:block=e->intent_value;break;
  case 2:case 3:debuff=2;break; /* Weak or Vulnerable */
  case 4:buff=2;block=8;break;
  case 5:case 8:case 10:debuff=1;break; /* Burn, energy drain, Daze */
  case 6:debuff=2;break; /* poison */
  case 9:buff=-2;block=e->intent_value/2;break; /* Heal and shield */
  case 11:debuff=1;block=e->intent_value/2;break; /* Dread plus shield */
  case 12:buff=-2;break; /* leech heals enemy */
  case 13:buff=1;block=e->intent_value;break;
 }
 if(block>=0&&count<3){
  glyph[count]=rift116_icon_shield;
  snprintf(value[count],sizeof(value[count]),"%d",block);
  count++;
 }
 if(buff!=-1&&count<3){
  glyph[count]=rift116_icon_up;
  if(buff==-2)value[count][0]=0; /* Heal: numeric amount varies */
  else snprintf(value[count],sizeof(value[count]),"%d",buff);
  count++;
 }
 if(debuff>=0&&count<3){
  glyph[count]=rift116_icon_down;
  if(e->intent==2||e->intent==3||e->intent==6)
   snprintf(value[count],sizeof(value[count]),"%d",debuff);
  else value[count][0]=0; /* curse card or energy penalty */
  count++;
 }
 if(!count)return;
 int widths[3],total=0;
 for(int j=0;j<count;j++){
   widths[j]=24+(int)strlen(value[j])*7+3;
   total+=widths[j];
 }
 int x=cx-total/2;
 for(int j=0;j<count;j++){
   /* Transparent illustrated icons; no black rectangles or text panels. */
   sprite_scaled(x,36,23,23,glyph[j],24,24);
   if(value[j][0]){
     text(x+24,45,value[j],RGB(16,21,31),1);
     text(x+23,44,value[j],j==0&&hits?GOLD:WHITE,1);
   }
   x+=widths[j];
 }
}
'''
s=s.replace('static void combat(const Game*g){',func+'\nstatic void combat(const Game*g){',1)
assert 'dark_rect(ex-46,67,92,17,231)' not in s
assert 'dark_rect(ex-43,47,86,19,199)' not in s
r.write_text(s)
print("PASS v1.1.6: art-backed sword/shield/buff/debuff telegraphs; no black intent boxes; smaller hero and larger enemies")
