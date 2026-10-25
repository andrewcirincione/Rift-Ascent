#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void boss_flow(int act){
 Game g;game_new(&g,0x12340000u+(unsigned)act);g.act=act;
 game_start_battle(&g,NODE_BOSS);
 assert(g.screen==COMBAT);
 assert(g.enemy_count>0);
 g.enemies[0].hp=1;g.enemies[0].block=0;
 for(int i=1;i<g.enemy_count;i++)g.enemies[i].hp=0;
 g.hand_count=1;g.hand[0]=0;g.energy=4;g.selection=0;
 int before_relic=g.relic_count;
 game_play_card(&g,0,0);
 assert(g.screen==RELIC_RESULT); /* random relic is automatically granted */
 assert(g.relic_count>=before_relic);
 int reward_relic=g.reward[0];
 if(reward_relic>=0)assert(g.relic_count==before_relic+1);
 assert(g.win_screen==(act==3?2:1));
 game_action(&g,BTN_OK);
 assert(g.screen==REWARD);assert(game_validate(&g));
 g.selection=3;game_action(&g,BTN_OK); /* Skip card; still progresses */
 assert((act==3&&g.screen==VICTORY)||
        (act!=3&&g.screen==MAP&&g.act==act+1));
}
int main(void){
 Game g;game_new(&g,0xabcddcba);
 g.screen=SHOP;g.selection=4;g.gold=70;
 int old=g.deck_count;
 game_action(&g,BTN_OK);
 assert(g.screen==DECK_VIEW && g.gold==70 && g.prior_screen==SHOP);
 game_action(&g,BTN_BACK);
 assert(g.screen==SHOP && g.gold==70 && g.deck_count==old);
 g.selection=4;game_action(&g,BTN_OK);
 g.selection=0;game_action(&g,BTN_OK);
 assert(g.screen==SHOP && g.gold==10 && g.deck_count==old-1);
 boss_flow(1);boss_flow(2);boss_flow(3);
 assert(strstr(card_description(69|UPBIT),"Cost")==NULL);
 printf("PASS v1.2.0: cancel-safe shop removal; one-time charge; automatic boss relic and card reward in all 3 acts\n");
 return 0;
}
