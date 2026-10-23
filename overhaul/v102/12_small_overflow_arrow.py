#!/usr/bin/env python3
"""1.0.2 item 12: subtle, compact arrows for offscreen hand cards."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
for a,b in [
 ("gilded_panel(383,166,25,74,0);","gilded_panel(392,200,16,27,0);"),
 ('text(386,194,">",GOLD,2);','text(396,207,">",GOLD,1);'),
 ('if(start>0)text(42,189,"<",GOLD,2);','if(start>0)text(42,209,"<",GOLD,1);')
]:
 assert a in s,a;s=s.replace(a,b,1)
p.write_text(s)
print("PASS 12: smaller arrow box and font.")
