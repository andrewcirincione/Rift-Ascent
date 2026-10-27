#include "source/game.c"
#include <assert.h>
#include <stdio.h>
#include <math.h>
int main(void){Game g,t;int offers=0;
 for(int rarity=0;rarity<3;rarity++){
  int pool[128],n=0;for(int id=3;id<CARD_COUNT;id++)if(solo_reward_eligible(id)&&cards[id].rarity==rarity)pool[n++]=id;
  for(int mode=0;mode<4;mode++){
   game_new(&g,140+rarity*33+mode);g.deck_count=1;g.deck[0]=0;int owned[128]={0},seen[128]={0},own=0,got=0;
   for(int i=0;i<n;i++)if(mode==3||(mode>0&&i%2==0)){owned[pool[i]]=1;own++;g.deck[g.deck_count++]=pool[i]|(mode==2?UPBIT:0);}
   t=g;int samples=60000;
   for(int j=0;j<samples;j++){int id=reward_of_rarity(&g,rarity,-1,-1);assert(solo_reward_eligible(id)&&cards[id].rarity==rarity);seen[id]++;got+=!owned[id];offers++;}
   double expected=(double)(3*(n-own))/(3*(n-own)+own),actual=(double)got/samples;assert(fabs(actual-expected)<.012);
   for(int i=0;i<n;i++)assert(seen[pool[i]]>0);
   assert(g.deck_count==t.deck_count&&!memcmp(g.deck,t.deck,sizeof(g.deck)));
   printf("PASS rarity %d ownership mode %d: unseen %.3f, expected %.3f; every eligible card remains possible\n",rarity,mode,actual,expected);
   /* Multiple owned copies do not further suppress a card. */
   if(mode==2){g=t;g.deck[g.deck_count++]=pool[0];int a=reward_of_rarity(&g,rarity,-1,-1),b=reward_of_rarity(&t,rarity,-1,-1);assert(a==b&&g.rng==t.rng);}
  }
 }
 int counts[3]={0};
 for(int seed=1;seed<=20000;seed++){
  game_new(&g,seed);for(int id=3;id<CARD_COUNT;id++)if(solo_reward_eligible(id)&&id%2==0)g.deck[g.deck_count++]=id|UPBIT;t=g;
  encounter(&g,NODE_SHOP);encounter(&t,NODE_SHOP);assert(!memcmp(&g,&t,sizeof(g)));
  for(int i=0;i<3;i++){int id=g.shop_cards[i];assert(solo_reward_eligible(id));counts[cards[id].rarity]++;for(int j=0;j<i;j++)assert(id!=g.shop_cards[j]);}
  if(seed==1){assert(game_save(&g,"qa/v150/evidence/weighted-shop.dat"));assert(game_load(&t,"qa/v150/evidence/weighted-shop.dat"));assert(!memcmp(&g,&t,sizeof(g)));}
 }
 assert(fabs(counts[2]/60000.-.09)<.012&&fabs(counts[1]/60000.-.37)<.012&&fabs(counts[0]/60000.-.54)<.012);
 printf("PASS %d weighted selections and 60000 distinct shop offers; 3:1 unseen/owned card weights, base/upgraded equivalence, all-owned fallback, deterministic save and unchanged 54/37/9 shop rarity rates (%d/%d/%d)\n",offers,counts[0],counts[1],counts[2]);
}
