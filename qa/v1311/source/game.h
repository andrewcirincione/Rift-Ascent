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
#define ENEMY_COUNT 36
#define MAX_RELICS 40
#define MAX_POTIONS 3
#define MAP_ROWS 14
#define MAP_LANES 3
#define GAME_VERSION 19
#define CARD_STATUS_MIN 50
#define CARD_STATUS_MAX 53
#define CARD_PLAYABLE 87
#define CARD_GIANT_ROCK 91
#define CARD_FREE 0x100
#define CARD_FORCE_EXHAUST 0x200
#define CARD_EPHEMERAL 0x400

enum {TITLE,MAP,COMBAT,REWARD,REST,SHOP,EVENT,DECK_VIEW,VICTORY,DEFEAT,HELP,BOSS_REWARD,RELIC_VIEW,PAUSE_MENU,EVENT_RESULT,RELIC_RESULT,CARD_CHOICE,POTION_VIEW,BLESSING,REWARD_PREVIEW,ACT_INTRO};
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
enum {BTN_UP=1,BTN_DOWN=2,BTN_LEFT=4,BTN_RIGHT=8,BTN_OK=16,BTN_BACK=32,BTN_ALT=64,BTN_POTION=128,BTN_START=256,BTN_PAUSE=512};
#define UPBIT 0x80
#define CARD_ID(x) ((x)&0x7f)
#define CARD_UP(x) (((x)&UPBIT)!=0)

typedef struct {char name[24];int hp,maxhp,block,strength,weak,vulnerable,poison;int intent,intent_value,phase;int art;} Enemy;
typedef struct {
 uint32_t magic,version,seed,rng;
 int screen,prior_screen,act,floor,lane,selection,target,scroll;
 int hp,maxhp,block,energy,max_energy,gold,turn,energy_penalty,strength,dexterity,weak,vulnerable,poison;
 int potion_count,potions[MAX_POTIONS],relic_count,relics[MAX_RELICS];
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
const char *node_name(int type);
const char *game_enemy_name(int id);
const char *card_description(int raw);
int game_card_is_playable(int raw);
int game_card_is_upgradeable(int raw);
int game_card_cost(int raw);
int game_card_cost_now(const Game *g,int raw);
void game_new(Game *g,uint32_t seed);
int game_moon_heal_now(const Game *g);
void game_action(Game *g,int button);
int game_pause_source(void);
void game_update(Game *g);
void game_render(const Game *g,uint32_t *pixels,int stride);
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
void game_use_potion(Game *g,int index);
/* Transient presentation events. Never serialized or emitted by previews. */
enum {GFX_ATTACK=1,GFX_BLOCK,GFX_POWER,GFX_PROC,GFX_SELF,GFX_STATUS};
#define MAX_COMBAT_FX 64
typedef struct {int kind,source,target,amount,absorbed,card;} GameCombatFx;
void game_combat_fx_begin(void);
void game_combat_fx_end(void);
const GameCombatFx *game_combat_fx_events(int *count);
int game_impact_tier(int damage);
void game_render_update_fx(unsigned tick);
int game_render_fx_busy(unsigned tick);
int game_render_music_paused(unsigned tick);


uint32_t game_random(Game *g);
#endif
