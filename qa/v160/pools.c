#include "source/game.c"
#include <assert.h>
int main(void){Game g;int seen[REGENT_COUNT]={0},count=0;for(int mode=0;mode<3;mode++)for(int seed=1;seed<=2000;seed++){
 game_new(&g,seed);game_choose_character(&g,CHAR_REGENT);game_start_battle(&g,mode==0?NODE_FIGHT:mode==1?NODE_ELITE:NODE_BOSS);for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;game_end_turn(&g);if(g.screen==RELIC_RESULT)game_action(&g,BTN_BACK);assert(g.screen==REWARD);
 for(int i=0;i<3;i++){int id=CARD_ID(g.reward[i]);assert(id>=REGENT_FIRST&&id<REGENT_LAST&&cards[id].rarity<3);seen[id-REGENT_FIRST]++;if(mode==0)assert(cards[id].rarity<2);if(mode==2)assert(cards[id].rarity==2);count++;}
 }
 int eligible=0;for(int id=REGENT_FIRST;id<REGENT_LAST;id++)if(solo_reward_eligible(id)){assert(seen[id-REGENT_FIRST]>0);eligible++;}
 for(int act=2;act<=3;act++)for(int option=0;option<2;option++){game_new(&g,1);game_choose_character(&g,CHAR_REGENT);g.act=act;offer_blessing(&g);assert(game_validate(&g));g.selection=option;game_action(&g,BTN_OK);int expected=option?R_THE_SEALED_THRONE:R_METEOR_SHOWER,found=0;for(int i=0;i<g.deck_count;i++)found+=CARD_ID(g.deck[i])==expected;assert(found==1);}
 printf("PASS %d Proxy reward draws; all %d eligible Regent cards reached (including five labeled solo adaptations); normal fights exclude rares; bosses only rares; both Ancient gifts obtainable\n",count,eligible);return 0;}
