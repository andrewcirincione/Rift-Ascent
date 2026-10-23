#!/usr/bin/env python3
"""1.0.2 item 05: deck and forge use 3-column D-pad grid navigation."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
old=''' case DECK_VIEW:
  if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(g->deck_count-1,g->selection+1);'''
new=''' case DECK_VIEW:
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN&&g->selection+3<g->deck_count)g->selection+=3;
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)
    g->selection++;'''
assert old in s
p.write_text(s.replace(old,new,1))
print("PASS 05: Up/Down change rows; Left/Right navigate columns without wrapping.")
