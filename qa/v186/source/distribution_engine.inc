/* Rift Ascent distribution rules. Declarative operations replace the inherited
   character card engines. Echo is bounded; no operation schedules itself. */
static int dist_has_op(int raw,int kind){int id=CARD_ID(raw);if(id>=CARD_COUNT)return 0;for(int i=0;i<dist_cards[id].count;i++)if(dist_cards[id].op[i].kind==kind)return 1;return 0;}
static int dist_base_damage(int raw){int id=CARD_ID(raw),n=0;if(id>=CARD_COUNT)return 0;for(int i=0;i<dist_cards[id].count;i++){const DistOp*o=&dist_cards[id].op[i];if(o->kind==DC_HIT||o->kind==DC_SWEEP||o->kind==DC_BRACE_HIT||o->kind==DC_MARK_HIT)n+=o->n+CARD_UP(raw)*o->u;}return n;}
static int ninja_flags(int raw){(void)raw;return 0;}
static int regent_flags(int raw){(void)raw;return 0;}
static int ninja_stamp(Game*g,int raw){(void)g;return raw;}
static int regent_stamp(Game*g,int raw){(void)g;return raw;}
int game_card_is_sly(const Game*g,int raw){(void)g;(void)raw;return 0;}
int game_card_is_retained(const Game*g,int raw){(void)g;return CARD_MOD(raw)==MOD_BOUND||!!(raw&CARD_RETAIN_TURN);}
static int card_character(int id){return id>=0&&id<CARD_COUNT?dist_cards[id].character:-1;}
static unsigned custom_deck_characters(const Game*g){unsigned mask=0;for(int i=0;i<g->deck_count;i++){int c=card_character(CARD_ID(g->deck[i]));if(c>=0)mask|=1u<<c;}return mask?mask:1u<<g->character;}
static int ninja_owned_card(const Game*g,int id){if(id<0||id>=CARD_COUNT||!ascent_card_available(g,id))return 0;int c=card_character(id);return c<0|| (g->revision184.custom?!!(custom_deck_characters(g)&(1u<<c)):c==g->character);}
static int ninja_base_damage(const Game*g,int raw,int in_hand){(void)g;(void)in_hand;return dist_base_damage(raw);}
static int regent_base_damage(const Game*g,int raw){(void)g;return dist_base_damage(raw);}
static int regent_cost_now(const Game*g,int raw){(void)g;return game_card_cost(raw);}
int game_card_star_cost(const Game*g,int raw){(void)g;(void)raw;return 0;}
int game_shows_stars(const Game*g){return g!=0;}
static void regent_gain_stars(Game*g,int n){if(n<=0)return;int add=MIN(n,9-g->regent.stars);if(add<=0)return;g->regent.stars+=add;record_combat_fx(GFX_STARS,-1,-1,add,0);}
static void regent_forge(Game*g,int n){/* Event resource conversion: no permanent sword exists. */regent_gain_stars(g,(n+2)/3);}
static void ninja_proc(Game*g,int e,int n,int ignore){if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0||n<=0)return;int b=ignore?0:MIN(n,g->enemies[e].block);g->enemies[e].block-=b;g->enemies[e].hp-=n-b;record_combat_fx(GFX_PROC,-1,e,n,b);}
static void ninja_poison(Game*g,int e,int n){if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0||n<=0)return;g->enemies[e].poison=MIN(999,g->enemies[e].poison+n);record_combat_fx(GFX_STATUS,-1,e,4,0);}
static void ninja_poison_all(Game*g,int n){for(int e=0;e<g->enemy_count;e++)ninja_poison(g,e,n);}
static void ninja_on_draw(Game*g,int raw){(void)g;(void)raw;}
static void ninja_on_hit(Game*g,int e,int n){(void)g;(void)e;(void)n;}
static void regent_on_draw(Game*g,int raw){(void)g;(void)raw;}
static void regent_on_hit(Game*g,int e,int n){(void)g;(void)e;(void)n;}
static void regent_created(Game*g){(void)g;}
static void regent_after_shuffle(Game*g){(void)g;}
static void ninja_process(Game*g){(void)g;}
static void regent_process(Game*g){(void)g;}
static void regent_foreign_begin(Game*g,int raw){(void)g;(void)raw;}
static void regent_foreign_finish(Game*g,int raw){(void)g;(void)raw;}
static void ninja_start_turn_before(Game*g){(void)g;}
static void ninja_start_turn_after(Game*g){(void)g;}
static void regent_before_turn(Game*g){(void)g;}
static int ninja_prepare_retain(Game*g){(void)g;return 0;}
static void ninja_choose(Game*g,int index,int finish){(void)g;(void)index;(void)finish;}
static void regent_choose(Game*g,int index,int finish){(void)g;(void)index;(void)finish;}
int game_ninja_choice_eligible(const Game*g,int i){(void)g;(void)i;return 0;}
const char*game_ninja_choice_title(const Game*g){(void)g;return "CHOOSE A CARD";}
int game_regent_choice_count(const Game*g){(void)g;return 0;}
int game_regent_choice_raw(const Game*g,int i){(void)g;(void)i;return 0;}
int game_regent_choice_eligible(const Game*g,int i){(void)g;(void)i;return 0;}
const char*game_regent_choice_title(const Game*g){(void)g;return "CHOOSE A CARD";}
static void regent_start_turn(Game*g){
 memset(g->dist.relic_turn,0,sizeof(g->dist.relic_turn));g->dist.last_type=-1;g->dist.earned=g->dist.skills=g->dist.spent=g->dist.flags=0;
 if(g->character==CHAR_PROXY)regent_gain_stars(g,1+MIN(2,g->dist.bank));
 g->dist.bank=0;
 if(g->dist.next_energy){gain_energy(g,g->dist.next_energy);g->dist.next_energy=0;}
 if(g->dist.next_guard){relic_block(g,g->dist.next_guard);g->dist.next_guard=0;}
 int draws=g->dist.next_draw;g->dist.next_draw=0;for(int i=0;i<draws;i++)draw_card(g);
 if(g->dist.power[DP_BEACON]&&g->regent.stars>=4){relic_block(g,g->dist.power[DP_BEACON]);draw_card(g);}
 if(g->dist.power[DP_WELL]&&!g->regent.stars){regent_gain_stars(g,2);relic_block(g,g->dist.power[DP_WELL]);}
 if(g->dist.power[DP_MEND]&&(g->turn==3||g->turn==6))heal(g,g->dist.power[DP_MEND]);
}
static void regent_end_turn(Game*g){
 if(g->ic[38])return;
 g->dist.bank=MIN(2,g->energy);
 if(g->energy>0&&g->dist.power[DP_RESERVE])relic_block(g,g->dist.power[DP_RESERVE]);
 if(g->regent.stars>=6&&g->dist.power[DP_CHORUS])ninja_poison_all(g,g->dist.power[DP_CHORUS]);
}
static int dist_release(Game*g,int cap){
 int n=MIN(cap,g->regent.stars);g->regent.stars-=n;g->dist.spent+=n;
 if(n&&!(g->dist.flags&2)){g->dist.flags|=2;if(g->dist.power[DP_WELD])relic_block(g,g->dist.power[DP_WELD]);}
 if(n>=3&&g->dist.power[DP_RETURN]&&!(g->dist.flags&4)){g->dist.flags|=4;relic_block(g,g->dist.power[DP_RETURN]);g->dist.next_energy++;}
 return n;
}
static void dist_effect(Game*g,int raw,int e){
 const DistCard*d=&dist_cards[CARD_ID(raw)];int up=CARD_UP(raw);
 for(int i=0;i<d->count&&g->hp>0;i++){
  const DistOp*o=&d->op[i];int n=o->n+o->u*up,p=o->aux,release;
  switch(o->kind){
  case DC_HIT:for(int k=0;k<MAX(1,p);k++)deal(g,e,n);break;
  case DC_SWEEP:all_damage(g,n);break;
  case DC_GUARD:card_block(g,n);break;
  case DC_WITHER:if(p)ninja_poison_all(g,n);else ninja_poison(g,e,n);break;
  case DC_WEAK:for(int j=0;j<g->enemy_count;j++)if((p||j==e)&&g->enemies[j].hp>0)g->enemies[j].weak=MIN(99,g->enemies[j].weak+n);break;
  case DC_EXPOSE:for(int j=0;j<g->enemy_count;j++)if(p||j==e)apply_vulnerable(g,&g->enemies[j],n);break;
  case DC_DRAW:for(int j=0;j<n;j++)draw_card(g);break;
  case DC_ECHO:regent_gain_stars(g,n);break;
  case DC_VENT_HIT:release=dist_release(g,p);if(release)deal(g,e,n*release);break;
  case DC_VENT_GUARD:release=dist_release(g,p);if(release)card_block(g,n*release);break;
  case DC_VENT_WITHER:release=dist_release(g,p);if(release)ninja_poison(g,e,n*release);break;
  case DC_BRACE_HIT:deal(g,e,n+(g->block>0?p:0));break;
  case DC_MARK_HIT:deal(g,e,n+(g->enemies[e].poison>0?p:0));break;
  case DC_THREAT_GUARD:{int yes=0;for(int j=0;j<g->enemy_count;j++)yes|=game_enemy_intent_hits(g,j)>0;card_block(g,n+(yes?p:0));break;}
  case DC_LOW_GUARD:card_block(g,n+(g->hp*2<g->maxhp?p:0));break;
  case DC_RESERVE_GUARD:card_block(g,n+p*MIN(4,g->regent.stars));break;
  case DC_TEMP_FORCE:g->strength+=n;g->ic[IC_TEMP_STRENGTH]+=n;break;
  case DC_NEXT_GUARD:g->dist.next_guard=MIN(999,g->dist.next_guard+n);break;
  case DC_NEXT_DRAW:g->dist.next_draw=MIN(MAX_HAND,g->dist.next_draw+n);break;
  case DC_HEAL:heal(g,n);break;
  case DC_RECALL:if(g->discard_count&&g->hand_count<MAX_HAND){int at=0;for(int j=1;j<g->discard_count;j++)if(game_card_cost(g->discard[j])<game_card_cost(g->discard[at]))at=j;int c=g->discard[at];for(int j=at;j+1<g->discard_count;j++)g->discard[j]=g->discard[j+1];g->discard_count--;add_generated(g,c);}break;
  case DC_FOLD:if(g->hand_count&&g->draw_count<MAX_PILE){g->draw[g->draw_count++]=g->hand[--g->hand_count];card_block(g,n);}break;
  case DC_POWER:g->dist.power[p]=MIN(999,g->dist.power[p]+n);g->dist.power_raw[p]=raw;break;
  case DC_SHARD:for(int j=0;j<n;j++)add_generated(g,RIFT_CARD_165|(up?UPBIT:0));break;
  case DC_STRIP:g->enemies[e].block=MAX(0,g->enemies[e].block-n);break;
  }
 }
}
static void resolve_card(Game*g,int index,int enemy,int replay_raw){
 if(!g||g->screen!=COMBAT||index>=g->hand_count||(index<0&&replay_raw<0))return;
 int raw=index<0?replay_raw:g->hand[index],id=CARD_ID(raw);
 if(!game_card_is_playable(raw))return;
 int cost=index<0?0:game_card_cost_now(g,raw);if(cost<0||g->energy<cost){msg(g,"More Energy is needed.");return;}
 if(enemy<0||enemy>=g->enemy_count||g->enemies[enemy].hp<=0)enemy=random_living_foe(g);if(enemy<0)return;
 int saved_raw=r150_card_raw,saved_attack=r140_attack_context,saved_fx=combat_fx_card,saved_phase=preview_phase;
 int spent_before=g->dist.spent;int type=cards[id].type,old_guard=g->block,switched=type<2&&g->dist.last_type>=0&&g->dist.last_type!=type;
 r150_card_raw=raw;r140_attack_context=type==0;r140_rust_targets=0;
 if(!preview_trace)combat_fx_card=raw;
 if(preview_trace){preview_depth++;preview_phase=preview_depth<=2;if(preview_depth==2)preview_trace->repeated++;}
 g->energy-=cost;
 if(index>=0){for(int j=index;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];g->hand_count--;}
 if(type==0&&g->dist.last_type==1&&!(g->dist.flags&8)){g->regent.attack_vigor=g->dist.power[DP_WHEEL];g->dist.flags|=8;}
 int before[9];for(int j=0;j<g->enemy_count;j++){before[j*3]=g->enemies[j].weak;before[j*3+1]=g->enemies[j].vulnerable;before[j*3+2]=g->enemies[j].poison;}
 dist_effect(g,raw,enemy);g->regent.attack_vigor=0;
 if(CARD_MOD(raw)==MOD_KINDLED)draw_card(g);
 g->turn_played++;g->dist.cards_played++;
 if(type==0){g->ic[IC_ATTACKS_THIS_TURN]++;g->shop_relic++;if(legacy_relic(g,5)&&g->ic[IC_ATTACKS_THIS_TURN]%3==0)g->strength++;if(legacy_relic(g,24)&&g->ic[IC_ATTACKS_THIS_TURN]%3==0)heal(g,3);}
 if(type==1){g->dist.skills++;if(++g->ic[36]%3==0&&legacy_relic(g,22))g->dexterity++;}
 if(g->character==CHAR_WARDEN&&type==1&&g->block>old_guard&&g->dist.earned<2){int danger=0;for(int j=0;j<g->enemy_count;j++)danger|=game_enemy_intent_hits(g,j)>0;if(danger){g->dist.earned++;regent_gain_stars(g,1);}}
 if(g->character==CHAR_VESPER&&switched&&g->dist.earned<3){g->dist.earned++;regent_gain_stars(g,1);}
 if(switched&&g->dist.power[DP_BRIDGE]&&!(g->dist.flags&1)){g->dist.flags|=1;relic_block(g,g->dist.power[DP_BRIDGE]);regent_gain_stars(g,1);}
 if(g->turn_played==4&&g->dist.power[DP_REFRAIN]){for(int j=0;j<g->enemy_count;j++)ninja_proc(g,j,g->dist.power[DP_REFRAIN],0);draw_card(g);}
 if(g->dist.skills==3&&type==1&&g->dist.power[DP_BELLOWS])for(int j=0;j<g->enemy_count;j++)ninja_proc(g,j,g->dist.power[DP_BELLOWS],0);
 if(type==0&&g->enemies[enemy].poison&&g->dist.power[DP_VEIL]&&!(g->dist.flags&16)){g->dist.flags|=16;relic_block(g,g->dist.power[DP_VEIL]);}
 if(type<2)g->dist.last_type=type;
 dist_relic_play(g,type,switched,g->dist.spent-spent_before);
 if(legacy_relic(g,43)&&type<2){g->ic[IC_DUELIST]|=type?2:1;if(g->ic[IC_DUELIST]==3){g->ic[IC_DUELIST]|=4;gain_energy(g,1);}}
 if(index>=0&&CARD_MOD(raw)==MOD_ECHO&&g->hp>0)resolve_card(g,-1,enemy,raw|CARD_EPHEMERAL|CARD_FREE);
 if(index>=0){int clean=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);if(type==2){/* Powers leave the cycle. */}else if(dist_cards[id].exile||(raw&CARD_FORCE_EXHAUST)||CARD_MOD(raw)==MOD_FLEETING||CARD_MOD(raw)==MOD_KINDLED)exhaust_card(g,clean);else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=clean;}
 if(g->target<0||g->target>=g->enemy_count||g->enemies[g->target].hp<=0)select_live_enemy(g,1);
 if(!preview_trace)check_win(g); /* Preview must never persist an account result. */
 r150_card_raw=saved_raw;r140_attack_context=saved_attack;combat_fx_card=saved_fx;
 if(preview_trace){preview_depth--;preview_phase=saved_phase;}
}
void game_play_card(Game*g,int index,int enemy){if(index>=0)resolve_card(g,index,enemy,-1);}
static void regent_play(Game*g,int index,int target){game_play_card(g,index,target);}

#include "distribution_relic_engine.inc"
