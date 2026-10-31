#include <assert.h>
#include <stdio.h>
#include "../v185/source/game.c"

static void arena(Game *g,int type,int intent,int value,int weak,int vulnerable,int block,int krane){
 game_new(g,18506);game_choose_character(g,0);g->act=2;g->floor=8;
 game_start_battle(g,type);g->enemy_count=1;g->turn=1;g->relic_count=0;
 g->hp=g->maxhp=80;g->block=block;g->poison=0;g->vulnerable=vulnerable;
 g->hand_count=g->discard_count=0;g->draw_count=30;
 for(int i=0;i<30;i++)g->draw[i]=1;
 Enemy *e=&g->enemies[0];e->hp=e->maxhp=1000;e->intent=intent;
 e->intent_value=value;e->strength=0;e->weak=weak;e->vulnerable=e->poison=e->block=0;
 if(krane){g->relic_count=1;g->relics[0]=NR_KRANE;}
 g->selection=0;
}
static void fixture(const char *name,const Game *g){
 char path[200];assert(game_validate(g));
 snprintf(path,sizeof(path),"qa/v185/evidence/fixture-%s.bin",name);
 FILE *f=fopen(path,"wb");assert(f&&fwrite(g,sizeof(*g),1,f)==1);fclose(f);
}
int main(void){
 /* Fixed expected outcomes exercise the full turn resolver and telegraph,
    including integer rounding, multi-hit Block, debuffs and Paper Krane. */
 const int cases[][9]={
  /* intent,value,weak,vulnerable,block,krane,boss shown,other shown,fixture */
  {0,16,0,0,0,0,21,20,1},
  {0,16,0,0,15,0,21,20,0},
  {7,32,0,0,15,0,42,40,1},
  {16,48,0,0,15,0,63,60,0},
  {0,24,1,1,7,0,36,33,1},
  {0,24,1,1,7,1,28,27,0}
 };
 const int types[]={NODE_FIGHT,NODE_ELITE,NODE_BOSS};
 Game g,h;int checked=0;
 FILE *meta=fopen("qa/v185r6/evidence/boss-fixtures.json","w");assert(meta);fputs("[\n",meta);int written=0;
 for(int t=0;t<3;t++)for(unsigned c=0;c<sizeof(cases)/sizeof(cases[0]);c++){
  const int *v=cases[c];arena(&g,types[t],v[0],v[1],v[2],v[3],v[4],v[5]);
  int shown=v[t==2?6:7],lost=MAX(0,shown-v[4]);
  assert(game_enemy_intent_damage(&g,0)==shown);
  if((t==2&&v[8])||(t==1&&c==0)){
   char name[64];snprintf(name,sizeof(name),"r6-%s-%u",t==2?"boss":"elite",c);fixture(name,&g);
   fprintf(meta,"%s {\"fixture\":\"%s\",\"shown\":%d,\"hp_before\":80,\"hp_after\":%d}",written++?",\n":"",name,shown,80-lost);
  }
  game_end_turn(&g);assert(g.hp==80-lost&&g.screen==COMBAT);checked++;
 }
 fputs("\n]\n",meta);fclose(meta);
 arena(&g,NODE_BOSS,0,16,0,0,0,0);
 assert(game_save(&g,"/tmp/rift185r6-boss.dat")&&game_load(&h,"/tmp/rift185r6-boss.dat"));
 assert(h.battle_type==NODE_BOSS&&game_enemy_intent_damage(&h,0)==21);
 game_end_turn(&h);assert(h.hp==59);
 for(int t=0;t<3;t++){
  arena(&g,types[t],1,20,0,0,0,0);game_end_turn(&g);
  assert(g.hp==80&&g.enemies[0].block==20);
 }
 printf("PASS %d attack cases: boss +5%%, ordinary/elite controls, displayed intent equals actual damage, two/three hits, Block, Weak/Vulnerable and Paper Krane.\n",checked);
 puts("PASS boss save/resume preserves damage exactly once; defensive intents unchanged; four exact-EBOOT fixtures generated.");
 return 0;
}
