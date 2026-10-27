#include "source/game.c"
#include <assert.h>
static void rich(Game*g,int id){game_new(g,150+id);g->hp=50;g->gold=500;g->deck[g->deck_count++]=9;g->deck[g->deck_count++]=54;game_begin_event(g,id);}
int main(void){Game g,b,h;int outcomes=0;
 for(int id=0;id<EVENT_COUNT;id++)for(int option=0;option<3;option++){
  rich(&g,id);g.selection=option;b=g;assert(game_validate(&g));assert(game_save(&g,"qa/v150/evidence/event-save.dat"));assert(game_load(&h,"qa/v150/evidence/event-save.dat"));assert(!memcmp(&g,&h,sizeof(g)));
  game_action(&g,BTN_OK);
  if(option==2){assert(g.screen==MAP&&g.hp==b.hp&&g.gold==b.gold&&g.deck_count==b.deck_count);continue;}
  assert(g.screen!=EVENT);
  if(g.screen==EVENT_CARD){
   int saved=g.selection;assert(g.gold==b.gold&&g.hp==b.hp&&g.maxhp==b.maxhp);
   assert(game_save(&g,"qa/v150/evidence/event-save.dat"));assert(game_load(&h,"qa/v150/evidence/event-save.dat"));assert(!memcmp(&g,&h,sizeof(g)));
   game_action(&g,BTN_BACK);assert(g.screen==EVENT&&g.selection==option&&g.gold==b.gold&&g.hp==b.hp);
   game_action(&g,BTN_OK);assert(g.selection==saved);int raw=g.deck[g.selection],idx=g.selection;game_action(&g,BTN_OK);assert(g.screen==EVENT_RESULT);
   const EventOffer*o=&event_offers[id][option];assert(g.gold==b.gold-o->gold&&g.hp==b.hp-o->hp);
   if(o->action<MOD_COUNT){assert(g.deck[idx]==(raw|(o->action<<CARD_MOD_SHIFT)));assert(g.deck_count==b.deck_count);}
   if(o->action==EVENT_UPGRADE)assert(g.deck[idx]==(raw|UPBIT));
   if(o->action==EVENT_REMOVE)assert(g.deck_count==b.deck_count-1);
   if(o->action==EVENT_COPY)assert(g.deck_count==b.deck_count+1&&g.deck[g.deck_count-1]==raw);
  }
  assert(game_validate(&g));game_action(&g,BTN_OK);assert(g.screen==MAP);outcomes++;
 }
 // Picker transaction must recheck eligibility/cost; invalid card cannot eat resources.
 for(int id=12;id<=14;id++)for(int option=0;option<2;option++){
  rich(&g,id);g.selection=option;game_action(&g,BTN_OK);assert(g.screen==EVENT_CARD);
  g.deck[g.selection]|=MOD_BOUND<<CARD_MOD_SHIFT;b=g;game_action(&g,BTN_OK);assert(g.screen==EVENT_CARD&&g.hp==b.hp&&g.gold==b.gold&&!memcmp(g.deck,b.deck,sizeof(g.deck)));
  g.deck[g.selection]&=~CARD_MOD_MASK;g.gold=0;g.hp=1;b=g;game_action(&g,BTN_OK);assert(g.hp==b.hp&&g.gold==0);
 }
 for(int id=0;id<EVENT_COUNT;id++)for(int option=0;option<2;option++){
  rich(&g,id);g.gold=0;g.hp=1;g.maxhp=25;g.potion_count=3;g.deck_count=MAX_DECK;for(int i=0;i<MAX_DECK;i++)g.deck[i]=0;
  if(!game_event_choice_available(&g,option)){b=g;g.selection=option;game_action(&g,BTN_OK);assert(g.screen==EVENT&&g.gold==b.gold&&g.hp==b.hp&&g.maxhp==b.maxhp&&g.deck_count==b.deck_count&&g.potion_count==b.potion_count);}
 }
 // Known choices, not post-payment random rolls. Auction alternatives distinct and saved.
 for(int seed=1;seed<=2000;seed++){
  game_new(&g,seed);g.gold=500;g.hp=50;game_begin_event(&g,15);assert(g.reward[0]!=g.reward[1]);int r=g.reward[seed%2];g.selection=seed%2;b=g;game_action(&g,BTN_OK);assert(relic(&g,r)&&g.relic_count==b.relic_count+1);assert(g.gold==b.gold-(seed%2?0:95));assert(g.maxhp==b.maxhp-(seed%2?10:0)+(r==4?14:r==11?-12:r==31?12:0));
  rich(&g,2);assert(cards[CARD_ID(g.reward[0])].rarity==1&&CARD_UP(g.reward[0]));assert(cards[CARD_ID(g.reward[1])].rarity==0&&!CARD_UP(g.reward[1]));
  rich(&g,6);assert(cards[g.reward[0]].type==0&&cards[g.reward[1]].type==1&&cards[g.reward[0]].rarity==1&&cards[g.reward[1]].rarity==1);
 }
 rich(&g,16);g.deck[0]=0|UPBIT|(MOD_KEEN<<CARD_MOD_SHIFT);g.selection=1;game_action(&g,BTN_OK);g.selection=0;game_action(&g,BTN_OK);assert(g.deck[g.deck_count-1]==g.deck[0]);
 rich(&g,1);g.gold=9;g.scroll=20;game_action(&g,BTN_OK);assert(g.hp==54&&g.gold==1);
 rich(&g,1);g.potion_count=MAX_POTIONS;b=g;g.selection=1;game_action(&g,BTN_OK);assert(g.gold==b.gold&&g.potion_count==MAX_POTIONS&&g.screen==EVENT);
 rich(&g,1);g.potion_count=2;g.potions[0]=1;g.potions[1]=5;g.selection=1;game_action(&g,BTN_OK);assert(g.potion_count==3&&g.potions[0]==1&&g.potions[1]==5&&g.potions[2]==0);
 printf("PASS %d committed offers across all 17 rooms; cancel/no-charge, insufficient resources, deck/belt caps, 2,000 named relic alternatives and rarity offers, modified copying, partial healing\n",outcomes);
}
