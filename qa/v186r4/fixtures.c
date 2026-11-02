#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../v186/source/game.c"
static AscentAccount empty;
static void reset(void){ascent_account=empty;memset(&ascent_finale,0,sizeof(ascent_finale));ascent_finale_generation=0;}
static void save(const char*name,const Game*g){
 char path[240];assert(game_validate(g));const void*data[]={g,&ascent_account,&ascent_finale};const unsigned size[]={sizeof(*g),sizeof(ascent_account),sizeof(ascent_finale)};const char*kind[]={"fixture","account","finale"};
 for(int i=0;i<3;i++){snprintf(path,sizeof(path),"qa/v186r4/evidence/%s-%s.bin",kind[i],name);FILE*f=fopen(path,"wb");assert(f&&fwrite(data[i],size[i],1,f)==1);fclose(f);}
}
static void battle(Game*g,int act){g->act=act;g->endless=act>3;g->floor=MAP_ROWS;g->hp=g->maxhp=120;g->relic_count=0;game_start_battle(g,NODE_BOSS);g->energy=10;g->hand_count=3;for(int i=0;i<3;i++)g->hand[i]=g->character==0?0:g->character==1?N_STRIKE_SILENT:R_STRIKE_REGENT;g->selection=g->target=0;for(int e=0;e<g->enemy_count;e++){g->enemies[e].hp=e?0:1;g->enemies[e].block=0;}}
int main(void){
 char root[]="/tmp/r186r4-fixtures-XXXXXX",name[80];assert(mkdtemp(root)&&ascent_open(root));empty=ascent_account;Game g;
 for(int c=0;c<3;c++)for(int prior=3;prior<=10;prior++)if(prior==3||prior==4||prior==9||prior==10){
  reset();for(int i=0;i<prior;i++){assert(ascent_begin(&g,0,c,400+c*100+i));g.act=3;assert(ascent_complete(&g,0));}
  assert(ascent_begin(&g,0,c,18604+c));battle(&g,3);snprintf(name,sizeof(name),"gate-%d-%d",c,prior);save(name,&g);
 }
 const int orders[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
 for(int order=0;order<6;order++){
  reset();for(int c=0;c<3;c++){ascent_account.wins[c]=5;ascent_account.cards[c]=ASCENT_ALL_CARDS;}
  for(int i=0;i<3;i++){
   assert(ascent_begin(&g,i,orders[order][i],900+order*3+i));battle(&g,4);snprintf(name,sizeof(name),"order-%d-%d",order,i);save(name,&g);
   for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;check_win(&g);for(int p=0;p<3;p++)game_action(&g,BTN_OK);
   if(i==2)for(int p=0;p<ASCENT_FINALE_PAGES;p++)game_action(&g,BTN_OK);
   assert(g.screen==VICTORY);
  }
  assert(ascent_begin(&g,3,orders[order][2],1000+order));battle(&g,4);snprintf(name,sizeof(name),"repeat-%d",order);save(name,&g);
 }
 for(int wins=4;wins<=5;wins++){
  reset();for(int c=0;c<3;c++)ascent_account.wins[c]=wins;game_new(&g,55);g.screen=CHARACTER_SELECT;g.selection=0;snprintf(name,sizeof(name),"badges-%d",wins);save(name,&g);
 }
 reset();for(int c=0;c<3;c++)ascent_account.wins[c]=5;assert(ascent_begin(&g,0,0,1600));g.act=4;g.endless=1;g.floor=6;game_start_battle(&g,NODE_FIGHT);save("combat",&g);
 for(int a=3;a<=7;a+=a==3?1:3){reset();ascent_account.wins[0]=5;assert(ascent_begin(&g,0,0,1700+a));battle(&g,a);for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=1;snprintf(name,sizeof(name),"finish%d",a);save(name,&g);}
 puts("PASS legal assisted PSP fixtures: fourth/fifth/tenth/eleventh gates, all six character orders, repeat victories, golden A, full-frame finishing controls.");return 0;
}
