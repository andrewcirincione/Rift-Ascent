from pathlib import Path
p=Path(__file__).parent/'source/render.c';s=p.read_text();a=s.index('static void room_icon(');b=s.index('static void relic_result_screen(',a)
s=s[:a]+'''static int r138_map_x(const Game*g,int row,int lane){return 73+lane*94+(int)(particle_hash(g->seed+(unsigned)(row*31+lane*997))%15u)-7;}
static void room_icon(int n,int x,int y){if(n>=0&&n<=6)r134_blit(x-13,y-13,26,26,r134_map_icons[n],36,36,255);}
static void map_screen(const Game*g){
 header(g,"FATE'S PATH");gilded_panel(15,41,299,209,0);gilded_panel(315,41,160,209,0);
 int focus=g->floor+g->scroll,first=CLAMP(focus-2,0,MAP_ROWS-4);
 for(int r=first;r<first+5&&r<MAP_ROWS;r++){
  int y=222-(r-first)*38;
  for(int c=0;c<3;c++)if(game_route_row_mask(g,r)&(1u<<c)){
   int x=r138_map_x(g,r,c);unsigned routes=r==MAP_ROWS-1?2u:game_route_mask(g,r,c);
   for(int d=0;d<3;d++)if(routes&(1u<<d)){
    int nx=r==MAP_ROWS-1?167:r138_map_x(g,r+1,d),ny=y-38;
    int travelled=r<g->floor-1&&g->map_selected[r]==c&&g->map_selected[r+1]==d;
    int from_current=r==g->floor-1&&c==g->lane;
    uint32_t ink=travelled?GOLD:from_current?RGB(239,211,157):r<g->floor?RGB(67,59,53):RGB(131,111,88);
    line(x,y-12,nx,ny+12,ink);
   }
  }
 }
 for(int r=first;r<first+5&&r<=MAP_ROWS;r++){
  int y=222-(r-first)*38;char num[12];snprintf(num,sizeof(num),"%02d",r+1);r134_text(25,y-4,num,MUTED,&r134_f8);
  if(r==MAP_ROWS){room_icon(NODE_BOSS,167,y);if(g->floor==MAP_ROWS)r134_blit(151,y+8,32,18,r138_choice,42,28,255);continue;}
  for(int c=0;c<3;c++)if(game_route_row_mask(g,r)&(1u<<c)){
   int x=r138_map_x(g,r,c),n=g->map[r][c],opacity=255;
   if((r<g->floor&&g->map_selected[r]!=c)||(r==g->floor&&!game_route_available(g,c)))opacity=70;
   r134_blit(x-13,y-13,26,26,r134_map_icons[n],36,36,opacity);
   if(r==g->floor-1&&c==g->lane)r134_blit(x-17,y-18,34,30,r138_pawn,34,30,255);
   else if(r<g->floor&&g->map_selected[r]==c)r134_text(x+11,y+7,"+",GOLD,&r134_f8);
   if(r==focus&&c==g->selection)r134_blit(x-14,y+7,28,13,r138_choice,42,28,255);
  }
 }
 r134_blit(330,60,34,30,r138_pawn,34,30,255);r134_text(370,62,"CURRENT",GOLD,&r134_f9b);
 char b[70];snprintf(b,sizeof(b),g->floor?"FLOOR %02d":"ENTRANCE",g->floor);r134_text(370,77,b,WHITE,&r134_f8);
 snprintf(b,sizeof(b),"ACT %d / 3",g->act);r134_text(335,103,b,GOLD,&r134_f10);
 r134_text(335,132,"NEXT",MUTED,&r134_f9);
 int n=focus>=MAP_ROWS?NODE_BOSS:g->map[focus][g->selection];r134_text(335,150,node_name(n),node_color(n),&r134_f10);
 const char*reward=n==NODE_FIGHT?"CARD REWARD":n==NODE_ELITE?"CARD + RELIC":n==NODE_SHOP?"BUY / REMOVE":n==NODE_REST?"HEAL / UPGRADE":n==NODE_EVENT?"MYSTERY":n==NODE_TREASURE?"RELIC":"BOSS";
 r134_text(335,174,reward,WHITE,&r134_f8);
 if(!g->scroll)r134_text(335,204,"X TAKE PATH",GOLD,&r134_f9);
 r134_text(335,224,"SQUARE DECK",MUTED,&r134_f8);
 footer("UP/DOWN SCROLL   LEFT/RIGHT PATH   X ENTER   SQ DECK");
}
''' +s[b:];p.write_text(s)
p=Path(__file__).parent/'source/game.c';s=p.read_text();s=s.replace('return game_route_row_mask(g,row);\n}\nint game_route_available', 'if(row==g->floor&&g->floor>0)return game_route_mask(g,g->floor-1,g->lane);\n return game_route_row_mask(g,row);\n}\nint game_route_available');p.write_text(s)
