#!/usr/bin/env python3
"""v1.3.2 short UI/Block/enemy impact sound routing; no new background music."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def one(a,b,label):
 global s
 n=s.count(a)
 if n!=1:raise RuntimeError(f"v132 {label}: {n} anchors")
 s=s.replace(a,b,1)
one('''    if(before_input.screen==COMBAT &&
       (game.block>before_input.block ||
        (game.enemy_count>0 &&
          game.enemies[0].block>before_input.enemies[0].block) ||
        (game.enemy_count>1 &&
          game.enemies[1].block>before_input.enemies[1].block)))
      rift_sound_effect(8);
    else if(before_input.screen==MAP && key==BTN_OK &&
            game.screen!=MAP)rift_sound_effect(7);
    else if(before_input.screen==game.screen &&
            (before_input.selection!=game.selection ||
             (game.screen==MAP && before_input.scroll!=game.scroll)) &&
            (game.screen==MAP||game.screen==COMBAT||
             game.screen==CARD_CHOICE||game.screen==DECK_VIEW))
      rift_sound_effect(6);
    else if(before_input.screen!=game.screen)rift_sound_effect(5);''',
'''    /* Enemy attacks still sound when fully absorbed by Block. Attack
       takes priority over Block; new Block sounds on any combat actor. */
    int enemy_attack=0;
    if(oldscreen==COMBAT && key==BTN_BACK){
      for(int i=0;i<before_input.enemy_count;i++)
        if(before_input.enemies[i].hp>0 &&
           game_enemy_intent_hits(&before_input,i)>0)enemy_attack=1;
    }
    int any_block=before_input.screen==COMBAT &&
      (game.block>before_input.block ||
       (game.enemy_count>0 &&
         game.enemies[0].block>before_input.enemies[0].block) ||
       (game.enemy_count>1 &&
         game.enemies[1].block>before_input.enemies[1].block));
    int moved=(before_input.selection!=game.selection ||
               before_input.scroll!=game.scroll ||
               before_input.target!=game.target);
    if(enemy_attack)rift_sound_effect(4);
    else if(any_block)rift_sound_effect(8);
    else if((key==BTN_UP||key==BTN_DOWN||key==BTN_LEFT||key==BTN_RIGHT)
             && before_input.screen==game.screen && moved)
      rift_sound_effect(6);
    else if(key==BTN_OK && before_input.screen!=COMBAT)
      rift_sound_effect(7);
    else if(before_input.screen!=game.screen)rift_sound_effect(5);''',
 "all menu/map cursor, shield and attack SFX")
one('''        }else if(inspect_open && game.screen==COMBAT) {
           if(key==BTN_BACK)''',
'''        }else if(inspect_open && game.screen==COMBAT) {
           if(key==BTN_LEFT||key==BTN_RIGHT||key==BTN_UP||key==BTN_DOWN)
              rift_sound_effect(6);
           if(key==BTN_BACK)''',"battle inspector tabs")
p.write_text(s)
print("PASS v1.3.2 SFX: menus, map, card selection, enemy impact and actor Block")
