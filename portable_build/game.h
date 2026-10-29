#ifndef RIFT_ASCENT_GAME_H
#define RIFT_ASCENT_GAME_H
#include <stdint.h>
#include <stddef.h>
#define W 480
#define H 272
#define MAX_DECK 128
#define MAX_HAND 10
#define MAX_PILE 128
#define MAX_ENEMIES 2
#define ENEMY_COUNT 47
#define INTENT_COUNT 24
#define MAX_RELICS 96
#define LEGACY_RELIC_CAP 64
#define MAX_POTIONS 3
#define MAP_ROWS 18
#define MAP_LANES 3
#define GAME_VERSION 182
#define CARD_STATUS_MIN 50
#define CARD_STATUS_MAX 53
#define CARD_PLAYABLE 87
#define CARD_GIANT_ROCK 91
#define CARD_FREE 0x400
#define CARD_FORCE_EXHAUST 0x800
#define CARD_EPHEMERAL 0x1000
#define CARD_MOD_SHIFT 13
#define CARD_MOD_MASK (7 << CARD_MOD_SHIFT)
#define CARD_MOD(raw) (((raw) >> CARD_MOD_SHIFT) & 7)
enum {MOD_NONE,MOD_ECHO,MOD_BOUND,MOD_KEEN,MOD_WARD,MOD_FLEETING,MOD_KINDLED,MOD_COUNT};
enum {IC_HOURGLASS=39,IC_BELL,IC_NEEDLE,IC_DUELIST,IC_PENDING_CARD=43,IC_COVENANT=44};
enum {EVENT_UPGRADE=7,EVENT_REMOVE,EVENT_COPY};
#define EVENT_COUNT 24
enum {R162_EVENT_SEEN=12,R162_EVENT_LAST=13};

enum {TITLE,MAP,COMBAT,REWARD,REST,SHOP,EVENT,DECK_VIEW,VICTORY,DEFEAT,HELP,BOSS_REWARD,RELIC_VIEW,PAUSE_MENU,EVENT_RESULT,RELIC_RESULT,CARD_CHOICE,POTION_VIEW,BLESSING,REWARD_PREVIEW,ACT_INTRO,EVENT_CARD,CHARACTER_SELECT,NINJA_CHOICE,REGENT_CHOICE,NEW_RUN_CONFIRM,STORY_ACT,STORY_BOSS,STORY_ACT_BEFORE_BLESSING,ASCENT_SLOTS_VIEW,ASCENT_OVERWRITE,ASCENT_COLLECTION,ASCENT_OPTIONS,ASCENT_ENDING,ASCENT_CARDS,ASCENT_POTIONS,ASCENT_RELIC,ASCENT_REVELATION,ASCENT_BOSS,ASCENT_TRUE_END,ASCENT_SAVE_ERROR,EXPEDITION_STORY,EXPEDITION_GIFT,ABOMINATION_BEFORE,ABOMINATION_AFTER,MYSTERY_STORY,MYSTERY_CARDS};
enum {NODE_FIGHT,NODE_ELITE,NODE_EVENT,NODE_SHOP,NODE_REST,NODE_TREASURE,NODE_BOSS};
enum {
 IC_AGGRESSION,IC_BARRICADE,IC_COLOSSUS,IC_CORRUPTION,
 IC_CRIMSON,IC_CRIMSON_COPIES,IC_CRUELTY,IC_DARK_EMBRACE,
 IC_ENERGY_LOCK,IC_HELLRAISER,IC_JUGGERNAUT,IC_JUGGLING,
 IC_MANGLE0,IC_MANGLE1,IC_PYRE,IC_RAMPAGE,IC_TEMP_STRENGTH,
 IC_HPLOSS_TURN,IC_HPLOSS_COMBAT,IC_STAMPEDE,IC_PLATING,
 IC_TANK,IC_UNMOVABLE,IC_UNMOVABLE_USED,IC_UNRELENTING,
 IC_VICIOUS,IC_NEXT_ATTACK_REPEAT,IC_CHAIN_DEPTH,IC_PREVENT_EXHAUST,
 IC_ATTACKS_THIS_TURN,IC_OWN_TURN,IC_GUARD,IC_CASCADE_QUEUE
};
enum {BTN_UP=1,BTN_DOWN=2,BTN_LEFT=4,BTN_RIGHT=8,BTN_OK=16,BTN_BACK=32,BTN_ALT=64,BTN_POTION=128,BTN_START=256,BTN_PAUSE=512,BTN_TAB_LEFT=4096,BTN_TAB_RIGHT=8192,BTN_DEBUG_KILL=16384,BTN_DEBUG_ACT=32768};
#define UPBIT 0x200
#define CARD_ID(x) ((x)&0x1ff)
#define CARD_UP(x) (((x)&UPBIT)!=0)


#define CARD_SLY (1<<16)
#define CARD_SLY_TURN (1<<17)
#define CARD_INKY (1<<18)
#define CARD_RETAIN_TURN (1<<19)
#define CARD_COUNTER_SHIFT 20
#define CARD_COUNTER_MASK (3<<CARD_COUNTER_SHIFT)
#define CARD_COUNTER(raw) (((raw)>>CARD_COUNTER_SHIFT)&3)
#define CARD_INSTANCE_SHIFT 22
#define CARD_INSTANCE_MASK (511<<CARD_INSTANCE_SHIFT)
#define CARD_INSTANCE(raw) (((raw)>>CARD_INSTANCE_SHIFT)&511)
#define CARD_RAW_MASK 0x7fffffff
#define NINJA_FIRST 96
#define NINJA_COUNT 89
#define NINJA_LAST (NINJA_FIRST+NINJA_COUNT)
enum {CHAR_WARDEN,CHAR_NIGHTSHADE,CHAR_REGENT};
enum {NR_SNAKE=44,NR_SKULL,NR_TINGSHA,NR_BANDAGES,NR_FUNNEL,NR_SCROLL,NR_KRANE,NR_DART,NR_DRAKE};
enum {NS_ACCURACY,NS_AFTERIMAGE,NS_THORNS,NS_ACCELERANT,NS_ENVENOM,NS_FAN,NS_PHANTOM,NS_BLADES,NS_FUMES,NS_OUTBREAK,NS_OUTBREAK_COUNT,NS_PLANNER,NS_SERPENT,NS_SPEEDSTER,NS_TOOLS,NS_PLANS,NS_TRACKING,NS_WRAITH,NS_INTANGIBLE,NS_TEMP_DEX,NS_BLUR,NS_NEXT_BLOCK,NS_NEXT_DRAW,NS_SHADOW_NEXT,NS_SHADOW_NOW,NS_SHADOWMELD,NS_BURST,NS_POUNCE,NS_CORROSIVE,NS_DRAWS_COMBAT,NS_DISCARDS_TURN,NS_SKILLS_TURN,NS_SHIVS_TURN,NS_STRANGLE0,NS_STRANGLE1,NS_BONUS_REWARDS,NS_SNEAKY,NS_RETAIN_READY,NS_OPENING_DRAW,NS_LAST_DRAW,NS_RESOLVING,NS_COUNT};
#define NINJA_TASK_MAX 512
typedef struct {int kind,raw,target,n,aux,depth;} NinjaTask;
typedef struct {
 int v[NS_COUNT],powers[NINJA_COUNT];int next_instance;
 NinjaTask tasks[NINJA_TASK_MAX];int task_count;
 int choice_kind,choice_remaining,choice_count,choice_raws[MAX_HAND],choice_source,choice_target,choice_depth;
 int nightmare_raw[MAX_HAND],nightmare_n[MAX_HAND],nightmare_count;
} NinjaState;
enum {
 N_ABRASIVE=96,
 N_ACCELERANT=97,
 N_ACCURACY=98,
 N_ACROBATICS=99,
 N_ADRENALINE=100,
 N_AFTERIMAGE=101,
 N_ANTICIPATE=102,
 N_ASSASSINATE=103,
 N_BACKFLIP=104,
 N_BACKSTAB=105,
 N_BLADE_DANCE=106,
 N_BLADE_OF_INK=107,
 N_BLUR=108,
 N_BOUNCING_FLASK=109,
 N_BUBBLE_BUBBLE=110,
 N_BULLET_TIME=111,
 N_BURST=112,
 N_CALCULATED_GAMBLE=113,
 N_CLOAK_AND_DAGGER=114,
 N_CORROSIVE_WAVE=115,
 N_DAGGER_SPRAY=116,
 N_DAGGER_THROW=117,
 N_DASH=118,
 N_DEADLY_POISON=119,
 N_DEFEND_SILENT=120,
 N_DEFLECT=121,
 N_DODGE_AND_ROLL=122,
 N_ECHOING_SLASH=123,
 N_ENVENOM=124,
 N_ESCAPE_PLAN=125,
 N_EXPERTISE=126,
 N_EXPOSE=127,
 N_FAN_OF_KNIVES=128,
 N_FINISHER=129,
 N_FLANKING=130,
 N_FLECHETTES=131,
 N_FLICK_FLACK=132,
 N_FOOTWORK=133,
 N_GRAND_FINALE=134,
 N_HAND_TRICK=135,
 N_HAZE=136,
 N_HIDDEN_DAGGERS=137,
 N_INFINITE_BLADES=138,
 N_KNIFE_TRAP=139,
 N_LEADING_STRIKE=140,
 N_LEG_SWEEP=141,
 N_MALAISE=142,
 N_MASTER_PLANNER=143,
 N_MEMENTO_MORI=144,
 N_MIRAGE=145,
 N_MURDER=146,
 N_NEUTRALIZE=147,
 N_NIGHTMARE=148,
 N_NOXIOUS_FUMES=149,
 N_OUTBREAK=150,
 N_PHANTOM_BLADES=151,
 N_PIERCING_WAIL=152,
 N_PINPOINT=153,
 N_POISONED_STAB=154,
 N_POUNCE=155,
 N_PRECISE_CUT=156,
 N_PREDATOR=157,
 N_PREPARED=158,
 N_REFLEX=159,
 N_RICOCHET=160,
 N_SCARE=161,
 N_SERPENT_FORM=162,
 N_SHADOW_STEP=163,
 N_SHADOWMELD=164,
 N_SHIV=165,
 N_SKEWER=166,
 N_SLICE=167,
 N_SNAKEBITE=168,
 N_SNEAKY=169,
 N_SPEEDSTER=170,
 N_STORM_OF_STEEL=171,
 N_STRANGLE=172,
 N_STRIKE_SILENT=173,
 N_SUCKER_PUNCH=174,
 N_SUPPRESS=175,
 N_SURVIVOR=176,
 N_TACTICIAN=177,
 N_THE_HUNT=178,
 N_TOOLS_OF_THE_TRADE=179,
 N_TRACKING=180,
 N_UNTOUCHABLE=181,
 N_UP_MY_SLEEVE=182,
 N_WELL_LAID_PLANS=183,
 N_WRAITH_FORM=184
};


#include "regent_ids.h"
#define REGENT_TASK_MAX 128
#define REGENT_INSTANCE_MAX 512
typedef struct {int kind,raw,target,n,aux,depth;} RegentTask;
typedef struct {
 int stars,forge,blade_created,created,stars_gained,skills,plays_combat;
 int shuffle_pending,shuffle_draws;
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
 int automation_phase[10],panache_phase[5];
} RegentState;

#include "ascent_progress.h"
#include "expedition.h"

typedef struct {char name[24];int hp,maxhp,block,strength,weak,vulnerable,poison;int intent,intent_value,phase;int art;} Enemy;
typedef struct {
 uint32_t magic,version,seed,rng;
 int screen,prior_screen,act,floor,lane,selection,target,scroll;
 int hp,maxhp,block,energy,max_energy,gold,turn,energy_penalty,strength,dexterity,weak,vulnerable,poison;
 int potion_count,potions[MAX_POTIONS],relic_count,relics[LEGACY_RELIC_CAP];
 int deck_count,deck[MAX_DECK],master_index;
 int draw_count,draw[MAX_PILE],discard_count,discard[MAX_PILE],exhaust_count,exhaust[MAX_PILE],hand_count,hand[MAX_HAND];
 int map[MAP_ROWS][MAP_LANES],map_selected[MAP_ROWS],enemy_count,reward[3],reward_gold;
 Enemy enemies[MAX_ENEMIES];
 int next_screen,battle_type,kill_count,defense_event,shop_cards[3],shop_relic;
 int reward_potion,win_screen,effect_turn,turn_played,used_potion,heal_after_battle;
 int permanent_strength,low_hp_triggered;
 char message[72], subtitle[72];
 int ic[48];
 int relic_pending,relic_resume,deck_readonly,shop_selection;
 int endless,combat_music;
 int character;NinjaState ninja;
 /* Appended to preserve the complete v21 prefix for migration. */
 struct {int potion_offer,card_done;int relic[16];} v151;
 RegentState regent;
 AscentRun ascent;
 Expedition181 expedition;
 /* Append-only 1.8.2 state: leave every older save offset intact. */
 struct {int rare_offset,room_pending,room_type;} revision182;
} Game;

/* Persistent lifetime achievements are stored separately from the run slot. */
typedef struct {
 uint32_t magic, version, runs, wins, losses;
 uint32_t furthest_floor, total_kills, best_kills, best_deck;
 uint32_t last_seed, last_result;
} RiftProfile;
void profile_init(RiftProfile *profile);
int profile_record(RiftProfile *profile,const Game *finished_run);
int profile_save(const RiftProfile *profile,const char *path);
int profile_load(RiftProfile *profile,const char *path);
void game_render_set_profile(const RiftProfile *profile);
typedef struct {const char *name;const char *desc;int cost,rarity,type;} CardDef;
typedef struct {const char *name;const char *desc;} RelicDef;
extern const CardDef cards[];
extern const int CARD_COUNT;
extern const RelicDef relic_defs[];
extern const int RELIC_COUNT;
int game_relic_id(const Game*,int);
int expedition_scene(int screen);
const char*expedition_title(const Game*);
const char*expedition_line(const Game*);
int expedition_art(const Game*);
const char *node_name(int type);
const char *game_enemy_name(int id);
const char *card_description(int raw);
const char *game_mod_name(int mod);
const char *game_mod_description(int mod);
int game_can_modify(int raw,int mod);
const char *game_event_title(int id);
const char *game_event_lore(int id);
void game_event_option(const Game *g,int option,char *title,int title_cap,char *detail,int detail_cap);
int game_event_choice_count(const Game *g);
int game_event_choice_available(const Game *g,int option);
int game_event_card_eligible(const Game *g,int raw);
void game_begin_event(Game *g,int id);
int game_next_event(Game*g);
int game_card_is_playable(int raw);
int game_card_is_upgradeable(int raw);
int game_card_cost(int raw);
int game_card_star_cost(const Game*g,int raw);
int game_regent_choice_count(const Game*g);
int game_regent_choice_raw(const Game*g,int index);
int game_regent_choice_eligible(const Game*g,int index);
const char *game_regent_choice_title(const Game*g);
void game_set_resume_available(int available);
int game_resume_available(void);
int game_card_cost_now(const Game *g,int raw);
void game_new(Game *g,uint32_t seed);
void game_choose_character(Game *g,int character);
const char *game_character_name(const Game *g);
int game_card_is_sly(const Game *g,int raw);
int game_card_is_retained(const Game *g,int raw);
int game_ninja_choice_eligible(const Game *g,int index);
const char *game_ninja_choice_title(const Game *g);
int game_moon_heal_now(const Game *g);
void game_action(Game *g,int button);
int game_pause_source(void);
int game_pause_selection(void);
void game_update(Game *g);
void game_story_start(Game*g,int boss);
int game_story_kind(const Game*g);
const char*game_story_text(const Game*g);
const char*game_story_speaker(const Game*g);
int game_render_transition_silent(const Game*g,unsigned tick);
const Game*game_render_audio_state(const Game*g,unsigned tick);
int game_render_story_input(const Game*g,int key,unsigned tick);
void game_render(const Game *g,uint32_t *pixels,int stride);
void game_render_reset_fx(void);
void game_render_frame(const Game *g,uint32_t *pixels,int stride,unsigned tick);
/* Cosmetic battle effects: attack=1, defend=2, potion=3, hit=4. */
void game_effect_start(int kind,unsigned tick);
int game_save(const Game *g,const char *path);
int game_load(Game *g,const char *path);
int game_validate(const Game *g);
void game_start_battle(Game *g,int mode);
void game_start_turn(Game *g);
void game_end_turn(Game *g);
void game_prepare_end_turn(Game *g);
int game_music_track(const Game *g);
int game_music_silent(const Game *g);
void game_play_card(Game *g,int index,int enemy);
void game_choose_node(Game *g,int lane);
int game_enemy_intent_damage(const Game *g,int index);
int game_enemy_intent_hits(const Game *g,int index);
void game_enemy_intent_text(const Game*g,int index,char*out,int cap);

unsigned game_route_row_mask(const Game *g,int row);
unsigned game_route_visible_mask(const Game *g,int row);
int game_status_count(const Game *g);
void game_status_line(const Game *g,int index,char *out,int capacity);
unsigned game_route_mask(const Game *g,int row,int lane);
int game_route_available(const Game *g,int lane);
const char *game_intent_description(int intent);
const char *game_intent_title(int intent);
const char *game_potion_name(int id);
const char *game_potion_description(int id);
const char *game_potion_description_now(const Game *g,int id);
int game_effective_damage(const Game *g,int raw,int target);
/* Current per-hit damage is before enemy Block. Previewing never advances RNG
   or changes the live run. Ranges represent unresolved random outcomes. */
typedef struct {
 int low[2],high[2],hits[2],total[2];
 int block_low,block_high,block_first,block_second,block_count,block_total;
 int repeated,repeat_hits,uncertain,random;
} CardPreview;
void game_card_preview(const Game *g,int raw,int target,CardPreview *out);
void game_card_description_now(const Game *g,int raw,int target,char *out,int cap);
int game_enemy_status_count(const Game *g,int enemy);
void game_enemy_status_line(const Game *g,int enemy,int index,char *out,int cap);
void game_use_potion(Game*g,int index);
int game_inventory_available(const Game*g);
int game_inventory_action(Game*,int,int*,int*,int*,int*);
void game_discard_potion(Game*g,int index);
/* Transient presentation events. Never serialized or emitted by previews. */
enum {GFX_ATTACK=1,GFX_BLOCK,GFX_POWER,GFX_PROC,GFX_SELF,GFX_STATUS,GFX_STARS};
#define MAX_COMBAT_FX 64
typedef struct {int kind,source,target,amount,absorbed,card;} GameCombatFx;
void game_combat_fx_begin(void);
void game_combat_fx_end(void);
const GameCombatFx *game_combat_fx_events(int *count);
int game_impact_tier(int damage);
void game_render_update_fx(unsigned tick);
int game_render_fx_busy(unsigned tick);
int game_render_music_paused(unsigned tick);
void game_render_begin_transition(const Game *before,unsigned tick);
int game_render_transition_busy(unsigned tick);


uint32_t game_random(Game *g);
const AscentAccount*ascent_profile(void);
const AscentSlot*ascent_slot(int slot);
int ascent_popcount(unsigned mask);
int ascent_secret(int character);
int ascent_open(const char*root);
int ascent_begin(Game*g,int slot,int character,unsigned seed);
int ascent_resume(Game*g,int slot);
int ascent_checkpoint(Game*g);
int ascent_card_available(const Game*g,int id);
int ascent_debug(void);
int ascent_set_debug(int enabled);
int ascent_story_screen(int screen);
#endif
