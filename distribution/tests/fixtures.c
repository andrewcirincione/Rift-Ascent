#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../../qa/v186/source/game.c"
static AscentAccount empty;
static void reset(void){ascent_account=empty;memset(&ascent_finale,0,sizeof(ascent_finale));ascent_finale_generation=0;}
static void save(const char*name,const Game*g){
 char path[240];if(!game_validate(g)){fprintf(stderr,"Invalid fixture: %s screen=%d act=%d\n",name,g->screen,g->act);abort();}const void*data[]={g,&ascent_account,&ascent_finale};const unsigned size[]={sizeof(*g),sizeof(ascent_account),sizeof(ascent_finale)};const char*kind[]={"fixture","account","finale"};
 for(int i=0;i<3;i++){snprintf(path,sizeof(path),"distribution/evidence/%s-%s.bin",kind[i],name);FILE*f=fopen(path,"wb");assert(f&&fwrite(data[i],size[i],1,f)==1);fclose(f);}
}
static void battle(Game*g,int act){g->act=act;g->endless=act>3;g->floor=MAP_ROWS;g->hp=g->maxhp=120;g->relic_count=0;game_start_battle(g,NODE_BOSS);g->energy=10;g->hand_count=3;for(int i=0;i<3;i++)g->hand[i]=g->character==0?0:g->character==1?RIFT_CARD_173:RIFT_CARD_269;g->selection=g->target=0;for(int e=0;e<g->enemy_count;e++){g->enemies[e].hp=e?0:1;g->enemies[e].block=0;}}
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
 for(int c=0;c<3;c++){
  reset();assert(ascent_begin(&g,0,c,1900+c));g.act=7;g.endless=1;g.floor=MAP_ROWS;game_start_battle(&g,NODE_BOSS);
  g.maxhp=g.hp=987;g.energy=12;g.block=0;g.regent.stars=3;g.hand_count=4;g.selection=g.target=0;g.potion_count=1;g.potions[0]=2;g.message[0]=0;
  int attacks[]={0,173,269},skills[]={1,120,210},releases[]={2,147,213};
  g.hand[0]=skills[c];g.hand[1]=attacks[c]|(MOD_ECHO<<CARD_MOD_SHIFT);g.hand[2]=releases[c];
  for(int id=0;id<CARD_COUNT;id++)if(dist_cards[id].character==c&&cards[id].type==2){g.hand[3]=id;break;}
  for(int e=0;e<g.enemy_count;e++){g.enemies[e].hp=g.enemies[e].maxhp=600;g.enemies[e].block=0;}
  snprintf(name,sizeof(name),"cards-%d",c);save(name,&g);
  for(int step=1;step<=6;step++){
   if(step<=4)game_play_card(&g,0,0);else if(step==5)game_use_potion(&g,0);else game_end_turn(&g);
   snprintf(name,sizeof(name),"cards-%d-expected-%d",c,step);save(name,&g);
  }
 }
 reset();assert(ascent_begin(&g,0,0,2000));g.screen=BETA_FORGE;g.expedition.gift_kind=2;g.selection=0;save("forge",&g);
 for(int a=1;a<=4;a++){
  reset();assert(ascent_begin(&g,0,2,2100+a));g.act=a;g.endless=a>3;g.screen=STORY_ACT;g.scroll=a==3?9:0;g.selection=0;
  snprintf(name,sizeof(name),"review-entry-%d",a);save(name,&g);
 }
 reset();assert(ascent_begin(&g,0,2,2200));g.act=3;g.screen=STORY_BOSS;g.scroll=4;g.selection=0;save("review-boss",&g);
 for(int id=2;id<=13;id+=id==2?5:6){
  reset();assert(ascent_begin(&g,0,2,2300+id));g.screen=EXPEDITION_STORY;g.expedition.story_id=id;g.selection=0;
  snprintf(name,sizeof(name),"review-event-%d",id);save(name,&g);
 }
 reset();assert(ascent_begin(&g,0,2,2400));battle(&g,3);save("review-first-ending",&g);
 reset();assert(ascent_begin(&g,0,2,2500));g.screen=EXPEDITION_GIFT;g.expedition.gift_kind=2;g.selection=0;save("review-forge-gift",&g);
 puts("PASS legal assisted PSP fixtures: fourth/fifth/tenth/eleventh gates, all six character orders, repeat victories, golden A, full-frame finishing controls.");return 0;
}
