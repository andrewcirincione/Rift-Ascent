#!/usr/bin/env python3
"""1.0.2 item 14: gray out unavailable and unplayable cards."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a=''' if(!active)rect(x+3,y+h-4,w-6,2,RED);'''
b=''' if(!active){
    dark_rect(x+2,y+2,w-4,h-4,160);
    border(x+2,y+2,w-4,h-4,RGB(101,105,108));
    if(h>=74)fittext(x+5,y+h-15,"NO ENERGY",9,RGB(202,205,210));
 }'''
assert a in s
p.write_text(s.replace(a,b,1))
print("PASS 14: unaffordable cards visibly dim and carry an availability label.")
