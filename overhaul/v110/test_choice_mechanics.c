#include "game.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FIX FAIL %d %s\n",__LINE__,#x);return 1;}}while(0)
static void start(Game*g,int seed){
 game_new(g,(unsigned)seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i].hp=250;g->enemies[i].maxhp=250;g->enemies[i].block=0;}
}
int main(void){
 Game g;
 start(&g,42);g.hand_count=3;g.hand[0]=18;g.hand[1]=11;g.hand[2]=0;g.energy=3;int hp=g.hp;
 game_play_card(&g,0,0);
 CHECK(g.screen==CARD_CHOICE&&g.hand_count==2&&g.hp==hp-1&&g.strength==0);
 CHECK(game_validate(&g));
 CHECK(game_save(&g,"/tmp/rift_choice_save.dat"));
 {Game saved;CHECK(game_load(&saved,"/tmp/rift_choice_save.dat"));CHECK(saved.screen==CARD_CHOICE);}
 g.selection=1;game_action(&g,BTN_OK);
 CHECK(g.screen==COMBAT&&g.exhaust_count>=1&&g.hand_count==1&&g.strength==1);
 start(&g,43);g.hand_count=3;g.hand[0]=23;g.hand[1]=11;g.hand[2]=0;g.energy=3;
 game_play_card(&g,0,0);CHECK(g.screen==CARD_CHOICE&&g.block==5);
 g.selection=1;game_action(&g,BTN_OK);
 CHECK(g.screen==COMBAT&&CARD_UP(g.hand[1])&&!CARD_UP(g.hand[0]));
 start(&g,44);g.hand_count=3;g.hand[0]=43;g.hand[1]=0;g.hand[2]=1;
 g.draw_count=4;for(int i=0;i<4;i++)g.draw[i]=0;g.energy=3;
 game_play_card(&g,0,0);CHECK(g.screen==CARD_CHOICE);
 game_action(&g,BTN_OK);
 CHECK(g.screen==COMBAT&&g.exhaust_count>0&&g.draw_count==2&&g.hand_count==3);
 start(&g,45);g.hand_count=3;g.hand[0]=17|UPBIT;g.hand[1]=0;g.hand[2]=1;
 g.energy=3;game_play_card(&g,0,0);CHECK(g.screen==CARD_CHOICE);
 game_action(&g,BTN_OK);CHECK(g.exhaust_count>0&&g.block==9);
 start(&g,46);g.hand_count=2;g.hand[0]=36;g.hand[1]=0;g.energy=4;
 game_play_card(&g,0,0);CHECK(g.enemies[0].hp==242);
 g.hand_count=2;g.hand[0]=33;g.hand[1]=0;g.energy=3;
 int oldblock=g.block;game_play_card(&g,0,0);game_play_card(&g,0,0);
 CHECK(g.block==oldblock+3&&g.shop_relic==2);
 g.hand_count=1;g.hand[0]=36;game_play_card(&g,0,0);
 CHECK(g.enemies[0].hp==242-6-8);
 start(&g,47);g.hand_count=1;g.hand[0]=41;g.energy=3;
 game_play_card(&g,0,0);CHECK(g.shop_cards[0]==1&&g.exhaust_count>=1);
 g.hand_count=1;g.hand[0]=29;hp=g.hp;game_play_card(&g,0,0);
 CHECK(g.hp==hp-2&&g.strength==1);
 start(&g,48);g.hand_count=1;g.hand[0]=24;g.energy=3;
 game_play_card(&g,0,0);CHECK(g.block==12&&g.shop_cards[1]==4);
 g.enemies[0].intent=g.enemies[1].intent=0;
 g.enemies[0].intent_value=g.enemies[1].intent_value=1;
 game_end_turn(&g);CHECK(g.enemies[0].hp==246&&g.enemies[1].hp==246);
 start(&g,49);g.hand_count=2;g.hand[0]=26;g.hand[1]=0;g.energy=3;
 g.draw_count=2;g.draw[0]=0;g.draw[1]=1;
 game_play_card(&g,0,0);CHECK(g.screen==CARD_CHOICE);
 int count=g.draw_count;game_action(&g,BTN_OK);
 CHECK(g.screen==COMBAT&&g.draw_count==count+1);
 puts("PASS: save/load mid-selection, Brand, Armaments, Burning Pact, True Grit, War Cry, Flame Barrier, Rupture, Rage and Conflagration");
 return 0;
}
