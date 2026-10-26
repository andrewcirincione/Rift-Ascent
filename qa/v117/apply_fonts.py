#!/usr/bin/env python3
"""1.1.7 QA typography correction: pack legible pre-rasterized FreeType
5x8/6x9 antialiased glyphs. No smudgy subsampling of the default font."""
from pathlib import Path
import base64,zlib,struct,re,sys
root=Path(sys.argv[1]);p=root/"render.c";s=p.read_text()
blob=zlib.decompress(base64.b64decode((Path(__file__).parent/"assets/card_fonts.b64").read_text()))
assert blob[:8]==b"R117FONT"
nt,ns,nl=struct.unpack_from("<III",blob,8)
assert (nt,ns,nl)==(95*24,95*40,95*54)
pieces=[blob[20:20+nt],blob[20+nt:20+nt+ns],blob[20+nt+ns:]]
h=["#ifndef RIFT117_CARD_FONTS_H","#define RIFT117_CARD_FONTS_H","#include <stdint.h>"]
for key,width,ht,values in zip(("4x6","5x8","6x9"),(4,5,6),(6,8,9),pieces):
 h.append(f"static const uint8_t rift117_font_{width}x{ht}[95][{width*ht}]={{")
 for idx in range(95):
  glyph=values[idx*width*ht:(idx+1)*width*ht]
  h.append("{"+",".join(str(v) for v in glyph)+"},")
 h.append("};")
h.append("#endif")
(root/"rift117_card_fonts.h").write_text("\n".join(h)+"\n")
inc='#include "rift113_cards.h"'
if s.count(inc)!=1:raise RuntimeError("Missing 1.1.3 art include")
s=s.replace(inc,inc+'\n#include "rift117_card_fonts.h"',1)
beg=s.index("static void rift117_small_text(")
end=s.index("/* Returns the number of characters consumed;",beg)
new=r'''static void rift117_small_text(int x,int y,const char *s,uint32_t ink,int cw,int ch){
 if(!s)return;
 for(int n=0;s[n];n++){
  unsigned chr=(unsigned char)s[n];
  if(chr<32||chr>126)chr='?';
  const uint8_t *glyph=(cw==6?rift117_font_6x9[chr-32]:
                       cw==5?rift117_font_5x8[chr-32]:
                             rift117_font_4x6[chr-32]);
  for(int yy=0;yy<ch;yy++)for(int xx=0;xx<cw;xx++){
    int alpha=glyph[yy*cw+xx];
    if(alpha>20&&(unsigned)(x+xx)<W&&(unsigned)(y+yy)<H){
      uint32_t *dst=&pix[(size_t)(y+yy)*sw+(x+xx)];
      *dst=blend_text(*dst,ink,alpha);
    }
  }
  x+=cw;
 }
}
'''
s=s[:beg]+new+s[end:]
s=s.replace("int cw=selected?6:5,ch=selected?8:7;",
            "int cw=selected?6:5,ch=selected?9:8;",1)
s=s.replace("cw=selected?5:4;ch=selected?7:6;",
            "cw=selected?5:4;ch=selected?8:6;",1)
p.write_text(s)
print("PASS 1.1.7 readable actual pre-rendered 6x9 glyphs with smaller fallback")
