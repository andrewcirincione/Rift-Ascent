#!/usr/bin/env python3
"""1.0.2 item 03: fixed, separated sword glyph and enemy attack value."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 assert a in s,a[:110];s=s.replace(a,b,1)
sub("dark_rect(ex-46,45,92,34,199);","""dark_rect(ex-49,44,98,37,215);
    if(hits){
      /* Distinct left-side weapon glyph, outside the text bounds. */
      line(ex-42,56,ex-31,66,WHITE);
      line(ex-43,55,ex-38,51,GOLD);
      line(ex-45,63,ex-37,71,RED);
    }""")
sub("center(ex,47,b,hits?RED:GOLD,1);","center(ex+12,47,b,hits?RED:GOLD,1);")
p.write_text(s)
print("PASS 03: sword has its own glyph area, damage text starts to its right.")
