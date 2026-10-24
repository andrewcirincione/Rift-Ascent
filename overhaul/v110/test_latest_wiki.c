#include "game.h"
#include <stdio.h>
#define TEST(c) do{if(!(c)){fprintf(stderr,"WIKI FAIL %d %s\n",__LINE__,#c);return 1;}}while(0)
static void start(Game*g,int seed){
 game_new(g,(unsigned)seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=200;
 g->enemies[0].block=0;g->enemies[0].vulnerable=0;
 g->hand_count=1;g->energy=3;
}
int main(void){
 Game g;start(&g,111);g.hand[0]=6|UPBIT;int hp=g.hp;
 game_play_card(&g,0,0);
 TEST(g.hp==hp-3 && g.energy==6 && g.screen==COMBAT);
 start(&g,112);g.hand[0]=48;
 game_play_card(&g,0,0);
 TEST(g.heal_after_battle==3);
 g.hand_count=1;g.hand[0]=48|UPBIT;g.energy=3;
 game_play_card(&g,0,0);TEST(g.heal_after_battle==7);
 int strength=g.strength;game_start_turn(&g);
 TEST(g.strength==strength+7);
 start(&g,113);g.hand[0]=37;g.exhaust_count=0;
 game_play_card(&g,0,0);
 TEST(g.enemies[0].hp==200&&g.hand_count==0&&g.energy==3);
 g.hand_count=1;g.hand[0]=37;g.exhaust_count=3;
 game_play_card(&g,0,0);TEST(g.enemies[0].hp==182);
 puts("PASS: updated official STS2 Bloodletting+, Demon Form stacks and Pact End conditional.");
 return 0;
}
