#include "game.h"
#include <stdio.h>
#define C(x) do{if(!(x)){fprintf(stderr,"1.0.2 regression FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 Game g;game_new(&g,0x10201020u);
 g.map[0][1]=NODE_SHOP;
 game_action(&g,BTN_OK);
 C(g.screen==MAP);
 game_choose_node(&g,1);
 C(g.screen==SHOP && g.floor==1);
 game_action(&g,BTN_BACK);C(g.screen==MAP);
 game_action(&g,BTN_UP);C(g.scroll==1);
 int floor=g.floor;
 game_action(&g,BTN_OK);C(g.floor==floor && g.screen==MAP);
 game_action(&g,BTN_DOWN);C(g.scroll==0);
 game_new(&g,0x10202020u);
 g.screen=DECK_VIEW;g.prior_screen=REST;g.selection=0;
 game_action(&g,BTN_DOWN);C(g.selection==3);
 game_action(&g,BTN_LEFT);C(g.selection==3);
 game_action(&g,BTN_RIGHT);C(g.selection==4);
 game_new(&g,0x10203030u);
 g.effect_turn=6;g.screen=EVENT;g.selection=0;
 int hp=g.hp,deck=g.deck_count;
 game_action(&g,BTN_OK);
 C(g.screen==EVENT_RESULT&&g.hp==hp-10&&g.deck_count==deck+2);
 C(CARD_ID(g.reward[0])<CARD_COUNT && CARD_ID(g.reward[1])<CARD_COUNT);
 game_action(&g,BTN_OK);C(g.screen==MAP);
 game_new(&g,0x10204040u);
 g.screen=BOSS_REWARD;g.reward[0]=1;g.reward[1]=2;g.reward[2]=3;
 int relics=g.relic_count;
 game_action(&g,BTN_OK);
 C(g.screen==RELIC_RESULT && g.relic_count==relics+1 && g.act==1);
 game_action(&g,BTN_OK);C(g.screen==REWARD && g.act==1);
 g.selection=3;game_action(&g,BTN_OK);C(g.screen==MAP && g.act==2 && g.floor==0);
 game_new(&g,0x10205050u);
 game_start_battle(&g,NODE_FIGHT);
 g.selection=1;int selected=g.selection;
 game_action(&g,BTN_ALT);
 C(g.screen==DECK_VIEW && g.prior_screen==COMBAT);
 C(g.scroll==0&&g.master_index==selected);
 game_action(&g,BTN_RIGHT);C(g.scroll==1);
 game_action(&g,BTN_RIGHT);C(g.scroll==2);
 game_action(&g,BTN_BACK);C(g.screen==COMBAT&&g.selection==selected);
 puts("PASS v1.0.2: shop stays open; preview-only routes; grid controls; armory details; boss relic confirmation; battle pile navigation");
 return 0;
}
