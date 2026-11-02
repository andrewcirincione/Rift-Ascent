#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../../qa/v186/source/game.c"
int main(void){
 char root[]="/tmp/rift-distribution-integration-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;long offers=0;
 for(int c=0;c<3;c++)for(int a=1;a<=7;a++)for(int seed=0;seed<8;seed++)for(int id=0;id<EVENT_COUNT;id++){
  game_new(&g,3000+seed);game_choose_character(&g,c);g.act=a;g.endless=a>3;g.hp=180;g.maxhp=200;g.gold=999;g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;
  game_begin_event(&g,id);for(int option=0;option<game_event_choice_count(&g);option++)if(game_event_choice_available(&g,option)){
   h=g;h.selection=option;game_action(&h,BTN_OK);
   if(h.screen==EVENT_CARD){int found=-1;for(int i=0;i<h.deck_count;i++)if(game_event_card_eligible(&h,h.deck[i])){found=i;break;}assert(found>=0);h.selection=found;game_action(&h,BTN_OK);}
   assert(h.hp>0&&h.hp<=h.maxhp&&h.gold>=0&&h.deck_count<=MAX_DECK&&h.relic_count<=MAX_RELICS&&h.potion_count<=MAX_POTIONS);
   assert(h.screen!=EVENT_CARD);offers++;
  }
 }
 for(int a=1;a<=7;a++)for(int c=0;c<3;c++)for(int option=0;option<3;option++){
  game_new(&g,503+a);game_choose_character(&g,c);g.act=a;g.endless=a>3;g.screen=BETA_FORGE;g.expedition.gift_kind=2;g.selection=option;
  int open=0,tab=0,scroll=0,potion=0;assert(game_inventory_action(&g,BTN_ALT,&open,&tab,&scroll,&potion)==1&&open&&tab==5);assert(g.selection==option&&g.screen==BETA_FORGE);
  assert(game_inventory_action(&g,BTN_BACK,&open,&tab,&scroll,&potion)==1&&!open);
  game_action(&g,BTN_OK);assert(g.screen==BETA_FORGE_CARD);int at=g.selection,before=g.deck[at],expected=game_beta_preview(&g,option,before),hp=g.maxhp,cost=option==2?game_beta_echo_cost(&g):0;
  game_action(&g,BTN_OK);assert(g.screen==MAP&&g.deck[at]==expected&&g.maxhp==hp-cost);
 }
 for(int id=0;id<26;id++){
  game_new(&g,1);game_start_battle(&g,NODE_FIGHT);g.hp=40;g.maxhp=100;g.potion_count=1;g.potions[0]=id;g.regent.stars=4;g.relic_count=0;
  for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=g.enemies[e].maxhp=1000;
  game_use_potion(&g,0);assert(g.potion_count==0&&!g.message[0]&&g.hp>0&&g.hp<=g.maxhp&&g.regent.stars>=0&&g.regent.stars<=9);
 }
 printf("PASS %ld available story-event choices across all characters/Acts 1-7, 63 forge transactions with deck access, and all 26 potion consumption/notification paths.\n",offers);
 return 0;
}
