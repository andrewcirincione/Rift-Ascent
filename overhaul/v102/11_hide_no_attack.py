#!/usr/bin/env python3
"""1.0.2 item 11: remove misleading NO ATTACK label for nonattacking foes."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='''    else snprintf(b,sizeof(b),"NO ATTACK");
    center(ex+12,47,b,hits?RED:GOLD,1);'''
b='''    if(hits)center(ex+12,47,b,RED,1);'''
assert a in s
p.write_text(s.replace(a,b,1))
print("PASS 11: only positive enemy attacks display sword + damage.")
