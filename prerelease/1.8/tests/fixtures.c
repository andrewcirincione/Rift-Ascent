/* Test-only saved-game fixtures. Never compiled into the release executable. */
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#ifdef LAYOUT_ONLY
#define F(x) printf("\"%s\":%zu,",#x,offsetof(Game,x))
#define P(x) printf("\"profile_%s\":%zu,",#x,offsetof(A18Profile,x))
int main(void){printf("{");F(screen);F(selection);F(character);F(act);F(floor);F(endless);F(hand_count);F(hand);F(enemies);F(energy);F(a18);P(wins);P(act4_clears);P(best_acts);P(true_endings);printf("\"size\":%zu,\"profile_size\":%zu}",sizeof(Game),sizeof(A18Profile));return 0;}
#elif defined(ATTACK_IDS_ONLY)
int main(void){int first=1;printf("[");for(int i=0;i<CARD_COUNT;i++)if(cards[i].type==0&&cards[i].cost!=-2){printf("%s%d",first?"":",",i);first=0;}puts("]");return 0;}
#else
static void win3(Game*g){g->act=3;g->floor=18;game_start_battle(g,NODE_BOSS);g->energy=100;g->hand_count=1;g->hand[0]=0;g->enemies[0].block=0;g->enemies[0].hp=1;game_play_card(g,0,0);assert(g->screen==A18_ENDING);a18_checkpoint(g);}
int main(int argc,char**argv){if(argc<4)return 2;const char*root=argv[1],*mode=argv[2];int c=atoi(argv[3]);Game g;assert(a18_storage_open(root));
 if(!strcmp(mode,"tenth")||!strcmp(mode,"boss4")||!strcmp(mode,"archive")){
  int chars=!strcmp(mode,"archive")?3:1;
  for(int k=0;k<chars;k++){int ch=chars==3?k:c;int n=!strcmp(mode,"tenth")?9:10;for(int i=0;i<n;i++){assert(a18_begin_run(&g,0,ch,1900+ch*33+i));win3(&g);}}
 }
 assert(a18_begin_run(&g,0,c,180041+c));
 if(!strcmp(mode,"archive")){g.screen=A18_COLLECTION;g.selection=0;g.a18.collection_character=c;}
 else if(!strcmp(mode,"boss4")){g.act=4;g.floor=18;g.permanent_strength=999;game_story_start(&g,1);assert(g.screen==A18_BOSS_INTRO);}
 else{g.act=!strcmp(mode,"tenth")?3:1;g.floor=!strcmp(mode,"tenth")?18:4;game_start_battle(&g,!strcmp(mode,"tenth")?NODE_BOSS:NODE_FIGHT);g.hand_count=5;g.hand[0]=0;g.hand[1]=334+c*20;g.hand[2]=335+c*20;g.hand[3]=336+c*20;g.hand[4]=337+c*20;g.selection=0;g.energy=10;g.a18.available_cards=A18_CARD_MASK;
  if(!strcmp(mode,"tenth")){g.enemies[0].hp=1;g.enemies[0].block=0;}
  else{g.enemy_count=2;g.enemies[1]=g.enemies[0];g.enemies[1].art=9;strcpy(g.enemies[1].name,"Cinder Hound");g.enemies[1].hp=g.enemies[1].maxhp=50;g.enemies[1].block=7;g.block=12;}
 }
 assert(game_validate(&g));assert(a18_checkpoint(&g));printf("FIXTURE %s char%d screen%d\n",mode,c,g.screen);return 0;}
#endif
