#!/usr/bin/env python3
"""Emulator-only demo UI inputs. Not compiled in production EBOOT."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
anchor="        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);"
inject="""#ifdef RIFT_MAP_DEMO
        if(frame==4 && game.screen==TITLE) {
            inspect_open=0;
            step_action(BTN_OK,frame);
        }
#endif
#ifdef RIFT_INSPECT_DEMO
        if(frame>29 && game.screen==COMBAT){
            inspect_open=1;
            inspect_tab=1;
            inspect_potion=0;
        }
#endif
        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);"""
if anchor not in s:raise RuntimeError("front UI anchor absent")
p.write_text(s.replace(anchor,inject,1))
print("Ready: two independent emulator-only map / glossary screens.")
