#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../v186/source/game.c"
int main(void){char root[]="/tmp/r186r2-fixtures-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g;for(int act=3;act<=7;act+=act==3?1:3){assert(ascent_begin(&g,0,0,18602));g.act=act;g.endless=act>3;g.floor=MAP_ROWS;g.ascent.secret=act==4;game_start_battle(&g,NODE_BOSS);g.relic_count=0;g.hp=g.maxhp=120;g.hand_count=3;g.hand[0]=g.hand[1]=g.hand[2]=0;g.energy=10;g.selection=g.target=0;for(int i=0;i<g.enemy_count;i++){g.enemies[i].hp=1;g.enemies[i].block=0;}assert(game_validate(&g));char path[120];snprintf(path,sizeof(path),"qa/v186r2/evidence/fixture-finish%d.bin",act);FILE*f=fopen(path,"wb");assert(f&&fwrite(&g,sizeof(g),1,f)==1);fclose(f);}puts("PASS generated isolated ending fixtures for Acts 3, 4, 7");}
