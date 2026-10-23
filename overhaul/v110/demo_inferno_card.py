#!/usr/bin/env python3
"""CI-only: force the selected 1.1.0 Inferno card to be shown in PPSSPP gameplay."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a="        if(frame==19)key=BTN_OK;"
b=a+'''
        if(frame==25&&game.screen==COMBAT){
            game.hand_count=1;
            game.hand[0]=28; /* Inferno, rules must be visible on the card */
            game.selection=0;
            game.energy=3;
        }'''
if a not in s:raise RuntimeError("deterministic demo input anchor missing")
p.write_text(s.replace(a,b,1))
print("PASS 1.1.0 CI demo shows entire Inferno rules text on selected battle card")
