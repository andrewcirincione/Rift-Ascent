#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../v186/source/game.c"
static AscentAccount empty;
static void base(Game*g,int c,int wins,int seed){ascent_account=empty;ascent_account.wins[c]=wins;ascent_account.cards[c]=ASCENT_ALL_CARDS;ascent_account.potions=0xfffff;ascent_account.relics=1023;assert(ascent_begin(g,0,c,seed));g->hp=g->maxhp=120;g->relic_count=0;}
static void save(const char*name,const Game*g){char path[240];assert(game_validate(g));snprintf(path,sizeof(path),"qa/v186r3/evidence/fixture-%s.bin",name);FILE*f=fopen(path,"wb");assert(f&&fwrite(g,sizeof(*g),1,f)==1);fclose(f);snprintf(path,sizeof(path),"qa/v186r3/evidence/account-%s.bin",name);f=fopen(path,"wb");assert(f&&fwrite(&ascent_account,sizeof(ascent_account),1,f)==1);fclose(f);}
int main(void){char root[]="/tmp/r186r3-fixtures-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));empty=ascent_account;Game g;char name[80];
 for(int c=0;c<3;c++)for(int prior=0;prior<=10;prior++)if(prior==0||prior==8||prior==9||prior==10){
  base(&g,c,prior,18603+c);g.act=3;g.floor=MAP_ROWS;game_start_battle(&g,NODE_BOSS);g.energy=10;g.hand_count=3;for(int i=0;i<3;i++)g.hand[i]=c==0?0:c==1?N_STRIKE_SILENT:R_STRIKE_REGENT;g.enemies[0].hp=1;g.enemies[0].block=0;g.enemies[1].hp=0;g.selection=g.target=0;snprintf(name,sizeof(name),"ending-%d-%d",c,prior);save(name,&g);
 }
 for(int a=1;a<=5;a+=2){
  base(&g,0,10,18603);g.act=a;g.endless=a>3;g.screen=BETA_FORGE;g.expedition.story_id=5;g.expedition.gift_kind=2;g.selection=2;g.deck_count=3;g.deck[0]=0;g.deck[1]=1;g.deck[2]=8;snprintf(name,sizeof(name),"forge-%d",a);save(name,&g);
  g.screen=EXPEDITION_GIFT;g.selection=0;g.expedition.story_id=0;expedition_prepare_gift(&g);snprintf(name,sizeof(name),"gift-%d",a);save(name,&g);
 }
 base(&g,0,10,18603);g.act=3;g.floor=10;g.screen=EXPEDITION_GIFT;g.expedition.story_id=4;expedition_prepare_gift(&g);save("relic-3",&g);
 base(&g,0,10,18603);g.act=3;g.floor=10;g.gold=200;g.deck_count=3;g.deck[0]=0;g.deck[1]=1;g.deck[2]=8;game_begin_event(&g,12);save("mystery-forge-3",&g);
 game_begin_event(&g,34);save("mystery-cards-3",&g);
 g.act=5;g.endless=1;expedition_mystery(&g);g.screen=MYSTERY_CARDS;save("foreign-5",&g);
 base(&g,0,10,18603);g.act=4;g.endless=1;g.floor=6;game_start_battle(&g,NODE_FIGHT);save("combat-4",&g);
 /* Existing fade regression fixture names, with valid account state. */
 for(int a=3;a<=7;a+=a==3?1:3){base(&g,0,10,18603);g.act=a;g.endless=a>3;g.floor=MAP_ROWS;g.ascent.secret=a==4;game_start_battle(&g,NODE_BOSS);g.hand_count=3;g.hand[0]=g.hand[1]=g.hand[2]=0;g.energy=10;g.selection=g.target=0;for(int e=0;e<g.enemy_count;e++){g.enemies[e].hp=1;g.enemies[e].block=0;}snprintf(name,sizeof(name),"finish%d",a);save(name,&g);}
 puts("PASS validated PSP fixtures and explicit account snapshots for ending gates, rewards, Forge and full fade sequences.");return 0;
}
