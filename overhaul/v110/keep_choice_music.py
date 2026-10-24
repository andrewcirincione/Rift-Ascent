#!/usr/bin/env python3
"""Keep battle music when choosing a card or pausing its selection screen."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a="(game.screen==COMBAT||"
b="(game.screen==COMBAT||game.screen==CARD_CHOICE||"
if a not in s:raise RuntimeError("Missing soundtrack scene anchor")
s=s.replace(a,b,1)
a="(game.screen==PAUSE_MENU&&game_pause_source()==COMBAT))"
b="(game.screen==PAUSE_MENU&&(game_pause_source()==COMBAT||game_pause_source()==CARD_CHOICE)))"
if a not in s:raise RuntimeError("Missing pause scene soundtrack anchor")
s=s.replace(a,b,1)
p.write_text(s)
print("PASS: battle soundtrack continues through pending card selection and pause")
