#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../source/game.c"
static Game fixture(void){Game g;game_new(&g,123);game_choose_character(&g,CHAR_NIGHTSHADE);game_start_battle(&g,NODE_FIGHT);g.enemy_count=2;for(int i=0;i<2;i++){g.enemies[i].hp=g.enemies[i].maxhp=100000;g.enemies[i].block=g.enemies[i].weak=g.enemies[i].vulnerable=g.enemies[i].poison=g.enemies[i].strength=0;g.enemies[i].intent=1;g.enemies[i].intent_value=0;}g.hand_count=g.discard_count=g.exhaust_count=g.draw_count=0;g.energy=50;g.hp=g.maxhp=500;g.strength=g.dexterity=g.block=0;memset(g.ic,0,sizeof(g.ic));memset(&g.ninja,0,sizeof(g.ninja));g.ic[IC_OWN_TURN]=1;g.relic_count=0;return g;}
static void play(Game*g,int raw){g->hand[g->hand_count++]=raw;game_play_card(g,g->hand_count-1,0);}
static void choices(Game*g){for(int steps=0;g->screen==NINJA_CHOICE&&steps<100;steps++){int n=0;while(n<g->hand_count&&!game_ninja_choice_eligible(g,n))n++;assert(n<g->hand_count);game_action(g,BTN_OK);}assert(g->screen!=NINJA_CHOICE);}
int main(void){
 assert(CARD_COUNT==185&&RELIC_COUNT==63);
 Game g=fixture();assert(g.deck_count==12&&g.deck[10]==N_NEUTRALIZE&&g.deck[11]==N_SURVIVOR);
 g=fixture();play(&g,N_BLADE_DANCE);assert(g.hand_count==3&&g.exhaust_count==1);play(&g,N_ACCURACY);game_play_card(&g,0,0);assert(g.enemies[0].hp==99992);
 g=fixture();g.hand[g.hand_count++]=N_TACTICIAN;play(&g,N_SURVIVOR);assert(g.screen==NINJA_CHOICE&&g.energy==49&&g.block==8);game_action(&g,BTN_OK);assert(g.screen==COMBAT&&g.energy==50&&g.discard_count==2);
 g=fixture();g.hand[g.hand_count++]=N_REFLEX;g.draw[g.draw_count++]=N_SLICE;g.draw[g.draw_count++]=N_STRIKE_SILENT;play(&g,N_PREPARED);assert(g.screen==NINJA_CHOICE);game_action(&g,BTN_OK);assert(g.ninja.v[NS_DISCARDS_TURN]==1&&g.hand_count>=2);
 for(int id=NINJA_FIRST;id<NINJA_LAST;id++)for(int up=0;up<2;up++){
  if(id==N_FLANKING||id==N_SNEAKY)continue;
  g=fixture();for(int i=0;i<3;i++)g.hand[g.hand_count++]=N_DEFEND_SILENT;
  for(int i=0;i<5;i++)g.draw[g.draw_count++]=N_STRIKE_SILENT;
  if(id==N_GRAND_FINALE)g.draw_count=0;
  g.enemies[0].poison=8;
  int raw=id|(up?UPBIT:0);CardPreview pv;g.hand[g.hand_count++]=raw;Game copy=g;game_card_preview(&g,raw,0,&pv);assert(!memcmp(&copy,&g,sizeof(g)));
  game_play_card(&g,g.hand_count-1,0);choices(&g);assert(g.screen==COMBAT);assert(g.ninja.task_count==0);assert(game_validate(&g));
 }
 printf("PASS 174 base/upgraded solo card resolutions and core Sly/Shiv interactions\n");return 0;
}
