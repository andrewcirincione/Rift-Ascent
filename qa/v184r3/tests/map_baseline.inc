static void map_generate184r2_reference(Game*g){
 g->revision184.route_style=1;
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);
 for(int r=0;r<MAP_ROWS;r++){
  g->map_selected[r]=-1;
  for(int c=0;c<3;c++){
   uint32_t h=route_hash(seed+(unsigned)(r*131+c*977));int n=NODE_FIGHT;
   if(r==1)n=(h%3)?NODE_EVENT:NODE_FIGHT;
   else if(r==3||r==9){
    int earlyshop=(int)(route_hash(seed+c*127u)&1u);
    n=((r==3)==earlyshop)?NODE_SHOP:((h&1)?NODE_EVENT:NODE_REST);
   }else if(r==5)n=(h%3==0)?NODE_EVENT:NODE_FIGHT;
   else if(r==7)n=NODE_TREASURE;
   else if(r==11)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==13)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==15)n=(h%3)?NODE_EVENT:NODE_SHOP;
   else if(r==17)n=NODE_REST;
   else if(r>=4&&(h%100)<(unsigned)(r>=10?38:24))n=NODE_ELITE;
   g->map[r][c]=n;
  }
 }
 map_distinguish_forks(g);
 /* Guaranteed rests by floor 6 and floor 12, then before the boss. No route
    can stack the old random rest rolls or go an entire act without recovery. */
 for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<3;c++)if(g->map[r][c]==NODE_REST)g->map[r][c]=NODE_EVENT;
 int early=4+(seed&1u),middle=10+((seed>>1)&1u);
 for(int c=0;c<3;c++){g->map[early][c]=NODE_REST;g->map[middle][c]=NODE_REST;g->map[17][c]=NODE_REST;}
 /* An optional fourth camp is a visible recovery-versus-event choice. */
 if(seed&4u){int lane=(int)((seed>>3)%3u);g->map[14][lane]=NODE_REST;}
 /* A guaranteed camp may occupy a fork's original distinguishing room.
    Preserve a real choice on the following floor before the merge. */
 for(int r=0;r<MAP_ROWS-2;r++)for(int c=0;c<3;c++){
  unsigned edges=room_route_layer(g,r,c);if(!(room_route_layer(g,r,-1)&(1u<<c))||!(edges&(edges-1)))continue;
  int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
  if(g->map[r+1][a]==g->map[r+1][b]&&g->map[r+2][a]==g->map[r+2][b]&&r+2!=early&&r+2!=middle&&r+2!=17){
   g->map[r+2][a]=NODE_FIGHT;g->map[r+2][b]=NODE_ELITE;
  }
 }
 map_distinguish_committed_routes(g);
}
