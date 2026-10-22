#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)

int main(void) {
 Game a,b;
 game_new(&a,0xCAFE4321u);
 CHECK(game_validate(&a));
 CHECK(a.screen==TITLE);
 game_action(&a,BTN_DOWN);
 CHECK(a.selection==1);
 game_action(&a,BTN_OK);
 CHECK(a.screen==HELP);
 game_action(&a,BTN_BACK);
 CHECK(a.screen==TITLE);
 game_action(&a,BTN_UP);
 game_action(&a,BTN_OK);
 CHECK(a.screen==MAP);
 for(int act=1;act<=3;act++){
  a.act=act;
  for(int r=0;r<MAP_ROWS-1;r++){
   for(int col=0;col<MAP_LANES;col++){
     unsigned mask=game_route_mask(&a,r,col);
     CHECK(mask!=0 && (mask&~7u)==0);
     for(int to=0;to<MAP_LANES;to++)
      if(mask&(1u<<to))CHECK(abs(col-to)<=1);
   }
  }
 }
 a.act=1; a.floor=2; a.lane=1; a.screen=MAP; a.selection=0;
 int legal=0,illegal=0;
 for(int lane=0;lane<3;lane++) {
  if(game_route_available(&a,lane))legal++;
  else illegal++;
 }
 CHECK(legal>=1 && legal<=2);
 for(int lane=0;lane<3;lane++)
  if(!game_route_available(&a,lane)){
   int floor=a.floor;
   game_choose_node(&a,lane);
   CHECK(a.floor==floor);
  }
 game_new(&b,0xAB351077u);
 game_start_battle(&b,NODE_FIGHT);
 CHECK(b.screen==COMBAT && b.enemy_count>0);
 b.hand_count=1;b.hand[0]=0;b.selection=0;b.target=0;
 int turn=b.turn;
 game_action(&b,BTN_RIGHT);
 CHECK(b.selection==b.hand_count);
 game_action(&b,BTN_OK);
 CHECK(b.screen!=COMBAT || b.turn>turn);
 game_new(&b,0xAABBCCDDu);
 game_start_battle(&b,NODE_FIGHT);
 b.strength=3;b.weak=0;b.enemies[0].vulnerable=0;b.turn_played=1;
 int normal=game_effective_damage(&b,0,0);
 CHECK(normal==9);
 b.enemies[0].vulnerable=2;
 CHECK(game_effective_damage(&b,0,0)==13);
 b.weak=2;
 CHECK(game_effective_damage(&b,0,0)==9);
 b.weak=0;
 CHECK(game_effective_damage(&b,0|UPBIT,0)==18);
 CHECK(game_effective_damage(&b,1,0)==0);
 b.hp=35;b.potion_count=2;b.potions[0]=0;b.potions[1]=1;
 game_use_potion(&b,0);
 CHECK(b.hp==53);
 CHECK(b.potion_count==1 && b.potions[0]==1);
 CHECK(strstr(game_intent_description(7),"Two")!=0);
 CHECK(strstr(game_intent_description(6),"Poison")!=0);
 CHECK(strstr(game_potion_description(0),"18")!=0);
 CHECK(!strcmp(game_potion_name(1),"Might"));
 game_new(&a,0x10203040u);
 CHECK(game_save(&a,"/tmp/rift_overhaul_save.bin")==1);
 CHECK(game_load(&b,"/tmp/rift_overhaul_save.bin")==1);
 CHECK(game_validate(&b)&&b.seed==a.seed);
 remove("/tmp/rift_overhaul_save.bin");
 puts("PASS: original save layout, 3-act branching, enemy intent glossary, potions, menu, END TURN, damage modifiers");
 return 0;
}
