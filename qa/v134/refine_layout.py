from pathlib import Path
p=Path('GAME/src/render.c');s=p.read_text();pos=s.index('static void card_render(')
s=s[:pos]+'''static void r134_card_frame(int x,int y,int w,int h,int type){
 /* Preserve the generated sculpture but reserve a rectangular text interior.
    Nine-slicing keeps the corner ornaments proportional on hand thumbnails. */
 int side=w<100?7:12,top=h<105?11:20,base=h<105?9:16;
 const uint32_t*src=r134_card_frames[type];
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if((unsigned)dy>=H)continue;
  int sy=yy<top?yy*43/top:yy>=h-base?134+(yy-h+base)*32/base:43+(yy-top)*91/(h-top-base);
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if((unsigned)dx>=W)continue;
   int sx=xx<side?xx*24/side:xx>=w-side?102+(xx-w+side)*24/side:24+(xx-side)*78/(w-2*side);
   uint32_t c=src[sy*126+sx];int a=c>>24;if(a){uint32_t*d=&pix[dy*sw+dx];*d=r134_mix(*d,c,a);}
  }
 }
}
'''+s[pos:]
s=s.replace('inset=small?8:14','inset=small?10:17').replace('int ty=y+(small?7:12),titlelead=small?9:11,arty=y+(small?27:37),arth=small?17:CLAMP(h/4-3,25,39);','int ty=y+(small?14:24),titlelead=small?9:11,arty=y+(small?33:49),arth=small?11:CLAMP(h/5-4,20,29);')
s=s.replace('r134_blit(x,y,w,h,r134_card_frames[type],126,166,255);','r134_card_frame(x,y,w,h,type);')
s=s.replace('int orb=small?14:18','int orb=small?12:16').replace('arty+(small?2:4)','arty+(small?1:3)')
s=s.replace('int by=arty+arth+(small?3:6),bottom=y+h-(small?8:13),lead=small?9:11;','int by=arty+arth+(small?3:5),bottom=y+h-(small?10:18),lead=small?9:11;')
s=s.replace('if(!small&&r134_lines(rules,inner,body,check)>rows){body=&r134_f8;lead=10;rows=(bottom-by)/lead;}','if(!small&&r134_lines(rules,inner,body,check)>rows){body=&r134_f8;lead=9;rows=(bottom-by)/lead;}')
# Map panel inner padding. Avoid decoration intersecting explanatory text.
s=s.replace('gilded_panel(315,41,160,209,0);','gilded_panel(315,41,160,209,0);')
for x in ['324','325']:
 s=s.replace('text('+x+',','text(335,').replace('fittext('+x+',','fittext(335,')
s=s.replace('"STANDARD ENCOUNTER"','"BATTLE"').replace('"DANGEROUS / RELIC"','"ELITE / RELIC"').replace('"UNKNOWN OUTCOME"','"MYSTERY"')
# Wider reward cards prevent full descriptions being squeezed.
s=s.replace('57+i*132,68,102,152','33+i*144,53,126,166')
# Describe card choices with full cards rather than a clipped wall of text.
a=s.index(' gilded_panel(346,67,126,172,0);');b=s.index(' footer(',a)
s=s[:a]+''' if(count){
  int c=pending==25?g->discard[g->selection]:g->hand[g->selection];
  if(pending==23&&game_card_is_upgradeable(c))c|=UPBIT;
  card_render(341,70,126,166,c,1,1);
 }
'''+s[b:]
# Use a measured, wrapping title in help and reward panels, safely inset.
s=s.replace('controller_icon(15,91,0);controller_icon(15,111,1);\n controller_icon(15,131,3);controller_icon(15,151,2);','')
s=s.replace('char value[3][16];','char value[3][32];')
p.write_text(s)
