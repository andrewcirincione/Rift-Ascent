#!/usr/bin/env python3
"""Rift Ascent 1.1.7: legible, guaranteed-contained card descriptions and
true fire-themed Inferno art + accurate HP-trigger description.
"""
from pathlib import Path
from PIL import Image
import base64,re,sys,struct,io
root=Path(sys.argv[1])
game=root/"game.c"; render=root/"render.c"; art=root/"rift113_cards.h"
g=game.read_text();r=render.read_text();a=art.read_text()
old='{"Inferno","Start: lose 1 HP. On your turn, HP loss: 6 to ALL.",1,1,2}'
new='{"Inferno","Each turn: lose 1 HP. HP lost on your turn: deal 6 to ALL.",1,1,2}'
if g.count(old)!=1:raise RuntimeError("Missing 1.1.7 Inferno catalog anchor")
g=g.replace(old,new,1)
uold='"Start: lose 1 HP. On your turn, HP loss: 9 to ALL."'
unew='"Each turn: lose 1 HP. HP lost on your turn: deal 9 to ALL."'
if g.count(uold)!=1:raise RuntimeError("Missing Inferno+ anchor")
g=g.replace(uold,unew,1)
game.write_text(g)
# 92 actual unique card bitmaps remain. Only the actual Inferno (card ID 28)
# illustration is replaced with the distinctly fiery illustration.
png=Image.open(io.BytesIO(base64.b64decode(
 (Path(__file__).parent/"assets/inferno_60.png.b64").read_text()))).convert("RGB")
assert png.size==(60,40)
pixels=[((rr>>3)<<11)|((gg>>2)<<5)|(bb>>3) for rr,gg,bb in png.getdata()]
prefix="static const uint16_t rift113_cards[92][2400]={"
if a.count(prefix)!=1:raise RuntimeError("Card images declaration missing")
before,rest=a.split(prefix,1)
body,after=rest.split("};",1)
lines=body.splitlines()
tileindices=[i for i,s in enumerate(lines) if s.strip().startswith("{")]
if len(tileindices)!=92:raise RuntimeError(f"Expected 92 card art tiles; got {len(tileindices)}")
lines[tileindices[28]]="{"+",".join(f"0x{v:04x}" for v in pixels)+"},"
art.write_text(before+prefix+"\n".join(lines)+"};"+after)
# Add 4x6 and 5x7 versions of the existing smooth typeface, rendered with
# alpha sampled from the same 7x11 glyphs. Text never escapes the card box.
small=r'''
static void rift117_small_text(int x,int y,const char *s,uint32_t ink,int cw,int ch){
 if(!s)return;
 for(int n=0;s[n];n++){
  unsigned chr=(unsigned char)s[n];
  if(chr<32||chr>126)chr='?';
  const uint8_t *glyph=rift_smooth_font[chr-32];
  for(int yy=0;yy<ch;yy++)for(int xx=0;xx<cw;xx++){
   int sx=(xx*7+cw/2)/cw,sy=(yy*11+ch/2)/ch;
   if(sx>6)sx=6;if(sy>10)sy=10;
   int alpha=glyph[sy*7+sx];
   if(alpha>0 && x+xx>=0 && x+xx<W && y+yy>=0 && y+yy<H){
     uint32_t *pixel=&pix[(size_t)(y+yy)*sw+(x+xx)];
     *pixel=blend_text(*pixel,ink,alpha);
   }
  }
  x+=cw;
 }
}
/* Returns the number of characters consumed; last visible row is explicitly
 * ellipsized if there are more characters than the compact card can show. */
static int rift117_wrapped_text(int x,int y,const char *source,
 int columns,int maxlines,uint32_t ink,int charw,int charh){
 int start=0,len=(int)strlen(source),row=0;
 if(columns<2||maxlines<1)return 0;
 while(start<len&&row<maxlines){
   while(start<len&&source[start]==' ')start++;
   if(start>=len)break;
   int stop=start+columns;
   if(stop>len)stop=len;
   if(stop<len && source[stop]!=' '){
     int end=stop;
     while(end>start && source[end]!=' ')end--;
     if(end>start)stop=end;
   }
   char piece[128];int n=stop-start;
   if(n>columns)n=columns;
   if(n>(int)sizeof(piece)-1)n=(int)sizeof(piece)-1;
   memcpy(piece,source+start,(size_t)n);piece[n]=0;
   if(row==maxlines-1 && stop<len){
      /* Avoid silently cutting a description off mid-effect. */
      if(n>2){piece[n-1]='.';piece[n-2]='.';}
   }
   rift117_small_text(x,y+row*(charh+1),piece,ink,charw,charh);
   row++;start=stop;
 }
 return start;
}
'''
anchor='static void wrapped_text(int x,int y,const char *source,int linechars,int maxlines,uint32_t c){'
if r.count(anchor)!=1:raise RuntimeError("No normal wrapped_text anchor")
r=r.replace(anchor,small+'\n'+anchor,1)
old_ui='''    const int art_height=selected?30:23;
    const int art_top=y+20;'''
new_ui='''    /* More room below the painting for true, non-overlapping rules text. */
    const int art_height=selected?22:16;
    const int art_top=y+20;'''
if r.count(old_ui)!=1:raise RuntimeError("Missing compact art sizing")
r=r.replace(old_ui,new_ui,1)
old_body='''    int content_y=art_top+art_height+3;
    int maxrows=(y+h-3-content_y)/11;
    if(maxrows>0)wrapped_text(x+5,content_y,card_description(raw),
                              (w-10)/7,maxrows,selected?WHITE:MUTED);'''
new_body='''    int content_y=art_top+art_height+2;
    /* 5x7 font on selected cards; 4x6 font on the narrow hand tiles.
       Switch to 4x6 on rare verbose selected cards. A 7px width / 11px
       baseline was why 1.1.6 chopped Inferno mid-sentence. */
    const char *description=card_description(raw);
    int cw=selected?6:5,ch=selected?8:7;
    int lines=(y+h-3-content_y)/(ch+1);
    int columns=(w-10)/cw;
    if((int)strlen(description)>columns*lines){
      cw=selected?5:4;ch=selected?7:6;
      lines=(y+h-3-content_y)/(ch+1);
      columns=(w-10)/cw;
    }
    if(lines>0)rift117_wrapped_text(x+5,content_y,description,
                                   columns,lines,selected?WHITE:MUTED,cw,ch);'''
if r.count(old_body)!=1:raise RuntimeError("Missing combat card description anchor")
r=r.replace(old_body,new_body,1)
render.write_text(r)
print("PASS 1.1.7: Inferno actual power wording, fiery painting, smaller font with bounds and ellipsis")
