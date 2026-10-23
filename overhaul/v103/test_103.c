#include "game.h"
#include <stdio.h>
#include <string.h>
#define TEST(x) do{if(!(x)){fprintf(stderr,"1.0.3 FAIL %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
int main(void){
 Game g;
 game_new(&g,0x10310333u);
 game_action(&g,BTN_OK);
 TEST(g.screen==MAP);
 g.floor=1;g.scroll=0;
 for(int i=1;i<MAP_ROWS-g.floor;i++){
    game_action(&g,BTN_UP);
    TEST(g.scroll==i);
    int old_floor=g.floor;
    game_action(&g,BTN_OK); /* previews cannot advance the world */
    TEST(g.floor==old_floor&&g.screen==MAP);
 }
 game_action(&g,BTN_UP);
 TEST(g.scroll==MAP_ROWS-g.floor-1);
 for(int i=MAP_ROWS-g.floor-2;i>=0;i--){
    game_action(&g,BTN_DOWN);
    TEST(g.scroll==i);
 }
 TEST(g.scroll==0);
 game_new(&g,0x10310334u);
 game_start_battle(&g,NODE_FIGHT);
 TEST(g.screen==COMBAT);
 g.enemy_count=2;
 g.enemies[0].hp=16;g.enemies[1].hp=0;g.target=0;
 game_action(&g,BTN_DOWN);
 TEST(g.target==0);
 game_action(&g,BTN_UP);
 TEST(g.target==0);
 g.enemies[1].hp=18;g.enemies[0].hp=0;g.target=0;
 game_action(&g,BTN_DOWN);
 TEST(g.target==1);
 game_action(&g,BTN_UP);
 TEST(g.target==1);
 TEST(cards[10].cost==-1); /* real X-cost whirlwind */
 TEST(!strcmp(cards[0].name,"Strike"));
 TEST(!strcmp(cards[1].name,"Defend"));
 TEST(!strcmp(cards[2].name,"Bash"));
 TEST(!strcmp(cards[10].name,"Whirlwind"));
 TEST(!strcmp(cards[6].name,"Bloodletting"));
 TEST(cards[5].cost==1);
 TEST(cards[7].cost==3);
 TEST(cards[8].cost==1);
 TEST(cards[9].cost==2);
 TEST(strstr(card_description(2),"Vulnerable")!=0);
 g.screen=COMBAT;
 g.enemies[0].hp=40;g.enemies[1].hp=0;g.target=1;
 g.energy=3;g.hand_count=1;g.hand[0]=10;
 game_play_card(&g,0,g.target); /* dead target auto rerouted */
 TEST(g.energy==0);
 TEST(g.enemies[0].hp<40);
 puts("PASS 1.0.3 native simulation: full map preview, dead-target skipping, X costs and combat");
 return 0;
}
