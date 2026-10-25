#!/usr/bin/env python3
"""Rift Ascent 1.2.0 gameplay corrections; fail on stale anchors."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c"
s=p.read_text()
def patch(old,new,label):
 global s
 n=s.count(old)
 if n!=1:raise RuntimeError(f"v120 {label}: expected one anchor, got {n}")
 s=s.replace(old,new,1)
patch(
 'else if(choice==5){if(g->gold>=60&&g->deck_count>1){g->gold-=60;g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;msg(g,"Choose a card to remove.");}else msg(g,"Not enough gold.");}',
 'else if(choice==5){if(g->gold>=60&&g->deck_count>1){g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;msg(g,"Choose card to remove. No charge until confirmed.");}else msg(g,"Not enough gold.");}',
 'shop removal must not charge for opening')
patch(
 'else if(g->prior_screen==SHOP&&g->deck_count>1){n=g->selection;for(int i=n;i<g->deck_count-1;i++)g->deck[i]=g->deck[i+1];g->deck_count--;g->screen=SHOP;msg(g,"Card removed.");}',
 'else if(g->prior_screen==SHOP&&g->deck_count>1){if(g->gold<60){g->screen=SHOP;msg(g,"Not enough gold for removal.");break;}n=g->selection;g->gold-=60;for(int i=n;i<g->deck_count-1;i++)g->deck[i]=g->deck[i+1];g->deck_count--;g->screen=SHOP;g->selection=5;msg(g,"Card removed. 60 gold paid.");}',
 'charge removal exactly on confirmation')
old=''' if(g->battle_type==NODE_BOSS){
  if(g->act==3){g->screen=VICTORY;msg(g,"THE SPIRE HAS FALLEN");return;}
  for(int i=0;i<3;i++){
   int r=0,tries=0;
   do { r=random_relic(g); if(r<0)break;tries++; }
   while(tries<100&&((i>0&&r==g->reward[0])||(i>1&&r==g->reward[1])));
   g->reward[i]=r;
  }
  g->selection=0;g->screen=BOSS_REWARD;
  msg(g,"The guardian falls. Claim a power.");
  return;
 }'''
new=''' if(g->battle_type==NODE_BOSS){
  int gift=random_relic(g);
  if(gift>=0)add_relic(g,gift);
  g->reward[0]=gift;
  g->win_screen=(g->act==3?2:1);
  g->selection=0;g->screen=RELIC_RESULT;
  if(gift>=0)snprintf(g->message,sizeof(g->message),
        "BOSS RELIC: %s. Card reward next.",relic_defs[gift].name);
  else msg(g,"All relics found. Card reward next.");
  return;
 }'''
patch(old,new,'automatic boss relic including final boss')
patch(
 ''' case RELIC_RESULT:
  if(b==BTN_OK||b==BTN_BACK){
    if(g->win_screen){
      heal(g,g->maxhp/3);
      g->act++;g->floor=0;g->lane=1;map_generate(g);
      enter_map(g);msg(g,"A new ascent begins. Choose a path.");
    }else enter_map(g);
  }
  break;''',
 ''' case RELIC_RESULT:
  if(b==BTN_OK||b==BTN_BACK){
    if(g->win_screen){
      fill_rewards(g);g->selection=0;g->screen=REWARD;
      msg(g,"Guardian defeated. Choose a card or skip.");
    }else enter_map(g);
  }
  break;''',
 'boss relic then card reward')
patch(
 '''  else if(b==BTN_OK){if(g->selection<3)add_card(g,g->reward[g->selection]);enter_map(g);msg(g,"Onward, Warden.");}''',
 '''  else if(b==BTN_OK){
    if(g->selection<3)add_card(g,g->reward[g->selection]);
    if(g->win_screen){
      int final=g->win_screen==2;g->win_screen=0;
      if(final){g->screen=VICTORY;msg(g,"THE SPIRE HAS FALLEN");}
      else{heal(g,g->maxhp/3);g->act++;g->floor=0;g->lane=1;
        map_generate(g);enter_map(g);msg(g,"A new ascent begins.");}
    }else{enter_map(g);msg(g,"Onward, Warden.");}
  }''',
 'boss card reward after relic, including skip')
patch(
 '''"Cost 1. Draw Strike: auto-play it."''',
 '''"Draw a Strike; auto-play it against a random foe."''',
 'Hellraiser upgrade text redundant cost')
# Expand clarity for several relics whose implementation is misunderstood.
patch('{"Copper Ring","First turn gains 1 energy"}',
      '{"Copper Ring","Start each combat with 1 extra Energy on turn one."}',
      'copper ring description')
patch('{"Last Ember","Survive lethal hit once per act"}',
      '{"Last Ember","Once per act, survive lethal damage at 1 HP."}',
      'last ember description')
patch('{"Sun Token","Gain +1 max energy, lose 1 draw"}',
      '{"Sun Token","Gain +1 Energy per turn; draw 1 fewer card each turn."}',
      'sun token drawback clarity')
patch('{"Old Compass","Gain 1 draw on turn one"}',
      '{"Old Compass","Draw 1 extra card on the first turn of combat."}',
      'old compass tooltip')
p.write_text(s)
print("PASS: v1.2.0 game fixes shop cancellation, boss relic/card, descriptions")
