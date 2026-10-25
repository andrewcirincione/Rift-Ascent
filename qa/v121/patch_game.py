#!/usr/bin/env python3
"""Rift Ascent v1.2.1: correct economics, encounters and X-cost handling."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c"
s=p.read_text()
def one(before,after,label):
 global s
 n=s.count(before)
 if n!=1:raise RuntimeError(f"v121 {label}: expected 1 anchor, found {n}")
 s=s.replace(before,after,1)
# Keep all historical map branches visible; interactivity still checks only
# accessible lanes. The map renderer draws the full original graph.
one(''' if(g->floor==0)return game_route_row_mask(g,row);
 if(row<g->floor-1) {
   int selected=g->map_selected[row];
   return selected>=0&&selected<MAP_LANES?(1u<<selected):game_route_row_mask(g,row);
 }
 unsigned live=1u<<g->lane;
 for(int r=g->floor-1;r<row;r++) {
   unsigned next=0;
   for(int c=0;c<MAP_LANES;c++)if(live&(1u<<c))
      next|=game_route_mask(g,r,c);
   live=next;
 }
 return live;''',
 ''' return game_route_row_mask(g,row);''','preserve map graph visibility')

# Realistic act progression and a meaningful number of two-foe hallways.
one(''' g->enemy_count=mode==NODE_BOSS?1:(mode==NODE_ELITE?1:(rnd(g,9)==0?2:1));''',
 ''' g->enemy_count=mode==NODE_BOSS?1:(mode==NODE_ELITE?(rnd(g,5)==0?2:1):
                      (rnd(g,100)<(g->act==1?38:g->act==2?47:56)?2:1));''','multi-enemy encounters')
one('''  init_enemy(g,&g->enemies[i],id);
 }''',
 '''  if(i>0 && id==g->enemies[0].art)
     id=enemy_normal[g->act-1][(i+g->floor+g->act)%8];
  init_enemy(g,&g->enemies[i],id);
 }''','avoid clone fights')
one(''' e->maxhp=(short)((d->hp*(d->tier==2?120:(d->tier==1?115:100))+50)/100+(g->floor>8?3:0));''',
 ''' int original=(d->hp*(d->tier==2?120:(d->tier==1?115:100))+50)/100;
 int scaling=100+(g->act-1)*13+(g->floor/4)*4;
 e->maxhp=(short)MIN(999,(original*scaling+50)/100);
 if(e->maxhp<1)e->maxhp=1;''','scale encounter HP by act/floor')
one(''' e->intent_value=(d->attack*85+50)/100+(g->floor>8?1:0)+(e->phase?2:0);''',
 ''' e->intent_value=(d->attack*85+50)/100+2*(g->act-1)+g->floor/5+(e->phase?2:0);''','scale attack severity')

# Merchant: remove heal service without changing save layout/shop indices.
one(''' else if(choice==4){if(g->gold>=45){g->gold-=45;heal(g,18);msg(g,"Restored 18 health.");}else msg(g,"Not enough gold.");}''',
 ''' else if(choice==4){msg(g,"No healing service. Use campfires or potions.");}''','remove merchant healing')
one(''' case 1:heal(g,20);g->gold=MAX(0,g->gold-40);msg(g,"The spring restores you. It claims 40 coins.");break;''',
 ''' case 1:{
   /* Fair partial payment: 2 gold heals 1 HP, up to 20 for 40 gold. */
   int paid=MIN(g->gold,40);
   int restored=MIN(20,paid/2);
   g->gold-=restored*2;
   heal(g,restored);
   snprintf(g->message,sizeof(g->message),
      "Moon Spring: healed %d HP for %d gold.",restored,restored*2);
   break;
 }''','Moon Spring actual partial payment')

# An X-cost card always consumes the whole energy pool on a manual play.
# CARD_FREE can still be applied to AUTOPLAY copies.
one(''' if(cost==-1)cost=g->energy;
 if(g->energy<cost)return;''',
 ''' if(cost==-1 || (id==10 && g->ic[IC_CHAIN_DEPTH]==0))cost=g->energy;
 if(g->energy<cost)return;''','Whirlwind manual X-cost')

# Old little exposition string from Rusted Forge.
one('''msg(g,"Choose cards to upgrade.");return;''',
 '''msg(g,"Choose cards to upgrade.");return;''','forge unchanged hook for regression')
# Correct overly long, redundant descriptions in source card text.
one('''"Cost 2. Skills cost 0 and Exhaust.",''',
 '''"Skills cost 0. Playing them Exhausts them.",''','card cost repetition')
one('''"MULTIPLAYER: cost 0. Protect allies.",''',
 '''"Multiplayer-only: protects allies.",''','multiplayer description')

# The merchant has SEVEN genuine options; don't leave a fake "no heal" row.
one(''' else if(choice==4){msg(g,"No healing service. Use campfires or potions.");}''',
''' else if(choice==4){if(g->gold>=60&&g->deck_count>1){
   g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;
   msg(g,"Choose card to remove; charge only when confirmed.");
 }else msg(g,"Not enough gold.");}''','shift removal to fourth purchase row')
one(''' else if(choice==5){if(g->gold>=60&&g->deck_count>1){g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;msg(g,"Choose card to remove. No charge until confirmed.");}else msg(g,"Not enough gold.");}''',
''' else if(choice==5){if(g->gold>=50&&g->potion_count<MAX_POTIONS){
   g->gold-=50;g->potions[g->potion_count++]=rnd(g,6);
   msg(g,"Purchased a potion.");
 }else msg(g,"No funds or potion slots.");}''','shift potion')
one(''' else if(choice==7){if(g->gold>=50&&g->potion_count<MAX_POTIONS){g->gold-=50;g->potions[g->potion_count++]=rnd(g,6);msg(g,"Purchased a potion.");}else msg(g,"No funds or potion slots.");}''',
 '''''','remove old potion slot')
one(''' else if(b==BTN_DOWN)g->selection=MIN(7,g->selection+1);''',
 ''' else if(b==BTN_DOWN)g->selection=MIN(6,g->selection+1);''','shop cursor seven rows')

p.write_text(s)
print("PASS 1.2.1: full route graph, tougher/multiple foes, no shop heal, spring partial payment, X-cost")
