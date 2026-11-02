/* Later journey gifts improve without adding choices or extra story rolls.
   Quality rolls use a room hash; browsing never rerolls a prepared reward. */
static unsigned journey_quality(const Game*g,unsigned salt){
 return route_hash(g->seed^(unsigned)g->act*17011u^(unsigned)g->floor*619u^salt*7919u);
}
static int journey_rarity(const Game*g,int rarity,unsigned salt){
 if(g->act>=3)rarity=MAX(rarity,2);
 else if(g->act==2&&journey_quality(g,salt)%2u==0)rarity=MIN(2,rarity+1);
 if(g->act>=5&&journey_quality(g,salt+71)%100u<(unsigned)MIN(50,25+5*(g->act-5)))rarity=3;
 return rarity;
}
static int journey_upgrade(const Game*g,int raw,unsigned salt){
 int chance=g->act>=4?100:g->act==3?50:g->act==2?25:0;
 return game_card_is_upgradeable(raw)&&journey_quality(g,salt+193)%100u<(unsigned)chance?raw|UPBIT:raw;
}
static int journey_relic(Game*g,int preferred,int excluded){
 if(g->relic_count>=MAX_RELICS)return -1;
 if(g->act<=1)return preferred>=0&&!relic(g,preferred)&&preferred!=excluded?preferred:random_relic_except(g,excluded);
 /* Reliable immediate or recurring combat benefits replace narrow late gifts.
    Act 3 adds Severed Psalm; all candidates remain unowned and unlock-safe. */
 static const int early[]={74,75,76,77,83,85,89,93};
 static const int late[]={74,75,77,83,85,87,89,93};
 const int*ids=g->act==2?early:late;int pool[8],n=0;
 for(int i=0;i<8;i++)if(ids[i]!=excluded&&!relic(g,ids[i])){if(ids[i]==preferred)return preferred;pool[n++]=ids[i];}
 return n?pool[rnd(g,n)]:random_relic_except(g,excluded);
}
