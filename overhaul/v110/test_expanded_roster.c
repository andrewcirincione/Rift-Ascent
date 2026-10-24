#include "game.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ROSTER FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void start(Game*g,int seed){
 game_new(g,(unsigned)seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i].hp=g->enemies[i].maxhp=250;g->enemies[i].block=0;g->enemies[i].vulnerable=0;}
 g->hand_count=1;g->energy=4;
}
int main(void){
 Game g;
 start(&g,1);g.hand[0]=13;g.exhaust_count=2;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==238);
 start(&g,2);g.hand[0]=14;int hp=g.hp;game_play_card(&g,0,0);
 CHECK(g.hp==hp-1&&g.enemies[0].hp==241&&g.enemies[1].hp==241);
 start(&g,3);g.hand_count=2;g.hand[0]=15;g.hand[1]=1;
 game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==232&&g.exhaust_count==1&&g.hand_count==0);
 start(&g,4);g.hand[0]=20;game_play_card(&g,0,0);
 CHECK(g.shop_cards[2]==3&&g.exhaust_count==0);
 g.hand_count=2;g.hand[0]=43;g.hand[1]=1;g.energy=3;game_play_card(&g,0,0);
 CHECK(g.screen==CARD_CHOICE);game_action(&g,BTN_OK);
 CHECK(g.block>=3&&(g.reward_potion&4));
 start(&g,5);g.hand[0]=22;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp+g.enemies[1].hp==491);
 start(&g,6);g.hand[0]=30;game_play_card(&g,0,0);
 CHECK(g.enemies[0].vulnerable==3&&g.exhaust_count==1);
 start(&g,7);g.enemies[0].vulnerable=1;g.hand[0]=32;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==226);
 start(&g,8);g.enemies[0].vulnerable=3;g.hand[0]=35;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==235);
 start(&g,9);g.hand_count=4;g.hand[0]=45;g.hand[1]=1;g.hand[2]=0;g.hand[3]=50;
 game_play_card(&g,0,0);CHECK(g.block==10&&g.hand_count==1&&g.exhaust_count==2);
 start(&g,10);g.deck_count=7;for(int i=0;i<5;i++)g.deck[i]=0;
 g.deck[5]=11;g.deck[6]=49;g.hand[0]=49;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==230);
 start(&g,11);g.hand_count=2;g.hand[0]=16;g.hand[1]=30;g.energy=3;
 game_play_card(&g,1,0);CHECK(g.reward_potion&4);
 game_play_card(&g,0,0);CHECK(g.block==16);
 puts("PASS: 11 Ironclad wiki cards, dynamic damage, Exhaust triggers, FNP and Evil Eye");
 return 0;
}
