#!/usr/bin/env python3
"""1.0.2 item 01 PSP frontend: save/quit without corrupting nested menus."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("pause PSP anchor: "+a[:100])
 s=s.replace(a,b,1)
sub("if (down & PSP_CTRL_START) return BTN_START;",
    "if (down & PSP_CTRL_SELECT) return BTN_PAUSE;\n    if (down & PSP_CTRL_START) return BTN_START;")
sub('''        /* START opens a readable battle glossary; Triangle opens potions.''','''        /* Save a validated snapshot from the original screen. NEVER store PAUSE_MENU
           and never exit if the memory-stick write failed. */
        if(game.screen==PAUSE_MENU && key==BTN_OK) {
            Game snapshot=game;
            snapshot.screen=game_pause_source();
            if(game_save(&snapshot,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT")){
                boot_trace("save and quit result",1);
                sceKernelExitGame();
                return 0;
            }else {
                boot_trace("save and quit FAILED",-1);
                key=0;
            }
        }
        /* START opens a readable battle glossary; Triangle opens potions.''')
p.write_text(s)
print("1.0.2 item01 PSP save/quit handling installed.")
