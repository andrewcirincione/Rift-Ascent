#!/usr/bin/env python3
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a="void game_render_set_overlay(int kind,int tab,int potion);"
if a not in s:raise RuntimeError("missing UI declarations")
s=s.replace(a,a+"\nvoid game_render_set_enemy_windup(int ticks);",1)
a="        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);"
if a not in s:raise RuntimeError("missing inspector integration")
s=s.replace(a,"        game_render_set_enemy_windup(enemy_turn_delay);\n"+a,1)
p.write_text(s)
print("Displayed enemy windup in PSP rendering loop.")
