#include <stdio.h>
#ifdef BALANCE_SOURCE
#include BALANCE_SOURCE
#else
#include "../v185/source/game.c"
#endif
int main(void){puts("seed,act,mode,enemy,art,hp,intent,damage,rng");for(int seed=1;seed<=20;seed++)for(int act=1;act<=7;act++){const int modes[]={NODE_FIGHT,NODE_ELITE,NODE_BOSS};for(int m=0;m<3;m++){Game g;game_new(&g,seed);g.act=act;g.floor=8;game_start_battle(&g,modes[m]);for(int i=0;i<g.enemy_count;i++){Enemy*e=&g.enemies[i];printf("%d,%d,%d,%d,%d,%d,%d,%d,%u\n",seed,act,m,i,e->art,e->maxhp,e->intent,game_enemy_intent_damage(&g,i),g.rng);}}}return 0;}
