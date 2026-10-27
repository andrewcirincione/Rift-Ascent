#include "source/game.c"
#include <assert.h>
#include <stdio.h>
static void baseline(Game*g){uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<3;c++){uint32_t h=route_hash(seed+(unsigned)(r*131+c*977));int n=NODE_FIGHT;if(r==1)n=(h%3)?NODE_EVENT:NODE_FIGHT;else if(r==3||r==9){int earlyshop=(int)(route_hash(seed+c*127u)&1u);n=((r==3)==earlyshop)?NODE_SHOP:((h&1)?NODE_EVENT:NODE_REST);}else if(r==5)n=(h%3==0)?NODE_EVENT:NODE_FIGHT;else if(r==7)n=NODE_TREASURE;else if(r==11)n=(h&1)?NODE_EVENT:NODE_REST;else if(r==13)n=NODE_REST;else if(r>=4&&(h%100)<(unsigned)(r>=10?38:24))n=NODE_ELITE;g->map[r][c]=n;}}
int main(void){Game g,old,loaded;int forks=0,oldsame=0;
 for(int seed=1;seed<=5000;seed++)for(int act=1;act<=8;act++){
  game_new(&g,seed);g.act=act;g.endless=act>3;map_generate(&g);old=g;baseline(&old);assert(g.map[1][0]!=g.map[1][2]);
  for(int r=0;r<MAP_ROWS-1;r++)for(int lane=0;lane<3;lane++)if(game_route_row_mask(&g,r)&(1u<<lane)){
   unsigned e=game_route_mask(&g,r,lane);if(!(e&(e-1)))continue;int a=0,b=2;while(!(e&(1u<<a)))a++;while(!(e&(1u<<b)))b--;
   int distinct=g.map[r+1][a]!=g.map[r+1][b],prior=old.map[r+1][a]!=old.map[r+1][b];
   if(!distinct){assert(r+2<MAP_ROWS);assert(game_route_mask(&g,r+1,a)==(1u<<a)&&game_route_mask(&g,r+1,b)==(1u<<b));distinct=g.map[r+2][a]!=g.map[r+2][b];}
   if(!prior&&r+2<MAP_ROWS)prior=old.map[r+2][a]!=old.map[r+2][b];assert(distinct);oldsame+=!prior;forks++;
  }
  for(int c=0;c<3;c++){int shops=0;for(int r=0;r<MAP_ROWS;r++)shops+=g.map[r][c]==NODE_SHOP;assert(shops>=1&&shops<=2&&g.map[17][c]==NODE_REST);}
  if(seed==1){assert(game_save(&g,"qa/v150/evidence/map-choice-save.dat"));assert(game_load(&loaded,"qa/v150/evidence/map-choice-save.dat"));assert(!memcmp(&g,&loaded,sizeof(g)));}
 }
 printf("PASS 40000 maps, %d forks: every fork has different room types within two floors before merging; %d previously identical two-floor branch segments eliminated; distinct starts, one or two shops per lane, final rest, stable save/load\n",forks,oldsame);
}
