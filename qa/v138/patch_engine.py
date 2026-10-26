from pathlib import Path
p=Path(__file__).parent/'source/game.c';s=p.read_text()
a=s.index('unsigned game_route_mask(');b=s.index('unsigned game_route_visible_mask',a)
s=s[:a]+'''static uint32_t route_hash(uint32_t z){z^=z>>16;z*=0x7feb352du;z^=z>>15;z*=0x846ca68bu;return z^(z>>16);}
/* Two distinct starts, three optional forks, and at most three merges per act.
   Every active room has an exit. Ordered neighbors prevent crossing paths. */
static unsigned route_layer(const Game*g,int wanted,int lane){
 unsigned live=5u;
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*0x9e3779b9u);
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1u,2u,4u};uint32_t h=route_hash(seed+(unsigned)r*9199u);
  int fork=(r==1+(int)(seed&1u)||r==6+(int)((seed>>1)&1u)||r==10+(int)((seed>>2)&1u));
  int merge=(r==4||r==8||r==12);
  if(fork&&live!=7u){
   int gap=!(live&2)?1:!(live&1)?0:2;
   int from=gap==1?((h&1)?0:2):1;
   if(live&(1u<<from))edges[from]|=1u<<gap;
  }else if(merge&&live==7u){int from=(h&1)?0:2;edges[from]=2u;}
  /* Inactive lanes still have a valid exit for compatible older saves. */
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
unsigned game_route_mask(const Game*g,int row,int lane){
 if(!g||row<0||row>=MAP_ROWS-1||lane<0||lane>=3)return 0;
 return route_layer(g,row,lane);
}
unsigned game_route_row_mask(const Game*g,int row){
 if(!g||row<0||row>=MAP_ROWS)return 0;
 unsigned mask=route_layer(g,row,-1);
 if(row<g->floor&&g->map_selected[row]>=0)mask|=1u<<g->map_selected[row];
 if(row==g->floor&&g->floor>0)mask|=game_route_mask(g,g->floor-1,g->lane);
 return mask;
}
''' + s[b:]
s=s.replace('if(g->floor==0)return 1;', 'if(g->floor==0)return (game_route_row_mask(g,0)&(1u<<lane))!=0;')
a=s.index('static void map_generate(Game*g){');b=s.index('static void shuffle',a)
s=s[:a]+'''static void map_generate(Game*g){
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
   else if(r==13)n=NODE_REST;
   else if(r>=4&&(h%100)<(unsigned)(r>=10?38:24))n=NODE_ELITE;
   g->map[r][c]=n;
  }
 }
}
''' +s[b:]
s=s.replace('e->intent_value=(d->attack*100+50)/100+2*(g->act-1)+g->floor/5+(e->phase?2:0);', 'int percent=g->act==1?70:g->act==2?80:90;\n e->intent_value=MAX(1,(d->attack*percent+50)/100+g->floor/7+(e->phase?2:0));')
s=s.replace('int original=(d->hp*(d->tier==2?120:(d->tier==1?115:100))+50)/100;\n int scaling=(100+(g->act-1)*13+(g->floor/4)*4)*120/100;\n e->maxhp=(short)MIN(999,(original*scaling+50)/100);', 'int percent=g->act==1?82:g->act==2?90:100;\n if(d->tier==2)percent=90;else percent+=g->floor;\n int hp=(d->hp*percent+50)/100;\n if(g->enemy_count==2)hp=(hp*72+50)/100;\n e->maxhp=(short)MIN(999,hp);')
a=s.index(' g->enemy_count=mode==NODE_BOSS?1:');b=s.index(' for(int i=0;i<g->enemy_count;i++){',a)
s=s[:a]+''' uint32_t encounter=route_hash(g->seed^(uint32_t)g->act*7919u);
 int chance=g->act==1?25:g->act==2?40:50;
 int candidate=(int)(route_hash(encounter+g->floor*101u)%100u)<chance;
 int previous=g->floor>0&&(int)(route_hash(encounter+(g->floor-1)*101u)%100u)<chance;
 g->enemy_count=mode==NODE_FIGHT&&!(g->act==1&&g->floor<3)&&candidate&&!previous?2:1;
''' +s[b:]
p.write_text(s)
