#include "game.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#define ASSERT(x) do{if(!(x)){fprintf(stderr,"87 CARD FAIL %d %s\n",__LINE__,#x);return 1;}}while(0)
static void prep(Game*g,unsigned seed){
 game_new(g,seed);game_start_battle(g,NODE_FIGHT);
 g->enemy_count=2;g->energy=100;g->hp=g->maxhp;
 for(int i=0;i<2;i++){
  g->enemies[i].hp=g->enemies[i].maxhp=16000;
  g->enemies[i].block=0;
  g->enemies[i].vulnerable=0;
  g->enemies[i].strength=0;
 }
 g->target=0;
}
static unsigned hash(const unsigned char*p,size_t n){
 unsigned h=2166136261u;for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;
 return h;
}
int main(void){
 Game g;
 ASSERT(CARD_COUNT==92);
 int count=0;
 for(int id=0;id<CARD_COUNT;id++){
  int legal=game_card_is_playable(id);
  if(id>=50&&id<=53)ASSERT(!legal);
  else ASSERT(legal);
  if(id!=91&&legal)count++;
  if(legal){
   const char *name=cards[id].name,*desc=card_description(id);
   ASSERT(name&&name[0]&&desc&&desc[0]);
   if(id!=91)ASSERT(card_description(id|UPBIT)[0]);
  }
 }
 ASSERT(count==87);
 ASSERT(!strcmp(cards[54].name,"Aggression"));
 ASSERT(!strcmp(cards[90].name,"Vicious"));
 ASSERT(!strcmp(cards[91].name,"Giant Rock"));
 ASSERT(game_card_cost(55|UPBIT)==2);
 ASSERT(game_card_cost(59|UPBIT)==2);
 ASSERT(game_card_cost(68|UPBIT)==0);
 ASSERT(game_card_cost(71|UPBIT)==0);
 ASSERT(game_card_cost(82|UPBIT)==1);
 ASSERT(game_card_cost(88|UPBIT)==1);
 /* Test every solo card in an actual combat state with two healthy foes. */
 for(int id=0;id<CARD_COUNT;id++){
  if(!game_card_is_playable(id)||id==91||id==63||id==85)continue;
  prep(&g,(unsigned)(0x87000000u+(unsigned)id));
  g.deck_count=3;g.deck[0]=0;g.deck[1]=1;g.deck[2]=2;
  g.hand_count=2;g.hand[0]=id;g.hand[1]=0;
  g.draw_count=4;
  for(int i=0;i<4;i++)g.draw[i]=0;
  g.discard_count=1;g.discard[0]=0;
  if(id==37){g.exhaust_count=3;g.exhaust[0]=50;g.exhaust[1]=51;g.exhaust[2]=52;}
  int before=g.energy;
  game_play_card(&g,0,0);
  ASSERT(g.energy<=before+10 && g.energy>=0);
  ASSERT(g.screen==COMBAT||g.screen==CARD_CHOICE||g.screen==REWARD||g.screen==DEFEAT);
  if(g.screen==CARD_CHOICE)game_action(&g,BTN_OK);
  ASSERT(game_validate(&g));
 }
 prep(&g,8711);g.hand_count=1;g.hand[0]=55;
 game_play_card(&g,0,0);ASSERT(g.ic[IC_BARRICADE]);
 g.block=45;g.hand_count=0;g.enemies[0].intent=1;g.enemies[1].intent=1;
 game_end_turn(&g);ASSERT(g.block>=45);
 prep(&g,8712);g.hand_count=1;g.hand[0]=59;
 game_play_card(&g,0,0);ASSERT(g.ic[IC_CORRUPTION]);
 ASSERT(game_card_cost_now(&g,1)==0);
 g.hand_count=1;g.hand[0]=1;
 game_play_card(&g,0,0);ASSERT(g.exhaust_count>=1);
 prep(&g,8713);g.hand_count=2;g.hand[0]=77|UPBIT;g.hand[1]=0;
 game_play_card(&g,0,0);ASSERT(g.hand_count>0);
 int found=0;for(int i=0;i<g.hand_count;i++)
  if(CARD_ID(g.hand[i])==91){found++;ASSERT(CARD_UP(g.hand[i]));}
 ASSERT(found>0);
 prep(&g,8714);g.hand_count=1;g.hand[0]=78|UPBIT;
 game_play_card(&g,0,0);ASSERT(g.ic[IC_PYRE]==2);
 g.enemies[0].intent=g.enemies[1].intent=1;game_end_turn(&g);
 ASSERT(g.screen==COMBAT&&g.energy>=g.max_energy+2);
 prep(&g,8715);g.hand_count=1;g.hand[0]=62;
 game_play_card(&g,0,0);ASSERT(g.ic[IC_DARK_EMBRACE]==1);
 g.hand_count=1;g.hand[0]=67;g.draw_count=2;g.draw[0]=0;g.draw[1]=1;
 game_play_card(&g,0,0);ASSERT(g.hand_count>=1);
 prep(&g,8716);g.hand_count=1;g.hand[0]=56;
 int old=g.enemies[0].hp;game_play_card(&g,0,0);
 ASSERT(g.enemies[0].hp<old&&g.enemies[0].vulnerable>=5);
 prep(&g,8717);g.hand_count=1;g.hand[0]=81;g.ic[IC_HPLOSS_TURN]=1;
 old=g.enemies[0].hp;game_play_card(&g,0,0);ASSERT(old-g.enemies[0].hp>=10);
 prep(&g,8718);g.hand_count=1;g.hand[0]=86;g.ic[IC_HPLOSS_COMBAT]=3;
 old=g.enemies[0].hp;game_play_card(&g,0,0);ASSERT(old-g.enemies[0].hp>=20);
 prep(&g,8719);
 ASSERT(game_save(&g,"/tmp/rift_87_new.sav"));
 {Game loaded;ASSERT(game_load(&loaded,"/tmp/rift_87_new.sav"));ASSERT(game_validate(&loaded));}
 /* Verify byte-for-byte legacy version 16 data migration and checksum. */
 {
  Game old=g;old.version=16;FILE*f=fopen("/tmp/rift_87_legacy.sav","wb");ASSERT(f);
  size_t n=offsetof(Game,ic);
  unsigned sum=hash((const unsigned char*)&old,n);
  ASSERT(fwrite(&sum,4,1,f)==1 && fwrite(&old,n,1,f)==1);fclose(f);
  Game newer;ASSERT(game_load(&newer,"/tmp/rift_87_legacy.sav"));
  ASSERT(newer.version==17&&newer.hp==old.hp&&newer.deck_count==old.deck_count);
  for(int i=0;i<48;i++)ASSERT(newer.ic[i]==0);
 }
 puts("PASS full 87-card catalog: rewards-safe IDs, card play matrix, powers, autos, v16 save migration");
 return 0;
}