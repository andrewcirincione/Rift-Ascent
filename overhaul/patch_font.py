#!/usr/bin/env python3
"""Swap legacy hard bitmap glyph rendering for 8-bit antialiased font atlas."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c"
s=p.read_text()
assert '#include "font.h"' in s
s=s.replace('#include "font.h"','#include "font_smooth.h"',1)
start=s.index("static int glyph_on(")
end=s.index("\nstatic void center(",start)
s=s[:start]+"""static void text(int x,int y,const char *s,uint32_t ink,int size) {
 if(!s)return;
 for(int n=0;s[n]&&x<W;n++) {
    unsigned chr=(unsigned char)s[n];
    if(chr<32||chr>126)chr='?';
    const uint8_t *glyph=rift_smooth_font[chr-32];
    for(int yy=0;yy<11;yy++)
     for(int xx=0;xx<7;xx++){
       unsigned alpha=glyph[yy*7+xx];
       if(!alpha)continue;
       int px=x+xx*size,py=y+yy*size;
       for(int sy=0;sy<size;sy++)
        for(int sx=0;sx<size;sx++){
          int tx=px+sx,ty=py+sy;
          if((unsigned)tx<W&&(unsigned)ty<H) {
             uint32_t *dst=&pix[(size_t)ty*sw+tx];
             *dst=blend_text(*dst,ink,(int)alpha);
          }
        }
     }
    x+=7*size;
 }
}"""+s[end:]
p.write_text(s)
print("Swapped all HUD characters to antialiased alpha-blended atlas.")
