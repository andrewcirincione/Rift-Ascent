#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void multi_and_cost(void){
 Game g;int multi=0,total=0;
 for(unsigned seed=1;seed<300;seed++){
  game_new(&g,seed*17u);g.act=2;g.floor=8;
  game_start_battle(&g,NODE_FIGHT);
  total++;if(g.enemy_count==2)multi++;
  assert(game_validate(&g));
  assert(g.enemies[0].hp>=50);
 }
 assert(multi>100&&multi<225);
 game_new(&g,23);game_start_battle(&g,NODE_FIGHT);
 g.energy=3;g.hand_count=1;g.hand[0]=10;g.selection=0;
 game_play_card(&g,0,0);assert(g.energy==0);
 game_new(&g,24);game_start_battle(&g,NODE_FIGHT);
 g.energy=3;g.hand_count=1;g.hand[0]=10|CARD_FREE;
 g.selection=0;game_play_card(&g,0,0);
 assert(g.energy==0); /* manual X drains even if free flag erroneously applied */
}
static void spring(void){
 Game g;game_new(&g,98);
 g.screen=EVENT;g.effect_turn=1;g.gold=13;g.hp=45;
 game_action(&g,BTN_OK);
 assert(g.hp==51 && g.gold==1);
 game_new(&g,99);g.screen=EVENT;g.effect_turn=1;
 g.gold=0;g.hp=45;game_action(&g,BTN_OK);
 assert(g.hp==45&&g.gold==0);
 game_new(&g,100);g.screen=EVENT;g.effect_turn=1;
 g.gold=80;g.hp=45;game_action(&g,BTN_OK);
 assert(g.hp==65&&g.gold==40);
}
static void shop(void){
 Game g;game_new(&g,111);g.screen=SHOP;g.gold=200;
 int hp=g.hp;g.hp=20;g.selection=4;game_action(&g,BTN_OK);
 assert(g.hp==20&&g.gold==200);
 (void)hp;
}
static void map(void){
 Game g;game_new(&g,43);g.floor=7;g.lane=1;
 for(int r=0;r<MAP_ROWS;r++)
  assert(game_route_visible_mask(&g,r)==game_route_row_mask(&g,r));
}
int main(void){multi_and_cost();spring();shop();map();
 puts("PASS v1.2.1: map paths, encounter counts, X costs, spring partial gold, no merchant heal");
 return 0;
}
