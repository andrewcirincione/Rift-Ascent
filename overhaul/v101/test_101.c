#include "game.h"
#include <stdio.h>
#include <string.h>
#define ASSERT(x) do {if(!(x)){fprintf(stderr,"1.0.1 FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
int main(void){
 Game g;
 game_new(&g,0x10100101u);
 game_action(&g,BTN_OK);
 ASSERT(g.screen==MAP);
 g.map[0][1]=NODE_SHOP;
 game_choose_node(&g,1);
 ASSERT(g.screen==SHOP && g.floor==1);
 for(int i=0;i<20;i++)game_update(&g);
 ASSERT(g.screen==SHOP);
 game_new(&g,0x10100102u);
 g.screen=EVENT;g.effect_turn=10;g.selection=0;g.gold=100;
 game_action(&g,BTN_OK);
 ASSERT(g.screen==DECK_VIEW && g.prior_screen==EVENT && g.next_screen==2);
 ASSERT(g.gold==88);
 game_action(&g,BTN_OK);
 ASSERT(CARD_UP(g.deck[0])&&g.next_screen==1&&g.screen==DECK_VIEW);
 game_action(&g,BTN_OK);
 ASSERT(g.next_screen==1 && g.screen==DECK_VIEW);
 game_action(&g,BTN_DOWN);
 game_action(&g,BTN_OK);
 ASSERT(g.screen==MAP && CARD_UP(g.deck[1]));
 game_new(&g,0x10100103u);
 for(int trial=0;trial<50;trial++){
   game_start_battle(&g,NODE_ELITE);
   for(int k=0;k<g.enemy_count;k++)g.enemies[k].hp=0;
   int old=g.relic_count;
   game_end_turn(&g);
   ASSERT(g.screen==REWARD && g.relic_count>old);
   for(int c=0;c<3;c++)ASSERT(CARD_ID(g.reward[c])>2);
   g.screen=MAP;
 }
 game_action(&g,BTN_START);
 ASSERT(g.screen==RELIC_VIEW && g.relic_count>0);
 game_action(&g,BTN_BACK);
 ASSERT(g.screen==MAP);
 game_new(&g,0x10100104u);
 game_start_battle(&g,NODE_FIGHT);
 g.enemy_count=1;
 g.enemies[0].hp=30;g.enemies[0].intent=0;
 g.enemies[0].intent_value=12;g.enemies[0].strength=0;
 g.enemies[0].weak=2;
 g.vulnerable=0;
 ASSERT(game_enemy_intent_hits(&g,0)==1);
 ASSERT(game_enemy_intent_damage(&g,0)==9);
 g.vulnerable=1;
 ASSERT(game_enemy_intent_damage(&g,0)==13);
 g.enemies[0].intent=9;
 ASSERT(game_enemy_intent_damage(&g,0)==0);
 g.screen=REWARD;g.selection=0;
 game_action(&g,BTN_DOWN);ASSERT(g.selection==3);
 puts("PASS 1.0.1: shop persists, chosen forge upgrades, no duplicate upgrades, elite relics, rewards, live damage, relic ledger");
 return 0;
}
