#!/usr/bin/env python3
"""1.0.3 animated oversized claymore layered on the existing hero artwork."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a=''' sprite(RIFT_HERO_X+recoil,RIFT_HERO_Y+hero_bob,RIFT_HERO_W,RIFT_HERO_H,rift_hero);'''
b=a+'''
 /* A distinctive two-handed claymore instead of the short axe/hammer profile.
    Painted over the right hand, with a double-edged broad steel blade. */
 {
   int by=hero_bob;
   uint32_t steel=RGB(191,198,206),edge=RGB(226,236,240);
   int x=148+recoil,y=140+by;
   /* The hilt, wrapped grip and broad angled blade face the monsters. */
   line(x-8,y+13,x+6,y-4,RGB(84,62,43));
   line(x-7,y+12,x+7,y-5,RGB(179,131,67));
   line(x-13,y+3,x+8,y+20,GOLD);
   for(int w=-3;w<=3;w++){
     line(x+6+w,y-6,x+57+w,y-73,w==0?edge:steel);
   }
   line(x+7,y-5,x+59,y-80,edge);
   line(x+4,y-7,x+59,y-80,RGB(112,133,155));
   triangle(x+58,y-80,x+62,y-86,x+59,y-70,edge);
   circle(x-8,y+14,3,RGB(154,101,53));
 }
'''
if a not in s:raise RuntimeError("hero art anchor missing")
p.write_text(s.replace(a,b,1))
print("PASS 1.0.3 oversized two-handed steel claymore.")
