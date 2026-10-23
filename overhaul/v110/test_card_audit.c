#include "game.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"AUDIT FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void start(Game*g,unsigned seed){
 game_new(g,seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=250;
 g->enemies[0].block=0;g->enemies[0].weak=0;
 g->enemies[0].vulnerable=0;
}
int main(void){
 Game g;start(&g,0x40404040u);
 C(game_card_cost(21)==1);
 C(game_card_cost(21|UPBIT)==0);
 C(cards[4].type==1 && cards[6].type==1 && cards[18].type==1 &&
   cards[34].type==1 && cards[38].type==1 && cards[43].type==1);
 C(strstr(card_description(11|UPBIT),"10")!=0);
 C(strstr(card_description(11|UPBIT),"2")!=0);
 C(strstr(card_description(21|UPBIT),"Cost 0")!=0);
 C(strstr(card_description(23|UPBIT),"5 Block")!=0);
 C(strstr(card_description(34),"Apply 1 Vulnerable")!=0);
 g.hand_count=1;g.hand[0]=21|UPBIT;g.energy=0;g.block=17;
 game_play_card(&g,0,0);
 C(g.enemies[0].hp==233 && g.energy==0);
 start(&g,0x40404041u);
 g.hand_count=1;g.hand[0]=11|UPBIT;
 g.draw_count=2;g.draw[0]=0;g.draw[1]=1;
 g.energy=3;
 game_play_card(&g,0,0);
 C(g.enemies[0].hp==240 && g.hand_count==2);
 start(&g,0x40404042u);
 g.hand_count=1;g.hand[0]=4;g.energy=3;
 g.draw_count=6;for(int i=0;i<6;i++)g.draw[i]=0;
 game_play_card(&g,0,0);
 C(g.hand_count==3 && g.reward_potion==1 && g.draw_count==3);
 g.hand_count=1;g.hand[0]=5;
 game_play_card(&g,0,0);
 C(g.hand_count==0 && g.draw_count==3);
 start(&g,0x40404043u);
 g.hand_count=1;g.hand[0]=34;g.energy=3;
 g.enemies[0].vulnerable=2;
 game_play_card(&g,0,0);
 C(g.enemies[0].vulnerable==3 && g.strength==3 && g.exhaust_count==1);
 start(&g,0x40404044u);
 g.hand_count=3;g.hand[0]=23|UPBIT;g.hand[1]=0;g.hand[2]=1;
 g.energy=3;
 game_play_card(&g,0,0);
 C(g.block==5 && CARD_UP(g.hand[0]) && CARD_UP(g.hand[1]));
 puts("PASS: audit corrections - cost, exact upgrade effects, no-draw, Vulnerable, Exhaust, Power/Skill types");
 return 0;
}
