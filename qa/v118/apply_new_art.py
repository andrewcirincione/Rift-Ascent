#!/usr/bin/env python3
"""v1.1.8: FULL artwork replacement from three approved NEW images, never
composite new art with previous pixels. Rounded outer corners on every card.
Called after v1.1.7 art, font and intent patches and before PSP compilation.
"""
from pathlib import Path
from PIL import Image
import base64,io,re,struct,sys
root=Path(sys.argv[1])
assetdir=Path(__file__).parent/"assets"
def load(n):
 im=Image.open(io.BytesIO(base64.b64decode((assetdir/n).read_text())))
 return im
hero=load("hero.png.b64").convert("RGBA")
menu=load("menu.jpg.b64").convert("RGB")
assert hero.size==(210,127) and menu.size==(480,272)
def rgb565(im,key=False):
 out=[]
 for px in im.getdata():
  rr,gg,bb=px[:3]
  if key and px[3]<120:v=0xF81F
  else:
   v=((rr>>3)<<11)|((gg>>2)<<5)|(bb>>3)
   if key and v==0xF81F:v=0xF81E
  out.append(v)
 return out
def replace_texture(label,vals,filenames):
 pat=r'((?:static\s+)?const\s+uint16_t\s+'+re.escape(label)+r'\s*\[\s*\d+\s*\][^=;]{0,200}=\s*\{).*?(\};)'
 new="\n"+",\n".join(",".join(f"0x{v:04x}" for v in vals[i:i+40]) for i in range(0,len(vals),40))+"\n"
 for filename in filenames:
  path=root/filename
  if not path.exists():continue
  s=path.read_text()
  r,n=re.subn(pat,lambda m:m.group(1)+new+m.group(2),s,count=1,flags=re.S)
  if n==1:
   path.write_text(r)
   print("REPLACED entire source texture",label,"in",filename,"pixels",len(vals))
   return
 raise RuntimeError("Failed finding entire texture "+label)
replace_texture("rift_hero",rgb565(hero,key=True),("motion_art.c","artwork.c"))
replace_texture("ra_title_bg",rgb565(menu),("artwork.c","motion_art.c"))

path=root/"render.c"; s=path.read_text()
# Full NEW title illustration is composed for text on the right, with no
# old splash/title fragments. Remove the previous blank black label rectangle.
old=""" rect(176,131,135,19,RGB(11,19,31));
 button(162,149,161,32,"BEGIN ASCENT",g->selection==0);
 button(162,185,161,30,"HOW TO PLAY",g->selection==1);"""
new=""" /* The completely new knight scene leaves quiet sky at right.
    Keep all UI off the player and his oversized sword. */
 text(304,73,"RIFT ASCENT",RGB(34,20,24),2);
 text(302,71,"RIFT ASCENT",GOLD,2);
 button(300,149,164,32,"BEGIN ASCENT",g->selection==0);
 button(300,190,164,30,"HOW TO PLAY",g->selection==1);"""
if s.count(old)!=1:raise RuntimeError("Title menu anchor not found")
s=s.replace(old,new)
s=s.replace('center(244,230,b,GOLD,1);','center(378,229,b,GOLD,1);',1)
s=s.replace('center(244,230,"ONE WARRIOR / THREE ACTS",GOLD,1);',
            'center(380,229,"ONE WARRIOR / THREE ACTS",GOLD,1);',1)

# Round only cards. Other panels remain square to retain their functional UI.
# Four-pixel radius, polished golden/steel edges with the current selected state.
hook="static void card_render(int x,int y,int w,int h,int raw,int selected,int active){"
if s.count(hook)!=1:raise RuntimeError("Card renderer anchor not found")
rounded=r'''
static int rift118_corner_inset(int row,int h,int radius){
 int dist=row<h-1-row?row:h-1-row;
 if(dist>=radius)return 0;
 if(dist==0)return radius-1;
 if(dist==1)return radius-2;
 return 1;
}
static void rift118_round_outline(int x,int y,int w,int h,uint32_t ink,int radius){
 if(w<radius*3||h<radius*3)return;
 for(int j=0;j<h;j++){
   int ins=rift118_corner_inset(j,h,radius);
   if(j==0||j==h-1)rect(x+ins,y+j,w-ins*2,1,ink);
   else {
     put(x+ins,y+j,ink);
     put(x+w-ins-1,y+j,ink);
   }
 }
}
static void rift118_card_panel(int x,int y,int w,int h,int selected){
 const int radius=4;
 uint32_t face=RGB(15,25,38);
 uint32_t edge=selected?GOLD:RGB(121,97,69);
 /* Each row begins/ends inside its rounded contour: no square fragments. */
 for(int j=0;j<h;j++){
   int ins=rift118_corner_inset(j,h,radius);
   rect(x+ins,y+j,w-2*ins,1,face);
 }
 rift118_round_outline(x,y,w,h,edge,radius);
 rect(x+4,y+3,w-8,1,selected?GOLD:RGB(59,77,91));
 rect(x+4,y+h-4,w-8,1,RGB(44,54,64));
}
'''
s=s.replace(hook,rounded+"\n"+hook)
start=s.index(hook);end=s.index("static void rift_status_chip(",start)
card=s[start:end]
if card.count("gilded_panel(x,y,w,h,selected);")!=2:
 raise RuntimeError("Expected exactly two card panel branches")
card=card.replace("gilded_panel(x,y,w,h,selected);",
                  "rift118_card_panel(x,y,w,h,selected);")
card=card.replace("border(x-1,y-1,w+2,h+2,GOLD);",
                  "rift118_round_outline(x-1,y-1,w+2,h+2,GOLD,5);")
# also remove isolated square corner pixels for selected card edge
card=card.replace("put(x-2,y-2,GOLD);put(x+w+1,y+h+1,GOLD);","")
s=s[:start]+card+s[end:]
path.write_text(s)
print("PASS v1.1.8: FULL new assets, no pasted old sprite, menu at right, cards rounded r=4")
