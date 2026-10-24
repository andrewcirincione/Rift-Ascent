/* Assisted deterministic 3-act itinerary smoke, not fair-game balance evidence. */
#include "game.h"
#include <stdio.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"FAIL CAMPAIGN %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
int main(void){
 Game g; game_new(&g,0x1A2B3C4Du);game_action(&g,BTN_OK);
 int battles=0,shops=0,events=0,rests=0,act=1,steps=0,stalls=0,old=-1;
 for(;steps<1700&&g.screen!=VICTORY&&g.screen!=DEFEAT;steps++){
  REQUIRE(game_validate(&g));
  switch(g.screen){
   case TITLE:game_action(&g,BTN_OK);break;
   case MAP:{
    if(g.floor>=MAP_ROWS){game_action(&g,BTN_OK);break;}
    int dest=-1,best=-1000;
    for(int i=0;i<MAP_LANES;i++)if(game_route_available(&g,i)){
      int type=g.map[g.floor][i],score=0;
      if(type==NODE_SHOP&&shops==0)score+=100;
      if(type==NODE_EVENT&&events==0)score+=100;
      if(type==NODE_REST&&rests==0)score+=100;
      if(type==NODE_ELITE||type==NODE_FIGHT)score+=20;
      if(type==NODE_BOSS)score+=10;
      score-=i;
      if(score>best){best=score;dest=i;}
    }
    REQUIRE(dest>=0);
    g.selection=dest;game_action(&g,BTN_OK);break;
   }
   case COMBAT:{
    battles++;
    if(g.turn>2||battles%5==0)
      for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;
    int played=0;
    for(int i=0;i<g.hand_count;i++){
      int cost=cards[CARD_ID(g.hand[i])].cost;
      if(cost!=-2&&(cost==-1||cost<=g.energy)){
       game_play_card(&g,i,g.target);played=1;break;
      }
    }
    if(!played&&g.screen==COMBAT)game_end_turn(&g);
    break;
   }
   case REWARD:g.selection=0;game_action(&g,BTN_OK);break;
   case CARD_CHOICE:game_action(&g,BTN_OK);break;
   case REST:rests++;g.selection=0;game_action(&g,BTN_OK);break;
   case SHOP:shops++;game_action(&g,BTN_BACK);break;
   case EVENT:events++;g.selection=1;game_action(&g,BTN_OK);break;
   case EVENT_RESULT:case RELIC_RESULT:case BOSS_REWARD:
      game_action(&g,BTN_OK);break;
   case DECK_VIEW:case RELIC_VIEW:case HELP:case PAUSE_MENU:
      game_action(&g,BTN_BACK);break;
   default:REQUIRE(0);
  }
  if(g.act>act)act=g.act;
  if(g.screen==old)stalls++;else stalls=0;
  old=g.screen;
  REQUIRE(stalls<120);
  if(g.hp<25)g.hp=g.maxhp; /* explicit test assistance */
 }
 REQUIRE(g.screen==VICTORY&&g.act==3);
 REQUIRE(shops>0&&events>0&&rests>0&&battles>0);
 printf("PASS: 3-act assisted itinerary, %d actions, %d battle actions, %d shops, %d events, %d rests\n",steps,battles,shops,events,rests);
 return 0;
}
