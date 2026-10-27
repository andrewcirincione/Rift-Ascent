#include "source/game.c"
#include <assert.h>
#include <stdio.h>
int main(void){Game g,b,t;int seen[6]={0};
 for(int seed=1;seed<=2000;seed++){
  game_new(&g,seed);g.screen=SHOP;g.selection=5;g.gold=500;g.potion_count=0;
  for(int n=0;n<3;n++){b=g;game_action(&g,BTN_OK);assert(g.potion_count==n+1&&g.gold==b.gold-50);for(int j=0;j<n;j++)assert(g.potions[j]==b.potions[j]);seen[g.potions[n]]++;}
  b=g;for(int n=0;n<10;n++)game_action(&g,BTN_OK);assert(g.gold==b.gold&&g.potion_count==3&&!memcmp(g.potions,b.potions,sizeof(g.potions)));
  assert(game_save(&g,"qa/v150/evidence/full-belt-save.dat"));assert(game_load(&t,"qa/v150/evidence/full-belt-save.dat"));assert(!memcmp(g.potions,t.potions,sizeof(g.potions))&&g.gold==t.gold);
  for(int slot=0;slot<3;slot++){
   t=g;game_start_battle(&t,NODE_FIGHT);t.enemies[0].hp=t.enemies[0].maxhp=1000;if(t.enemy_count>1)t.enemies[1].hp=t.enemies[1].maxhp=1000;
   t.hp=20;game_use_potion(&t,slot);assert(t.potion_count==2);
   for(int j=0;j<2;j++)assert(t.potions[j]==g.potions[j+(j>=slot)]);
   t.screen=SHOP;t.selection=5;b=t;game_action(&t,BTN_OK);assert(t.potion_count==3&&t.gold==b.gold-50&&t.potions[0]==b.potions[0]&&t.potions[1]==b.potions[1]);
  }
 }
 for(int id=0;id<6;id++)assert(seen[id]>500);
 for(int origin=MAP;origin<=SHOP;origin+=SHOP-MAP)for(int orb=0;orb<2;orb++)for(int slot=0;slot<3;slot++){
  game_new(&g,140);g.screen=origin;g.selection=origin==MAP?2:5;g.scroll=origin==MAP?3:0;g.hp=20;g.potion_count=3;g.potions[0]=1;g.potions[1]=2;g.potions[2]=5;g.potions[slot]=0;
  if(orb)g.relics[g.relic_count++]=17;g.relics[g.relic_count++]=21;b=g;
  game_action(&g,BTN_POTION);assert(g.screen==POTION_VIEW&&g.prior_screen==origin&&g.selection==0);
  for(int i=0;i<slot;i++)game_action(&g,BTN_DOWN);
  game_action(&g,BTN_OK);assert(g.hp==20+18+5*orb&&g.potion_count==2&&g.block==b.block&&g.strength==b.strength&&g.screen==POTION_VIEW);
  for(int j=0;j<2;j++)assert(g.potions[j]==b.potions[j+(j>=slot)]);
  int hp=g.hp;game_action(&g,BTN_OK);assert(g.hp==hp&&g.potion_count==2);
  assert(game_save(&g,"qa/v150/evidence/map-potion-save.dat"));assert(game_load(&t,"qa/v150/evidence/map-potion-save.dat"));assert(!memcmp(&g,&t,sizeof(g)));
  game_action(&g,BTN_BACK);assert(g.screen==origin&&g.selection==b.selection&&g.scroll==b.scroll&&g.floor==b.floor);
 }
 game_new(&g,140);g.screen=MAP;b=g;game_use_potion(&g,0);assert(g.potion_count==b.potion_count&&g.hp==g.maxhp);
 game_use_potion(&g,-1);game_use_potion(&g,3);assert(g.potion_count==b.potion_count);
 printf("PASS 6000 shop purchases, all six potion types (%d/%d/%d/%d/%d/%d); no overwritten slots, 20000 full-belt denials without charge; 6000 consume/refill cases; map/shop healing, Orb, full-health retention, save and exact return path\n",seen[0],seen[1],seen[2],seen[3],seen[4],seen[5]);
}
