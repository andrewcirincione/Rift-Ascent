#include <assert.h>
#include <stdio.h>
#ifdef BASELINE
#include "../../v184/source/game.c"
#else
#include "../source/game.c"
#endif
int main(void){Game g;unsigned digest=2166136261u,types[8]={0};
#ifndef BASELINE
 unsigned fork_count=0,commitments=0,min_choices=100,max_forced=0,topologies[10000]={0},unique=0;
#endif
 for(int seed=1;seed<=2000;seed++)for(int act=1;act<=5;act++){
  game_new(&g,seed);g.act=act;map_generate(&g);
  for(int r=0;r<MAP_ROWS;r++){unsigned rowtypes[8]={0};for(int c=0;c<3;c++){unsigned n=g.map[r][c];assert(n<8);types[n]++;rowtypes[n]++;}for(int i=0;i<8;i++)digest=(digest^rowtypes[i])*16777619u;}
#ifndef BASELINE
  int least[MAP_ROWS][3]={{0}},run[3]={0},longest[3]={0};unsigned topo=2166136261u;
  for(int r=0;r<MAP_ROWS;r++){
   unsigned live=game_route_row_mask(&g,r);assert(ascent_popcount(live)>=2);topo=(topo^live)*16777619u;
   if(r==MAP_ROWS-1)break;
   int prevmax=-1,next_run[3]={0},next_long[3]={0};
   for(int c=0;c<3;c++)if(live&(1u<<c)){
    unsigned edges=game_route_mask(&g,r,c);assert(edges);topo=(topo^edges)*16777619u;int lo=0,hi=2;while(!(edges&(1u<<lo)))lo++;while(!(edges&(1u<<hi)))hi--;assert(lo>=prevmax);prevmax=hi;
    int fork=ascent_popcount(edges)>1,n=fork?0:run[c]+1;
    for(int d=0;d<3;d++)if(edges&(1u<<d)){next_run[d]=MAX(next_run[d],n);next_long[d]=MAX(next_long[d],MAX(longest[c],n));}
    if(fork){fork_count++;assert(r+2<MAP_ROWS);unsigned a=game_route_mask(&g,r+1,lo),b=game_route_mask(&g,r+1,hi);assert(!(a&b));commitments++;}
   }
   memcpy(run,next_run,sizeof(run));memcpy(longest,next_long,sizeof(longest));
  }
  int seen=0;for(unsigned i=0;i<unique;i++)seen|=topologies[i]==topo;if(!seen)topologies[unique++]=topo;
  for(int r=MAP_ROWS-2;r>=0;r--)for(int c=0;c<3;c++){
   unsigned edges=game_route_mask(&g,r,c);int n=100;for(int d=0;d<3;d++)if(edges&(1u<<d))n=MIN(n,least[r+1][d]);least[r][c]=n+(ascent_popcount(edges)>1);
  }
  unsigned start=game_route_row_mask(&g,0);for(int c=0;c<3;c++){if(start&(1u<<c)){assert(least[0][c]>=4);min_choices=MIN(min_choices,least[0][c]);}max_forced=MAX(max_forced,(unsigned)longest[c]);}
  assert(max_forced<=4);
  /* Actual selection controls must expose all and only connected rooms. */
  for(int r=1;r<MAP_ROWS;r++){g.floor=r;for(int c=0;c<3;c++){g.lane=c;for(int d=0;d<3;d++)assert(game_route_available(&g,d)==!!(game_route_mask(&g,r-1,c)&(1u<<d)));}}
#endif
 }
 printf("ROOMS %08x",digest);for(int i=0;i<8;i++)printf(" %u",types[i]);puts("");
#ifndef BASELINE
 printf("PASS 10000 maps: %u distinct topologies, at least %u choices on every complete route, at most %u consecutive forced transitions, %u/%u forks commit to at least two separate rooms; no single-route bottlenecks or crossing edges.\n",unique,min_choices,max_forced,commitments,fork_count);
#endif
 return 0;}
