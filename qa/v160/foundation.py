from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'game.h';s=p.read_text().replace('#define GAME_VERSION 22','#define GAME_VERSION 23')
s=s.replace('#define CARD_FREE 0x200','#define CARD_FREE 0x400').replace('#define CARD_FORCE_EXHAUST 0x400','#define CARD_FORCE_EXHAUST 0x800').replace('#define CARD_EPHEMERAL 0x800','#define CARD_EPHEMERAL 0x1000').replace('CARD_MOD_SHIFT 12','CARD_MOD_SHIFT 13').replace('UPBIT 0x100','UPBIT 0x200').replace('((x)&0xff)','((x)&0x1ff)')
for name,old in [('CARD_SLY',15),('CARD_SLY_TURN',16),('CARD_INKY',17),('CARD_RETAIN_TURN',18)]:s=s.replace(f'{name} (1<<{old})',f'{name} (1<<{old+1})')
s=s.replace('CARD_COUNTER_SHIFT 19','CARD_COUNTER_SHIFT 20').replace('CARD_INSTANCE_SHIFT 21','CARD_INSTANCE_SHIFT 22').replace('(1023<<CARD_INSTANCE_SHIFT)','(511<<CARD_INSTANCE_SHIFT)').replace('&1023)','&511)')
s=s.replace('CHAR_WARDEN,CHAR_NIGHTSHADE','CHAR_WARDEN,CHAR_NIGHTSHADE,CHAR_REGENT').replace('CHARACTER_SELECT,NINJA_CHOICE','CHARACTER_SELECT,NINJA_CHOICE,REGENT_CHOICE')
state='''
#include "regent_ids.h"
#define REGENT_TASK_MAX 128
#define REGENT_INSTANCE_MAX 512
typedef struct {int kind,raw,target,n,aux,depth;} RegentTask;
typedef struct {
 int stars,forge,blade_created,created,stars_gained,skills,plays_combat;
 int powers[EXTRA_LAST-REGENT_FIRST],played[EXTRA_LAST-REGENT_FIRST];
 int hit_count[MAX_ENEMIES],conqueror[MAX_ENEMIES];
 int vigor,attack_vigor,monologue,next_energy,next_stars,next_draw,next_block,next_seek;
 int retain_all,orbit_spent,draws,nostalgia_used,void_used,panic,reflect,gambit;
 int instance_damage[REGENT_INSTANCE_MAX],instance_cost[REGENT_INSTANCE_MAX],instance_replay[REGENT_INSTANCE_MAX];
 int return_cards[MAX_PILE],return_count;
 int bomb_ticks[16],bomb_damage[16];
 int choice_kind,choice_remaining,choice_raw,choice_target,choice_aux,choice_depth;
 int options[MAX_PILE],option_count;
 int task_count,resolving;RegentTask tasks[REGENT_TASK_MAX];
} RegentState;
'''
s=s.replace('typedef struct {char name[24];',state+'\ntypedef struct {char name[24];')
s=s.replace('struct {int potion_offer,card_done;int relic[16];} v151;','struct {int potion_offer,card_done;int relic[16];} v151;\n RegentState regent;')
s=s.replace('int game_card_cost_now(','''int game_card_star_cost(const Game*g,int raw);
int game_regent_choice_count(const Game*g);
int game_regent_choice_raw(const Game*g,int index);
int game_regent_choice_eligible(const Game*g,int index);
const char *game_regent_choice_title(const Game*g);
void game_set_resume_available(int available);
int game_resume_available(void);
int game_card_cost_now(''')
p.write_text(s)
# The 9-bit card ID needs one additional bit. Instance tags are now 9-bit too;
# at most 394 physical combat pile entries can coexist, so no legal pile loses IDs.
for p in q.iterdir():
 if p.suffix not in ('.c','.inc'):continue
 s=p.read_text()
 if p.name not in ('save_legacy.inc','save_v20.inc'):
  s=s.replace('id>=NINJA_FIRST)', 'id>=NINJA_FIRST&&id<NINJA_LAST)').replace('id>=NINJA_FIRST){','id>=NINJA_FIRST&&id<NINJA_LAST){')
  s=s.replace('id>=NINJA_FIRST)return','id>=NINJA_FIRST&&id<NINJA_LAST)return')
  s=s.replace('&~255','&~511')
 if p.name=='ninja_engine.inc':
  s=s.replace('i<1023','i<511').replace('%1023','%511')
  s=s.replace('if(CARD_ID(raw)<NINJA_FIRST||CARD_INSTANCE(raw))','if(CARD_ID(raw)<NINJA_FIRST||CARD_ID(raw)>=NINJA_LAST||CARD_INSTANCE(raw))')
  s=s.replace('return id<NINJA_FIRST?g->character==CHAR_WARDEN:g->character==CHAR_NIGHTSHADE;','return id< NINJA_FIRST?g->character==CHAR_WARDEN:id<NINJA_LAST?g->character==CHAR_NIGHTSHADE:id<REGENT_LAST?g->character==CHAR_REGENT:0;')
  s=s.replace('return CARD_MOD(raw)==MOD_BOUND', 'return (regent_flags(raw)&2)||g->regent.retain_all||CARD_MOD(raw)==MOD_BOUND')
 p.write_text(s)
p=q/'game.c';s=p.read_text().replace('#include "ninja_cards.inc"','#include "ninja_cards.inc"\n#include "regent_cards.inc"').replace('#include "ninja_upgrades.inc"','#include "ninja_upgrades.inc"\n#include "regent_upgrades.inc"').replace('#include "ninja_meta.inc"','#include "ninja_meta.inc"\n#include "regent_meta.inc"')
s=s.replace('if(id>=NINJA_FIRST&&id<NINJA_LAST)return CARD_UP(raw)?', 'if(id>=REGENT_FIRST)return CARD_UP(raw)?regent_cards[id-REGENT_FIRST].upcost:cards[id].cost;\n if(id>=NINJA_FIRST&&id<NINJA_LAST)return CARD_UP(raw)?',1)
protos='''
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
'''
s=s.replace('static int ninja_flags(int raw);',protos+'\nstatic int ninja_flags(int raw);')
s=s.replace('int pool[256],weight[256]','int pool[512],weight[512]')
s=s.replace('g->character>1','g->character>2').replace('g->screen>NINJA_CHOICE','g->screen>REGENT_CHOICE').replace('g->selection>1))return 0;\n if(g->screen==NINJA_CHOICE','g->selection>2))return 0;\n if(g->screen==NINJA_CHOICE')
s=s.replace('raw=ninja_stamp(g,raw);\n if(g->hand_count','raw=regent_stamp(g,ninja_stamp(g,raw));\n if(g->hand_count')
s=s.replace('g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);\n}', 'g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);\n regent_created(g);\n}',1)
s=s.replace('ninja_on_draw(g,drawn);','ninja_on_draw(g,drawn);regent_on_draw(g,drawn);')
s=s.replace('g->draw[i]=ninja_stamp(g,g->draw[i])','g->draw[i]=regent_stamp(g,ninja_stamp(g,g->draw[i]))')
s=s.replace('if(g->character==CHAR_NIGHTSHADE){int ordered', 'if(g->character==CHAR_REGENT){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(regent_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(regent_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}\n if(g->character==CHAR_NIGHTSHADE){int ordered')
s=s.replace('ninja_on_hit(g,idx,amt);','ninja_on_hit(g,idx,amt);regent_on_hit(g,idx,amt);')
s=s.replace('amt=MAX(0,amt+g->strength+', 'amt=MAX(0,amt+(r140_attack_context?g->regent.attack_vigor:0)+g->strength+')
s=s.replace('return amt;\n}\nstatic void damage_enemy', 'if(r140_attack_context&&CARD_ID(r150_card_raw)==R_SOVEREIGN_BLADE&&g->regent.conqueror[idx])amt=(int)MIN(1000000000LL,(long long)amt*2);\n return amt;\n}\nstatic void damage_enemy')
s=s.replace('static void card_block(Game*g,int n){gain_block', 'static void card_block(Game*g,int n){if(g->regent.panic)return;gain_block')
s=s.replace('if(g->screen!=COMBAT||g->ninja.task_count>0)return;', 'if(g->screen!=COMBAT||g->ninja.task_count>0||g->regent.task_count||regent_processing)return;')
s=s.replace('g->kill_count+=g->enemy_count;', 'g->gold+=g->regent.powers[R_ROYALTIES-REGENT_FIRST];g->kill_count+=g->enemy_count;')
s=s.replace('memset(&g->ninja,0,sizeof(g->ninja));','memset(&g->ninja,0,sizeof(g->ninja));memset(&g->regent,0,sizeof(g->regent));if(g->character==CHAR_REGENT)g->regent.stars=3;')
s=s.replace('ninja_start_turn_before(g);','ninja_start_turn_before(g);regent_before_turn(g);')
s=s.replace('ninja_start_turn_after(g);','ninja_start_turn_after(g);regent_start_turn(g);')
s=s.replace('if(ninja_prepare_retain(g))return;', 'if(ninja_prepare_retain(g))return;\n regent_end_turn(g);if(g->screen!=COMBAT)return;')
s=s.replace('if(game_card_is_retained(g,g->hand[i]))g->hand[retained++]', 'if(regent_flags(g->hand[i])&8){exhaust_card(g,g->hand[i]);continue;}\n  if(game_card_is_retained(g,g->hand[i]))g->hand[retained++]')
s=s.replace('g->hand_count=retained;','g->hand_count=retained;g->regent.retain_all=0;g->regent.monologue=0;')
s=s.replace('if(g&&id>=NINJA_FIRST&&id<NINJA_LAST){','if(g&&id>=REGENT_FIRST)return regent_cost_now(g,raw);\n if(g&&id>=NINJA_FIRST&&id<NINJA_LAST){',1)
s=s.replace('#include "relics151.inc"','#include "relics151.inc"\n#include "regent_engine.inc"')
s=s.replace('if(id>=NINJA_FIRST&&id<NINJA_LAST)return ninja_base_damage(g,raw,1);','if(id>=REGENT_FIRST)return regent_base_damage(g,raw);\n if(id>=NINJA_FIRST&&id<NINJA_LAST)return ninja_base_damage(g,raw,1);')
s=s.replace('if(id>=NINJA_FIRST&&id<NINJA_LAST)return g->ninja.powers', 'if(id>=REGENT_FIRST)return g->regent.played[id-REGENT_FIRST];\n if(id>=NINJA_FIRST&&id<NINJA_LAST)return g->ninja.powers')
s=s.replace('if(id>=NINJA_FIRST&&id<NINJA_LAST){\n const char*rule=', 'if(id>=REGENT_FIRST){snprintf(out,(size_t)cap,"%s: %d played",cards[id].name,g->regent.played[id-REGENT_FIRST]);return;}\n if(id>=NINJA_FIRST&&id<NINJA_LAST){\n const char*rule=')
p.write_text(s)
p=q/'card_mods.inc';s=p.read_text().replace('case MOD_WARD:if(id>=NINJA_FIRST','case MOD_WARD:if(id>=REGENT_FIRST)return regent_cards[id-REGENT_FIRST].block>0; if(id>=NINJA_FIRST');p.write_text(s)
