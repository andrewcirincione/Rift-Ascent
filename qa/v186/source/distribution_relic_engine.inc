/* Called at explicit combat boundaries; each trigger owns its counter. */
static void dist_relic_trigger(Game*g,int event){
 int empty=!g->regent.stars;
 for(int i=0;i<g->relic_count;i++){
  int id=game_relic_id(g,i);if(id<0||id>=RELIC_COUNT)continue;
  const DistRelic*r=&dist_relics[id];int a=r->a,b=r->b,t=r->trigger;
  if(event==0){
   if(t==DR_START){relic_block(g,a);regent_gain_stars(g,b);}
   if(t==DR_SHOP)regent_gain_stars(g,b);
   if(t==DR_WITHER){ninja_poison_all(g,a);relic_block(g,b);}
   if(t==DR_TWOFOES&&g->enemy_count>=2){relic_block(g,a);g->strength+=b;}
   if(t==DR_WARDEN)regent_gain_stars(g,b);
   if(t==DR_VESPER)for(int k=0;k<b;k++)draw_card(g);
   if(t==DR_PROXY)regent_gain_stars(g,a);
   if(t==DR_ENERGY){g->weak+=a;relic_block(g,b);}
  }else if(event==1){
   if(t==DR_TURN2&&g->turn==2){relic_block(g,a);for(int k=0;k<b;k++)draw_card(g);}
   if(t==DR_TURN3&&g->turn==3){relic_block(g,a);gain_energy(g,b);}
   if(t==DR_PROXY&&g->turn==3)for(int k=0;k<b;k++)draw_card(g);
   if(t==DR_TURN4&&g->turn==4){relic_block(g,a);g->strength+=b;}
   if(t==DR_LOWTURN&&g->hp*2<g->maxhp&&!g->dist.relic_used[id]){g->dist.relic_used[id]=1;relic_block(g,a);regent_gain_stars(g,b);}
  }else if(event==2){
   if(t==DR_EMPTY&&empty){relic_block(g,a);regent_gain_stars(g,b);}
   if(t==DR_BANK&&g->energy>0){relic_block(g,a);ninja_poison_all(g,b);}
  }else if(event==3){
   if(t==DR_WIN){heal(g,a);g->gold+=b;}
   if(t==DR_WARDEN)heal(g,a);
  }else if(event==4&&t==DR_POTION){relic_block(g,a);regent_gain_stars(g,b);}
  else if(event==5&&t==DR_HURT&&!g->dist.relic_used[id]){g->dist.relic_used[id]=1;relic_block(g,a);regent_gain_stars(g,b);}
  else if(event==6&&t==DR_SKIP){g->gold+=a;heal(g,b);}
  else if(event==7&&t==DR_REST){heal(g,a);g->gold+=b;}
 }
}
static void dist_relic_play(Game*g,int type,int switched,int spent){
 for(int i=0;i<g->relic_count;i++){
  int id=game_relic_id(g,i);if(id<0||id>=RELIC_COUNT)continue;
  const DistRelic*r=&dist_relics[id];int a=r->a,b=r->b,t=r->trigger,yes=0;
  if(t==DR_SWITCH&&switched&&!(g->dist.relic_turn[id]&1)){g->dist.relic_turn[id]|=1;yes=1;}
  if(t==DR_VESPER&&switched&&!(g->dist.relic_turn[id]&1)){g->dist.relic_turn[id]|=1;relic_block(g,a);}
  if(t==DR_RELEASE&&spent&&!(g->dist.relic_turn[id]&2)){g->dist.relic_turn[id]|=2;yes=1;}
  if(t==DR_FOURTH&&g->turn_played==4)yes=1;
  if(t==DR_SIXTH&&g->dist.cards_played==6)yes=1;
  if(t==DR_FIRST&&type==0&&!g->dist.relic_used[id]){g->dist.relic_used[id]=1;yes=1;}
  if(yes){relic_block(g,a);regent_gain_stars(g,b);}
  if(t==DR_POWER&&type==2&&!g->dist.relic_used[id]){g->dist.relic_used[id]=1;relic_block(g,a);for(int k=0;k<b;k++)draw_card(g);}
  if(t==DR_THIRD_SKILL&&type==1&&g->dist.skills==3){relic_block(g,a);ninja_poison_all(g,b);}
  if(t==DR_RELEASE3&&spent>=3&&!g->dist.relic_used[id]){g->dist.relic_used[id]=1;relic_block(g,a);gain_energy(g,b);}
 }
}
static int dist_shop_discount(const Game*g){int n=0;for(int i=0;i<g->relic_count;i++){int id=game_relic_id(g,i);if(id>=0&&id<RELIC_COUNT&&dist_relics[id].trigger==DR_SHOP)n+=dist_relics[id].a;}return MIN(65,n);}
