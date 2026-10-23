#include "game.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"1.1.0 FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
int main(void){
 Game g;game_new(&g,0x11001100u);game_start_battle(&g,NODE_FIGHT);
 CHECK(g.screen==COMBAT);
 CHECK(cards[28].cost==1);
 CHECK(strstr(card_description(28),"6")!=0);
 CHECK(strstr(card_description(28|UPBIT),"9")!=0);
 g.enemy_count=2;
 for(int i=0;i<2;i++){
   g.enemies[i].hp=200;g.enemies[i].maxhp=200;
   g.enemies[i].block=0;g.enemies[i].weak=0;g.enemies[i].vulnerable=0;
 }
 g.hand_count=1;g.hand[0]=28;g.energy=3;g.hp=70;g.maxhp=100;
 game_play_card(&g,0,0);
 CHECK(g.hp==70);/* Casting the Power does not immediately cost HP. */
 CHECK(g.used_potion==((1<<16)|6));
 int before0=g.enemies[0].hp,before1=g.enemies[1].hp;
 g.hand_count=1;g.hand[0]=29;g.energy=3;g.target=0;
 game_play_card(&g,0,0); /* Hemokinesis: 2 self-HP, 15 targeted, 6 all */
 CHECK(g.hp==68);
 CHECK(g.enemies[0].hp==before0-21);
 CHECK(g.enemies[1].hp==before1-6);

 g.hand_count=1;g.hand[0]=28|UPBIT;g.energy=3;
 game_play_card(&g,0,0);
 CHECK(g.used_potion==((2<<16)|15));
 int hp=g.hp,e0=g.enemies[0].hp,e1=g.enemies[1].hp;
 g.enemies[0].vulnerable=3;g.enemies[1].block=7;
 g.weak=3;
 game_start_turn(&g); /* 2 copies: -2 HP and 2*15 AoE */
 CHECK(g.screen==COMBAT);
 CHECK(g.hp==hp-2);
 CHECK(g.enemies[0].hp==e0-30); /* no Weak or Vulnerable modifier */
 CHECK(g.enemies[1].hp==e1-23); /* 30 damage minus seven Block */

 CHECK(game_validate(&g));
 puts("PASS 1.1.0 Inferno: wiki text/cost, HP trigger, AoE, stacked copies, ignores modifiers, honors Block");
 return 0;
}
