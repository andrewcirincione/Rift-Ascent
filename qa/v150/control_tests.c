/* Compiled by audit_controls.py with the release PSP button decoder itself. */
#include "source/game.c"
#include <assert.h>
static void setup(Game*g,int mod){
 game_new(g,150+mod);g->gold=500;g->hp=50;
 game_begin_event(g,12+(mod-1)/2);g->selection=(mod-1)%2;
}
static void possessions(const Game*a,const Game*b){
 assert(a->gold==b->gold&&a->hp==b->hp&&a->maxhp==b->maxhp);
 assert(a->deck_count==b->deck_count&&!memcmp(a->deck,b->deck,sizeof(a->deck)));
 assert(a->potion_count==b->potion_count&&!memcmp(a->potions,b->potions,sizeof(a->potions)));
 assert(a->relic_count==b->relic_count&&!memcmp(a->relics,b->relics,sizeof(a->relics)));
}
static void check_transaction(const Game*g,const Game*original,int mod,int committed){
 int changes=0;for(int i=0;i<original->deck_count;i++)if(g->deck[i]!=original->deck[i]){
  assert(g->deck[i]==(original->deck[i]|(mod<<CARD_MOD_SHIFT)));changes++;
 }
 assert(changes==committed&&g->deck_count==original->deck_count);
 const EventOffer*offer=&event_offers[12+(mod-1)/2][(mod-1)%2];
 assert(g->gold==original->gold-committed*offer->gold&&g->hp==original->hp-committed*offer->hp);
 assert(g->potion_count==original->potion_count&&g->relic_count==original->relic_count);
}
static void roundtrip(Game*g){
 Game snapshot=*g,loaded;if(snapshot.screen==PAUSE_MENU)snapshot.screen=game_pause_source();
 assert(game_save(&snapshot,"qa/v150/evidence/control-save.dat"));
 assert(game_load(&loaded,"qa/v150/evidence/control-save.dat"));
 assert(!memcmp(&snapshot,&loaded,sizeof(snapshot)));*g=loaded;
}
int main(void){
 const unsigned physical[]={PSP_CTRL_CROSS,PSP_CTRL_CIRCLE,PSP_CTRL_SQUARE,PSP_CTRL_TRIANGLE,PSP_CTRL_LEFT,PSP_CTRL_RIGHT,PSP_CTRL_UP,PSP_CTRL_DOWN,PSP_CTRL_SELECT,PSP_CTRL_START,PSP_CTRL_LTRIGGER,PSP_CTRL_RTRIGGER};
 int combinations=0,sequences=0,saves=0;Game original,base,g;
 for(int mod=1;mod<MOD_COUNT;mod++)for(int stage=0;stage<3;stage++){
  setup(&original,mod);base=original;if(stage)game_action(&base,BTN_OK);if(stage==2)game_action(&base,BTN_OK);
  for(int mask=0;mask<4096;mask++){
   unsigned down=0;for(int bit=0;bit<12;bit++)if(mask&(1<<bit))down|=physical[bit];
   g=base;int key=button(down);game_action(&g,key);assert(game_validate(&g));
   if(stage==1&&key==BTN_OK){assert(g.screen==EVENT_RESULT);check_transaction(&g,&original,mod,1);}
   else possessions(&g,&base);
   if(g.screen==PAUSE_MENU){assert(game_pause_source()==base.screen);game_action(&g,BTN_BACK);assert(g.screen==base.screen);possessions(&g,&base);}
   combinations++;
  }
 }
 for(int mod=1;mod<MOD_COUNT;mod++)for(int seed=1;seed<=1000;seed++){
  setup(&g,mod);original=g;int committed=0,committed_mod=mod;unsigned r=(unsigned)seed*2654435761u;
  for(int step=0;step<64&&g.screen!=MAP;step++){
   r^=r<<13;r^=r>>17;r^=r<<5;int key=button(physical[r%12]);
   int before=g.screen;game_action(&g,key);
   if(before==EVENT_CARD&&g.screen==EVENT_RESULT){committed++;committed_mod=g.next_screen;}
   assert(committed<=1);check_transaction(&g,&original,committed_mod,committed);
   if(g.screen==PAUSE_MENU){if(r&1){roundtrip(&g);saves++;}else game_action(&g,BTN_PAUSE);}
   if((r%7)==0&&g.screen!=MAP){roundtrip(&g);saves++;}
   assert(game_validate(&g));
  }
  sequences++;
 }
 // Save on every transaction boundary, then try to confirm it repeatedly.
 for(int mod=1;mod<MOD_COUNT;mod++){
  setup(&g,mod);original=g;roundtrip(&g);game_action(&g,BTN_OK);roundtrip(&g);
  game_action(&g,BTN_PAUSE);roundtrip(&g);assert(g.screen==EVENT_CARD);
  game_action(&g,BTN_BACK);possessions(&g,&original);game_action(&g,BTN_OK);
  game_action(&g,BTN_OK);check_transaction(&g,&original,mod,1);roundtrip(&g);
  game_action(&g,BTN_ALT);game_action(&g,BTN_POTION);game_action(&g,BTN_START);
  check_transaction(&g,&original,mod,1);game_action(&g,BTN_OK);assert(g.screen==MAP);
  check_transaction(&g,&original,mod,1);
 }
 printf("PASS %d physical button combinations using the release decoder; %d menu/shortcut sequences; %d save/resume boundaries; one modification and one charge at most; no free changes, potion loss or wrong-card mutation\n",combinations,sequences,saves);
}
