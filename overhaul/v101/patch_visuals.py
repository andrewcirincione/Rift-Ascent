#!/usr/bin/env python3
"""1.0.1 staged hit/death animation and clearer controller symbols."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("visual anchor missing "+a[:100])
 s=s.replace(a,b,1)
sub("static unsigned fx_tick,fx_transition_tick;",
'''static Game victory_scene;
static int victory_replay=0;
static unsigned victory_tick=0;
static unsigned fx_tick,fx_transition_tick;''')
sub("if(before->screen!=after->screen){fx_transition=1;fx_transition_tick=tick;}",
'''if(before->screen!=after->screen){fx_transition=1;fx_transition_tick=tick;}
 if(before->screen==COMBAT && after->screen==REWARD){
    victory_scene=*before;
    victory_tick=tick;
    victory_replay=1;
 }''')
# Bigger, readable combat numbers, no "ENEMY TURN" meter.
sub('''center(ex,90-(int)(age/2),amount,age<8?WHITE:RED,1);''',
    '''center(ex,86-(int)(age/2),amount,age<8?WHITE:RED,2);''')
sub('''center(95,85-(int)(age/2),amount,RED,1);''',
    '''center(95,85-(int)(age/2),amount,RED,2);''')
sub(''' if(enemy_windup>0) {
    dark_rect(175,67,158,32,184);
    border(175,67,158,32,RED);
    center(254,74,"ENEMY TURN",GOLD,1);
    rect(181,90,(enemy_windup*146)/24,3,RED);
 }''',
''' if(enemy_windup>0) {
    /* Windup is conveyed through enemy movement and flashing slash sparks. */
    int x=g->enemy_count==1?337:(g->target==0?333:425);
    int y=105-(24-enemy_windup)/3;
    if((enemy_windup/3)%2==0) {
      line(x-17,y-11,x+14,y+6,WHITE);
      line(x-9,y-14,x+20,y+2,RED);
      circle(x+14,y+5,3,GOLD);
    }
 }''')
sub(''' case REWARD:reward_screen(g);break;''',
''' case REWARD:
   if(victory_replay && tick>=victory_tick && tick-victory_tick<39) {
     Game replay=victory_scene;
     replay.screen=COMBAT;
     for(int i=0;i<replay.enemy_count;i++)replay.enemies[i].hp=1;
     combat(&replay);
     if(tick-victory_tick>12) {
       int fade=(int)((tick-victory_tick-12)*180/27);
       dark_rect(0,31,W,219,fade);
     }
     if(tick-victory_tick>18)center(240,95,"VICTORY",GOLD,2);
   }else{victory_replay=0;reward_screen(g);}
   break;''')
# A small custom gamepad legend, not just button letters in text.
sub("static void help_screen(const Game*g){",'''static void controller_icon(int x,int y,int kind){
 circle(x,y,9,RGB(38,50,68));
 circle(x,y,8,RGB(90,109,128));
 if(kind==0){line(x-4,y-4,x+4,y+4,WHITE);line(x+4,y-4,x-4,y+4,WHITE);}
 if(kind==1)circle(x,y,5,WHITE);
 if(kind==2)triangle(x,y-5,x-5,y+4,x+5,y+4,WHITE);
 if(kind==3){rect(x-5,y-5,10,10,WHITE);rect(x-3,y-3,6,6,RGB(90,109,128));}
}
static void help_screen(const Game*g){''')
sub(''' text(29,167,"Win fights, grow your deck, defeat 3 bosses.",GOLD,1);''',
''' controller_icon(15,91,0);controller_icon(15,111,1);
 controller_icon(15,131,3);controller_icon(15,151,2);
 text(29,167,"Win fights, grow your deck, defeat 3 bosses.",GOLD,1);''')
# Show the act change prominently instead of just silently resetting map.
sub(''' if(g->screen==COMBAT)inspector(g);''',
''' if(g->screen==COMBAT)inspector(g);
 if(g->screen==MAP && fx_transition && animation_tick>=fx_transition_tick &&
    animation_tick-fx_transition_tick<23 && g->floor==0 && g->act>1){
    dark_rect(83,87,317,72,177);
    char act_label[32];snprintf(act_label,sizeof(act_label),"ACT %d BEGINS",g->act);
    center(240,108,act_label,GOLD,2);
 }''')
p.write_text(s)
print("1.0.1 staged hits, kill effect, new-act banner, face-button icons applied.")
