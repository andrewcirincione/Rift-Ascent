#!/usr/bin/env python3
"""Suppress obsolete intent lettering baked into the 1.1.x scenic background.

Reconstruct a small sky strip from neighboring background colors, softly
feathered to preserve the art. Current intent icons are still drawn above this.
No panel, box, text or music is generated.
"""
from pathlib import Path
import sys
path=Path(sys.argv[1]); source=path.read_text()
needle=""" art_blit(0,0,W,H,src,W,H);
}
static unsigned particle_hash"""
replacement=""" art_blit(0,0,W,H,src,W,H);
 if(g->screen==COMBAT){
   /* Earlier art had old red "STRIKE 13" lettering at x=330..384,
      y=33..42. It must not appear behind the new live intent icons.
      Reconstruct the sky from rows above and below, with soft edges. */
   for(int y=27;y<55;y++){
     int t=(y-27)*255/28;
     for(int x=316;x<405;x++){
       int margin=x-316;
       if(404-x<margin)margin=404-x;
       if(y-27<margin)margin=y-27;
       if(54-y<margin)margin=54-y;
       int alpha=CLAMP(margin*52,0,255);
       if(!alpha)continue;
       uint32_t c0=pix[(size_t)23*sw+x];
       uint32_t c1=pix[(size_t)59*sw+x];
       uint32_t orig=pix[(size_t)y*sw+x];
       int r=((int)(c0&255)*(255-t)+(int)(c1&255)*t)/255;
       int gr=((int)((c0>>8)&255)*(255-t)+(int)((c1>>8)&255)*t)/255;
       int b=((int)((c0>>16)&255)*(255-t)+(int)((c1>>16)&255)*t)/255;
       r=((int)(orig&255)*(255-alpha)+r*alpha)/255;
       gr=((int)((orig>>8)&255)*(255-alpha)+gr*alpha)/255;
       b=((int)((orig>>16)&255)*(255-alpha)+b*alpha)/255;
       pix[(size_t)y*sw+x]=RGB(r,gr,b);
     }
   }
 }
}
static unsigned particle_hash"""
if source.count(needle)!=1:raise RuntimeError("Cannot locate scenic background blit")
source=source.replace(needle,replacement,1)
path.write_text(source)
print("PASS: removed legacy baked enemy intent lettering from combat scenery")
