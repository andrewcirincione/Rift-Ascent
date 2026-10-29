#include <assert.h>
#include <stdio.h>
#ifdef BASELINE
#include "../../v184r2/source/game.c"
#else
#include "../source/game.c"
#include "map_baseline.inc"
#endif
int main(void){Game g;unsigned distribution=2166136261u,legacy=2166136261u;int forks=0;
 for(int seed=1;seed<=10000;seed++)for(int act=1;act<=7;act++){
  game_new(&g,seed);g.act=act;map_generate(&g);unsigned types[8]={0};
  for(int row=0;row<MAP_ROWS;row++){
   unsigned fixed[3]={0};for(int c=0;c<3;c++){int type=g.map[row][c];assert(type>=0&&type<8);types[type]++;fixed[0]+=type==NODE_REST;fixed[1]+=type==NODE_SHOP;fixed[2]+=type==NODE_TREASURE;}
   for(int i=0;i<3;i++)distribution=(distribution^fixed[i])*16777619u;
  }
  for(int i=0;i<8;i++)distribution=(distribution^types[i])*16777619u;
#ifndef BASELINE
  Game previous=g;map_generate184r2_reference(&previous);assert(!memcmp(previous.map,g.map,sizeof(g.map[0])*2));
  for(int row=0;row<4;row++){int was=0,now=0;for(int c=0;c<3;c++){was+=previous.map[row][c]==NODE_ELITE;now+=g.map[row][c]==NODE_ELITE;}assert(now<=was);}
  unsigned starts=game_route_row_mask(&g,0);int a=0,b=2;while(!(starts&(1u<<a)))a++;while(!(starts&(1u<<b)))b--;assert(g.map[1][a]!=g.map[1][b]);
  for(int row=0;row<MAP_ROWS-1;row++)for(int lane=0;lane<3;lane++)if(game_route_row_mask(&g,row)&(1u<<lane)){
   unsigned edges=game_route_mask(&g,row,lane);if(ascent_popcount(edges)<2)continue;int lo=0,hi=2;while(!(edges&(1u<<lo)))lo++;while(!(edges&(1u<<hi)))hi--;
   /* Track only pairs of prefixes whose room sequences are still equal.
      Any such pair that reaches the same node is a counterfeit choice. */
   unsigned pairs=1u<<(lo*3+hi);forks++;
   for(int r=row+1;r<MAP_ROWS&&pairs;r++){
    unsigned next=0;for(int x=0;x<3;x++)for(int y=0;y<3;y++)if(pairs&(1u<<(x*3+y))){
     if(g.map[r][x]!=g.map[r][y])continue;assert(x!=y);if(r==MAP_ROWS-1)assert(0);
     unsigned xa=game_route_mask(&g,r,x),ya=game_route_mask(&g,r,y);
     for(int u=0;u<3;u++)for(int v=0;v<3;v++)if((xa&(1u<<u))&&(ya&(1u<<v)))next|=1u<<(u*3+v);
    }pairs=next;
   }
  }
#endif
  for(int style=0;style<2;style++){g.revision184.route_style=style;for(int r=0;r<MAP_ROWS;r++){legacy=(legacy^game_route_row_mask(&g,r))*16777619u;for(int c=0;c<3;c++)legacy=(legacy^game_route_mask(&g,r,c))*16777619u;}}
 }
 printf("PRESERVED distribution=%08x legacy_routes=%08x maps=70000\n",distribution,legacy);
#ifndef BASELINE
 printf("PASS %d forks: every pair of routes differs in room sequence before reconnection; all 70,000 opening pairs differ by floor two; no new elites in the opening floors.\n",forks);
#endif
 return 0;
}
