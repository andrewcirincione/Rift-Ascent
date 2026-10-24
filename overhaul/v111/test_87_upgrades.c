#include "game.h"
#include <stdio.h>
#include <string.h>
#define OK(x) do{if(!(x)){fprintf(stderr,"87 UPGRADE FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void setup(Game*g,unsigned seed){
 game_new(g,seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=2;
 for(int e=0;e<2;e++){
  g->enemies[e].hp=g->enemies[e].maxhp=10000;
  g->enemies[e].block=0;g->enemies[e].strength=0;
  g->enemies[e].vulnerable=0;
 }
 g->hand_count=3;g->energy=60;g->target=0;
 g->hand[1]=0;g->hand[2]=1;
 g->draw_count=5;
 for(int i=0;i<5;i++)g->draw[i]=(i%2)?0:1;
 g->discard_count=2;g->discard[0]=0;g->discard[1]=11;
}
int main(void){
 Game g;int plays=0,choices=0;
 for(int id=0;id<CARD_COUNT;id++){
  if(!game_card_is_playable(id))continue;
  if(id==63||id==85)continue; /* explicit multiplayer-only cards */
  for(int up=0;up<2;up++){
   setup(&g,(unsigned)(0x87010100u+id*2+up));
   int raw=id|(up?UPBIT:0);
   g.hand[0]=raw;
   if(id==37){g.exhaust_count=3;g.exhaust[0]=50;g.exhaust[1]=51;g.exhaust[2]=52;}
   if(id==77)g.hand[1]=0;
   if(id==67)g.reward_potion|=4;
   if(id==81)g.ic[IC_HPLOSS_TURN]=1;
   if(id==86)g.ic[IC_HPLOSS_COMBAT]=4;
   int before=g.energy;
   game_play_card(&g,0,0);
   OK(g.energy>=0&&g.energy<=before+60);
   OK(g.hp>0&&g.screen!=DEFEAT);
   OK(game_validate(&g));
   if(g.screen==CARD_CHOICE){
     choices++;
     game_action(&g,BTN_OK);
     OK(game_validate(&g));
     OK(g.screen==COMBAT||g.screen==REWARD);
   }
   plays++;
  }
 }
 /* Solo rewards must never be unusable multiplayer cards, statuses, token. */
 for(int k=0;k<120;k++){
  setup(&g,0x8a000000u+k);
  g.enemies[0].hp=0;g.enemies[1].hp=0;
  game_end_turn(&g);
  OK(g.screen==REWARD||g.screen==BOSS_REWARD);
  for(int i=0;i<3;i++){
    int id=CARD_ID(g.reward[i]);
    OK(game_card_is_playable(id));
    OK(id!=63&&id!=85&&id!=91);
  }
 }
 /* Both Ancient cards exist in catalog, but not random normal rewards. */
 OK(cards[56].rarity==3&&cards[59].rarity==3);
 /* Example long-lasting Powers, generated-card flags, and free cost. */
 setup(&g,0x10101010u);g.hand[0]=59|UPBIT;
 game_play_card(&g,0,0);OK(g.ic[IC_CORRUPTION]);
 OK(game_card_cost_now(&g,43)==0);
 setup(&g,0x10101011u);g.hand[0]=76|UPBIT;
 game_play_card(&g,0,0);OK(g.ic[IC_NEXT_ATTACK_REPEAT]==2);
 setup(&g,0x10101012u);g.hand[0]=89;
 game_play_card(&g,0,0);OK(g.ic[IC_UNRELENTING]==1);
 OK(game_card_cost_now(&g,0)==0);
 printf("PASS %d playable base/upgrade cases, %d card-choice screens, 120 safe reward draws\n",plays,choices);
 return 0;
}