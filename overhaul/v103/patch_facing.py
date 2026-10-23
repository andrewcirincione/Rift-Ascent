#!/usr/bin/env python3
"""Flip only the eleven creatures facing away from the hero, after sprite-sheet inspection."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
anchor="static void dark_rect("
helper="""static void sprite_scaled_facing(int dx,int dy,int dw,int dh,
    const uint16_t *src,int iw,int ih,int flip){
 if(!flip){sprite_scaled(dx,dy,dw,dh,src,iw,ih);return;}
 if(dw<1||dh<1)return;
 int x0=dx<0?0:dx,y0=dy<0?0:dy,x1=dx+dw>W?W:dx+dw,y1=dy+dh>H?H:dy+dh;
 for(int y=y0;y<y1;y++){
  const uint16_t *line=src+(size_t)((y-dy)*ih/dh)*iw;
  uint32_t *dst=pix+(size_t)y*sw;
  for(int x=x0;x<x1;x++){
   int u=(dw-1-(x-dx))*iw/dw;
   if(u<0)u=0;if(u>=iw)u=iw-1;
   uint16_t c=line[u];if(c==RIFT_TRANSPARENT_565)continue;
   unsigned r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   dst[x]=RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
  }
 }
}
"""
if anchor not in s:raise RuntimeError("sprite orientation anchor missing")
s=s.replace(anchor,helper+anchor,1)
a="""   sprite_scaled(ex-dw/2,169-dh+bob,dw,dh,
     rift_enemy_sprites[e->art],RIFT_ENEMY_W,RIFT_ENEMY_H);"""
b="""   int facing_wrong=(e->art==1||e->art==4||e->art==9||e->art==10||
                     e->art==14||e->art==15||e->art==16||e->art==20||
                     e->art==26||e->art==29||e->art==32);
   sprite_scaled_facing(ex-dw/2,169-dh+bob,dw,dh,
     rift_enemy_sprites[e->art],RIFT_ENEMY_W,RIFT_ENEMY_H,facing_wrong);"""
if a not in s:raise RuntimeError("enemy sprite call missing")
s=s.replace(a,b,1)
p.write_text(s)
print("PASS 1.0.3: 11 wrong-way enemy sprites face player; frontal sprites unaffected.")
