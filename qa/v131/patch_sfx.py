#!/usr/bin/env python3
"""Short original UI/Block sounds, never synthesized background music."""
from pathlib import Path
import sys
psp=Path(sys.argv[1]);sound=Path(sys.argv[2]);s=psp.read_text();t=sound.read_text()
def swap(src,old,new,label):
 n=src.count(old)
 if n!=1:raise RuntimeError(f"{label}: {n} anchors")
 return src.replace(old,new,1)
t=swap(t,'if(kind<=0||kind>5)return;','if(kind<=0||kind>8)return;','SFX range')
t=swap(t,'''    case 5: /* reward */
     sfx=pulse(s->sfx_phase,90)*amp;s->sfx_phase+=840+s->fx_age/28;break;''',
'''    case 5: /* reward */
     sfx=pulse(s->sfx_phase,90)*amp;s->sfx_phase+=840+s->fx_age/28;break;
    case 6: /* brief cursor click */
     if(s->fx_age<1100){
       int env=(1100-s->fx_age)*amp/1100;
       sfx=pulse(s->sfx_phase,64)*env/2;s->sfx_phase+=2200;
     }
     break;
    case 7: /* map/selection confirm */
     if(s->fx_age<2600){
       int env=(2600-s->fx_age)*amp/2600;
       sfx=pulse(s->sfx_phase,125)*env;
       s->sfx_phase+=1550+(s->fx_age/10);
     }
     break;
    case 8: /* metallic Block shield gain */
     if(s->fx_age<3400){
       int env=(3400-s->fx_age)*amp/3400;
       sfx=pulse(s->sfx_phase,110)*env+
           ((s->noise&1u)?env/5:-env/5);
       s->sfx_phase+=2400-s->fx_age/13;
     }
     break;''','SFX envelope')
s=swap(s,'''        if(key==BTN_OK && game.hand_count>0) {
            int raw=game.hand[game.selection];''',
'''        if(key==BTN_OK && game.hand_count>0 &&
           game.selection>=0 && game.selection<game.hand_count) {
            int raw=game.hand[game.selection];''','safe hand index')
s=swap(s,'''    if(before_input.screen!=game.screen)rift_sound_effect(5);''',
'''    if(before_input.screen==COMBAT &&
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
    'map and card selection/block SFX')
sound.write_text(t);psp.write_text(s)
print('PASS: cursor, select and shield SFX integrated')
