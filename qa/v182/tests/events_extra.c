#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../source/game.c"
static void fresh(Game*g,int c){game_new(g,182+c);game_choose_character(g,c);g->relic_count=0;g->hp=60;g->maxhp=100;g->gold=250;g->potion_count=2;g->potions[0]=1;g->potions[1]=4;g->screen=MAP;}
int main(void){setbuf(stdout,NULL);char tmp[]="/tmp/rift-182-extra-XXXXXX";assert(mkdtemp(tmp)&&ascent_open(tmp));Game g,b;
 for(int c=0;c<3;c++)for(int id=24;id<39;id++)for(int choice=0;choice<3;choice++){
  fresh(&g,c);game_begin_event(&g,id);if(choice>=game_event_choice_count(&g)-1)continue;if(!game_event_choice_available(&g,choice))continue;g.selection=choice;b=g;game_action(&g,BTN_OK);
  if(g.screen==EVENT_CARD){assert(g.gold==b.gold&&g.hp==b.hp&&g.potion_count==b.potion_count);game_action(&g,BTN_BACK);assert(g.screen==EVENT&&g.selection==choice);game_action(&g,BTN_OK);game_action(&g,BTN_OK);}
  assert(g.screen==EVENT_RESULT||g.screen==RELIC_RESULT);assert(game_validate(&g));assert(g.hp>0&&g.gold>=0);
 }
 fresh(&g,0);game_begin_event(&g,33);do_event(&g,0);int gold=g.gold;assert(g.screen==EVENT_CARD&&g.potion_count==2);g.potion_count=0;event_card_commit(&g);assert(g.gold==gold&&g.screen==EVENT_CARD);g.potion_count=2;event_card_commit(&g);assert(g.screen==EVENT_RESULT&&g.gold==gold-20&&g.potion_count==1&&g.potions[0]==4&&CARD_UP(g.deck[0]));
 for(int c=0;c<3;c++){fresh(&g,c);game_begin_event(&g,37);g.deck[0]=g.deck[0]|UPBIT|(MOD_KEEN<<CARD_MOD_SHIFT);int old=CARD_ID(g.deck[0]),count=g.deck_count;do_event(&g,0);g.selection=0;event_card_commit(&g);assert(g.deck_count==count&&CARD_ID(g.deck[0])!=old&&!CARD_UP(g.deck[0])&&!CARD_MOD(g.deck[0])&&cards[g.deck[0]].rarity==1&&ninja_owned_card(&g,g.deck[0]));}
 puts("PASS 15 extra rooms for all heroes; costs committed once; potion upgrade cancel/revalidation; transformative card eligibility and removal of old metadata");
 for(int c=0;c<3;c++){
  assert(ascent_begin(&g,c,c,182));g.relic_count=0;g.hp=60;g.maxhp=100;g.gold=250;
  game_begin_event(&g,28);do_event(&g,0);game_begin_event(&g,29);do_event(&g,0);game_begin_event(&g,35);do_event(&g,1);game_begin_event(&g,38);do_event(&g,0);
  assert(g.revision182.next_combat[0]==5&&g.revision182.next_combat[1]==10&&g.revision182.next_combat[2]==2&&g.revision182.next_combat[3]==2&&g.revision182.next_combat[4]==2);assert(game_status_count(&g)>=5);
  assert(ascent_checkpoint(&g)&&ascent_resume(&b,c)&&!memcmp(g.revision182.next_combat,b.revision182.next_combat,20));g=b;
  game_start_battle(&g,NODE_FIGHT);assert(g.strength==5&&g.block==10&&g.hand_count==7&&g.weak==2&&g.vulnerable==2);for(int i=0;i<5;i++)assert(!g.revision182.next_combat[i]);
  assert(ascent_checkpoint(&g)&&ascent_resume(&b,c)&&b.strength==5&&!b.revision182.next_combat[0]);game_start_battle(&b,NODE_FIGHT);assert(b.strength==0&&b.block==0&&b.hand_count==5&&!b.weak&&!b.vulnerable);
 }
 fresh(&g,0);g.revision182.next_combat[0]=30;game_begin_event(&g,28);assert(!game_event_choice_available(&g,0));g.revision182.next_combat[0]=0;g.revision182.next_combat[4]=4;assert(!game_event_choice_available(&g,0));
 puts("PASS queued boons/debuffs in status, persistence before/after combat, exact opening stats/cards, single consumption and capacity gates");
 fresh(&g,0);uint64_t visited=0;int seen_high=0;
 for(int i=0;i<5000;i++){int id=game_next_event(&g);if(id==EVENT_COUNT)continue;visited|=1ull<<id;seen_high|=g.revision182.event_seen_hi!=0;assert(g.revision182.event_seen_hi<128);assert(game_validate(&g));}
 assert(visited==((1ull<<39)-1)&&seen_high);assert(ascent_begin(&g,0,0,123));g.screen=MAP;g.revision182.event_seen_hi=127;g.v151.relic[R162_EVENT_SEEN]=(int)0xffffffffu;assert(ascent_checkpoint(&g)&&ascent_resume(&b,0)&&b.revision182.event_seen_hi==127&&(unsigned)b.v151.relic[R162_EVENT_SEEN]==0xffffffffu);
 for(int i=0;i<100;i++)game_next_event(&b);assert(b.revision182.event_seen_hi<=127);
 puts("PASS all 39 rooms reached, safe 64-bit bag and high-bit save/reload/reset");
 for(int id=24;id<39;id++){fresh(&g,0);game_begin_event(&g,id);g.gold=0;g.hp=1;g.maxhp=25;g.potion_count=0;g.deck_count=MAX_DECK;for(int i=0;i<MAX_DECK;i++)g.deck[i]=0;for(int choice=0;choice<game_event_choice_count(&g);choice++){b=g;b.selection=choice;game_action(&b,BTN_OK);assert(b.gold>=0&&b.hp>0&&b.maxhp>=25&&b.potion_count>=0&&b.potion_count<=MAX_POTIONS&&b.deck_count<=MAX_DECK);}}
 puts("PASS resource-poor/full-deck boundaries for every additional offer");return 0;}
