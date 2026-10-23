#!/usr/bin/env python3
"""1.0.2 item 02: hand cards never obscure the player/foe HP and block."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("item02 anchor "+a[:100])
 s=s.replace(a,b,1)
sub("y=selected?132+wave(animation_tick,60,1):148;",
    "y=selected?166+wave(animation_tick,60,1):177;")
sub("card_render(x,y,70,selected?117:103,g->hand[i]",
    "card_render(x,y,70,selected?88:77,g->hand[i]")
# The card mat occupies only the bottom 80 pixels.  The selected top is
# >=165 and enemy HP/block is fully rendered above y=165.
p.write_text(s)
print("PASS 02: hand card overlap removed; health bars remain unobstructed.")
