#include "game.h"
#include "story170_data.h"
#include "expedition_stories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A private observer is enabled only while resolving a copied Game for a
   preview. No renderer, audio, save, or live RNG is touched by the probe. */
static CardPreview *preview_trace;
static int ascent_complete(Game*,int);
static int custom_action(Game*,int);
static int ascent_begin_mode(Game*,int,int,unsigned,int);
static int expedition_action(Game*,int);
static void heal(Game*,int);
static void relic_block(Game*,int);
static void expedition_roll(Game*);
static void expedition_prepare_gift(Game*);
static void expedition_mystery(Game*);
static void expedition_boss_spoils(Game*);
static int expedition_relic(Game*);
static void revision182_migrate_room(Game*);
static void ascent_record_act(Game*);
static void ascent_repair_ending(Game*);
static int ascent_random_potion(Game*);
static int preview_depth,preview_phase,preview_choice=-1,preview_nested;
/* Transient call-stack value: a repeated X Attack retains its original X. */
static int repeated_x=-1;
static void preview_hit(int enemy,int amount){
 if(!preview_trace||!preview_phase)return;
 CardPreview*p=preview_trace;
 if(preview_nested)p->uncertain=1;
 if(!p->hits[enemy]||amount<p->low[enemy])p->low[enemy]=amount;
 if(!p->hits[enemy]||amount>p->high[enemy])p->high[enemy]=amount;
 p->hits[enemy]++;p->total[enemy]+=amount;
 if(preview_depth>1)p->repeat_hits++;
}

/* Presentation observes resolution; it never changes combat or consumes RNG. */
static GameCombatFx combat_fx[MAX_COMBAT_FX];
static int r140_attack_context,r140_rust_targets;
static int r150_card_raw;
static int combat_fx_count,combat_fx_recording,combat_fx_source=-1,combat_fx_card=-1;
void game_combat_fx_begin(void){combat_fx_count=0;combat_fx_recording=1;combat_fx_source=combat_fx_card=-1;}
void game_combat_fx_end(void){combat_fx_recording=0;}
const GameCombatFx *game_combat_fx_events(int*count){if(count)*count=combat_fx_count;return combat_fx;}
int game_impact_tier(int damage){return damage>20?3:damage>10?2:damage>0?1:0;}
static void record_combat_fx(int kind,int source,int target,int amount,int absorbed){
 if(!combat_fx_recording||preview_trace||combat_fx_count>=MAX_COMBAT_FX)return;
 GameCombatFx*e=&combat_fx[combat_fx_count++];
 e->kind=kind;e->source=source;e->target=target;e->amount=amount;e->absorbed=absorbed;e->card=combat_fx_card;
}

#include "distribution_catalog.inc"
#include "new_potions.h"
static int dist_has_op(int raw,int kind);
static int dist_base_damage(int raw);
static int card_cost_base(int raw){int id=CARD_ID(raw);return id>=0&&id<CARD_COUNT?cards[id].cost:-2;}
static const char *card_description_base(int raw){
 int id=CARD_ID(raw);
 if(id<0||id>=CARD_COUNT)return "Unknown card";
 if(CARD_UP(raw)&&id<CARD_COUNT&&upgrade_descriptions[id])return upgrade_descriptions[id];
 return cards[id].desc;
}

#include "card_mods.inc"


static void exhaust_card(Game*g,int raw);
static void resolve_card(Game*g,int index,int enemy,int replay_raw);
static int regent_flags(int raw);
static int regent_stamp(Game*g,int raw);
static void regent_created(Game*g);
static void regent_on_draw(Game*g,int raw);
static void regent_on_hit(Game*g,int e,int amount);
static void regent_after_shuffle(Game*g);
static void regent_start_turn(Game*g);
static void regent_before_turn(Game*g);
static void regent_end_turn(Game*g);
static int regent_base_damage(const Game*g,int raw);
static void regent_choose(Game*g,int index,int finish);
static void regent_play(Game*g,int index,int target);
static int regent_cost_now(const Game*g,int raw);
static void regent_gain_stars(Game*g,int n);
static int regent_processing;
static void regent_foreign_begin(Game*g,int raw);
static void regent_foreign_finish(Game*g,int raw);

static int ninja_flags(int raw);
static int ninja_stamp(Game*g,int raw);
static int ninja_owned_card(const Game*g,int id);
static void r151_self_loss(Game*g);
static void r151_exhaust(Game*g);
static void r151_played(Game*g,int type);
static void r151_status_snapshot(const Game*g,int out[6]);
static void r151_after_effect(Game*g,const int before[6]);
static void r151_skip_reward(Game*g);
static int ninja_base_damage(const Game*g,int raw,int in_hand);
static void ninja_on_draw(Game*g,int raw);
static void ninja_on_hit(Game*g,int enemy,int unblocked);
static void ninja_poison(Game*g,int enemy,int n);
static void ninja_poison_all(Game*g,int n);
static void ninja_process(Game*g);
static void ninja_start_turn_before(Game*g);
static void ninja_start_turn_after(Game*g);
static int ninja_prepare_retain(Game*g);
static void ninja_choose(Game*g,int index,int finish);


#include "distribution_relics.inc"
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CLAMP(x,a,b) MAX(a,MIN(x,b))

uint32_t game_random(Game *g){uint32_t x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;g->rng=x?x:0x4563871bu;return g->rng;}
static int rnd(Game *g,int max){return max>0?(int)(game_random(g)%(uint32_t)max):0;}
int game_relic_id(const Game*g,int i){if(i<0||i>=MAX_RELICS)return -1;return i<LEGACY_RELIC_CAP?g->relics[i]:g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int*relic_slot(Game*g,int i){return i<LEGACY_RELIC_CAP?&g->relics[i]:&g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int relic(const Game *g,int id){for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==id)return 1;return 0;}
static int legacy_relic(const Game*g,int id){(void)g;(void)id;return 0;}
static void dist_relic_trigger(Game*g,int event);
static void dist_relic_play(Game*g,int type,int switched,int spent);
static int dist_shop_discount(const Game*g);
int game_shop_discount(const Game*g){return dist_shop_discount(g);}
static int dist_evaluating;
static void enter_map(Game*g){
 if(g->revision182.room_pending){g->revision182.room_pending=0;expedition_roll(g);}
 if(g->expedition.story_pending){g->expedition.story_pending=0;g->screen=EXPEDITION_STORY;g->selection=0;expedition_prepare_gift(g);return;}
 g->screen=MAP;g->scroll=0;g->selection=CLAMP(g->lane,0,2);
 if(g->floor<MAP_ROWS&&!game_route_available(g,g->selection))
   for(int k=0;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}
}
static void msg(Game*g,const char *s){snprintf(g->message,sizeof(g->message),"%s",s);}
static void add_card(Game*g,int c){if(CARD_ID(c)>=ASCENT_CARD_FIRST&&!ascent_card_available(g,CARD_ID(c)))return;
 if(g->deck_count<MAX_DECK)g->deck[g->deck_count++]=c;}
int game_card_is_playable(int raw){
 int id=CARD_ID(raw);
 return id>=0&&id<CARD_COUNT&&!(id>=CARD_STATUS_MIN&&id<=CARD_STATUS_MAX);
}
int game_card_is_upgradeable(int raw){return game_card_is_playable(raw)&&!CARD_UP(raw)&&CARD_ID(raw)!=91&&CARD_ID(raw)!=RIFT_CARD_165&&CARD_ID(raw)!=RIFT_CARD_208;}
static int solo_reward_eligible(int id){return id>=3&&id<CARD_COUNT&&game_card_is_playable(id)&&id!=91&&id!=RIFT_CARD_165&&id!=RIFT_CARD_208&&cards[id].rarity<=3;}
/* Legacy Proxy starters/tokens share rarity 3 with Ancient gifts. Keep old
   saved offers readable, but never generate these as new reward/shop cards. */
static int combat_reward_eligible(int id){return solo_reward_eligible(id)&&id!=RIFT_CARD_269&&id!=RIFT_CARD_210&&id!=RIFT_CARD_278&&id!=RIFT_CARD_213&&id!=RIFT_CARD_173&&id!=RIFT_CARD_120;}
int game_collection_unlock_count(int tab){return tab==4?10:20;}
static int collection_base_card(int tab,int id){return tab>=0&&tab<3&&id>=0&&id<ASCENT_CARD_FIRST&&game_card_is_playable(id)&&(dist_cards[id].character==tab||dist_cards[id].character<0);}
int game_collection_id(int tab,int index){
 if(tab<0||tab>=5||index<0)return -1;
 int custom=game_collection_unlock_count(tab);
 if(tab<3){
  int first=index<custom?ASCENT_CARD_FIRST+20*tab:0,last=index<custom?first+20:ASCENT_CARD_FIRST;
  if(index>=custom)index-=custom;
  for(int type=0;type<4;type++)for(int id=first;id<last;id++)if(cards[id].type==type&&(first||collection_base_card(tab,id))&&index--==0)return id;
  return -1;
 }
 if(index<custom)return tab==3?6+index:64+index;
 index-=custom;if(tab==3)return index<6?index:-1;
 for(int id=0;id<RELIC_COUNT;id++)if((id<64||id>=74)&&id!=30&&(id<37||id>39)&&index--==0)return id;
 return -1;
}
int game_collection_unlocked(int tab,int index){
 int id=game_collection_id(tab,index);if(id<0)return 0;
 if(index>=game_collection_unlock_count(tab))return 1;
 const AscentAccount*a=ascent_profile();unsigned mask=tab<3?a->cards[tab]:tab==3?a->potions:a->relics;
 int bit=tab<3?id-(ASCENT_CARD_FIRST+20*tab):tab==3?id-6:id-64;
 return !!(mask&(1u<<bit));
}
int game_collection_count(int tab){
 if(tab<0||tab>=5)return 0;
 if(tab==3)return 26;if(tab==4)return 90;
 int count=20;for(int id=0;id<ASCENT_CARD_FIRST;id++)count+=collection_base_card(tab,id);
 return count;
}
static int reward_of_rarity(Game*g,int rarity,int excluded0,int excluded1){
 int pool[512],weight[512],n=0,total=0;
 for(int id=3;id<CARD_COUNT;id++)if(combat_reward_eligible(id)&&ninja_owned_card(g,id)&&cards[id].rarity==rarity&&id!=excluded0&&id!=excluded1){
  int owned=0;for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==id){owned=1;break;}
  /* Upgraded copies count as owned. Duplicates remain possible, at one third
     the per-card weight of an unseen card in the same rarity tier. */
  pool[n]=id;weight[n]=owned?1:3;total+=weight[n++];
 }
 if(!n&&rarity==2){for(int id=3;id<CARD_COUNT;id++)if(combat_reward_eligible(id)&&ninja_owned_card(g,id)&&cards[id].rarity==2)return id;}
 if(!n&&rarity==3)return reward_of_rarity(g,2,excluded0,excluded1);
 if(!n)return g->character==CHAR_VESPER?RIFT_CARD_104:g->character==CHAR_PROXY?RIFT_CARD_210:5;
 int roll=rnd(g,total);for(int i=0;i<n;i++){if(roll<weight[i])return pool[i];roll-=weight[i];}
 return pool[n-1];
}
/* Pity-adjusted reward bands. Ancient cards are
   reserved for character story gifts and explicit mystery rewards. */
static int roll_rarity(Game*g,int rare_base,int uncommon,int use_offset,int advance){
 int rare=rare_base+(use_offset?g->revision182.rare_offset:0),roll=rnd(g,100);
 int result=roll<rare?2:roll<rare+uncommon?1:0;
 if(advance)g->revision182.rare_offset=result==2?-5:MIN(40,g->revision182.rare_offset+1);
 return result;
}
static int card_reward_except(Game*g,int a,int b){return reward_of_rarity(g,roll_rarity(g,9,37,1,0),a,b);}
static int card_reward(Game*g){return reward_of_rarity(g,roll_rarity(g,3,34,0,0),-1,-1);}
static void fill_rewards(Game*g){
 for(int i=0;i<3;i++){
  int rare=(g->battle_type==NODE_BOSS||g->win_screen)?(rnd(g,100)<10?3:2):roll_rarity(g,g->battle_type==NODE_ELITE?10:3,g->battle_type==NODE_ELITE?40:37,1,1);
  if(g->battle_type==NODE_BOSS)g->revision182.rare_offset=-5;
  int c=reward_of_rarity(g,rare,i>0?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1);
  int upgraded=legacy_relic(g,13)||(rare<2&&rnd(g,100)<MIN(2,g->act-1)*25);
  g->reward[i]=c|(upgraded?UPBIT:0);
 }
}
static void repair_generated_offers(Game*g){
 ascent_repair_ending(g);
 /* Keep old saves and already-owned cards intact, replacing only unclaimed
    Debris offers produced when its legacy rarity was mistaken for Ancient. */
 /* The removed third story-gift choice becomes Leave in an old save. */
 if(g->screen==EXPEDITION_GIFT&&g->selection==2)g->selection=1;
 int screen=g->screen;
 if(screen==DECK_VIEW||screen==POTION_VIEW||screen==RELIC_VIEW)screen=g->prior_screen;
 if((screen==REWARD||screen==REWARD_PREVIEW)&&!g->v151.card_done){
  for(int i=0;i<3;i++)if(CARD_ID(g->reward[i])==RIFT_CARD_208)
   g->reward[i]=reward_of_rarity(g,3,CARD_ID(g->reward[(i+1)%3]),CARD_ID(g->reward[(i+2)%3]))|(g->reward[i]&UPBIT);
 }
 if(screen==SHOP)for(int i=0;i<3;i++)if(g->shop_cards[i]>=0&&CARD_ID(g->shop_cards[i])==RIFT_CARD_208)
  g->shop_cards[i]=reward_of_rarity(g,3,-1,-1)|(g->shop_cards[i]&UPBIT);
 if(screen==EVENT&&(g->effect_turn==2||g->effect_turn==6||g->effect_turn==17||g->effect_turn==34))
  for(int i=0;i<(g->effect_turn==17?1:2);i++)if(CARD_ID(g->reward[i])==RIFT_CARD_208)g->reward[i]=reward_of_rarity(g,3,-1,-1)|(g->reward[i]&UPBIT);
 if((screen==EXPEDITION_STORY||screen==EXPEDITION_GIFT)&&!g->expedition.gift_kind&&CARD_ID(g->expedition.gift)==RIFT_CARD_208)
  g->expedition.gift=reward_of_rarity(g,3,-1,-1)|(g->expedition.gift&UPBIT);
}
static void add_relic(Game*g,int id){
 if(id<0||id>=RELIC_COUNT)return;
 const DistRelic*r=&dist_relics[id];int repeat=id>=37&&id<=39;
 if(!repeat&&(g->relic_count>=MAX_RELICS||relic(g,id)))return;
 if(!repeat)*relic_slot(g,g->relic_count++)=id;
 if(r->trigger==DR_PICKHP){g->maxhp+=r->a;g->hp=MIN(g->maxhp,g->hp+r->a);g->gold+=r->b;}
 if(r->trigger==DR_PICKFORCE){g->permanent_strength+=r->a;heal(g,r->b);}
 if(r->trigger==DR_PICKGOLD){g->gold+=r->a;heal(g,r->b);}
 if(r->trigger==DR_ENERGY)g->max_energy++;
}
static int random_relic_except(Game*g,int excluded){
 int pool[MAX_RELICS],n=0;if(g->relic_count>=MAX_RELICS)return -1;
 for(int id=1;id<RELIC_COUNT;id++)if((id<=27||(id>=40&&id<44)||(id>=53&&id<63)||(g->character==CHAR_VESPER&&id>=45&&id<=51)||(id>=64&&id<74&&(g->ascent.relics&(1u<<(id-64))))||(id>=74&&id<94))&&id!=excluded&&!relic(g,id))pool[n++]=id;
 return n?pool[rnd(g,n)]:-1;
}
static int random_relic(Game*g){return random_relic_except(g,-1);}
static void offer_blessing(Game*g){
 /* One dependable act boon, one general relic, one build-focused relic.
    Each slot has a broad unowned pool; repeated stat gifts are the fallback. */
 int pools[3][64],counts[3]={0};
 for(int id=28;id<40;id++)if(id!=30&&(id>=37||!relic(g,id)))pools[0][counts[0]++]=id;
 for(int id=1;id<28;id++)if(id!=11&&id!=25&&!relic(g,id))pools[1][counts[1]++]=id;
 for(int id=40;id<63;id++)if(id!=44&&(id<45||id>52||g->character==CHAR_VESPER)&&
     (id!=52||g->act>1)&&!relic(g,id))pools[2][counts[2]++]=id;
 for(int i=0;i<3;i++)g->reward[i]=counts[i]?pools[i][rnd(g,counts[i])]:37+i;
 if(g->act>1&&rnd(g,4)==0){
  g->reward[2]=1000+reward_of_rarity(g,2,-1,-1);
 }
 g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose one blessing for your ascent.");
}
static void begin_act(Game*g){g->revision182.room_pending=0;g->revision182.rare_offset=-5;g->screen=ACT_INTRO;g->scroll=0;g->selection=0;}
int game_moon_heal_now(const Game*g){return MIN(20,g->maxhp-g->hp);}
/* Deterministic branching edges: no save struct layout changes. Edges are
   seeded by the run and act, so maps persist correctly across reloads. */
static uint32_t route_hash(uint32_t z){z^=z>>16;z*=0x7feb352du;z^=z>>15;z*=0x846ca68bu;return z^(z>>16);}
#include "journey_rewards.inc"
/* Two distinct starts, three optional forks, and at most three merges per act.
   Every active room has an exit. Ordered neighbors prevent crossing paths. */
static unsigned route_layer_legacy(const Game*g,int wanted,int lane){
 unsigned live=5u;
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*0x9e3779b9u);
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1u,2u,4u};uint32_t h=route_hash(seed+(unsigned)r*9199u);
  int fork=(r==1+(int)(seed&1u)||r==6+(int)((seed>>1)&1u)||r==10+(int)((seed>>2)&1u)||r==14+(int)((seed>>3)&1u));
  int merge=(r==4||r==8||r==12||r==16);
  if(fork&&live!=7u){
   int gap=!(live&2)?1:!(live&1)?0:2;
   int from=gap==1?((h&1)?0:2):1;
   if(live&(1u<<from))edges[from]|=1u<<gap;
  }else if(merge&&live==7u){int from=(h&1)?0:2;edges[from]=2u;}
  /* Inactive lanes still have a valid exit for compatible older saves. */
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
/* Room placement retains the published 1.8.4 seed schedule. This helper is
   deliberately separate from the navigable paths below. */
/* Seeded nearby shifts, forks and merges. Ordered destinations forbid crossings. */
static unsigned room_route_layer(const Game*g,int wanted,int lane){
 if(!g->revision184.route_style)return route_layer_legacy(g,wanted,lane);
 uint32_t seed=route_hash(g->seed^(unsigned)g->act*0x9e3779b9u);
 static const unsigned starts[]={1,2,4,3,5,6,7};unsigned live=starts[seed%7];
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1,2,4};uint32_t h=route_hash(seed+(unsigned)r*9199u);int prior_max=-1;
  for(int c=0;c<3;c++)if(live&(1u<<c)){
   uint32_t z=route_hash(h+(unsigned)c*731u);int lo=MAX(0,c-1),hi=MIN(2,c+1);
   lo=MAX(lo,prior_max);if(lo>hi)lo=hi;
   int dest=lo+(int)(z%(unsigned)(hi-lo+1));edges[c]=1u<<dest;
   /* A single route opens sooner; broad maps fork more sparingly. */
   if((z>>8)%100u<(live==1||live==2||live==4?60u:23u)&&dest<hi)edges[c]|=1u<<(dest+1);
   prior_max=dest+!!(edges[c]&(1u<<(dest+1)));
  }
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
/* Two live routes split into three, stay separate for two or three rooms,
   then reconnect on alternating/random sides. Every live route has a choice
   at the next split; no branch is a one-room detour or an absorbing edge. */
static unsigned route_layer184r2(const Game*g,int wanted,int lane){
 if(!g->revision184.route_style)return route_layer_legacy(g,wanted,lane);
 uint32_t seed=route_hash(g->seed^(unsigned)g->act*0x9e3779b9u);
 static const unsigned starts[]={3,5,6};unsigned live=starts[seed%3];
 int split=(seed>>5)&1u,merge=split+2+((seed>>6)&1u),merge_lane=((seed>>7)&1u)*2;
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1,2,4};
  if(r==split){
   int gap=!(live&1)?0:!(live&2)?1:2;
   for(int c=0;c<3;c++)if(live&(1u<<c)){int other=c<gap?c+1:c-1;edges[c]|=1u<<other;}
  }else if(r==merge){
   edges[merge_lane]=2;uint32_t h=route_hash(seed+(unsigned)r*9199u);
   split=r+1;merge=split+2+((h>>8)&1u);merge_lane=((h>>9)&1u)*2;
   if(split>MAP_ROWS-3)split=merge=MAP_ROWS+1;
  }
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
/* Two or three rooms per commitment, with four or five recurring forks.
   Room placement below makes every scheduled fork a distinct choice. */
typedef struct {int split,merge,side;} RouteJunction;
static int route_variety(const Game*g,int first,int last){
 for(int row=first;row<=last&&row<MAP_ROWS;row++)if(g->map[row][0]!=g->map[row][1]||g->map[row][1]!=g->map[row][2])return 1;
 return 0;
}
static int route_junctions(const Game*g,RouteJunction*out){
 uint32_t seed=route_hash(g->seed^(unsigned)g->act*0x9e3779b9u);
 int split=1+((seed>>5)&1u),merge=MIN(MAP_ROWS-1,split+2+((seed>>6)&1u)),side=((seed>>7)&1u)*2,n=0;
 while(split<=MAP_ROWS-3){
  /* A uniform shop followed by a guaranteed camp cannot form a choice.
     Include the next room instead; this remains a three-room commitment. */
  int movable=0;for(int r=split+1;r<=merge;r++)for(int c=0;c<3;c++)movable|=g->map[r][c]==NODE_FIGHT||g->map[r][c]==NODE_ELITE||g->map[r][c]==NODE_EVENT;
  if(!movable&&!route_variety(g,split+1,merge)&&merge==split+2&&merge+1<MAP_ROWS)merge++;
  out[n++]=(RouteJunction){split,merge,side};uint32_t h=route_hash(seed+(unsigned)merge*9199u);
  split=merge+1;merge=MIN(MAP_ROWS-1,split+2+((h>>8)&1u));side=((h>>9)&1u)*2;
 }
 return n;
}
static unsigned route_layer(const Game*g,int wanted,int lane){
 if(g->revision184.route_style<2)return route_layer184r2(g,wanted,lane);
 uint32_t seed=route_hash(g->seed^(unsigned)g->act*0x9e3779b9u);
 static const unsigned starts[]={3,5,6};unsigned choices[3],count=0;
 for(int i=0;i<3;i++){int a=starts[i]&1?0:1,b=starts[i]&4?2:1;if(g->map[1][a]!=g->map[1][b])choices[count++]=starts[i];}
 unsigned live=count?choices[seed%count]:starts[seed%3];RouteJunction plan[MAP_ROWS];int total=route_junctions(g,plan),junction=0;
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1,2,4};
  if(junction<total&&r==plan[junction].split){
   int gap=!(live&1)?0:!(live&2)?1:2;
   for(int c=0;c<3;c++)if(live&(1u<<c)){
    int other=c<gap?c+1:c-1,different=0;
    for(int row=r+1;row<=plan[junction].merge;row++)different|=g->map[row][c]!=g->map[row][other];
    /* Even an unusual unrepaired layout must never offer a false choice. */
    if(different)edges[c]|=1u<<other;
   }
  }else if(junction<total&&r==plan[junction].merge){if(live==7)edges[plan[junction].side]=2;junction++;}
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
unsigned game_route_mask(const Game*g,int row,int lane){
 if(!g||row<0||row>=MAP_ROWS-1||lane<0||lane>=3)return 0;
 unsigned edges=route_layer(g,row,lane);
 if(row<g->floor-1&&g->map_selected[row]==lane&&g->map_selected[row+1]>=0)edges|=1u<<g->map_selected[row+1];
 return edges;
}
unsigned game_route_row_mask(const Game*g,int row){
 if(!g||row<0||row>=MAP_ROWS)return 0;
 unsigned mask=route_layer(g,row,-1);
 if(row<g->floor&&g->map_selected[row]>=0)mask|=1u<<g->map_selected[row];
 if(row==g->floor&&g->floor>0)mask|=game_route_mask(g,g->floor-1,g->lane);
 return mask;
}
unsigned game_route_visible_mask(const Game *g,int row) {
 if(!g||row<0||row>=MAP_ROWS)return 0;
 if(row==g->floor&&g->floor>0)return game_route_mask(g,g->floor-1,g->lane);
 return game_route_row_mask(g,row);
}
int game_route_available(const Game *g,int lane) {
 if(!g||g->floor>=MAP_ROWS||lane<0||lane>=MAP_LANES)return 0;
 if(g->floor==0)return (game_route_row_mask(g,0)&(1u<<lane))!=0;
 return (game_route_mask(g,g->floor-1,g->lane)&(1u<<lane))!=0;
}
static void map_select_reachable(Game*g) {
 if(g->floor>=MAP_ROWS)return;
 if(game_route_available(g,g->selection))return;
 for(int i=0;i<MAP_LANES;i++)
  if(game_route_available(g,i)){g->selection=i;return;}
}
/* Keep the planar topology, but make each fork a real room/reward choice.
   A fork's two child lanes differ within two floors, before any merge. */
static void map_distinguish_forks(Game*g){
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);
 int first=(int)(seed&1u)*2;g->map[1][first]=NODE_EVENT;g->map[1][2-first]=NODE_FIGHT;
 for(int r=0;r<MAP_ROWS-1;r++){
  unsigned live=room_route_layer(g,r,-1);
  for(int lane=0;lane<3;lane++)if(live&(1u<<lane)){
   unsigned edges=room_route_layer(g,r,lane);if((edges&(edges-1))==0)continue;
   int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
   uint32_t h=route_hash(seed+(unsigned)r*7117u);
   if(h&1u){int temp=a;a=b;b=temp;}
   int choice=r+1;if(choice==2||choice==7||choice==15)choice++;
   if(choice==3){
    g->map[3][a]=NODE_SHOP;g->map[3][b]=(h&2u)?NODE_REST:NODE_EVENT;
    /* Each lane still has exactly one shop, on floor 4 or floor 10. */
    g->map[9][a]=(h&4u)?NODE_REST:NODE_EVENT;g->map[9][b]=NODE_SHOP;
   }else if(choice==11){g->map[choice][a]=NODE_REST;g->map[choice][b]=NODE_EVENT;}
   else {g->map[choice][a]=NODE_ELITE;g->map[choice][b]=NODE_FIGHT;}
  }
 }
}
/* Move a room within its own floor only when a committed fork would otherwise
   show identical room sequences. Room counts and floor distribution stay exact. */
static void map_distinguish_committed_routes(Game*g){
 for(int r=0;r<MAP_ROWS-1;r++)for(int c=0;c<3;c++)if(route_layer184r2(g,r,-1)&(1u<<c)){
  unsigned edges=route_layer184r2(g,r,c);if(ascent_popcount(edges)<2)continue;
  int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
  unsigned aa=1u<<a,bb=1u<<b;int different=0,candidate=-1;
  for(int row=r+1;row<MAP_ROWS;row++){
   for(int x=0;x<3;x++)for(int y=0;y<3;y++)if((aa&(1u<<x))&&(bb&(1u<<y)))different|=g->map[row][x]!=g->map[row][y];
   if(different)break;
   if(candidate<0&&(g->map[row][0]!=g->map[row][1]||g->map[row][1]!=g->map[row][2]))candidate=row;
   if(row==MAP_ROWS-1)break;
   unsigned an=0,bn=0;for(int x=0;x<3;x++){if(aa&(1u<<x))an|=route_layer184r2(g,row,x);if(bb&(1u<<x))bn|=route_layer184r2(g,row,x);}
   if(an&bn)break;aa=an;bb=bn;
  }
  if(!different&&candidate>=0){int row=candidate,other=g->map[row][0]==g->map[row][1]?2:0,temp=g->map[row][1];g->map[row][1]=g->map[row][other];g->map[row][other]=temp;}
 }
}
static int route_choice_windows(const Game*g,const RouteJunction*plan,int n){
 int count=0;for(int i=0;i<n;i++)count+=route_variety(g,plan[i].split+1,plan[i].merge);return count;
}
static int route_movable_room(int type){return type==NODE_FIGHT||type==NODE_ELITE||type==NODE_EVENT;}
/* First repair uniform branch windows with the nearest legal exchange of
   combat/mystery rooms. Act totals, shops, treasures and camps stay intact;
   elites never move into the opening floors. Then one distinct center room
   makes BOTH adjacent branch pairs differ before their next merge. */
static void map_distinguish_routes184r3(Game*g){
 RouteJunction plan[MAP_ROWS];int n=route_junctions(g,plan);
 uint32_t seed=route_hash(g->seed^(unsigned)g->act*9973u);
 for(int i=0;i<n;i++)if(!route_variety(g,plan[i].split+1,plan[i].merge)){
  int before=route_choice_windows(g,plan,n),best=100000,ar=-1,ac=0,br=0,bc=0;
  for(int row=plan[i].split+1;row<=plan[i].merge;row++)for(int col=0;col<3;col++){
   int type=g->map[row][col];if(!route_movable_room(type))continue;
   for(int donor=2;donor<MAP_ROWS-1;donor++)if(donor!=row)for(int lane=0;lane<3;lane++){
    int other=g->map[donor][lane];if(!route_movable_room(other)||other==type||(other==NODE_ELITE&&row<4)||(type==NODE_ELITE&&donor<4))continue;
    int score=(row>donor?row-donor:donor-row)*100+((route_hash(seed+(unsigned)(row*311+col*79+donor*17+lane))>>8)%100u);if(score>=best)continue;
    g->map[row][col]=other;g->map[donor][lane]=type;
    int improves=route_choice_windows(g,plan,n)>before;
    g->map[row][col]=type;g->map[donor][lane]=other;
    if(improves){best=score;ar=row;ac=col;br=donor;bc=lane;}
   }
  }
  if(ar>=0){int temp=g->map[ar][ac];g->map[ar][ac]=g->map[br][bc];g->map[br][bc]=temp;}
 }
 for(int i=0;i<n;i++){
  int rows[3],count=0;for(int r=plan[i].split+1;r<=plan[i].merge;r++)if(route_variety(g,r,r))rows[count++]=r;
  if(!count)continue;
  int row=rows[route_hash(seed+(unsigned)plan[i].split*7117u)%(unsigned)count],other=-1;
  if(g->map[row][0]==g->map[row][1])other=2;else if(g->map[row][1]==g->map[row][2])other=0;
  if(other>=0){int temp=g->map[row][1];g->map[row][1]=g->map[row][other];g->map[row][other]=temp;}
 }
}
static void map_generate(Game*g){
 g->revision184.route_style=1;
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);
 for(int r=0;r<MAP_ROWS;r++){
  g->map_selected[r]=-1;
  for(int c=0;c<3;c++){
   uint32_t h=route_hash(seed+(unsigned)(r*131+c*977));int n=NODE_FIGHT;
   if(r==1)n=(h%3)?NODE_EVENT:NODE_FIGHT;
   else if(r==3||r==9){
    int earlyshop=(int)(route_hash(seed+c*127u)&1u);
    n=((r==3)==earlyshop)?NODE_SHOP:((h&1)?NODE_EVENT:NODE_REST);
   }else if(r==5)n=(h%3==0)?NODE_EVENT:NODE_FIGHT;
   else if(r==7)n=NODE_TREASURE;
   else if(r==11)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==13)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==15)n=(h%3)?NODE_EVENT:NODE_SHOP;
   else if(r==17)n=NODE_REST;
   else if(r>=4&&(h%100)<(unsigned)(r>=10?38:24))n=NODE_ELITE;
   g->map[r][c]=n;
  }
 }
 map_distinguish_forks(g);
 /* Guaranteed rests by floor 6 and floor 12, then before the boss. No route
    can stack the old random rest rolls or go an entire act without recovery. */
 for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<3;c++)if(g->map[r][c]==NODE_REST)g->map[r][c]=NODE_EVENT;
 int early=4+(seed&1u),middle=10+((seed>>1)&1u);
 for(int c=0;c<3;c++){g->map[early][c]=NODE_REST;g->map[middle][c]=NODE_REST;g->map[17][c]=NODE_REST;}
 /* An optional fourth camp is a visible recovery-versus-event choice. */
 if(seed&4u){int lane=(int)((seed>>3)%3u);g->map[14][lane]=NODE_REST;}
 /* A guaranteed camp may occupy a fork's original distinguishing room.
    Preserve a real choice on the following floor before the merge. */
 for(int r=0;r<MAP_ROWS-2;r++)for(int c=0;c<3;c++){
  unsigned edges=room_route_layer(g,r,c);if(!(room_route_layer(g,r,-1)&(1u<<c))||!(edges&(edges-1)))continue;
  int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
  if(g->map[r+1][a]==g->map[r+1][b]&&g->map[r+2][a]==g->map[r+2][b]&&r+2!=early&&r+2!=middle&&r+2!=17){
   g->map[r+2][a]=NODE_FIGHT;g->map[r+2][b]=NODE_ELITE;
  }
 }
 map_distinguish_committed_routes(g);
 map_distinguish_routes184r3(g);
 g->revision184.route_style=2;
}
static void shuffle(Game*g,int *p,int count){for(int i=count-1;i>0;i--){int j=rnd(g,i+1),temp=p[i];p[i]=p[j];p[j]=temp;}}
static int has_strike_tag(const char *name);
static void apply_vulnerable(Game*g,Enemy*e,int n);
static void draw_card(Game*g);
static int random_living_foe(Game*g){
 int count=0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)count++;
 if(!count)return -1;
 int which=rnd(g,count);
 for(int i=0;i<g->enemy_count;i++)
  if(g->enemies[i].hp>0&&which--==0)return i;
 return -1;
}
static int preview_living_foe(const Game*g,int choice){
 int count=0;for(int i=0;i<g->enemy_count;i++)count+=g->enemies[i].hp>0;
 if(!count)return -1;choice%=count;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0&&choice--==0)return i;
 return -1;
}

static void add_generated(Game*g,int raw){
 raw=regent_stamp(g,ninja_stamp(g,raw));
 if(g->hand_count<MAX_HAND)g->hand[g->hand_count++]=raw;
 else if(g->discard_count<MAX_PILE)
   g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);
 else return;
 regent_created(g);
}
static void gain_energy(Game*g,int n){
 if(n>0&&(g->screen==COMBAT||g->screen==CARD_CHOICE||g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE)&&!g->ic[IC_ENERGY_LOCK])
   g->energy=MIN(99,g->energy+n);
}
static void draw_card(Game*g){
 if((g->screen==COMBAT||g->screen==CARD_CHOICE)&&(g->reward_potion&1))return;
 if(g->hand_count>=MAX_HAND)return;
 if(g->regent.shuffle_pending){g->regent.shuffle_draws=MIN(MAX_PILE,g->regent.shuffle_draws+1);return;}
 if(!g->draw_count){if(!g->discard_count)return;memcpy(g->draw,g->discard,(size_t)g->discard_count*sizeof(int));g->draw_count=g->discard_count;g->discard_count=0;shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);if(g->regent.shuffle_pending){g->regent.shuffle_draws++;return;}}
 int drawn=g->draw[--g->draw_count];
 g->hand[g->hand_count++]=drawn;
 ninja_on_draw(g,drawn);regent_on_draw(g,drawn);
 if(g->ic[IC_HELLRAISER]>0&&g->ic[IC_CHAIN_DEPTH]<20&&
   CARD_ID(drawn)<CARD_COUNT&&cards[CARD_ID(drawn)].type==0&&
   has_strike_tag(cards[CARD_ID(drawn)].name)){
   int chosen=random_living_foe(g);
   if(chosen>=0){
    g->ic[IC_CHAIN_DEPTH]++;
    g->hand[g->hand_count-1]|=CARD_FREE;
    game_play_card(g,g->hand_count-1,chosen);
    g->ic[IC_CHAIN_DEPTH]--;
   }
 }
}
static void reshuffle_battle(Game*g){g->draw_count=g->deck_count;memcpy(g->draw,g->deck,(size_t)g->deck_count*sizeof(int));for(int i=0;i<g->draw_count;i++)g->draw[i]=regent_stamp(g,ninja_stamp(g,g->draw[i]));shuffle(g,g->draw,g->draw_count);g->discard_count=g->exhaust_count=g->hand_count=0;
 if(g->character==CHAR_PROXY||g->revision184.custom){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(regent_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(regent_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}
 if(g->character==CHAR_VESPER||g->revision184.custom){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(ninja_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(ninja_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}
}
static void heal(Game*g,int n){g->hp=MIN(g->maxhp,g->hp+MAX(n,0));}
static void player_death(Game*g){
 if(g->hp>0)return;
 for(int i=0;i<g->relic_count;i++){int id=game_relic_id(g,i);if(id>=0&&id<RELIC_COUNT&&dist_relics[id].trigger==DR_REVIVE&&g->defense_event!=g->act){g->defense_event=g->act;g->hp=MIN(g->maxhp,dist_relics[id].a);regent_gain_stars(g,dist_relics[id].b);msg(g,"Unspent Farewell rekindles your life.");return;}}
 g->hp=0;g->screen=DEFEAT;g->selection=0;msg(g,"The rift closes. Your echoes remain.");
}
#define RIFT_INFERNO_POWER(g) ((g)->used_potion & 0xffff)
#define RIFT_INFERNO_COPIES(g) ((g)->used_potion >> 16 ? (g)->used_potion >> 16 : ((g)->used_potion > 0 ? 1 : 0))
static void lose_hp(Game*g,int n){
 n=MAX(n,0);if(g->screen==COMBAT&&g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);g->hp-=n;
 if(n>0)record_combat_fx(GFX_SELF,-2,-1,n,0);
 if(n>0&&g->screen==COMBAT){
  g->ic[IC_HPLOSS_COMBAT]++;
  if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
 }
 if(n>0&&g->screen==COMBAT&&g->shop_cards[0]>0)g->strength+=g->shop_cards[0];
 if(n>0&&g->screen==COMBAT&&RIFT_INFERNO_POWER(g)>0){
   /* Trigger once per HP-loss event, not once for each point of lost HP.
      Inferno's AoE is a Power effect: it ignores Strength, Weak, Vulnerable
      and on-attack relics but respects enemy Block. */
   const int power=RIFT_INFERNO_POWER(g);
   for(int z=0;z<g->enemy_count;z++){
     Enemy *foe=&g->enemies[z];
     if(foe->hp<=0)continue;
     int blocked=MIN(foe->block,power);
     foe->block-=blocked;
     foe->hp-=power-blocked;
     record_combat_fx(GFX_PROC,-2,z,power,blocked);
   }
 }
 player_death(g);
 if(n>0)r151_self_loss(g);
 if(n>0&&g->screen==COMBAT&&g->ic[IC_OWN_TURN]&&legacy_relic(g,42)&&!g->ic[IC_NEEDLE]){
  g->ic[IC_NEEDLE]=1;draw_card(g);
 }
}
static void damage_player(Game*g,int n){
 if(n<=0)return;
 if(g->vulnerable) n=n*3/2;
 if(g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);
 int blocked=MIN(g->block,n);
 record_combat_fx(GFX_ATTACK,combat_fx_source,-1,n,blocked);
 g->block-=blocked;n-=blocked;
 if(g->regent.reflect&&blocked>0&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int b=MIN(e->block,blocked);e->block-=b;e->hp-=blocked-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,blocked,b);}
 if(g->regent.gambit&&n>0)g->hp=0;
 if(n>0){g->hp-=n;
  if(g->screen==COMBAT){
   g->ic[IC_HPLOSS_COMBAT]++;
   if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
  }
 }
 if(legacy_relic(g,2)&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int absorbed=MIN(4,e->block);e->block-=absorbed;e->hp-=4-absorbed;record_combat_fx(GFX_PROC,-1,combat_fx_source,4,absorbed);}
 if(g->ninja.v[NS_THORNS]&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int d=g->ninja.v[NS_THORNS],b=MIN(e->block,d);e->block-=b;e->hp-=d-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,d,b);}
 if(n>0&&g->hp>0&&legacy_relic(g,78)&&!g->expedition.combat[E181_HURT_BLOCK]){g->expedition.combat[E181_HURT_BLOCK]=1;relic_block(g,5);}
 if(g->hp>0&&(long long)g->hp*4<=g->maxhp&&legacy_relic(g,89)&&!g->expedition.combat[E181_LAST_MERCY]){g->expedition.combat[E181_LAST_MERCY]=1;heal(g,12);}
 if(n>0&&g->hp>0&&combat_fx_source>=0)dist_relic_trigger(g,5);
 player_death(g);
}
static int attack_amount(const Game*g,int idx,int amt){
 const Enemy *e=&g->enemies[idx];
 amt=MAX(0,amt+(r140_attack_context?g->regent.attack_vigor:0)+g->strength+(r140_attack_context&&CARD_MOD(r150_card_raw)==MOD_KEEN?2:0));
 if(g->weak)amt=amt*3/4;
 if(e->vulnerable){
  amt=amt*3/2;
  if(g->ic[IC_CRUELTY])amt=amt*(100+g->ic[IC_CRUELTY])/100;
 }
 if(r140_attack_context&&legacy_relic(g,3)&&g->ic[IC_ATTACKS_THIS_TURN]==0)amt+=5;
 if(r140_attack_context&&legacy_relic(g,26)&&g->hp*2<=g->maxhp)amt+=3;
 if(r140_attack_context&&legacy_relic(g,87)&&e->hp*2<e->maxhp)amt+=4;
 if(r140_attack_context&&e->weak)for(int i=0;i<MIN(30,g->ninja.v[NS_TRACKING])&&amt<1000000000;i++)amt=(int)MIN(1000000000LL,(long long)amt*2);
 if(r140_attack_context&&g->ninja.v[NS_SHADOW_NOW])amt=(int)MIN(1000000000LL,(long long)amt*2);
 if(r140_attack_context&&CARD_ID(r150_card_raw)==RIFT_CARD_265&&g->regent.conqueror[idx])amt=(int)MIN(1000000000LL,(long long)amt*2);
 return amt;
}
static void damage_enemy(Game*g,int idx,int amt,int ignore_block){
 if(idx<0||idx>=g->enemy_count||g->enemies[idx].hp<=0)return;
 Enemy *e=&g->enemies[idx];
 amt=attack_amount(g,idx,amt);
 preview_hit(idx,amt);
 record_combat_fx(GFX_ATTACK,-1,idx,amt,ignore_block?0:MIN(e->block,amt));
 if(!ignore_block){int b=MIN(e->block,amt);e->block-=b;amt-=b;}
 e->hp-=amt;
 ninja_on_hit(g,idx,amt);regent_on_hit(g,idx,amt);
 if(r140_attack_context&&legacy_relic(g,77)&&!g->expedition.combat[E181_WEAK_ATTACK]){g->expedition.combat[E181_WEAK_ATTACK]=1;e->weak=MIN(99,e->weak+1);}
 if(r140_attack_context&&legacy_relic(g,9)&&g->ic[IC_ATTACKS_THIS_TURN]==0&&!(r140_rust_targets&(1<<idx))){e->weak=MIN(99,e->weak+2);r140_rust_targets|=1<<idx;}

}
static void deal(Game*g,int idx,int n){damage_enemy(g,idx,n,0);}
static void apply_vulnerable(Game*g,Enemy*e,int n){
 if(!e||n<=0||e->hp<=0)return;
 e->vulnerable=MIN(99,e->vulnerable+n);
 if(g->ic[IC_VICIOUS]>0){
  int count=MIN(6,g->ic[IC_VICIOUS]);
  for(int i=0;i<count;i++)draw_card(g);
 }
}
static void proc_juggernaut(Game*g){
 if(g->ic[IC_JUGGERNAUT]<=0||g->screen!=COMBAT||g->ic[IC_GUARD])return;
 int foe=random_living_foe(g);
 if(foe<0)return;
 g->ic[IC_GUARD]=1;
 Enemy *e=&g->enemies[foe];
 int d=g->ic[IC_JUGGERNAUT],blocked=MIN(e->block,d);
 e->block-=blocked;e->hp-=d-blocked;
 record_combat_fx(GFX_PROC,-2,foe,d,blocked);
 g->ic[IC_GUARD]=0;
}
static void gain_block(Game*g,int n){
 int amount=MAX(0,n+g->dexterity+(legacy_relic(g,15)?2:0));
 if(amount>0&&g->ic[IC_UNMOVABLE]&&!g->ic[IC_UNMOVABLE_USED]){
  amount*=2;g->ic[IC_UNMOVABLE_USED]=1;
 }
 for(int i=0;i<MIN(30,g->ninja.v[NS_SHADOWMELD])&&amount<1000000000;i++)amount=(int)MIN(1000000000LL,(long long)amount*2);
 int gained=MIN(amount,999-g->block);
 if(gained>0)record_combat_fx(GFX_BLOCK,-1,-1,gained,0);
 if(preview_trace&&preview_phase){
  CardPreview*p=preview_trace;
  if(preview_nested)p->uncertain=1;
  if(!p->block_count||gained<p->block_low)p->block_low=gained;
  if(!p->block_count||gained>p->block_high)p->block_high=gained;
  if(!p->block_count)p->block_first=gained;
  if(p->block_count==1)p->block_second=gained;
  p->block_count++;p->block_total+=gained;
 }
 if(amount<=0)return;
 g->block=MIN(999,g->block+amount);
 proc_juggernaut(g);
}
static void card_block(Game*g,int n){if(g->regent.panic)return;gain_block(g,n+(CARD_MOD(r150_card_raw)==MOD_WARD?4:0));}
static void relic_block(Game*g,int n){
 for(int i=0;i<MIN(30,g->ninja.v[NS_SHADOWMELD])&&n<1000000000;i++)n=(int)MIN(1000000000LL,(long long)n*2);
 int amount=MIN(n,999-g->block);if(amount<=0)return;
 g->block+=amount;record_combat_fx(GFX_BLOCK,-1,-1,amount,0);proc_juggernaut(g);
}
static int random_relic(Game*g);
static void check_win(Game*g){
 if(g->screen!=COMBAT||g->ninja.task_count>0||g->regent.task_count||regent_processing)return;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)return;
 if(dist_evaluating){g->screen=REWARD;return;}
 dist_relic_trigger(g,3);
 g->ic[37]=0;g->v151.card_done=0;g->v151.potion_offer=-1;
 if(legacy_relic(g,57))heal(g,2*MIN(3,g->v151.relic[4]));
 g->gold+=g->regent.powers[RIFT_CARD_260-REGENT_FIRST];g->kill_count+=g->enemy_count;
 if(legacy_relic(g,81)&&(g->battle_type==NODE_ELITE||g->battle_type==NODE_BOSS))g->gold+=35;
 if(legacy_relic(g,84)&&g->expedition.combat[E181_POTION_USED])heal(g,4);
 if(legacy_relic(g,85))heal(g,1);
 if(g->battle_type==NODE_BOSS)ascent_record_act(g);
 if(g->battle_type==NODE_BOSS&&g->act==3&&!g->ascent.won){
  if(!ascent_complete(g,0)){g->screen=ASCENT_SAVE_ERROR;g->selection=0;return;}
  g->screen=ASCENT_ENDING;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }
 if(g->battle_type==NODE_BOSS&&g->act==4&&g->ascent.secret){
  if(!ascent_complete(g,1)){g->screen=ASCENT_SAVE_ERROR;g->selection=1;return;}
  g->screen=ASCENT_TRUE_END;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }
 if(rnd(g,100)<22)g->v151.potion_offer=ascent_random_potion(g);
 g->gold+=g->reward_gold+(legacy_relic(g,16)?20:0)+(legacy_relic(g,67)?8:0);
 if(legacy_relic(g,68))heal(g,3);
 if(legacy_relic(g,0))heal(g,6);if(legacy_relic(g,36))heal(g,3);
 if(g->battle_type==NODE_ELITE&&legacy_relic(g,12))g->permanent_strength++;
 if(g->battle_type==NODE_BOSS&&g->act==7){g->screen=ABOMINATION_AFTER;g->selection=0;g->win_screen=0;return;}
 if(g->battle_type==NODE_BOSS){
  int gift=expedition_relic(g);
  g->relic_pending=1;g->relic_resume=REWARD;
  g->reward[0]=gift;
  g->win_screen=(!g->endless&&g->act>=3?2:1);
  g->selection=0;g->screen=RELIC_RESULT;
  msg(g,""); /* The relic screen owns all boss-reward text. */
  return;
 }
 g->screen=REWARD;g->selection=0;
 if(g->battle_type==NODE_ELITE){
  int gift=expedition_relic(g);
  if(gift>=0){g->reward[0]=gift;g->relic_pending=1;
   g->relic_resume=REWARD;g->screen=RELIC_RESULT;g->selection=0;
   msg(g,"Elite defeated.");
  }else msg(g,"Elite defeated. No relics remain.");
 }else{fill_rewards(g);msg(g,"");}
}
/* Thirty-six deterministic, species-specific enemy profiles, balanced by act. */
typedef struct {const char *name;short hp,attack,tier; unsigned char moves[4];} EnemyDef;
static const EnemyDef enemy_defs[ENEMY_COUNT]={
 {"Ash Crawler",30,8,0,{0,0,1,6}},
 {"Bone Sentinel",35,8,0,{0,1,7,0}},
 {"Mist Gazer",32,9,0,{0,2,0,9}},
 {"Iron Wight",38,9,0,{0,3,1,0}},
 {"Grave Knight",53,11,0,{0,1,10,0}},
 {"Brine Witch",50,12,0,{6,0,2,1}},
 {"Stone Giant",87,13,1,{24,10,21,0}},
 {"Fallen Warden",155,18,1,{1,18,10,22}},
 {"THE HOLLOW KING",300,23,2,{13,16,10,31}},
 {"Cinder Hound",33,9,0,{7,0,6,0}},
 {"Crypt Archer",37,9,0,{0,3,0,1}},
 {"Glimmer Moth",29,8,0,{8,2,0,9}},
 {"Oathbreaker",42,10,0,{4,0,0,10}},
 {"Ember Seer",78,12,1,{5,19,9,18}},
 {"Bone Captain",85,11,1,{27,7,18,3}},
 {"THE FURNACE MAW",168,15,2,{24,19,10,31}},
 {"Rust Reaver",56,12,0,{0,10,1,0}},
 {"Sable Lurker",52,12,0,{7,8,0,2}},
 {"Mire Oracle",50,11,0,{6,9,5,0}},
 {"Wound Collector",62,12,0,{12,0,3,4}},
 {"Chain Keeper",66,14,0,{28,0,1,7}},
 {"Drowned Penitent",53,12,0,{13,2,0,9}},
 {"Ossuary Beast",126,15,1,{24,7,10,31}},
 {"Storm Golem",143,16,1,{13,26,16,31}},
 {"THE CRIMSON JUDGE",240,19,2,{3,14,21,31}},
 {"Void Disciple",82,17,0,{5,8,0,1}},
 {"Bone Harvester",92,17,0,{7,0,12,3}},
 {"Hollow Surveyor",78,16,0,{6,9,2,10}},
 {"Rift Channeler",79,16,0,{11,5,9,0}},
 {"Pale Executioner",103,18,0,{10,0,3,1}},
 {"Crownless Saint",91,16,0,{13,12,3,0}},
 {"Entropy Knight",102,19,0,{0,4,7,10}},
 {"Twilight Shade",88,18,0,{8,6,2,7}},
 {"Infernal Colossus",173,19,1,{4,19,10,21}},
 {"Spectral Marshal",163,19,1,{20,3,7,22}},
 {"Rift Behemoth",185,20,1,{1,10,30,31}},
 {"Gallows Stalker",34,8,0,{0,15,1,0}},
 {"Candle Wretch",32,8,0,{16,0,2,1}},
 {"Sepulcher Guard",62,12,0,{15,1,0,17}},
 {"Thorn Penitent",55,12,0,{16,2,0,4}},
 {"Veil Duelist",88,17,0,{14,0,16,1}},
 {"Tomb Cantor",94,16,0,{17,0,6,14}},
 {"Oathless Lancer",90,12,1,{14,15,18,16}},
 {"Cathedral Horror",148,17,1,{29,23,16,21}},
 {"THE UNMAKER",340,22,2,{24,11,30,31}},
 {"THE UNBEGOTTEN",610,31,2,{13,16,11,10}},
 {"Crown Eater",174,19,1,{22,7,21,18}},
 {"Cinder Sexton",37,9,0,{1,0,5,14}},
 {"Bellback Vermin",33,8,0,{0,14,1,3}},
 {"Waxbound Acolyte",39,9,0,{1,5,0,2}},
 {"Pyre Castellan",86,12,1,{24,19,25,31}},
 {"Saltglass Eel",57,13,0,{0,14,2,7}},
 {"Silt Reliquary",65,12,0,{1,0,3,14}},
 {"Rime Cantor",53,12,0,{2,0,11,14}},
 {"Abyssal Sacristan",145,17,1,{1,23,21,16}},
 {"Halo Leech",85,17,0,{1,12,0,14}},
 {"Starved Herald",91,17,0,{2,0,10,14}},
 {"Throne Husk",99,18,0,{1,0,14,3}},
 {"Eclipse Justiciar",182,20,1,{13,18,22,16}},
 {"Censer Bailiff",90,12,1,{1,24,0,31}},
 {"Rime Pallbearer",146,16,1,{1,0,27,31}},
 {"Vowbreaker Seraph",178,19,1,{24,0,7,31}},
 {"Crown Tether",44,9,1,{1,0,31,27}},
 {"Unborn Witness",44,9,1,{0,1,0,31}},
};
#ifndef RIFT186_ENCOUNTERS
#define RIFT186_ENCOUNTERS 1
#endif
#ifndef R186_BOSS3_HP
#define R186_BOSS3_HP 100
#endif
#ifndef R186_BOSS3_DAMAGE
#define R186_BOSS3_DAMAGE 96
#endif
#ifndef R186_BOSS7_HP
#define R186_BOSS7_HP 100
#endif
#ifndef R186_BOSS7_DAMAGE
#define R186_BOSS7_DAMAGE 98
#endif
/* Encounter composition is fixed for its lifetime, including after deaths. */
static int r186_boss_group(const Game*g){
 return g->battle_type==NODE_BOSS&&((g->act==3&&g->enemy_count==2&&g->enemies[0].art==8)||
        (g->act==7&&g->enemy_count==3&&g->enemies[0].art==45));
}
/* Normal / elite / boss pools are separate so bosses never show up as ordinary foes. */
static const unsigned char enemy_normal[3][13]={{0,1,2,3,9,10,11,12,36,37,47,48,49},{4,5,16,17,18,19,20,21,38,39,51,52,53},{25,26,27,28,29,30,31,32,40,41,55,56,57}};
static const unsigned char enemy_elite[3][5]={{6,13,14,42,50},{7,22,23,43,54},{33,34,35,46,58}};
static const unsigned char enemy_boss[3]={15,24,8};
static void enemy_intent(Game*g,Enemy *e){
 const EnemyDef*d=&enemy_defs[e->art];
 if(e->art==45){
  int phase=e->hp<=e->maxhp/3?2:e->hp<=e->maxhp*2/3?1:0;
  if(phase>e->phase){e->strength+=5*(phase-e->phase);e->phase=phase;e->weak=e->vulnerable=0;e->poison/=2;e->block+=18*phase;}
 }else if(d->tier==2 && e->phase==0 && e->hp<=e->maxhp/2){e->phase=1;e->strength+=3;}
 unsigned turn=(unsigned)g->turn;
 int slot=(int)(turn%4u);
 if(e->art==63)slot=(int)((turn+(unsigned)(e-g->enemies)-1u)%4u);
 /* Phase strengthens the boss without skipping its telegraphed cycle. */
 e->intent=d->moves[slot];
 if(e->art==45){static const unsigned char moves[3][4]={{13,16,11,10},{7,13,10,16},{10,7,13,16}};e->intent=moves[CLAMP(e->phase,0,2)][slot];}
 /* Most heavy openings are deferred; a deterministic one-in-eight exception
    keeps some danger while always showing the actual intended move. */
 if(g->turn==0&&(e->intent==4||e->intent==7||e->intent==10||e->intent==11)&&
    (route_hash(g->seed^(unsigned)e->art*101u^(unsigned)g->floor*37u)&7u))e->intent=0;
 if(d->tier==0&&(e->intent==1||e->intent==4||e->intent==9||e->intent==11||e->intent==13||(e->intent>=20&&e->intent<=23)||e->intent==25||e->intent==29)&&
    route_hash(g->seed^(unsigned)e->art*313u^(unsigned)g->floor*101u^(unsigned)g->turn*919u)%100u<9u)e->intent=0;
 /* A few basic strikes split their value between damage and protection.
    This adds mixed intents without increasing the number of attacking turns. */
 if(d->tier==0&&e->intent==0&&route_hash(g->seed^(unsigned)e->art*727u^(unsigned)g->floor*379u^(unsigned)g->turn*1429u)%10u==0)e->intent=24;
 long long percent=g->act==1?82:g->act==2?99:124LL+24LL*(g->act-3)+2LL*(g->act-3)*(g->act-3);
 if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=8;
 int attack=d->attack;
 /* Returning early-act species must meet the celestial encounter baseline.
    Dedicated minions retain their individually balanced profiles. */
 if(g->act>=4&&e->art<59)attack=MAX(attack,d->tier==2?23:d->tier==1?19:16);
 e->intent_value=(int)CLAMP((attack*percent+50)/100+g->floor/7+(e->phase?2:0),1,100000000);
}
static const char *intent_titles[INTENT_COUNT]={
 "IMPACT","GUARD","WEAKEN","BREACH","EMPOWER","CURSE","VENOM",
 "DOUBLE","DRAIN","MEND","CRUSH","DREAD","LEECH","FORTIFY","PIERCE","RENDING","FLURRY","ENTOMB","FRACTURE","SEAR","MUSTER","PURGE","GLOOM","BLIGHT","BRACED IMPACT","CINDER GUARD","STATIC CRUSH","BONE BULWARK","CHAIN LASH","GRAVE CHOIR","RIFT REND","RECOVER"
};
static const char *intent_details[INTENT_COUNT]={
 "One direct hit. Guard reduces its damage.",
 "Raises enemy guard. Enemy guard expires next turn.",
 "Small hit and 2 Falter: your attacks deal 25% less.",
 "Full hit and 2 Fracture: you take 50% more.",
 "Raises enemy Force by 2 and gains 8 Guard.",
 "Half-damage hit, then adds a Burn to discard.",
 "Half-damage hit and adds 2 Wither to you.",
 "Two separate attacks, each around half the shown value.",
 "Half-damage hit; lose 1 energy next turn.",
 "Heals enemy health and grants some Guard.",
 "Heavy attack and adds an unplayable Daze.",
 "Adds a Dread curse and gains Guard.",
 "Half-damage hit and heals for HP actually lost.",
 "Raises enemy Guard and Force by 1.",
 "A light attack that ignores your Guard. Falter still reduces it.",
 "Removes 4 of your Guard, then makes a light attack.",
 "Three small attacks. Each hit is reduced separately by Guard.",
 "Adds one unplayable Injury to discard for this combat and gains Guard.",
 "A half-strength hit; applies 2 Falter and 2 Fracture.",
 "Two small attacks, then adds a Burn to your discard pile.",
 "All living enemies gain 1 Force and 8 Guard.",
 "Removes enemy Falter and Fracture, halves Wither, then gains Guard.",
 "Applies 2 Falter and reduces your next turn Energy by 1. No direct hit.",
 "Adds 2 Wither and one Daze to your discard pile. No direct hit.",
 "A half-strength hit and gains half the normal Guard Guard.",
 "Gains half the normal Guard Guard and adds a Burn to your discard pile.",
 "Two small attacks; reduces your next turn Energy by 1.",
 "A light attack; every living enemy gains 4 Guard.",
 "Removes 3 of your Guard, makes a light attack, then drains 1 Energy next turn.",
 "All living enemies gain 5 Guard; applies 2 Wither to you.",
 "A light attack; applies 2 Fracture and adds a Daze to discard.",
 "No attack or Guard. Becomes Fracture for your next turn."
};
const char *game_intent_title(int n){return n>=0&&n<INTENT_COUNT?intent_titles[n]:"UNKNOWN";}
const char *game_intent_description(int n){return n>=0&&n<INTENT_COUNT?intent_details[n]:"No information";}
static const char*potion_titles[6]={"Hearth Sip","Rivet Tonic","Slate Draught","Censer Spark","Lantern Cordial","Murmur Ink"};
static const char*potion_details[6]={"Recover 15 HP. Gain 7 Gold.","Gain 1 Force and 8 Guard.","Gain 17 Guard and 2 Echo.","Hit all foes for 13. Apply 2 Wither to all.","Gain 1 Energy and 3 Echo.","Draw 2. Gain 6 Guard."};
const char *game_potion_name(int id){return id>=6&&id<26?new_potion_names[id-6]:id>=0&&id<6?potion_titles[id]:"None";}
const char *game_potion_description(int id){return id>=6&&id<26?new_potion_rules[id-6]:id>=0&&id<6?potion_details[id]:"Empty slot";}
const char*game_potion_description_now(const Game*g,int id){(void)g;return game_potion_description(id);}
static int strike_count(const Game*g);
static int card_base_damage(const Game*g,int raw,int target){(void)g;(void)target;return dist_base_damage(raw);}
int game_effective_damage(const Game*g,int raw,int target){
 CardPreview p;game_card_preview(g,raw,target,&p);
 return target>=0&&target<g->enemy_count?p.total[target]:0;
}
static void init_enemy(Game*g,Enemy*e,int id){
 const EnemyDef*d=&enemy_defs[id];
 memset(e,0,sizeof(*e));e->art=id;
 snprintf(e->name,sizeof(e->name),"%s",d->name);
 long long percent=g->act==1?94:g->act==2?132:188LL+50LL*(g->act-3);
 if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=10;
 if(d->tier==2)percent=g->act>=3?182LL+44LL*(g->act-3):84+g->act*24;else percent+=g->floor+MAX(0,g->floor-4)/2;
 if(g->act>3)percent+=5LL*(g->act-3)*(g->act-3);
 int base_hp=d->hp;
 if(g->act>=4&&id<59)base_hp=MAX(base_hp,d->tier==2?300:d->tier==1?163:78);
 long long hp=(base_hp*percent+50)/100;
 /* A gentle additional step above the existing act curve; Act 1 is unchanged. */
 hp=(hp*(100+CLAMP(g->act-1,0,20))+50)/100;
 /* A small endurance increase for the optional and act-ending challenges. */
 if(g->battle_type==NODE_ELITE)hp=(hp*108+50)/100;
 if(g->battle_type==NODE_BOSS)hp=(hp*110+50)/100;
 if(r186_boss_group(g)){
  int scale=id==8?R186_BOSS3_HP:id==45?R186_BOSS7_HP:id==62?55:30;
  hp=(hp*scale+50)/100;
 }else if(g->enemy_count>1)hp=(hp*(g->battle_type==NODE_ELITE?65:g->enemy_count==2?70:50)+50)/100;
 e->maxhp=(int)MIN(1000000000LL,hp);
 if(e->maxhp<1)e->maxhp=1;
 e->hp=e->maxhp;
 enemy_intent(g,e);
}
const char *game_enemy_name(int id){return id>=0&&id<ENEMY_COUNT?enemy_defs[id].name:"Unknown";}
void game_start_battle(Game*g,int mode){
 g->screen=COMBAT;g->battle_type=mode;
 g->revision182.room_type=mode;g->revision182.room_pending=mode!=NODE_BOSS;
 /* Music selection uses a separate seed hash, preserving gameplay RNG. */
 unsigned music_hash=route_hash(g->seed^(unsigned)g->act*17041u^(unsigned)g->floor*6781u);
 g->combat_music=mode==NODE_ELITE?(music_hash&1?3:6):(music_hash%3==0?2:music_hash%3==1?4:5);
 g->turn=0;
 unsigned encounter=route_hash(g->seed^(unsigned)g->act*7919u^(unsigned)g->floor*101u)%4u;
 g->enemy_count=mode==NODE_FIGHT?(encounter==0?2:encounter==1?3:1):1;
 int elite_pair=RIFT186_ENCOUNTERS&&mode==NODE_ELITE&&
  route_hash(g->seed^(unsigned)g->act*18731u^(unsigned)g->floor*619u)%6u==0;
 if(RIFT186_ENCOUNTERS&&mode==NODE_BOSS&&(g->act==3||g->act==7))g->enemy_count=g->act==3?2:3;
 if(elite_pair){g->enemy_count=2;if(g->revision184.seen_act!=g->act){g->revision184.seen_act=g->act;g->revision184.seen[0]=g->revision184.seen[1]=0;}}
 memset(&g->revision185,0,sizeof(g->revision185));
 for(int i=0;i<g->enemy_count;i++){
  int act=MIN(2,g->act-1);
  int id;
  if(mode==NODE_BOSS)id=i?(g->act==7?63:62):g->act==7?45:g->act==4&&g->ascent.secret?44:enemy_boss[g->endless&&g->act>3?rnd(g,3):act];
  else if(elite_pair){static const unsigned char leaders[]={50,54,58};id=i?59+act:leaders[act];g->revision184.seen[id/32]|=1u<<(id%32);}
  else {
   if(g->revision184.seen_act!=g->act){g->revision184.seen_act=g->act;g->revision184.seen[0]=g->revision184.seen[1]=0;}
   int pool[39],fresh[39],n=0,nf=0;
   for(int a=0;a<(g->act>=4?3:1);a++)for(int k=0;k<(mode==NODE_ELITE?5:13);k++){
    int candidate=mode==NODE_ELITE?enemy_elite[g->act>=4?a:act][k]:enemy_normal[g->act>=4?a:act][k];
    int duplicate=0;for(int j=0;j<i;j++)duplicate|=candidate==g->enemies[j].art;
    if(duplicate)continue;pool[n++]=candidate;
    if(!(g->revision184.seen[candidate/32]&(1u<<(candidate%32))))fresh[nf++]=candidate;
   }
   id=nf?fresh[rnd(g,nf)]:pool[rnd(g,n)];
   g->revision184.seen[id/32]|=1u<<(id%32);
  }
  init_enemy(g,&g->enemies[i],id);
 }
 memset(g->ic,0,sizeof(g->ic));
 memset(g->expedition.combat,0,sizeof(g->expedition.combat));
 memset(&g->dist,0,sizeof(g->dist));g->dist.last_type=-1;memset(&g->ninja,0,sizeof(g->ninja));memset(&g->regent,0,sizeof(g->regent));
 {int ledger=g->v151.relic[7],seen=g->v151.relic[R162_EVENT_SEEN],last=g->v151.relic[R162_EVENT_LAST];memset(&g->v151,0,sizeof(g->v151));g->v151.relic[7]=ledger;g->v151.relic[R162_EVENT_SEEN]=seen;g->v151.relic[R162_EVENT_LAST]=last;g->v151.potion_offer=-1;}
 g->reward_potion=0;g->shop_relic=0;g->shop_cards[0]=g->shop_cards[1]=g->shop_cards[2]=0;g->turn=0;g->turn_played=0;g->heal_after_battle=0;g->used_potion=0;g->selection=0;g->target=0;g->strength=g->permanent_strength+(legacy_relic(g,28)?1:0);g->low_hp_triggered=0;g->dexterity=legacy_relic(g,35)?1:0;
 if(legacy_relic(g,61))for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])>=50&&CARD_ID(g->deck[i])<=53)g->strength++;
 g->weak=g->vulnerable=g->poison=g->block=0;
 g->reward_gold=(mode==NODE_BOSS?95:mode==NODE_ELITE?48:18)+rnd(g,12);
 if(legacy_relic(g,70))g->strength++;if(legacy_relic(g,71))g->dexterity++;
 if(legacy_relic(g,65))for(int i=0;i<g->enemy_count;i++)g->enemies[i].poison+=2;
 if(legacy_relic(g,75))for(int i=0;i<g->enemy_count;i++)g->enemies[i].weak=2;
 if(legacy_relic(g,19))for(int i=0;i<g->enemy_count;i++)g->enemies[i].vulnerable=2;
 reshuffle_battle(g);msg(g,"The enemy is preparing an attack.");game_start_turn(g);dist_relic_trigger(g,0);
 if(legacy_relic(g,64))g->block+=6;if(legacy_relic(g,66))regent_gain_stars(g,3);
 if(legacy_relic(g,10))g->block+=12;if(legacy_relic(g,29))g->block+=10;
 int *boon=g->revision182.next_combat;int extra=boon[2],any=0;for(int i=0;i<5;i++)any|=boon[i];
 g->strength+=boon[0];g->block=MIN(999,g->block+boon[1]);g->weak+=boon[3];g->vulnerable+=boon[4];
 memset(boon,0,sizeof(g->revision182.next_combat));for(int i=0;i<extra;i++)draw_card(g);
 if(any)msg(g,"Your mystery bargains take effect. Inspect STATUS for details.");
}
static void check_win(Game*g);
void game_start_turn(Game*g){
 g->expedition.combat[E181_POWER_ENERGY]=g->expedition.combat[E181_WEAK_ATTACK]=g->expedition.combat[E181_HURT_BLOCK]=g->expedition.combat[E181_EXHAUST_HEAL]=0;
 if(g->screen!=COMBAT)return;
 g->turn++;g->v151.relic[0]=g->v151.relic[5]=0;
 ninja_start_turn_before(g);regent_before_turn(g);
 g->ic[IC_OWN_TURN]=1;
 g->ic[IC_HPLOSS_TURN]=0;
 g->ic[IC_ATTACKS_THIS_TURN]=0;g->ic[36]=0;
 g->ic[IC_BELL]=g->ic[IC_NEEDLE]=g->ic[IC_DUELIST]=0;
 g->ic[IC_UNMOVABLE_USED]=0;
 g->ic[IC_ENERGY_LOCK]=0;
 if(g->ic[IC_PLATING]>0)g->ic[IC_PLATING]--;
 if(g->heal_after_battle>0)g->strength+=g->heal_after_battle;
 if(RIFT_INFERNO_POWER(g)>0){
    /* Each copy loses 1 HP at turn start and triggers every copy's AoE. */
    const int copies=RIFT_INFERNO_COPIES(g);
    for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
    if(g->screen==COMBAT)check_win(g);
 }
 if(g->screen!=COMBAT)return;
 if(g->ic[IC_CRIMSON_COPIES]>0){
  int copies=g->ic[IC_CRIMSON_COPIES];
  for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
  if(g->screen==COMBAT&&g->ic[IC_CRIMSON]>0){
   int old_block=g->block;g->block=MIN(999,g->block+g->ic[IC_CRIMSON]);
   record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);
   proc_juggernaut(g);
  }
 }
 if(g->screen!=COMBAT)return;
 g->energy=MAX(0,g->max_energy+(g->turn==1?(legacy_relic(g,1)+legacy_relic(g,33)):0)-g->energy_penalty+g->ic[IC_PYRE]+g->ic[IC_HOURGLASS]);g->energy_penalty=0;g->ic[IC_HOURGLASS]=0;gain_energy(g,g->v151.relic[1]);g->v151.relic[1]=0;
 if(legacy_relic(g,73)&&g->turn%3==0)gain_energy(g,1);
 if(legacy_relic(g,20)&&g->turn%3==0)heal(g,4);if(legacy_relic(g,34)&&g->turn%3==0)g->strength++;
 g->turn_played=0;g->reward_potion=0;g->shop_relic=0;g->shop_cards[1]=0;
 if(legacy_relic(g,14)&&!g->low_hp_triggered&&g->hp*2<=g->maxhp){g->strength+=3;g->low_hp_triggered=1;}
 int n=5+(g->turn==1&&legacy_relic(g,69)?1:0)+(legacy_relic(g,18)?1:0)+(g->turn==1?2*legacy_relic(g,7)+2*legacy_relic(g,NR_SNAKE):0)+(g->turn<=3?2*legacy_relic(g,NR_DRAKE):0)-(legacy_relic(g,25)?1:0)+g->ninja.v[NS_NEXT_DRAW];g->ninja.v[NS_NEXT_DRAW]=0;
 /* Draw Primed cards first through the normal draw resolver. More than the
    hand limit stay in the pile; the operation never creates another copy. */
 if(g->turn==1){int primed=0;
  for(int i=g->draw_count-1;i>=0;i--)if(CARD_MOD(g->draw[i])==MOD_PRIMED){
   int raw=g->draw[i];for(int j=i;j+1<g->draw_count;j++)g->draw[j]=g->draw[j+1];g->draw[g->draw_count-1]=raw;primed++;
  }n=MAX(n,MIN(primed,MAX_HAND));
 }
 for(int i=0;i<n;i++)draw_card(g);
 if(g->ic[IC_AGGRESSION]>0){
  for(int j=0;j<g->ic[IC_AGGRESSION];j++){
   int count=0;
   for(int i=0;i<g->discard_count;i++)
    if(CARD_ID(g->discard[i])<CARD_COUNT&&cards[CARD_ID(g->discard[i])].type==0)count++;
   if(!count||g->hand_count>=MAX_HAND)break;
   int which=rnd(g,count);
   for(int i=0;i<g->discard_count;i++){
    int c=g->discard[i];
    if(cards[CARD_ID(c)].type==0&&which--==0){
     for(int k=i;k+1<g->discard_count;k++)g->discard[k]=g->discard[k+1];
     g->discard_count--;add_generated(g,c|UPBIT);break;
    }
   }
  }
 }
 g->selection=0;
 ninja_start_turn_after(g);regent_start_turn(g);dist_relic_trigger(g,1);
 if(legacy_relic(g,76)&&g->turn==2)gain_energy(g,1);
 if(legacy_relic(g,79)&&g->turn%4==0)g->strength+=2;
 if(legacy_relic(g,86)&&g->draw_count<=3)relic_block(g,6);
 if(g->expedition.combat[E181_NEXT_BLOCK]){relic_block(g,8);g->expedition.combat[E181_NEXT_BLOCK]=0;}
 if(legacy_relic(g,90)&&g->turn==3)ninja_poison_all(g,4);
 if(legacy_relic(g,91)&&g->turn==1){int powers=0;for(int i=0;i<g->deck_count;i++)powers+=cards[CARD_ID(g->deck[i])].type==2;if(powers>=3)draw_card(g);else relic_block(g,4);}
}
static int enemy_base_damage(const Enemy *e){
 int v=e->intent_value;
 switch(e->intent){
 case 0:return MAX(0,v+e->strength);
 case 2:return MAX(0,v/2+e->strength);
 case 3:return MAX(0,v+e->strength);
 case 5:case 6:case 8:case 18:case 24:case 27:case 28:case 30:return MAX(0,v/2+e->strength);
 case 7:return MAX(0,v/2+e->strength);
 case 10:return MAX(0,v+(enemy_defs[e->art].tier==2?v/4:v/8)+e->strength);
 case 12:case 14:case 15:return MAX(0,v/2+e->strength);
 case 16:case 19:case 26:return MAX(0,v/3+e->strength);
 default:return 0;
 }
}
/* Campaign tuning uses a fixed legal-action policy and held-out seeds. */
#ifndef RIFT_DAMAGE_PERCENT
#define RIFT_DAMAGE_PERCENT 123
#endif
static int enemy_raw_damage(const Enemy *e){
 return (int)(((long long)enemy_base_damage(e)*RIFT_DAMAGE_PERCENT+50)/100);
}
static int enemy_encounter_damage(const Game*g,const Enemy*e){
 int n=enemy_raw_damage(e);
 /* Original encounter size remains fixed after deaths. Combined raw damage
    is 140% for a pair and 135% for a trio, before statuses and multi-hits. */
 int share=g->enemy_count==3?45:g->enemy_count==2?70:100;
 if(r186_boss_group(g))share=e->art==8?R186_BOSS3_DAMAGE:e->art==45?R186_BOSS7_DAMAGE:e->art==62?55:45;
 else if(g->battle_type==NODE_ELITE&&g->enemy_count>1)share=60;
 int damage=(int)(((long long)n*share+50)/100);
 /* Boss attacks gain 5% per hit before statuses and player Block. */
 if(g->battle_type==NODE_BOSS)damage=(int)(((long long)damage*105+50)/100);
 damage=(int)(((long long)damage*(100+CLAMP(g->act-1,0,20))+50)/100);
 return damage;
}
static int enemy_attack_value(const Game*g,const Enemy *e){
 int n=enemy_encounter_damage(g,e);
 return e->weak?n*3/4:n;
}
int game_enemy_intent_hits(const Game *g,int index){
 if(!g||index<0||index>=g->enemy_count)return 0;
 const Enemy *e=&g->enemies[index];
 return e->hp>0&&enemy_raw_damage(e)>0?(e->intent==7||e->intent==19||e->intent==26?2:e->intent==16?3:1):0;
}
int game_enemy_intent_damage(const Game *g,int index){
 int hits=game_enemy_intent_hits(g,index);
 if(!hits)return 0;
 int n=enemy_attack_value(g,&g->enemies[index]);
 if(g->enemies[index].weak&&legacy_relic(g,NR_KRANE))n=enemy_encounter_damage(g,&g->enemies[index])*3/5;
 if(g->ic[IC_COLOSSUS]&&g->enemies[index].vulnerable)n=(n+1)/2;
 if(g->ic[IC_TANK])n*=2;
 if(g->vulnerable)n=n*3/2;
 if(g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);
 return n*hits; /* Total before block. */
}
void game_enemy_intent_text(const Game*g,int index,char*out,int cap){
 if(!out||cap<=0)return;out[0]=0;
 if(!g||index<0||index>=g->enemy_count)return;
 const Enemy*e=&g->enemies[index];
 if(e->hp<=0){snprintf(out,cap,"Defeated");return;}
 int total=game_enemy_intent_damage(g,index),hits=game_enemy_intent_hits(g,index);
 switch(e->intent){
 case 0:snprintf(out,cap,"Deal %d DMG",total);break;
 case 1:snprintf(out,cap,"Gain %d Guard",e->intent_value);break;
 case 2:snprintf(out,cap,"Deal %d DMG; apply 2 Falter",total);break;
 case 3:snprintf(out,cap,"Deal %d DMG; apply 2 Fracture",total);break;
 case 4:snprintf(out,cap,"Gain 2 Force and 8 Guard");break;
 case 5:snprintf(out,cap,"Deal %d DMG; add Burn to discard",total);break;
 case 6:snprintf(out,cap,"Deal %d DMG; apply 2 Wither",total);break;
 case 7:snprintf(out,cap,"Deal %d DMG twice",hits?total/hits:0);break;
 case 8:snprintf(out,cap,"Deal %d DMG; drain 1 Energy next turn",total);break;
 case 9:snprintf(out,cap,"Heal %d HP; gain %d Guard",MIN(e->maxhp-e->hp,e->maxhp/9+3),e->intent_value/2);break;
 case 10:snprintf(out,cap,"Deal %d DMG; add Daze to discard",total);break;
 case 11:snprintf(out,cap,"Add Dread to discard; gain %d Guard",e->intent_value/2);break;
 case 12:snprintf(out,cap,"Deal %d DMG; heal for HP lost",total);break;
 case 13:snprintf(out,cap,"Gain %d Guard and 1 Force",e->intent_value);break;
 case 14:snprintf(out,cap,"Deal %d DMG, ignoring Guard",total);break;
 case 15:snprintf(out,cap,"Remove 4 Guard, then deal %d DMG",total);break;
 case 16:snprintf(out,cap,"Deal %d DMG three times",hits?total/hits:0);break;
 case 17:snprintf(out,cap,"Add Injury to discard; gain %d Guard",e->intent_value/2);break;
 case 18:snprintf(out,cap,"Deal %d DMG; apply 2 Falter and Fracture",total);break;
 case 19:snprintf(out,cap,"Deal %d DMG twice; add Burn",hits?total/hits:0);break;
 case 20:snprintf(out,cap,"All foes gain 1 Force and 8 Guard");break;
 case 21:snprintf(out,cap,"Cleanse Falter/Fracture, halve Wither; %d Guard",e->intent_value/2);break;
 case 22:snprintf(out,cap,"Apply 2 Falter; drain 1 Energy next turn");break;
 case 23:snprintf(out,cap,"Apply 2 Wither; add Daze to discard");break;
 case 24:snprintf(out,cap,"Deal %d DMG; gain %d Guard",total,MAX(1,e->intent_value/2));break;
 case 25:snprintf(out,cap,"Gain %d Guard; add Burn to discard",MAX(1,e->intent_value/2));break;
 case 26:snprintf(out,cap,"Deal %d DMG twice; drain 1 Energy next turn",hits?total/hits:0);break;
 case 27:snprintf(out,cap,"Deal %d DMG; all foes gain 4 Guard",total);break;
 case 28:snprintf(out,cap,"Remove 3 Guard; deal %d DMG; drain 1 Energy",total);break;
 case 29:snprintf(out,cap,"All foes gain 5 Guard; apply 2 Wither");break;
 case 30:snprintf(out,cap,"Deal %d DMG; apply 2 Fracture; add Daze",total);break;
 case 31:snprintf(out,cap,"No attack; exposes itself next turn");break;
 }
}
static void enemy_hit(Game*g,Enemy*e,int raw){
 if(e->hp<=0)return;
 if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;
 if(g->ic[IC_TANK])raw*=2;
 if(e->weak&&legacy_relic(g,NR_KRANE)){int full=enemy_encounter_damage(g,e);raw=full*3/5;if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;if(g->ic[IC_TANK])raw*=2;}
 int old_source=combat_fx_source;combat_fx_source=(int)(e-g->enemies);
 damage_player(g,raw);if(g->screen==DEFEAT)snprintf(g->subtitle,sizeof(g->subtitle),"Slain by %s / %s",game_enemy_name(e->art),game_intent_title(e->intent));combat_fx_source=old_source;
 if(g->shop_cards[1]>0){
   int n=g->shop_cards[1],blocked=MIN(e->block,n);
   e->block-=blocked;e->hp-=n-blocked;
   record_combat_fx(GFX_PROC,-2,(int)(e-g->enemies),n,blocked);
 }
}
static void all_damage(Game*g,int n);
void game_prepare_end_turn(Game*g){
 if(g->screen!=COMBAT||g->ic[38]||g->ninja.task_count||g->regent.task_count)return;
 if(ninja_prepare_retain(g))return;
 regent_end_turn(g);if(g->screen!=COMBAT)return;dist_relic_trigger(g,2);
 g->ic[38]=1;
 if(g->ic[IC_STAMPEDE]>0&&g->hand_count>0){
  int copies=MIN(4,g->ic[IC_STAMPEDE]);
  for(int j=0;j<copies&&g->screen==COMBAT;j++){
   int count=0;
   for(int i=0;i<g->hand_count;i++)
    if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)count++;
   int foe=random_living_foe(g);
   if(!count||foe<0)break;
   int which=rnd(g,count);
   for(int i=0;i<g->hand_count;i++)
    if(cards[CARD_ID(g->hand[i])].type==0&&which--==0){
     g->hand[i]|=CARD_FREE;
     game_play_card(g,i,foe);break;
    }
  }
 }
 if(g->screen!=COMBAT)return;
 if(g->ic[IC_COVENANT]>0){
  int damage=g->ic[IC_COVENANT]*CLAMP(g->energy,0,2);
  for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0&&damage>0){
   int absorbed=MIN(g->enemies[e].block,damage);g->enemies[e].block-=absorbed;g->enemies[e].hp-=damage-absorbed;
   record_combat_fx(GFX_PROC,-1,e,damage,absorbed);
  }
 }
 if(g->ic[IC_PLATING]>0){
  int old_block=g->block;g->block=MIN(999,g->block+g->ic[IC_PLATING]);
  record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);
  proc_juggernaut(g);
 }
 if(legacy_relic(g,88)&&g->block==0)g->expedition.combat[E181_NEXT_BLOCK]=1;
 if(legacy_relic(g,54)&&!g->ic[IC_ATTACKS_THIS_TURN])g->v151.relic[1]=1;
 if(legacy_relic(g,59)&&g->block>=20)ninja_poison_all(g,2);
 check_win(g);
}
void game_end_turn(Game*g){
 if(g->screen!=COMBAT||g->ninja.task_count||g->regent.task_count)return;
 game_prepare_end_turn(g);
 if(g->screen!=COMBAT)return;
 g->ic[38]=0;
 if(g->ic[IC_TEMP_STRENGTH]>0){
  g->strength-=g->ic[IC_TEMP_STRENGTH];g->ic[IC_TEMP_STRENGTH]=0;
 }
 g->dexterity-=g->ninja.v[NS_TEMP_DEX];g->ninja.v[NS_TEMP_DEX]=0;
 g->ninja.v[NS_CORROSIVE]=g->ninja.v[NS_SHADOWMELD]=g->ninja.v[NS_BURST]=0;g->ninja.v[NS_STRANGLE0]=g->ninja.v[NS_STRANGLE1]=g->revision185.strangle2=0;
 g->ic[IC_OWN_TURN]=0;
 g->ic[IC_HOURGLASS]=legacy_relic(g,40)?CLAMP(g->energy,0,2):0;
 int retained=0;
 for(int i=0;i<g->hand_count;i++){
  g->hand[i]&=~CARD_FREE;
  int burden=CARD_ID(g->hand[i]);
  if(burden==50){lose_hp(g,1);g->regent.stars=MAX(0,g->regent.stars-1);}
  else if(burden==51)g->regent.stars=MAX(0,g->regent.stars-2);
  else if(burden==52)g->block=MAX(0,g->block-2);
  else if(burden==53){if(g->regent.stars>=2)g->regent.stars-=2;else lose_hp(g,2);}
  if(g->screen==DEFEAT)return;
  if(regent_flags(g->hand[i])&8){exhaust_card(g,g->hand[i]);continue;}
  if(game_card_is_retained(g,g->hand[i]))g->hand[retained++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);
 }
 g->hand_count=retained;g->regent.retain_all=0;g->regent.monologue=0;
 /* Hand Trick lasts for the whole turn, even after playing/reshuffling. */
 for(int i=0;i<g->draw_count;i++)g->draw[i]&=~CARD_SLY_TURN;
 for(int i=0;i<g->discard_count;i++)g->discard[i]&=~CARD_SLY_TURN;
 for(int i=0;i<g->exhaust_count;i++)g->exhaust[i]&=~CARD_SLY_TURN;
 /* Block shields the incoming enemy turn, then expires before the next draw. */
 for(int k=0;k<g->enemy_count;k++)g->enemies[k].block=0;
 for(int k=0;k<g->enemy_count;k++){
  Enemy *e=&g->enemies[k];if(e->hp<=0)continue;
  for(int p=0;p<=MIN(1000,g->ninja.v[NS_ACCELERANT])&&e->hp>0&&e->poison>0;p++){record_combat_fx(GFX_PROC,-1,k,e->poison,0);e->hp-=e->poison;e->poison=MAX(0,e->poison-2);}
  if(e->hp<=0)continue;
  switch(e->intent){
   case 0: {int dmg=enemy_attack_value(g,e);enemy_hit(g,e,dmg);break;}
   case 1:e->block+=e->intent_value;break;
   case 2:enemy_hit(g,e,enemy_attack_value(g,e));g->weak=MAX(g->weak,2);break;
   case 3:enemy_hit(g,e,enemy_attack_value(g,e));g->vulnerable=MAX(g->vulnerable,2);break;
   case 4:e->strength+=2;e->block+=8;break;
   case 5:enemy_hit(g,e,enemy_attack_value(g,e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 6:enemy_hit(g,e,enemy_attack_value(g,e));g->poison=MIN(12,g->poison+2);break;
   case 7:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(g,e));break;
   case 8:enemy_hit(g,e,enemy_attack_value(g,e));g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 9:e->hp=MIN(e->maxhp,e->hp+e->maxhp/9+3);e->block+=e->intent_value/2;break;
   case 10:enemy_hit(g,e,enemy_attack_value(g,e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;
   case 11:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=53;e->block+=e->intent_value/2;break;
   case 12:{int before=g->hp;enemy_hit(g,e,enemy_attack_value(g,e));e->hp=MIN(e->maxhp,e->hp+MAX(0,before-g->hp));break;}
   case 13:e->block+=e->intent_value;e->strength++;break;
   case 14:{int preserved=g->block;g->block=0;enemy_hit(g,e,enemy_attack_value(g,e));g->block+=preserved;break;}
   case 15:g->block=MAX(0,g->block-4);enemy_hit(g,e,enemy_attack_value(g,e));break;
   case 16:for(int hit=0;hit<3&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(g,e));break;
   case 17:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=52;e->block+=e->intent_value/2;break;
   case 18:enemy_hit(g,e,enemy_attack_value(g,e));g->weak=MAX(g->weak,2);g->vulnerable=MAX(g->vulnerable,2);break;
   case 19:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(g,e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 20:for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0){g->enemies[z].strength++;g->enemies[z].block+=8;}break;
   case 21:e->weak=e->vulnerable=0;e->poison/=2;e->block+=e->intent_value/2;break;
   case 22:g->weak=MAX(g->weak,2);g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 23:g->poison=MIN(12,g->poison+2);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;
   case 24:enemy_hit(g,e,enemy_attack_value(g,e));if(e->hp>0)e->block+=MAX(1,e->intent_value/2);break;
   case 25:e->block+=MAX(1,e->intent_value/2);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 26:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(g,e));if(g->screen==COMBAT)g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 27:enemy_hit(g,e,enemy_attack_value(g,e));for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)g->enemies[z].block+=4;break;
   case 28:g->block=MAX(0,g->block-3);enemy_hit(g,e,enemy_attack_value(g,e));if(g->screen==COMBAT)g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 29:for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)g->enemies[z].block+=5;g->poison=MIN(12,g->poison+2);break;
   case 31:e->vulnerable=MAX(2,e->vulnerable);break;
   case 30:enemy_hit(g,e,enemy_attack_value(g,e));g->vulnerable=MAX(g->vulnerable,2);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;
  }
  if(e->block>0)record_combat_fx(GFX_BLOCK,k,k,e->block,0);
  if(g->screen==DEFEAT)return;
  if(e->weak>0)e->weak--;
  if(e->vulnerable>0)e->vulnerable--;
 }
 if(g->poison>0){
   int inferno=g->used_potion;g->used_potion=0;
   lose_hp(g,g->poison);g->used_potion=inferno;
   g->poison=MAX(0,g->poison-2);if(g->screen==DEFEAT)return;
 }
 if(g->weak>0)g->weak--;
 if(g->vulnerable>0)g->vulnerable--;
 check_win(g);
 if(g->screen!=COMBAT)return;
 if(!g->ic[IC_BARRICADE]&&!g->ninja.v[NS_BLUR])g->block=0;
 if(g->ninja.v[NS_BLUR])g->ninja.v[NS_BLUR]--;
 if(g->ninja.v[NS_INTANGIBLE])g->ninja.v[NS_INTANGIBLE]--;
 g->ic[IC_COLOSSUS]=0;
 for(int i=0;i<g->enemy_count;i++){
  if((*MANGLE_SLOT(g,i))!=0){
   g->enemies[i].strength+=(*MANGLE_SLOT(g,i));
   (*MANGLE_SLOT(g,i))=0;
  }
 }
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)enemy_intent(g,&g->enemies[i]);
 game_start_turn(g);
}
static int has_strike_tag(const char *name){
 for(;*name;name++){
  const char *p=name,*key="Strike";
  while(*p&&*key&&*p==*key){p++;key++;}
  if(!*key)return 1;
 }
 return 0;
}
static int strike_count(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++){
  int id=CARD_ID(g->deck[i]);
  if(id>=0&&id<CARD_COUNT&&has_strike_tag(cards[id].name))count++;
 }
 return count;
}
static void all_damage(Game*g,int n){for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)deal(g,i,n);}
static void exhaust_card(Game*g,int raw){
 if(g->exhaust_count<MAX_PILE){
  g->exhaust[g->exhaust_count++]=raw;
  r151_exhaust(g);
  g->reward_potion|=4;
  if(legacy_relic(g,41)&&g->ic[IC_BELL]<3){g->ic[IC_BELL]++;relic_block(g,3);}
  if(g->ic[IC_DARK_EMBRACE]&&g->ic[IC_CHAIN_DEPTH]<20){
    g->ic[IC_CHAIN_DEPTH]++;
    for(int j=0;j<g->ic[IC_DARK_EMBRACE];j++)draw_card(g);
    g->ic[IC_CHAIN_DEPTH]--;
  }
  if(g->shop_cards[2]>0){int old_block=g->block;g->block=MIN(999,g->block+g->shop_cards[2]);record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);}
 }
}
static void select_live_enemy(Game*g,int direction);
int game_card_cost_now(const Game*g,int raw){(void)g;return raw&CARD_FREE?0:game_card_cost(raw);}
#include "relics151.inc"
#include "distribution_engine.inc"
static void settle_card_choice(Game*g,int idx){
 int up=g->scroll;
 if(g->screen!=CARD_CHOICE||idx<0||
 idx>=(g->next_screen==25?g->discard_count:g->hand_count))return;
 if(g->next_screen==25){
  int c=g->discard[idx];
  for(int i=idx;i+1<g->discard_count;i++)g->discard[i]=g->discard[i+1];
  g->discard_count--;
  if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=c;
  g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
  int headbutt=g->ic[IC_PENDING_CARD]?g->ic[IC_PENDING_CARD]:25|(up?UPBIT:0);g->ic[IC_PENDING_CARD]=0;
  if(CARD_MOD(headbutt)==MOD_FLEETING||CARD_MOD(headbutt)==MOD_KINDLED)exhaust_card(g,headbutt);
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=headbutt;
  g->selection=0;r151_finish_choice(g);check_win(g);return;
 }
 int pending=g->next_screen,c=g->hand[idx];
 if(pending==23){
  if(!game_card_is_upgradeable(c)){msg(g,"Choose an unupgraded card.");return;}
  g->hand[idx]|=UPBIT;
 }else if(pending==18||pending==43||pending==17||pending==26){
  for(int i=idx;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];
  g->hand_count--;
  if(pending==26){if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;}
  else exhaust_card(g,c);
 }
 g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
 g->selection=MIN(idx,g->hand_count);
 if(pending==18)g->strength+=1+up;
 if(pending==43)for(int i=0;i<2+up;i++)draw_card(g);
 r151_finish_choice(g);
 if(g->screen==COMBAT)check_win(g);
}
const char *node_name(int type){static const char*names[]={"BATTLE","ELITE","MYSTERY","MERCHANT","CAMPFIRE","RELIC","BOSS"};return type>=0&&type<=6?names[type]:"UNKNOWN";}
static void encounter(Game*g,int node){
 g->revision182.room_type=node;g->revision182.room_pending=node!=NODE_BOSS;
 g->selection=0;
 switch(node){
 case NODE_FIGHT:case NODE_ELITE:game_start_battle(g,node);break;
 case NODE_BOSS:game_story_start(g,1);break;
 case NODE_EVENT:game_begin_event(g,game_next_event(g));break;
 case NODE_SHOP:g->screen=SHOP;for(int i=0;i<3;i++)g->shop_cards[i]=card_reward_except(g,i>0?g->shop_cards[0]:-1,i>1?g->shop_cards[1]:-1);g->shop_relic=random_relic(g);break;
 case NODE_REST:g->screen=REST;break;
 case NODE_TREASURE:{
   int r=expedition_relic(g);
   if(r>=0){
      g->relic_pending=1;g->relic_resume=MAP;g->reward[0]=r;g->win_screen=0;g->screen=RELIC_RESULT;
      snprintf(g->message,sizeof(g->message),"FOUND RELIC: %s",relic_defs[r].name);
   }else{msg(g,"The chest was empty.");enter_map(g);}
   break;
 }
 }
}
void game_choose_node(Game*g,int lane){
 if(g->screen!=MAP||g->floor>=MAP_ROWS)return;
 if(lane<0||lane>=MAP_LANES)return;
 if(!game_route_available(g,lane))return;
 g->lane=lane;g->map_selected[g->floor]=lane;
 int type=g->map[g->floor][lane];g->floor++;encounter(g,type);
}
void game_new(Game*g,uint32_t seed){
 memset(g,0,sizeof(*g));g->magic=0x52494654u;g->version=GAME_VERSION;
 g->revision184.route_style=1;g->revision182.rare_offset=-5;g->v151.potion_offer=-1;g->seed=seed?seed:0x1affadeu;g->rng=g->seed;
 g->screen=TITLE;g->selection=1;g->act=1;g->floor=0;g->lane=1;
 g->maxhp=g->hp=78;g->max_energy=3;g->gold=95;
 for(int i=0;i<5;i++)add_card(g,0);
 for(int i=0;i<4;i++)add_card(g,1);
 add_card(g,2);add_relic(g,0);
 g->potion_count=2;g->potions[0]=0;g->potions[1]=1;
 map_generate(g);msg(g,"A lone Warden climbs the ruined tower.");
}
const char *game_character_name(const Game*g){return g&&g->character==CHAR_PROXY?"Proxy":g&&g->character==CHAR_VESPER?"Vesper":"Warden";}
void game_choose_character(Game*g,int character){
 if(!g||character<0||character>2)return;
 if(character==CHAR_PROXY){g->character=CHAR_PROXY;g->deck_count=g->relic_count=g->potion_count=0;g->max_energy=3;g->maxhp=g->hp=76;g->gold=99;for(int i=0;i<4;i++){add_card(g,RIFT_CARD_269);add_card(g,RIFT_CARD_210);}add_card(g,RIFT_CARD_213);add_card(g,RIFT_CARD_278);add_relic(g,63);msg(g,"The Proxy claims the ruined tower.");return;}
 g->character=character;g->deck_count=g->relic_count=0;g->potion_count=0;
 g->max_energy=3;g->maxhp=g->hp=character==CHAR_VESPER?73:78;g->gold=character==CHAR_VESPER?99:95;
 for(int i=0;i<5;i++)add_card(g,character==CHAR_VESPER?RIFT_CARD_173:0);
 for(int i=0;i<(character==CHAR_VESPER?5:4);i++)add_card(g,character==CHAR_VESPER?RIFT_CARD_120:1);
 if(character==CHAR_VESPER){add_card(g,RIFT_CARD_147);add_card(g,RIFT_CARD_176);add_relic(g,NR_SNAKE);}
 else{add_card(g,2);add_relic(g,0);g->potion_count=2;g->potions[0]=0;g->potions[1]=1;}
 msg(g,character==CHAR_VESPER?"Vesper enters the ruined tower.":"A lone Warden climbs the ruined tower.");
}
int game_story_kind(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 if(expedition_scene(screen))return screen==ABOMINATION_BEFORE?2:1;
 if(ascent_story_screen(screen))return screen==ASCENT_BOSS?2:1;
 return (screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING)?1:screen==STORY_BOSS?2:0;
}
static uint32_t story_hash(uint32_t x){x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
void game_story_start(Game*g,int boss){
 if(boss&&g->act==7){g->screen=ABOMINATION_BEFORE;g->selection=0;return;}
 if(boss&&g->act==4&&g->ascent.secret){g->screen=ASCENT_BOSS;g->selection=0;return;}
 if(boss&&g->act>3){game_start_battle(g,NODE_BOSS);return;}
 g->screen=boss?STORY_BOSS:STORY_ACT_BEFORE_BLESSING;g->selection=0;
 g->scroll=(int)(story_hash(g->seed^((uint32_t)g->character+1u)*0x9e3779b9u^((uint32_t)g->act)*0x85ebca6bu^(boss?0x170b055u:0x170ac7u))%(boss?5u:10u));
}
const char*game_story_text(const Game*g){
 const char*memory=game_loop_story(g);if(memory)return memory;
 int c=CLAMP(g->character,0,2),a=CLAMP(g->act-1,0,3);
 return game_story_kind(g)==2?r170_boss[c][MIN(a,2)][CLAMP(g->scroll,0,4)][CLAMP(g->selection,0,2)]:r170_entry[c][a][CLAMP(g->scroll,0,9)];
}
const char*game_story_speaker(const Game*g){
 static const char*names[]={"WARDEN","VESPER","PROXY"};
 return game_story_kind(g)==2&&g->selection==1?game_enemy_name(enemy_boss[CLAMP(g->act-1,0,2)]):names[CLAMP(g->character,0,2)];
}
void game_update(Game*g){if(g->screen==COMBAT&&g->ninja.task_count)ninja_process(g);if(g->screen==COMBAT&&(g->regent.task_count||g->regent.shuffle_pending))regent_process(g);if(g->screen==ACT_INTRO&&++g->scroll>=120)game_story_start(g,0);}
int game_music_silent(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 return (expedition_scene(screen)&&screen!=ABOMINATION_BEFORE)||screen==ACT_INTRO||screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING||(ascent_story_screen(screen)&&screen!=ASCENT_BOSS);
}
int game_music_track(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 if(screen==ASCENT_BOSS)return 11;
 if(screen==STORY_BOSS||screen==ABOMINATION_BEFORE)return 0;
 if(screen>=ASCENT_SLOTS_VIEW&&screen<=ASCENT_OPTIONS)return 10;
 if(custom_setup_screen(screen))return 10;
 if(screen==TITLE||screen==NEW_RUN_CONFIRM||screen==HELP||screen==CHARACTER_SELECT)return 10;
 if(screen==COMBAT||screen==CARD_CHOICE||screen==NINJA_CHOICE||screen==REGENT_CHOICE||(screen==DECK_VIEW&&g->prior_screen==COMBAT)){
  if(g->battle_type==NODE_ELITE)return g->combat_music==6?6:3;
  if(g->battle_type==NODE_BOSS)return g->act==4&&g->ascent.secret?11:0;
  return g->combat_music==4?4:g->combat_music==5?5:2;
 }
 return g->act==1?1:g->act==2?7:g->act==3?8:9;
}
static void shop_buy(Game*g,int choice){
 int discount=dist_shop_discount(g);
 if(choice<3){if(g->shop_cards[choice]<0){msg(g,"Already sold.");return;}int price=MAX(5,50+25*cards[g->shop_cards[choice]].rarity-discount);if(g->gold>=price&&g->deck_count<MAX_DECK){g->gold-=price;add_card(g,g->shop_cards[choice]);g->shop_cards[choice]=-1;msg(g,"Purchased a card.");}else msg(g,"Not enough gold.");}
 else if(choice==3){int price=155-discount;if(g->gold>=price&&g->shop_relic>=0){g->gold-=price;add_relic(g,g->shop_relic);g->shop_relic=-1;msg(g,"Relic purchased.");}else msg(g,"Not enough gold.");}
 else if(choice==4){if(g->gold>=60&&g->deck_count>1){
   g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;g->deck_readonly=0;g->shop_selection=4;
   msg(g,"Choose card to remove; charge only when confirmed.");
 }else msg(g,"Not enough gold.");}
 else if(choice==5){if(g->gold>=50&&g->potion_count<MAX_POTIONS){
   g->gold-=50;int potion=ascent_random_potion(g);g->potions[g->potion_count++]=potion;
   snprintf(g->message,sizeof(g->message),"Potion gained: %s",game_potion_name(potion));
 }else msg(g,g->potion_count==MAX_POTIONS?"Belt full. No gold spent.":"Not enough gold.");}
 else if(choice==6){enter_map(g);}

}
static int count_upgradeable(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++)
  if(game_card_is_upgradeable(g->deck[i]))count++;
 return count;
}
#include "events150.inc"
static void rest_pick(Game*g,int option){
 if(option==0){heal(g,g->maxhp*3/10);dist_relic_trigger(g,7);if(legacy_relic(g,8)&&count_upgradeable(g)){g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=g->scroll=0;msg(g,"Rested. Choose an upgrade.");}else{enter_map(g);msg(g,"Recovered at the campfire.");}}
 if(option==1){
   if(!count_upgradeable(g)){msg(g,"All cards already upgraded.");return;}
   g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=0;g->scroll=0;
   msg(g,"Choose an unupgraded card.");
 }
}
int game_inventory_available(const Game*g){
 int screen=g->screen;
 if(screen==PAUSE_MENU)screen=game_pause_source();
 if(screen==HELP)screen=g->prior_screen;
 /* Every in-run decision and result can be inspected. Story scenes and
    front-end setup have no overlay; its cursor never replaces menu state. */
 switch(screen){
 case COMBAT:case MAP:case SHOP:case REST:case EVENT:case EVENT_CARD:case EVENT_RESULT:
 case REWARD:case BOSS_REWARD:case REWARD_PREVIEW:case RELIC_RESULT:case BLESSING:
 case DECK_VIEW:case POTION_VIEW:case RELIC_VIEW:case CARD_CHOICE:case NINJA_CHOICE:case REGENT_CHOICE:
 case EXPEDITION_GIFT:case MYSTERY_CARDS:case BETA_FORGE:case BETA_FORGE_CARD:
 case VICTORY:case DEFEAT:case ASCENT_CARDS:case ASCENT_POTIONS:case ASCENT_RELIC:case ASCENT_SAVE_ERROR:return 1;
 default:return 0;
 }
}
int game_inventory_can_change(const Game*g){
 if(!game_inventory_available(g))return 0;
 /* Inspection must not consume/discard items during an unresolved card
    effect, paused game or completed run. Between-room choices remain usable. */
 switch(g->screen){case CARD_CHOICE:case NINJA_CHOICE:case REGENT_CHOICE:case PAUSE_MENU:case HELP:
 case VICTORY:case DEFEAT:case ASCENT_CARDS:case ASCENT_POTIONS:case ASCENT_RELIC:case ASCENT_SAVE_ERROR:return 0;
 case DECK_VIEW:return g->prior_screen!=COMBAT;
 default:return 1;
 }
}
void game_discard_potion(Game*g,int index){
 if(!game_inventory_can_change(g)||index<0||index>=g->potion_count)return;
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;msg(g,"Potion discarded.");
}
void game_use_potion(Game*g,int index){
 if(!game_inventory_can_change(g))return;
 int combat=g->screen==COMBAT;
 int outside=game_inventory_available(g)&&!combat;
 if((!combat&&!outside)||g->potion_count<=0)return;
 if(index<0||index>=g->potion_count)return;
 int id=g->potions[index];
 if(!combat&&id!=0&&id!=11&&id!=19&&id!=21){msg(g,"Use this potion during combat.");return;}
 if(!combat&&(id==0||id==11)&&g->hp==g->maxhp){msg(g,"Already at full health. Potion kept.");return;}
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;
 if(combat)g->expedition.combat[E181_POTION_USED]=1;
 switch(id){
#include "new_potion_effects.inc"
 }
 if(combat)dist_relic_trigger(g,4);
 msg(g,"");
 if(combat)check_win(g);
}
static int grid_down(int selected,int count);
#include "inventory151.inc"
static int ascent_action(Game*,int);
static int resume_available;
void game_set_resume_available(int available){resume_available=!!available;}
int game_resume_available(void){return resume_available;}
static int paused_from_screen=MAP;
static int paused_selection;
int game_pause_source(void){return paused_from_screen;}
int game_pause_selection(void){return paused_selection;}
static void select_live_enemy(Game*g,int direction){
 if(g->enemy_count<=0){g->target=0;return;}
 if(g->target<0||g->target>=g->enemy_count)g->target=0;
 int active=0;
 for(int i=0;i<g->enemy_count;i++)active+=g->enemies[i].hp>0;
 if(active==0)return;
 for(int k=0;k<g->enemy_count;k++){
    g->target=(g->target+direction+g->enemy_count)%g->enemy_count;
    if(g->enemies[g->target].hp>0)return;
 }
}
/* Preserve the column where possible; a short final row clamps to its last card.
   Down on the last row stays put instead of moving sideways. */
static int grid_down(int selected,int count){
 return (selected/3+1)*3<count?MIN(selected+3,count-1):selected;
}

#include "beta_game.inc"
void game_action(Game*g,int b){
 if(beta_action(g,b))return;
 if(custom_action(g,b)||expedition_action(g,b)||ascent_action(g,b))return;
 if(g->screen==COMBAT)g->ic[46]=0;
 if(b==BTN_PAUSE){
   if(g->screen==PAUSE_MENU){g->screen=paused_from_screen;return;}
   if(g->screen!=TITLE&&g->screen!=HELP&&g->screen!=NEW_RUN_CONFIRM&&g->screen!=CHARACTER_SELECT&&g->screen!=VICTORY&&g->screen!=DEFEAT){
     paused_from_screen=g->screen;paused_selection=0;g->screen=PAUSE_MENU;return;
   }
 }
 int n;
 switch(g->screen){
 /* Legacy post-blessing scenes must not award a second relic on resume. */
 case STORY_ACT:if(b==BTN_OK)enter_map(g);break;
 case STORY_ACT_BEFORE_BLESSING:if(b==BTN_OK)offer_blessing(g);break;
 case STORY_BOSS:if(b==BTN_OK&&++g->selection>=3)game_start_battle(g,NODE_BOSS);break;
 case ACT_INTRO:if(b==BTN_OK)game_story_start(g,0);break;
 case TITLE:
  if(b==BTN_UP)g->selection=(CLAMP(g->selection,0,4)+4)%5;
  else if(b==BTN_DOWN)g->selection=(CLAMP(g->selection,0,4)+1)%5;
  else if(b==BTN_OK){
   if(g->selection<3){g->revision184.custom=g->selection==2;g->screen=ASCENT_SLOTS_VIEW;g->scroll=g->selection;g->selection=0;}
   else {g->screen=g->selection==3?ASCENT_COLLECTION:ASCENT_OPTIONS;g->selection=g->scroll=0;}
  }break;
 case CHARACTER_SELECT:
  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=(g->selection+((b==BTN_LEFT||b==BTN_UP)?2:1))%3;
  else if(b==BTN_OK){int c=g->selection,slot=g->master_index;unsigned seed=game_random(g);Game old=*g;
   if(!ascent_begin_mode(g,slot,c,seed,old.revision184.custom)){*g=old;msg(g,"Cannot write save slot. Please try again.");}}
  else if(b==BTN_BACK){g->screen=ASCENT_SLOTS_VIEW;g->scroll=1+g->revision184.custom;g->selection=g->master_index;}break;
 case REGENT_CHOICE:{int n=game_regent_choice_count(g);
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;
  else if(b==BTN_OK)regent_choose(g,g->selection,0);
  else if(b==BTN_BACK)regent_choose(g,0,1);break;}
 case NINJA_CHOICE:{int n=g->hand_count;
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;
  else if(b==BTN_OK)ninja_choose(g,g->selection,0);
  else if(b==BTN_BACK)ninja_choose(g,0,1);break;}
 case HELP:if(b==BTN_BACK||b==BTN_OK)g->screen=g->prior_screen;break;
 case MAP:
  if(b==BTN_UP&&g->floor+g->scroll<MAP_ROWS-1) {
    g->scroll++;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll--;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_LEFT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection-1;j>=0;j--)if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_RIGHT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection+1;j<MAP_LANES;j++)if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_OK)
    ; /* Future rooms are inspectable without a notification. */
  else if(b==BTN_LEFT){for(int k=g->selection-1;k>=0;k--)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_RIGHT){for(int k=g->selection+1;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_OK){if(g->floor==MAP_ROWS){game_story_start(g,1);}else game_choose_node(g,g->selection);}
  else if(b==BTN_ALT){g->prior_screen=MAP;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;}
  else if(b==BTN_START){g->screen=RELIC_VIEW;g->prior_screen=MAP;g->selection=0;}
  else if(b==BTN_POTION){g->shop_selection=g->selection;g->prior_screen=MAP;g->screen=POTION_VIEW;g->selection=0;msg(g,"");}
  break;
 case CARD_CHOICE:{
  int choices=g->next_screen==25?g->discard_count:g->hand_count;
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,choices);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<choices)g->selection++;
  else if(b==BTN_OK)settle_card_choice(g,g->selection);
  else if(b==BTN_BACK)msg(g,"Choose a card to finish the effect.");
  break;}
 case COMBAT:
  if(g->ninja.task_count){ninja_process(g);break;}
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(g->hand_count,g->selection+1);
  else if(b==BTN_UP)select_live_enemy(g,-1);
  else if(b==BTN_DOWN)select_live_enemy(g,1);
  else if(b==BTN_OK){if(g->selection>=g->hand_count)game_end_turn(g);else game_play_card(g,g->selection,g->target);}
  else if(b==BTN_BACK)game_end_turn(g);
  else if(b==BTN_POTION)game_use_potion(g,g->potion_count-1);
  else if(b==BTN_ALT){
    g->master_index=g->selection;
    g->prior_screen=COMBAT;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;
  }
  break;
 case REWARD:
  if(g->selection==3)g->selection=5;
  if(b==BTN_DOWN){if(g->selection<3)g->selection=g->v151.potion_offer>=0?4:5;else g->selection=5;}
  else if(b==BTN_UP){if(g->selection==5&&g->v151.potion_offer>=0)g->selection=4;else if(g->selection>=4&&!g->v151.card_done)g->selection=1;}
  else if(b==BTN_LEFT&&g->selection<3&&!g->v151.card_done)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT&&g->selection<3&&!g->v151.card_done)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK||b==BTN_BACK){
    if(b==BTN_BACK)g->selection=5;
    if(g->selection<3&&!g->v151.card_done){
      if(g->deck_count>=MAX_DECK){msg(g,"Deck full. You can leave this card reward.");break;}
      add_card(g,g->reward[g->selection]);g->v151.card_done=1;
      if(g->ninja.v[NS_BONUS_REWARDS]>0){g->ninja.v[NS_BONUS_REWARDS]--;fill_rewards(g);g->v151.card_done=0;g->selection=0;msg(g,"The Hunt: choose an extra card reward.");break;}
      g->selection=g->v151.potion_offer>=0?4:5;break;
    }
    if(g->selection==4&&g->v151.potion_offer>=0){
      if(g->potion_count>=MAX_POTIONS){msg(g,"Belt full. Discard a potion in inventory.");break;}
      g->potions[g->potion_count++]=g->v151.potion_offer;g->ic[37]=g->v151.potion_offer+1;g->v151.potion_offer=-1;g->selection=g->v151.card_done?5:1;break;
    }
    if(g->selection!=5)break;
    if(!g->v151.card_done)r151_skip_reward(g);
    /* Leaving forfeits all remaining card picks and the unclaimed potion. */
    g->v151.card_done=1;g->v151.potion_offer=-1;g->ninja.v[NS_BONUS_REWARDS]=0;
    if(g->win_screen){
      int final=g->win_screen==2;g->win_screen=0;
      if(final){g->screen=VICTORY;g->scroll=g->master_index=0;msg(g,"THE RIFT HAS FALLEN");}
      else{g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;map_generate(g);begin_act(g);}
    }else{enter_map(g);snprintf(g->message,sizeof(g->message),"Onward, %s.",game_character_name(g));}
  }
  break;
 case REST:
  if(b==BTN_LEFT||b==BTN_RIGHT)g->selection=1-g->selection;
  else if(b==BTN_OK)rest_pick(g,g->selection);
  break;
 case SHOP:
  if(b==BTN_ALT){g->shop_selection=g->selection;g->deck_readonly=1;
    g->prior_screen=SHOP;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;}
  else if(b==BTN_POTION||b==BTN_START){g->shop_selection=g->selection;g->prior_screen=SHOP;g->screen=POTION_VIEW;g->selection=0;msg(g,"");}
  else if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(6,g->selection+1);
  else if(b==BTN_OK)shop_buy(g,g->selection);
  else if(b==BTN_BACK)enter_map(g);
  break;
 case EVENT:
  if(b==BTN_LEFT||b==BTN_UP)g->selection=(g->selection+game_event_choice_count(g)-1)%game_event_choice_count(g);
  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%game_event_choice_count(g);
  else if(b==BTN_BACK&&game_event_can_leave(g))do_event(g,game_event_choice_count(g)-1);
  else if(b==BTN_OK)do_event(g,g->selection);
  break;
 case EVENT_CARD:
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,g->deck_count);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)g->selection++;
  else if(b==BTN_BACK){g->screen=EVENT;g->selection=g->shop_selection;msg(g,"");}
  else if(b==BTN_OK)event_card_commit(g);
  break;
 case DECK_VIEW:
  if(g->prior_screen==COMBAT){
    if(b==BTN_LEFT){g->scroll=(g->scroll+3)%4;g->selection=0;}
    else if(b==BTN_RIGHT){g->scroll=(g->scroll+1)%4;g->selection=0;}
    else if(b==BTN_BACK){
      g->screen=COMBAT;
      g->selection=MIN(g->master_index,g->hand_count);
    }else{
      int count=g->scroll==0?g->draw_count:
         g->scroll==1?g->hand_count:
         g->scroll==2?g->discard_count:g->exhaust_count;
      if(b==BTN_UP&&g->selection>=3)g->selection-=3;
      if(b==BTN_DOWN)g->selection=grid_down(g->selection,count);
    }
    break;
  }
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,g->deck_count);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)
    g->selection++;
  else if(b==BTN_BACK){
    if(g->prior_screen==MAP)enter_map(g);
    else if(g->prior_screen==EVENT){g->screen=EVENT;g->selection=CLAMP(g->shop_selection,0,game_event_choice_count(g)-1);}
    else{g->screen=g->prior_screen;g->selection=(g->prior_screen==SHOP||g->prior_screen==REWARD)?g->shop_selection:0;g->deck_readonly=0;}
   }
  else if(b==BTN_OK){
   if((g->prior_screen==REST||g->prior_screen==EVENT)&&g->deck_count>0){
    int idx=g->selection;
    if(idx<0||idx>=g->deck_count)break;
    if(!game_card_is_upgradeable(g->deck[idx])){
      msg(g,"Card cannot be upgraded again.");break;
    }
    g->deck[idx]|=UPBIT;
    if(g->prior_screen==EVENT && --g->next_screen>0 && count_upgradeable(g)>0){
      snprintf(g->message,sizeof(g->message),"Upgraded %s. Choose %d more.",
               cards[CARD_ID(g->deck[idx])].name,g->next_screen);
    }else{
      char result[72];
      snprintf(result,sizeof(result),"Upgraded %s.",cards[CARD_ID(g->deck[idx])].name);
      enter_map(g);msg(g,result);
    }
   }
   else if(g->prior_screen==SHOP&&!g->deck_readonly&&g->deck_count>1){if(g->gold<60){g->screen=SHOP;msg(g,"Not enough gold for removal.");break;}n=g->selection;g->gold-=60;for(int i=n;i<g->deck_count-1;i++)g->deck[i]=g->deck[i+1];g->deck_count--;g->screen=SHOP;g->selection=4;msg(g,"Card removed. 60 gold paid.");}
  }break;
 case BOSS_REWARD:
  if(b==BTN_DOWN)g->selection=3;
  else if(b==BTN_UP&&g->selection==3)g->selection=1;
  else if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(3,g->selection+1);
  else if(b==BTN_OK||b==BTN_BACK){
    int id=(b==BTN_OK&&g->selection<3)?g->reward[g->selection]:-1;
    if(id>=0)add_relic(g,id);
    g->win_screen=(!g->endless&&g->act>=3?2:1);fill_rewards(g);
    g->screen=REWARD;g->selection=0;
  }
  break;
 case RELIC_RESULT:
  if(b==BTN_LEFT||b==BTN_RIGHT)g->selection=1-g->selection;
  else if(b==BTN_OK||b==BTN_BACK){
    if(g->relic_pending&&b==BTN_OK&&g->selection==0&&g->reward[0]>=0)
      add_relic(g,g->reward[0]);
    g->relic_pending=0;
    if(g->win_screen||g->relic_resume==REWARD){
      fill_rewards(g);g->selection=0;g->screen=REWARD;
      msg(g,"Choose a card or skip.");
    }else enter_map(g);
    g->relic_resume=0;
  }
  break;
 case BLESSING:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK){int gift=g->reward[g->selection];if(gift>=1000){int card=gift-1000;if(g->deck_count>=MAX_DECK){msg(g,"Deck full. Choose another blessing.");break;}add_card(g,card);}else add_relic(g,gift);enter_map(g);}
  break;
 case REWARD_PREVIEW:
  if(b==BTN_LEFT)g->selection=(g->selection+2)%3;
  else if(b==BTN_RIGHT)g->selection=(g->selection+1)%3;
  else if(b==BTN_BACK||b==BTN_POTION||b==BTN_OK)g->screen=REWARD;
  break;
 case POTION_VIEW:
  if(b==BTN_UP){g->selection=MAX(0,g->selection-1);msg(g,"");}
  else if(b==BTN_DOWN){g->selection=MIN(MAX(0,g->potion_count-1),g->selection+1);msg(g,"");}
  else if(b==BTN_OK){game_use_potion(g,g->selection);g->selection=MIN(g->selection,MAX(0,g->potion_count-1));}
  else if(b==BTN_BACK||b==BTN_POTION||b==BTN_START){g->screen=g->prior_screen==MAP?MAP:SHOP;g->selection=g->shop_selection;msg(g,"");}
  break;
 case PAUSE_MENU:
   if(b==BTN_UP||b==BTN_DOWN)paused_selection=1-paused_selection;
   else if(b==BTN_BACK||(b==BTN_OK&&paused_selection==1))g->screen=paused_from_screen;
   break;
 case EVENT_RESULT:
   if(b==BTN_OK||b==BTN_BACK)enter_map(g);
   break;
 case RELIC_VIEW:
  if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(MAX(0,g->relic_count-1),g->selection+1);
  else if(b==BTN_BACK||b==BTN_OK)enter_map(g);
  break;
 case VICTORY:
  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=1-CLAMP(g->selection,0,1);
  else if(b==BTN_BACK||(b==BTN_OK&&g->selection==1)){uint32_t seed=game_random(g);game_new(g,seed);}
  else if(b==BTN_OK){
   g->endless=1;g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;g->win_screen=0;g->relic_pending=0;
   map_generate(g);begin_act(g);
  }
  break;
 case DEFEAT:
  if(b==BTN_OK||b==BTN_BACK){if(g->ascent.id&&!ascent_forget(g)){msg(g,"Could not retire this save. Press X to retry.");break;}uint32_t seed=game_random(g);game_new(g,seed);}
  break;
 }
}
static int valid_card_raw(int raw){
 int id=CARD_ID(raw),mod=CARD_MOD(raw);
 if(raw<0||(raw&~CARD_RAW_MASK)||id>=CARD_COUNT||mod>=MOD_COUNT)return 0;
 if(mod&&!game_can_modify(raw&~(CARD_MOD_MASK|UPBIT),mod))return 0;
 return 1;
}
int game_validate(const Game*g){
 if(!g||g->regent.stars<0||g->regent.stars>9||g->dist.last_type< -1||g->dist.last_type>1||g->dist.earned<0||g->dist.earned>3||g->dist.next_guard<0||g->dist.next_guard>999||g->dist.next_draw<0||g->dist.next_draw>MAX_HAND||g->dist.next_energy<0||g->dist.next_energy>99||g->ninja.task_count||g->regent.task_count)return 0;
 for(int i=0;i<DP_COUNT;i++)if(g->dist.power[i]<0||g->dist.power[i]>999||(g->dist.power[i]&&(CARD_ID(g->dist.power_raw[i])>=CARD_COUNT||cards[CARD_ID(g->dist.power_raw[i])].type!=2)))return 0;
 for(int i=0;i<94;i++)if(g->dist.relic_used[i]>1||g->dist.relic_turn[i]>3)return 0;

 if(!g||g->character<0||g->character>2||g->ninja.task_count<0||g->ninja.task_count>NINJA_TASK_MAX||g->endless<0||g->endless>1||g->magic!=0x52494654u||g->version!=GAME_VERSION)return 0;
 if(g->ascent.id){
  if(g->ascent.slot>=4||g->ascent.cards>ASCENT_ALL_CARDS||g->ascent.potions>0xfffffu||g->ascent.relics>1023||g->ascent.secret>1||g->ascent.won>1||g->ascent.ascended>1)return 0;
  for(int i=0;i<2;i++){int a=g->ascent.unlocked_cards[i],b=g->ascent.unlocked_potions[i];if(a!=-1&&(a<334+20*g->character||a>=354+20*g->character))return 0;if(b!=-1&&(b<6||b>=26))return 0;}
  if(g->ascent.unlocked_relic!=-1&&(g->ascent.unlocked_relic<64||g->ascent.unlocked_relic>=74))return 0;
 }
 if(g->revision184.custom<0||g->revision184.custom>1||g->revision184.building<0||g->revision184.building>1||g->revision184.route_style<0||g->revision184.route_style>2||g->revision184.seen_act<0||g->revision184.seen_act>100000)return 0;
 if(g->revision184.building&&!g->revision184.custom)return 0;
 if(custom_setup_screen(g->screen)&&(!g->revision184.building||g->selection<0||g->revision184.tab<0||g->revision184.tab>3))return 0;
 if((g->screen==CUSTOM_CARDS||g->screen==CUSTOM_ITEMS)&&g->selection>=MAX(1,custom_count(g,g->revision184.tab)))return 0;
 if(g->screen==CUSTOM_CONFIRM&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==CUSTOM_QUANTITY){
  if((g->prior_screen!=CUSTOM_CARDS&&g->prior_screen!=CUSTOM_ITEMS)||g->revision184.quantity<1||g->revision184.quantity>MAX_DECK||g->revision184.return_selection<0)return 0;
  if(g->prior_screen==CUSTOM_CARDS){if(!valid_card_raw(g->revision184.item))return 0;}
  else if(g->revision184.item<0||g->revision184.item>=((g->revision184.tab==1||g->revision184.tab==3)?26:RELIC_COUNT))return 0;
 }
 if(g->revision182.rare_offset< -5||g->revision182.rare_offset>40||g->revision182.room_pending<0||g->revision182.room_pending>1||g->revision182.room_type<0||g->revision182.room_type>NODE_BOSS)return 0;
 if(g->revision182.event_seen_hi>((1u<<(EVENT_COUNT-32))-1u))return 0;
 const int next_limits[5]={30,99,5,5,5};for(int i=0;i<5;i++)if(g->revision182.next_combat[i]<0||g->revision182.next_combat[i]>next_limits[i])return 0;
 if(g->act<1||g->act>100000||(!g->endless&&g->act>3)||g->floor<0||g->floor>MAP_ROWS||g->lane<0||g->lane>=3)return 0;
 if(g->maxhp<=0||g->hp<0||g->hp>g->maxhp||g->gold<0)return 0;
 if(g->screen<TITLE||g->screen>ASCENT_FINALE||g->deck_count<(g->revision184.building?0:1)||g->deck_count>MAX_DECK)return 0;
 if(g->screen==ASCENT_FINALE&&(g->act!=4||!g->ascent.ascended||g->selection<0||g->selection>=ASCENT_FINALE_PAGES))return 0;
 if((g->screen==STORY_ACT||g->screen==STORY_ACT_BEFORE_BLESSING)&&(g->selection!=0||g->scroll<0||g->scroll>9))return 0;
 if(g->screen==STORY_BOSS&&(g->act>3||g->selection<0||g->selection>2||g->scroll<0||g->scroll>4))return 0;
 if(g->hand_count<0||g->hand_count>MAX_HAND||g->draw_count<0||g->draw_count>MAX_PILE||g->discard_count<0||g->discard_count>MAX_PILE||g->exhaust_count<0||g->exhaust_count>MAX_PILE)return 0;
 if(g->enemy_count<0||g->enemy_count>MAX_ENEMIES||g->potion_count<0||g->potion_count>MAX_POTIONS)return 0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].art<0||g->enemies[i].art>=ENEMY_COUNT||g->enemies[i].maxhp<1||g->enemies[i].hp>g->enemies[i].maxhp||g->enemies[i].intent<0||g->enemies[i].intent>=INTENT_COUNT)return 0;
 if(g->relic_count<0||g->relic_count>MAX_RELICS)return 0;
 for(int i=0;i<g->deck_count;i++)if(!valid_card_raw(g->deck[i]))return 0;
 for(int i=0;i<g->hand_count;i++)if(!valid_card_raw(g->hand[i]))return 0;
 for(int i=0;i<g->draw_count;i++)if(!valid_card_raw(g->draw[i]))return 0;
 for(int i=0;i<g->discard_count;i++)if(!valid_card_raw(g->discard[i]))return 0;
 for(int i=0;i<g->exhaust_count;i++)if(!valid_card_raw(g->exhaust[i]))return 0;
 for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)<0||game_relic_id(g,i)>=RELIC_COUNT)return 0;
 if(g->expedition.story_seen>0xfffffu||g->expedition.story_pending<0||g->expedition.story_pending>1||g->expedition.story_id<0||g->expedition.story_id>=20)return 0;
 if(expedition_scene(g->screen)&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==EXPEDITION_GIFT&&(g->selection<0||g->selection>2||g->expedition.gift_kind<0||g->expedition.gift_kind>2))return 0;
 if(g->screen==EXPEDITION_GIFT&&g->expedition.gift_kind!=2){int id=g->expedition.gift;if(g->expedition.gift_kind?(id<0||id>=RELIC_COUNT):!valid_card_raw(id))return 0;}
 if(g->screen==MYSTERY_CARDS){if(g->selection<0||g->selection>3)return 0;for(int i=0;i<3;i++)if(!valid_card_raw(g->reward[i])||cards[CARD_ID(g->reward[i])].rarity!=2)return 0;}
 for(int i=0;i<g->potion_count;i++)if(g->potions[i]<0||g->potions[i]>25)return 0;
 for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<MAP_LANES;c++)if(g->map[r][c]<NODE_FIGHT||g->map[r][c]>NODE_BOSS)return 0;
 if(g->screen==CARD_CHOICE&&
 ((g->next_screen==25?g->discard_count:g->hand_count)<1||g->selection<0||
 g->selection>=(g->next_screen==25?g->discard_count:g->hand_count)||
 (g->next_screen!=17&&g->next_screen!=18&&g->next_screen!=23&&g->next_screen!=25&&g->next_screen!=43)))return 0;
 if(g->ninja.nightmare_count<0||g->ninja.nightmare_count>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.nightmare_count;i++)if(!valid_card_raw(g->ninja.nightmare_raw[i])||g->ninja.nightmare_n[i]<1||g->ninja.nightmare_n[i]>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.task_count;i++){const VesperTask*t=&g->ninja.tasks[i];if(t->kind<1||t->kind>10||!valid_card_raw(t->raw)||t->target<0||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->ninja.choice_count;i++)if(i>=MAX_HAND||!valid_card_raw(g->ninja.choice_raws[i]))return 0;
 if(g->screen==CHARACTER_SELECT&&(g->selection<0||g->selection>2))return 0;
 for(int i=0;i<10;i++)if(g->regent.automation_phase[i]<0||g->regent.automation_phase[i]>1000000)return 0;
 for(int i=0;i<5;i++)if(g->regent.panache_phase[i]<0||g->regent.panache_phase[i]>100000000)return 0;
 if(g->regent.option_count<0||g->regent.option_count>MAX_PILE||g->regent.return_count<0||g->regent.return_count>MAX_PILE||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX||g->regent.shuffle_pending<0||g->regent.shuffle_pending>1||g->regent.shuffle_draws<0||g->regent.shuffle_draws>MAX_PILE)return 0;
 for(int i=0;i<g->regent.task_count;i++){const ProxyTask*t=&g->regent.tasks[i];if(t->kind<1||t->kind>10||!valid_card_raw(t->raw)||t->target< -1||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->regent.option_count;i++)if(!valid_card_raw(g->regent.options[i]))return 0;
 for(int i=0;i<g->regent.return_count;i++)if(!valid_card_raw(g->regent.return_cards[i]))return 0;
 for(int i=0;i<512;i++)if(g->regent.instance_cost[i]<0||g->regent.instance_cost[i]>99||g->regent.instance_damage[i]<0||g->regent.instance_damage[i]>1000000||g->regent.instance_replay[i]<0||g->regent.instance_replay[i]>20)return 0;
 for(int i=0;i<EXTRA_LAST-REGENT_FIRST;i++)if(g->regent.powers[i]<0||g->regent.powers[i]>1000000||g->regent.played[i]<0||g->regent.played[i]>1000000)return 0;
 if(g->screen==REGENT_CHOICE&&(g->regent.choice_kind<1||g->regent.choice_kind>15||g->regent.choice_remaining<1||g->selection<0||g->selection>=game_regent_choice_count(g)))return 0;
 if(g->regent.stars<0||g->regent.stars>999||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX)return 0;
 if(g->screen==NINJA_CHOICE&&(g->ninja.choice_kind<1||g->ninja.choice_kind>4||g->ninja.choice_remaining<1||g->ninja.choice_count<0||g->ninja.choice_count>MAX_HAND||g->selection<0||g->selection>=g->hand_count))return 0;
 if(g->screen==ASCENT_COLLECTION&&(g->scroll<0||g->scroll>=5||g->selection<0||g->selection>=game_collection_count(g->scroll)))return 0;
 if(g->screen==MAP&&(g->selection<0||g->selection>=MAP_LANES))return 0;
 if(g->screen==COMBAT&&(g->selection<0||g->selection>g->hand_count||g->target<0||g->target>=MAX(1,g->enemy_count)))return 0;
 if(g->v151.potion_offer< -1||g->v151.potion_offer>25||g->v151.card_done<0||g->v151.card_done>1)return 0;
 if(g->screen==BETA_FORGE&&(g->selection<0||g->selection>2||!game_beta_offer(g)))return 0;
 if(g->screen==BETA_FORGE_CARD&&(g->effect_turn<0||g->effect_turn>2||g->selection<0||g->selection>=g->deck_count||!game_beta_offer(g)))return 0;
 if(g->screen==REWARD&&(g->selection<0||g->selection>5))return 0;
 if(g->screen==BOSS_REWARD&&(g->selection<0||g->selection>3))return 0;
 if((g->screen==BLESSING||g->screen==REWARD_PREVIEW)&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==BLESSING)for(int i=0;i<3;i++){int id=g->reward[i];if(id>=1000){if(id-1000>=CARD_COUNT||!combat_reward_eligible(id-1000))return 0;}else if(id<0||id>=RELIC_COUNT)return 0;}

 if(g->screen==REWARD_PREVIEW)for(int i=0;i<3;i++)if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;
 if(g->screen==RELIC_RESULT&&(g->selection<0||g->selection>1||g->reward[0]<-1||g->reward[0]>=RELIC_COUNT))return 0;
 if(g->screen==REST&&(g->selection<0||g->selection>1))return 0;
 if(g->screen==EVENT&&(g->selection<0||g->selection>=game_event_choice_count(g)||g->effect_turn<0||g->effect_turn>=EVENT_COUNT))return 0;
 if(g->screen==EVENT_RESULT&&g->reward[0]!=-1&&!valid_card_raw(g->reward[0]))return 0;
 if(g->screen==EVENT&&(g->effect_turn==2||g->effect_turn==6||g->effect_turn==34))for(int i=0;i<2;i++)if(!valid_card_raw(g->reward[i])||!game_card_is_playable(g->reward[i]))return 0;
 if(g->screen==EVENT&&g->effect_turn==17&&(!valid_card_raw(g->reward[0])||!game_card_is_playable(g->reward[0])))return 0;
 if(g->screen==EVENT_CARD&&(g->effect_turn<0||g->effect_turn>=EVENT_COUNT||g->next_screen<1||g->next_screen>EVENT_TRANSFORM||g->selection<0||g->selection>=g->deck_count||g->shop_selection<0||g->shop_selection>=event_offer_counts[g->effect_turn]))return 0;
 if(g->screen==SHOP&&(g->selection<0||g->selection>7))return 0;
 if(g->screen==DECK_VIEW){
   int count=g->prior_screen==COMBAT?
      (g->scroll==0?g->draw_count:
       g->scroll==1?g->hand_count:
       g->scroll==2?g->discard_count:g->exhaust_count):g->deck_count;
   if(g->selection<0||g->selection>=MAX(1,count))return 0;
 }
 if(g->screen==REWARD)for(int i=0;i<3;i++)if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;
 if(g->screen==BOSS_REWARD)for(int i=0;i<3;i++)if(g->reward[i]<-1||g->reward[i]>=RELIC_COUNT)return 0;
 return 1;
}
/* Save is a checksummed snapshot: reject truncated/corrupted or incompatible saves. */
static uint32_t hash_bytes(const unsigned char*p,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
int game_save(const Game*g,const char*path){if(!game_validate(g))return 0;FILE*f=fopen(path,"wb");if(!f)return 0;uint32_t check=hash_bytes((const unsigned char*)g,sizeof(*g));int ok=fwrite(&check,4,1,f)==1&&fwrite(g,sizeof(*g),1,f)==1;return fclose(f)==0&&ok;}





int game_load(Game*g,const char*path){
 Game temp;uint32_t check;unsigned char extra;FILE*f=fopen(path,"rb");if(!f)return 0;
 int ok=fread(&check,4,1,f)==1&&fread(&temp,sizeof(temp),1,f)==1&&fread(&extra,1,1,f)==0;
 if(fclose(f))ok=0;
 if(!ok||check!=hash_bytes((const unsigned char*)&temp,sizeof(temp))||!game_validate(&temp))return 0;
 repair_generated_offers(&temp);*g=temp;return 1;
}

/* Persistent, versioned lifetime profile. One result per unique run seed. */
void profile_init(RiftProfile*p){
 memset(p,0,sizeof(*p));p->magic=0x52415046u;p->version=1;
}
static int profile_valid(const RiftProfile*p){
 return p&&p->magic==0x52415046u&&p->version==1&&
  p->wins<=p->runs&&p->losses<=p->runs&&p->wins+p->losses==p->runs;
}
int profile_record(RiftProfile*p,const Game*g){
 if(!profile_valid(p)||!g||g->revision184.custom||(g->screen!=VICTORY&&g->screen!=DEFEAT))return 0;
 if(p->runs&&p->last_seed==g->seed)return 0;
 p->runs++;
 if(g->screen==VICTORY)p->wins++;else p->losses++;
 unsigned distance=g->screen==VICTORY?3u*(MAP_ROWS+1):(unsigned)MIN(4294967295ULL,(unsigned long long)(g->act-1)*(MAP_ROWS+1)+g->floor);
 if(distance>p->furthest_floor)p->furthest_floor=distance;
 if(g->kill_count>0)p->total_kills+=(uint32_t)g->kill_count;
 if(g->kill_count>(int)p->best_kills)p->best_kills=(uint32_t)g->kill_count;
 if(g->deck_count>(int)p->best_deck)p->best_deck=(uint32_t)g->deck_count;
 p->last_seed=g->seed;p->last_result=(uint32_t)g->screen;
 return 1;
}
int profile_save(const RiftProfile*p,const char*path){
 if(!profile_valid(p))return 0;
 FILE*f=fopen(path,"wb");if(!f)return 0;
 uint32_t checksum=hash_bytes((const unsigned char*)p,sizeof(*p));
 int ok=fwrite(&checksum,sizeof(checksum),1,f)==1&&fwrite(p,sizeof(*p),1,f)==1;
 return fclose(f)==0&&ok;
}
int profile_load(RiftProfile*p,const char*path){
 RiftProfile stored;uint32_t checksum;
 FILE*f=fopen(path,"rb");if(!f)return 0;
 int ok=fread(&checksum,sizeof(checksum),1,f)==1&&fread(&stored,sizeof(stored),1,f)==1;
 if(fclose(f)!=0)ok=0;
 if(!ok||checksum!=hash_bytes((const unsigned char*)&stored,sizeof(stored))||!profile_valid(&stored))return 0;
 *p=stored;return 1;
}
/* Temporary status and persistent Powers are separate; each Power appears once. */
static int status_entries(const Game*g,int wanted,char*out,int cap){
 int n=0;
#define DS(cond,...) do{if(cond){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,__VA_ARGS__);n++;}}while(0)
 DS(1,"Echo: %d / 9. Persists between turns.",g->regent.stars);
 DS(1,"%s",g->character==CHAR_WARDEN?"Warden: guarded Skills under attack earn 1 Echo (max 2/turn).":g->character==CHAR_VESPER?"Vesper: switching Attack / Skill earns 1 Echo (max 3/turn).":"Proxy: turn start gains 1 Echo + up to 2 unused Energy.");
 DS(g->strength,"Force: %+d damage per hit",g->strength);
 DS(g->dexterity,"Poise: %+d Guard per card effect",g->dexterity);
 DS(g->weak,"Falter: %d turns; deal 25%% less damage",g->weak);
 DS(g->vulnerable,"Fracture: %d turns; take 50%% more damage",g->vulnerable);
 DS(g->poison,"Wither: lose %d HP at turn end",g->poison);
 DS(g->dist.next_guard,"Next turn: %d Guard",g->dist.next_guard);
 DS(g->dist.next_draw,"Next turn: draw %d more",g->dist.next_draw);
 DS(g->dist.next_energy,"Next turn: %d extra Energy",g->dist.next_energy);
 DS(g->ic[IC_TEMP_STRENGTH],"Temporary Force: %d expires this turn",g->ic[IC_TEMP_STRENGTH]);
 for(int i=0;i<DP_COUNT;i++)if(g->dist.power[i]){int raw=g->dist.power_raw[i];DS(1,"%s: active (%d total)",cards[CARD_ID(raw)].name,g->dist.power[i]);}
#undef DS
 return n;
}
int game_status_count(const Game*g){return status_entries(g,-1,0,0);}
void game_status_line(const Game*g,int index,char*out,int cap){
 if(out&&cap>0)out[0]=0;status_entries(g,index,out,cap);
}

#include "card_preview.inc"

#include "ascent_storage.inc"
#include "loop_story.inc"
#include "custom_run.inc"
#include "ascent_flow.inc"
#include "expedition_flow.inc"
