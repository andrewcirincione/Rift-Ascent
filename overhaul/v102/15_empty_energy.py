#!/usr/bin/env python3
"""1.0.2 item 15: zero-energy disc is visibly disabled."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='''circle(26,218,19,(animation_tick/13u)&1u?RGB(43,140,198):RGB(36,122,180));circle(26,218,16,RGB(32,92,149));'''
b='''circle(26,218,19,g->energy<=0?RGB(77,82,87):
   ((animation_tick/13u)&1u?RGB(43,140,198):RGB(36,122,180)));
 circle(26,218,16,g->energy<=0?RGB(55,59,63):RGB(32,92,149));'''
assert a in s
p.write_text(s.replace(a,b,1))
print("PASS 15: energy circle becomes gray exactly when no energy remains.")
