#!/usr/bin/env python3
"""1.0.1 targeted gameplay fixes; run after overhaul/patch_engine.py."""
from pathlib import Path
import sys
root=Path(sys.argv[1]); g=root/"game.c"; h=root/"game.h"
s=g.read_text(); hs=h.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("missing game anchor "+a[:80])
 s=s.replace(a,b,1)
def hsub(a,b):
 global hs
 if a not in hs:raise RuntimeError("missing header anchor "+a[:80])
 hs=hs.replace(a,b,1)
hsub("HELP,BOSS_REWARD};","HELP,BOSS_REWARD,RELIC_VIEW};")
hsub("void game_choose_node(Game *g,int lane);",'''void game_choose_node(Game *g,int lane);
int game_enemy_intent_damage(const Game *g,int index);
int game_enemy_intent_hits(const Game *g,int index);
''')
sub("if(cards[id].rarity==rarity||tries++>100)return id;","if(id>2 && (cards[id].rarity==rarity||tries++>100))return id;")
# Centralize all enemy attack calculations, including Weak and Vulnerable.
sub("void game_end_turn(Game*g){",'''static int enemy_raw_damage(const Enemy *e){
 int v=e->intent_value;
 switch(e->intent){
 case 0:return MAX(0,v+e->strength);
 case 2:return MAX(0,v/2+e->strength);
 case 3:return MAX(0,v);
 case 5:case 6:case 8:return MAX(0,v/2);
 case 7:return MAX(1,v/2+e->strength);
 case 10:return MAX(0,v+v/4+e->strength);
 case 12:return MAX(0,v/2+e->strength);
 default:return 0;
 }
}
static int enemy_attack_value(const Enemy *e){
 int n=enemy_raw_damage(e);
 return e->weak?n*3/4:n;
}
int game_enemy_intent_hits(const Game *g,int index){
 if(!g||index<0||index>=g->enemy_count)return 0;
 const Enemy *e=&g->enemies[index];
 return e->hp>0&&enemy_raw_damage(e)>0?(e->intent==7?2:1):0;
}
int game_enemy_intent_damage(const Game *g,int index){
 int hits=game_enemy_intent_hits(g,index);
 if(!hits)return 0;
 int n=enemy_attack_value(&g->enemies[index]);
 if(g->vulnerable)n=n*3/2;
 return n*hits; /* Total before block. */
}
void game_end_turn(Game*g){''')
for old,new in [
 ('MAX(0,e->intent_value+e->strength);if(e->weak)dmg=dmg*3/4;','enemy_attack_value(e);'),
 ('MAX(0,e->intent_value/2+e->strength)','enemy_attack_value(e)'),
 ('damage_player(g,e->intent_value);g->vulnerable','damage_player(g,enemy_attack_value(e));g->vulnerable'),
 ('damage_player(g,e->intent_value/2);if(g->discard_count','damage_player(g,enemy_attack_value(e));if(g->discard_count'),
 ('damage_player(g,e->intent_value/2);g->poison','damage_player(g,enemy_attack_value(e));g->poison'),
 ('MAX(1,e->intent_value/2+e->strength)','enemy_attack_value(e)'),
 ('damage_player(g,e->intent_value/2);g->energy_penalty','damage_player(g,enemy_attack_value(e));g->energy_penalty'),
 ('damage_player(g,e->intent_value+e->intent_value/4+e->strength);','damage_player(g,enemy_attack_value(e));'),
 ('damage_player(g,e->intent_value/2+e->strength);e->hp','damage_player(g,enemy_attack_value(e));e->hp')
]:sub(old,new)
sub("static void check_win(Game*g){","static int random_relic(Game*g);\nstatic void check_win(Game*g){")
sub(''' fill_rewards(g);if(g->potion_count<MAX_POTIONS&&rnd(g,100)<22)g->potions[g->potion_count++]=rnd(g,6);g->screen=REWARD;g->selection=0;
 msg(g,"Victory! Choose one reward.");''',''' fill_rewards(g);if(g->potion_count<MAX_POTIONS&&rnd(g,100)<22)g->potions[g->potion_count++]=rnd(g,6);g->screen=REWARD;g->selection=0;
 if(g->battle_type==NODE_ELITE){
  int gift=random_relic(g);
  if(gift>=0){add_relic(g,gift);
   snprintf(g->message,sizeof(g->message),"ELITE RELIC: %s",relic_defs[gift].name);
  }else msg(g,"Elite defeated. No relics remain.");
 }else msg(g,"Victory! Choose one reward.");''')
sub("static void do_event(Game*g,int option){",'''static int count_upgradeable(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++)
  if(!CARD_UP(g->deck[i])&&CARD_ID(g->deck[i])<50)count++;
 return count;
}
static void do_event(Game*g,int option){''')
sub(''' case 10:for(int j=0;j<2;j++)if(g->deck_count){int idx=rnd(g,g->deck_count);g->deck[idx]|=UPBIT;}g->gold=MAX(0,g->gold-12);msg(g,"Two cards tempered at the ancient forge.");break;''',''' case 10:
  if(g->gold<12){msg(g,"Forge requires 12 gold.");break;}
  if(!count_upgradeable(g)){msg(g,"All cards already upgraded.");break;}
  g->gold-=12;g->screen=DECK_VIEW;g->prior_screen=EVENT;
  g->selection=0;g->scroll=0;g->next_screen=MIN(2,count_upgradeable(g));
  msg(g,"Choose cards to upgrade.");return;''')
sub(''' if(option==1){g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=0;g->scroll=0;msg(g,"Choose a card to upgrade.");}''',''' if(option==1){
   if(!count_upgradeable(g)){msg(g,"All cards already upgraded.");return;}
   g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=0;g->scroll=0;
   msg(g,"Choose an unupgraded card.");
 }''')
sub('''   if(g->prior_screen==REST&&g->deck_count>0){g->deck[g->selection]|=UPBIT;enter_map(g);msg(g,"Card upgraded.");}''','''   if((g->prior_screen==REST||g->prior_screen==EVENT)&&g->deck_count>0){
    int idx=g->selection;
    if(idx<0||idx>=g->deck_count)break;
    if(CARD_UP(g->deck[idx])||CARD_ID(g->deck[idx])>=50){
      msg(g,"Card cannot be upgraded again.");break;
    }
    g->deck[idx]|=UPBIT;
    if(g->prior_screen==EVENT && --g->next_screen>0 && count_upgradeable(g)>0){
      snprintf(g->message,sizeof(g->message),"Upgraded %s. Choose %d more.",
               cards[CARD_ID(g->deck[idx])].name,g->next_screen);
    }else{
      char result[72];
      snprintf(result,sizeof(result),"Upgraded %s.",cards[CARD_ID(g->deck[idx])].name);
      enter_map(g);msg(g,result);
    }
   }''')
sub('''    if(g->prior_screen==MAP)enter_map(g);''','''    if(g->prior_screen==MAP||g->prior_screen==EVENT)enter_map(g);''')
sub(''' case REWARD:
  if(b==BTN_LEFT)''',''' case REWARD:
  if(b==BTN_DOWN)g->selection=3;
  else if(b==BTN_UP&&g->selection==3)g->selection=1;
  else if(b==BTN_LEFT)''')
sub('''  else if(b==BTN_START){g->prior_screen=MAP;g->screen=HELP;}''','''  else if(b==BTN_START){g->screen=RELIC_VIEW;g->prior_screen=MAP;g->selection=0;}''')
sub(''' case VICTORY:case DEFEAT:''',''' case RELIC_VIEW:
  if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(MAX(0,g->relic_count-1),g->selection+1);
  else if(b==BTN_BACK||b==BTN_OK)enter_map(g);
  break;
 case VICTORY:case DEFEAT:''')
h.write_text(hs);g.write_text(s)
print("Version 1.0.1 engine patch applied.")
