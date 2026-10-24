#!/usr/bin/env python3
"""Emulator ONLY: cycle actual event/shop/reward/boss/map render code.
Never added to production EBOOT: injected after production source copied.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a="        game_update(&game);"
b='''#ifdef RIFT_QA_SCENES
        /* Reproduce each requested UI screen, holding still for screenshots. */
        if(frame==6){game.screen=EVENT;game.effect_turn=11;game.selection=0;}
        if(frame==135){game.screen=SHOP;game.selection=3;game.shop_relic=1;}
        if(frame==270){game.screen=REWARD;game.selection=0;
           game.reward[0]=3;game.reward[1]=8;game.reward[2]=10;
           game.message[0]=0;}
        if(frame==405){game.screen=BOSS_REWARD;game.selection=0;
           game.reward[0]=1;game.reward[1]=3;game.reward[2]=4;}
        if(frame==540){game.screen=MAP;game.floor=11;game.lane=1;
           game.scroll=0;game.selection=1;}
#endif
        game_update(&game);'''
if a not in s:raise RuntimeError("Missing CI scene hook")
p.write_text(s.replace(a,b,1))
print("PASS QA scene cycling emulator-only, isolated from PSP release binary")
