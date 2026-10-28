#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#ifdef LAYOUT_ONLY
#define F(x) printf("\"%s\":%zu,",#x,offsetof(Game,x))
int main(void){printf("{");F(screen);F(selection);F(act);F(hp);F(maxhp);F(enemy_count);F(kill_count);printf("\"size\":%zu}",sizeof(Game));return 0;}
#else
int main(int argc,char**argv){assert(argc==2);Game g;assert(a18_storage_open(argv[1]));
 A18Profile*p=(A18Profile*)a18_profile();p->wins[0]=10;p->wins[1]=12;p->wins[2]=1;p->best_acts[0]=4;p->best_acts[1]=8;p->best_acts[2]=3;p->true_endings=3;
 assert(a18_begin_run(&g,0,0,9801));g.act=3;g.floor=5;game_start_battle(&g,NODE_ELITE);g.enemy_count=2;g.hp=g.maxhp=500;
 g.enemies[0].art=33;g.enemies[1]=g.enemies[0];g.enemies[1].art=35;strcpy(g.enemies[0].name,"Infernal Colossus");strcpy(g.enemies[1].name,"Rift Behemoth");g.selection=g.hand_count;assert(a18_checkpoint(&g));
 assert(a18_begin_run(&g,1,1,9810));g.hp=30;g.gold=100;game_begin_event(&g,1);assert(a18_checkpoint(&g));
 assert(a18_begin_run(&g,2,2,9811));g.act=4;g.endless=1;g.floor=7;game_start_battle(&g,NODE_FIGHT);assert(a18_checkpoint(&g));
 assert(game_set_debug_enabled(0));puts("PASS review saves seeded; release executable untouched");return 0;}
#endif
