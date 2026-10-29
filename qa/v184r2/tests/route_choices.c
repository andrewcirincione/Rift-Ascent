#include <stdio.h>
#include "../source/game.c"
int main(void){Game g;int forks=0,distinct=0,same[18]={0};for(int seed=1;seed<=10000;seed++){
 game_new(&g,seed);for(int r=0;r<MAP_ROWS-1;r++)for(int c=0;c<3;c++)if(game_route_row_mask(&g,r)&(1u<<c)){
  unsigned edges=game_route_mask(&g,r,c);if(ascent_popcount(edges)<2)continue;int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;unsigned aa=1u<<a,bb=1u<<b;int different=0;
  for(int row=r+1;row<MAP_ROWS;row++){
   for(int x=0;x<3;x++)for(int y=0;y<3;y++)if((aa&(1u<<x))&&(bb&(1u<<y)))different|=g.map[row][x]!=g.map[row][y];
   if(different||row==MAP_ROWS-1)break;unsigned an=0,bn=0;for(int x=0;x<3;x++){if(aa&(1u<<x))an|=game_route_mask(&g,row,x);if(bb&(1u<<x))bn|=game_route_mask(&g,row,x);}if(an&bn)break;aa=an;bb=bn;
  }forks++;distinct+=different;if(!different)same[r]++;
 }}printf("%d/%d forks offer distinct room sequences before reconnection (%.1f%%).\n",distinct,forks,100.*distinct/forks);for(int r=0;r<18;r++)printf("%s%d:%d",r?" ":"",r,same[r]);puts("");return 0;}
