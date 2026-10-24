#include "game.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LAST FIVE FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void start(Game*g,int seed){
 game_new(g,(unsigned)seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i].hp=g->enemies[i].maxhp=180;
   g->enemies[i].block=0;g->enemies[i].weak=0;g->enemies[i].vulnerable=0;}
 g->energy=3;g->target=0;g->hand_count=1;
}
int main(void){
 Game g;
 start(&g,1);g.hand[0]=3;CHECK(!strcmp(cards[3].name,"Stomp"));
 CHECK(game_card_cost_now(&g,3)==3);
 g.shop_relic=2;CHECK(game_card_cost_now(&g,3)==1);
 game_play_card(&g,0,0);
 CHECK(g.energy==2&&g.enemies[0].hp==168&&g.enemies[1].hp==168);
 start(&g,2);g.hand[0]=3|UPBIT;g.shop_relic=4;g.energy=0;
 CHECK(game_card_cost_now(&g,g.hand[0])==0);game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==165&&g.energy==0);
 start(&g,3);g.hand[0]=25;g.discard_count=2;g.discard[0]=0;g.discard[1]=11;
 game_play_card(&g,0,0);CHECK(g.screen==CARD_CHOICE&&g.discard_count==2);
 CHECK(game_validate(&g));
 CHECK(game_save(&g,"/tmp/rift_lastfive_save.dat"));
 {Game loaded;CHECK(game_load(&loaded,"/tmp/rift_lastfive_save.dat"));CHECK(loaded.screen==CARD_CHOICE);}
 game_action(&g,BTN_RIGHT);game_action(&g,BTN_OK);
 CHECK(g.screen==COMBAT&&g.draw_count>=1&&g.draw[g.draw_count-1]==11);
 CHECK(g.discard[g.discard_count-1]==25&&g.discard_count==2);
 start(&g,4);g.hand[0]=26;g.draw_count=3;
 g.draw[0]=1;g.draw[1]=0;g.draw[2]=0;
 game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==174&&g.hand_count==3&&g.hand[2]==1);
 start(&g,5);g.hand[0]=37;g.exhaust_count=0;
 game_play_card(&g,0,0);CHECK(g.energy==3&&g.hand_count==1);
 g.exhaust_count=3;g.exhaust[0]=50;g.exhaust[1]=0;g.exhaust[2]=51;
 game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==163&&g.enemies[1].hp==163&&g.energy==3);
 start(&g,6);g.hand[0]=40;g.enemies[0].vulnerable=2;
 game_play_card(&g,0,0);
 CHECK(g.enemies[0].vulnerable==4&&g.exhaust_count==1);
 CHECK(game_validate(&g));
 puts("PASS: Stomp dynamic cost, Headbutt discard choice + save, Pillage, Pact's End and Molten Fist");
 return 0;
}