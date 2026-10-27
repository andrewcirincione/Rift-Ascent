#include "source/game.c"
#include <assert.h>
static unsigned long routes;
static void path(const Game*g,int row,int lane,int rests,int first,int last,int gap){
 if(g->map[row][lane]==NODE_REST){rests++;if(first<0)first=row;last=row;gap=0;}else gap++;
 assert(gap<=6);
 if(row==MAP_ROWS-1){assert(rests>=3&&rests<=4);assert(first<=5&&last==17);routes++;return;}
 unsigned mask=game_route_mask(g,row,lane);assert(mask);
 for(int j=0;j<3;j++)if(mask&(1u<<j))path(g,row+1,j,rests,first,last,gap);
}
int main(void){Game g;unsigned event_first[EVENT_COUNT]={0},blessings[64]={0};int mod_first=0;
 for(int seed=1;seed<=2000;seed++)for(int act=1;act<=4;act++){
  game_new(&g,(unsigned)seed*8191u);g.act=act;g.endless=act>3;map_generate(&g);
  for(int lane=0;lane<3;lane++)if(game_route_row_mask(&g,0)&(1u<<lane))path(&g,0,lane,0,-1,-1,0);
 }
 for(int seed=1;seed<=3000;seed++){
  game_new(&g,(unsigned)seed*9973u);int seen=0,last=-1;
  for(int i=0;i<34;i++){
   int id=game_next_event(&g);assert(id>=0&&id<EVENT_COUNT);assert(!(last>=12&&last<=14&&id>=12&&id<=14));seen|=1<<id;
   if(!i){event_first[id]++;mod_first+=id>=12&&id<=14;}
   last=id;
  }
  assert(seen==((1<<EVENT_COUNT)-1));int bag=g.v151.relic[R162_EVENT_SEEN],prev=g.v151.relic[R162_EVENT_LAST];
  game_start_battle(&g,NODE_FIGHT);assert(g.v151.relic[R162_EVENT_SEEN]==bag&&g.v151.relic[R162_EVENT_LAST]==prev);
  for(int c=0;c<3;c++){game_new(&g,(unsigned)seed*4919u);game_choose_character(&g,c);g.act=1+seed%4;g.endless=g.act>3;offer_blessing(&g);assert(game_validate(&g));
   for(int i=0;i<3;i++){int id=g.reward[i];for(int j=0;j<i;j++)assert(id!=g.reward[j]);if(id<1000)blessings[id]++;}
   Game before=g;for(int i=0;i<3;i++){g=before;g.selection=i;game_action(&g,BTN_OK);assert(g.screen==MAP&&game_validate(&g));}
  }
 }
 for(int i=0;i<EVENT_COUNT;i++)assert(event_first[i]>70);
 assert(mod_first>300&&mod_first<750);
 int distinct=0;for(int i=0;i<RELIC_COUNT;i++)distinct+=blessings[i]>0;assert(distinct>=50);
 /* All independent reward combinations, including a full belt and skips. */
 for(int card=0;card<2;card++)for(int pot=0;pot<2;pot++)for(int order=0;order<2;order++){
  game_new(&g,1622);game_choose_character(&g,0);g.screen=REWARD;g.reward[0]=5;g.reward[1]=7;g.reward[2]=8;g.v151.potion_offer=2;g.potion_count=0;
  int n=g.deck_count;for(int step=0;step<2;step++){int which=step^order;if((which?pot:card)){g.selection=which?4:0;game_action(&g,BTN_OK);assert(g.screen==REWARD);}}
  g.selection=5;game_action(&g,BTN_OK);assert(g.screen==MAP&&g.deck_count==n+card&&g.potion_count==pot);
 }
 game_new(&g,162);game_choose_character(&g,0);g.screen=REWARD;g.reward[0]=5;g.reward[1]=7;g.reward[2]=8;g.v151.potion_offer=2;g.selection=4;g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;Game saved=g;game_action(&g,BTN_OK);assert(g.potion_count==3&&g.v151.potion_offer==2&&g.screen==REWARD);assert(!memcmp(g.potions,saved.potions,sizeof(g.potions)));
 int open=0,tab=0,scroll=0,potion=0;game_inventory_action(&g,BTN_POTION,&open,&tab,&scroll,&potion);assert(open&&tab==3);game_inventory_action(&g,BTN_POTION,&open,&tab,&scroll,&potion);assert(g.potion_count==2);game_inventory_action(&g,BTN_BACK,&open,&tab,&scroll,&potion);game_action(&g,BTN_OK);assert(g.potion_count==3&&g.v151.potion_offer<0&&g.screen==REWARD);
 /* Inventory grid navigation reaches a short last row from each column. */
 g.screen=MAP;g.deck_count=7;open=1;tab=5;scroll=2;game_inventory_action(&g,BTN_DOWN,&open,&tab,&scroll,&potion);assert(scroll==5);game_inventory_action(&g,BTN_DOWN,&open,&tab,&scroll,&potion);assert(scroll==6);game_inventory_action(&g,BTN_TAB_RIGHT,&open,&tab,&scroll,&potion);assert(tab==0);game_inventory_action(&g,BTN_TAB_LEFT,&open,&tab,&scroll,&potion);assert(tab==5);
 assert(game_save(&g,"qa/v162/evidence/logic-roundtrip.dat"));Game loaded;assert(game_load(&loaded,"qa/v162/evidence/logic-roundtrip.dat"));assert(!memcmp(&loaded,&g,sizeof(g)));
 printf("PASS %lu complete paths across 8000 act maps: 3-4 camps, first by floor 6, maximum 6 intervening rooms\n",routes);
 printf("PASS 3000 event bags: all 17 events, no consecutive modification rooms; first modification room %d/3000\n",mod_first);
 printf("PASS %d distinct act-start relics, 27000 validated selections; independent rewards, full belt, grid controls and save round-trip\n",distinct);
 return 0;
}
