#!/usr/bin/env python3
"""Adapt the proven PSP-3000 CRT-free startup to the upgraded game interactions.
Keep display flip order and NEXTVSYNC unchanged."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def repl(a,b):
 global s
 if a not in s:raise RuntimeError("psp frontend anchor missing "+a[:90])
 s=s.replace(a,b,1)
repl("music_battle=(game.screen==COMBAT);","music_battle=(game.act-1)*2+(game.screen==COMBAT?1:0);")
repl('#include "sound.h"',
'''#include "sound.h"
void game_render_set_overlay(int kind,int tab,int potion);
void game_render_feedback(const Game *before,const Game *after,int key,unsigned tick);
''')
repl("static volatile int music_battle=0;",
"""static volatile int music_battle=0;
static Game before_input;
static int inspect_open=0,inspect_tab=0,inspect_potion=0;
static int enemy_turn_delay=0,action_cooldown=0;
""")
repl('''static void step_action(int key,unsigned tick) {
    if(!key)return;''',
'''static void step_action(int key,unsigned tick) {
    if(!key)return;
    before_input=game;''')
repl('''    if(fx)game_effect_start(fx,tick);
}''',
'''    if(fx){
        game_effect_start(fx,tick);
        rift_sound_effect(fx);
    }
    if(before_input.screen!=game.screen)rift_sound_effect(5);
    if(before_input.screen!=game.screen ||
       before_input.hp!=game.hp||before_input.block!=game.block||
       before_input.hand_count!=game.hand_count||fx ||
       (key==BTN_BACK&&before_input.screen==COMBAT))
        game_render_feedback(&before_input,&game,key,tick);
}''')
repl('''    game_render_set_profile(&lifetime);
    boot_trace("profile set",1);
    sceCtrlSetSamplingCycle(0);''',
'''    game_render_set_profile(&lifetime);
    boot_trace("profile set",1);
    rift_sound_reset(&music);
    game_render_set_overlay(0,0,0);
    sceCtrlSetSamplingCycle(0);''')
anchor='''        step_action(key,frame);
        if(key && game.screen!=TITLE'''
if anchor not in s:raise RuntimeError("step_action block")
s=s.replace(anchor,'''        /* START opens a readable battle glossary; Triangle opens potions.
           Inputs are consumed while the inspector is open. */
        if(game.screen!=COMBAT && inspect_open)inspect_open=0;
        if(game.screen==COMBAT && key==BTN_START) {
           inspect_open=!inspect_open;
           if(inspect_open)inspect_tab=0;
           rift_sound_effect(2);key=0;
        }else if(game.screen==COMBAT && !inspect_open && key==BTN_POTION) {
           inspect_open=1;inspect_tab=3;
           inspect_potion=game.potion_count>0?game.potion_count-1:0;
           key=0;
        }else if(inspect_open && game.screen==COMBAT) {
           if(key==BTN_BACK){inspect_open=0;key=0;}
           else if(key==BTN_LEFT){inspect_tab=(inspect_tab+3)%4;key=0;}
           else if(key==BTN_RIGHT){inspect_tab=(inspect_tab+1)%4;key=0;}
           else if(key==BTN_UP){
             if(inspect_tab==3 && inspect_potion>0)inspect_potion--;
             else if(inspect_tab==1 && game.enemy_count>0)
               game.target=(game.target+game.enemy_count-1)%game.enemy_count;
             key=0;
           }else if(key==BTN_DOWN){
             if(inspect_tab==3&&inspect_potion+1<game.potion_count)inspect_potion++;
             else if(inspect_tab==1 && game.enemy_count>0)
               game.target=(game.target+1)%game.enemy_count;
             key=0;
           }else if(key==BTN_OK){
             if(inspect_tab==3 && game.potion_count>0){
                before_input=game;
                game_use_potion(&game,inspect_potion);
                game_render_feedback(&before_input,&game,BTN_POTION,frame);
                game_effect_start(3,frame);rift_sound_effect(3);
                if(inspect_potion>=game.potion_count)inspect_potion=game.potion_count-1;
                if(game.potion_count==0)inspect_open=0;
             }
             key=0;
           }else key=0;
        }
        /* Separate "end turn" selection from engine resolution so enemies
           telegraph their attack, then damage numbers remain visible. */
        if(game.screen==COMBAT&&!inspect_open&&!enemy_turn_delay &&
           (key==BTN_BACK || (key==BTN_OK&&game.selection>=game.hand_count))){
            enemy_turn_delay=23;
            action_cooldown=0;
            rift_sound_effect(2);
            key=0;
        }
        if(enemy_turn_delay>0){
            enemy_turn_delay--;
            if(enemy_turn_delay==0){
               step_action(BTN_BACK,frame);
               action_cooldown=20;
            }
            key=0;
        }
        if(action_cooldown>0){
           action_cooldown--;
           key=0;
        }
        if(key) {
           int prevscreen=game.screen;
           step_action(key,frame);
           if(prevscreen!=game.screen)action_cooldown=14;
           else if(prevscreen==COMBAT&&key==BTN_OK)action_cooldown=10;
        }
        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);
        if(key && game.screen!=TITLE''',1)
p.write_text(s)
print("Applied inspector, user-friendly controls, attack timing, sounds and scene feedback.")
