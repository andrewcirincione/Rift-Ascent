#!/usr/bin/env python3
"""Emulator-only independent QA screenshots, never compiled into production."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);src=p.read_text()
anchor="        game_update(&game);"
injection='''#if defined(RIFT_QA_EVENT) || defined(RIFT_QA_SHOP) || defined(RIFT_QA_REWARD) || defined(RIFT_QA_BOSS) || defined(RIFT_QA_MAP) || defined(RIFT_QA_REST) || defined(RIFT_QA_DECK)
        static int qa_injected=0;
        if(!qa_injected&&frame>=6){
          qa_injected=1;
#ifdef RIFT_QA_EVENT
          game.screen=EVENT;game.effect_turn=11;game.selection=0;
#elif defined(RIFT_QA_SHOP)
          game.screen=SHOP;game.selection=0;game.shop_cards[0]=11;game.shop_relic=1;
#elif defined(RIFT_QA_REWARD)
          game.screen=REWARD;game.selection=0;game.reward[0]=3;
          game.reward[1]=8;game.reward[2]=10;game.message[0]=0;
#elif defined(RIFT_QA_BOSS)
          game.screen=RELIC_RESULT;game.selection=0;
          game.reward[0]=1;game.win_screen=1;
#elif defined(RIFT_QA_MAP)
          #ifdef RIFT_QA_MAP
          game.screen=MAP;game.floor=11;game.lane=1;game.scroll=0;game.selection=1;
#endif
#elif defined(RIFT_QA_REST)
          game.screen=REST;game.selection=0;
#elif defined(RIFT_QA_DECK)
          game_start_battle(&game,NODE_ELITE);
          game.screen=DECK_VIEW;game.prior_screen=COMBAT;game.scroll=2;game.selection=0;
#endif
        }
#endif
        game_update(&game);'''
if src.count(anchor)!=1:raise RuntimeError("QA hook missing or ambiguous")
p.write_text(src.replace(anchor,injection,1))
print("PASS QA: independent isolated scene demo")
