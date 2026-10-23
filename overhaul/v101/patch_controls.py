#!/usr/bin/env python3
"""Make five inspection tabs navigable and give victory effects time to play."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("missing control anchor: "+a[:100])
 s=s.replace(a,b,1)
sub("inspect_tab=(inspect_tab+3)%4","inspect_tab=(inspect_tab+4)%5")
sub("inspect_tab=(inspect_tab+1)%4","inspect_tab=(inspect_tab+1)%5")
sub("if(prevscreen!=game.screen)action_cooldown=14;",
    "if(prevscreen==COMBAT&&game.screen==REWARD)action_cooldown=44;\n           else if(prevscreen!=game.screen)action_cooldown=14;")
# Don't overwrite menu state with a second action until victory vignette concludes.
sub('''        game_render_set_enemy_windup(enemy_turn_delay);''',
'''        if(game.screen==REWARD&&action_cooldown>0)key=0;
        game_render_set_enemy_windup(enemy_turn_delay);''')
p.write_text(s)
print("1.0.1 inspector and reward input pacing patched.")
