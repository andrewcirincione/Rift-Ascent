#!/usr/bin/env python3
"""Pre-resolution 12-frame card presentation on the real PSP input loop."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def replace(old,new,reason):
 global s
 count=s.count(old)
 if count!=1:raise RuntimeError(f"v120 PSP {reason}: anchor count {count}")
 s=s.replace(old,new,1)
replace('void game_render_feedback(const Game *before,const Game *after,int key,unsigned tick);',
 '''void game_render_feedback(const Game *before,const Game *after,int key,unsigned tick);
void game_render_preplay(const Game *g,unsigned tick);''',
 'preplay renderer declaration')
replace('''    int held=0;
    int last_screen=game.screen, last_act=game.act, last_floor=game.floor;''',
 '''    int held=0;
    int pending_card_ticks=0;
    int last_screen=game.screen, last_act=game.act, last_floor=game.floor;''',
 'pending input state')
replace('''        /* Separate "end turn" selection from engine resolution so enemies
           telegraph their attack, then damage numbers remain visible. */''',
 '''        /* Pre-resolution card presentation: game state is deliberately
           untouched for 12 rendered frames, then the action resolves.
           This covers Strike, Defend, and cards that trigger autoplay. */
        if(game.screen==COMBAT && !inspect_open && !enemy_turn_delay &&
           !pending_card_ticks && !action_cooldown && key==BTN_OK &&
           game.selection>=0 && game.selection<game.hand_count){
            int raw=game.hand[game.selection];
            int cost=game_card_cost_now(&game,raw);
            if(game_card_is_playable(raw)&&(cost==-1||cost<=game.energy)){
                game_render_preplay(&game,frame);
                pending_card_ticks=12;
                key=0;
            }
        }
        if(pending_card_ticks>0){
            pending_card_ticks--;
            key=0;
            if(pending_card_ticks==0){
                step_action(BTN_OK,frame);
                action_cooldown=12;
            }
        }
        /* Separate "end turn" selection from engine resolution so enemies
           telegraph their attack, then damage numbers remain visible. */''',
 'queue playable card before state changes')
p.write_text(s)
print("PASS 1.2.0: PSP cards present 12 frames before resolving effect")
