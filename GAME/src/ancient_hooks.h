/* Shared hook state. Independent counters avoid relics consuming each other's
   once-per-turn or once-per-combat triggers. All state is in the run snapshot. */
enum {SB_PLAYS,SB_ATTACKS,SB_SKILLS,SB_TYPES,SB_FIRST_POWER,SB_HP_TURN,SB_HP_PREV,SB_DEMON,SB_PUZZLE,SB_STICKS,SB_EYE,SB_VAMBRACE,SB_LEGION,SB_HAND_PLAYS,SB_LAST,SB_DEAD,SB_OPENING,SB_HP_COMBAT,SB_CREATED,SB_KILL_GUARD,SB_TOP_GUARD,SB_RUINED,SB_STR_BASE,SB_LAMP,SB_NEXT_BLOCK,SB_NEXT_DRAW,SB_NEXT_ENERGY,SB_PEN,SB_VIGOR,SB_IMBUED,SB_CORE_TICK,SB_RESHUFFLE,SB_CARRY,SB_TURN_SERIAL,SB_DRAW_LOCK,SB_EARRING,SB_TEA_ENERGY};
enum {SU_PICK,SU_REMAIN,SU_ARG,SU_RESUME,SU_OFFER0,SU_OFFER1,SU_OFFER2,SU_FOREIGN,SU_REROLLED,SU_MAW_OFF,SU_CAMP_USED,SU_CAMP_ACTIVE,SU_COMPASS_ACT,SU_REWARD_EXTRA,SU_REWARD_RARE,SU_REWARD_POWER,SU_REWARD_UP,SU_WAX_COUNT,SU_WAX_CLOCK,SU_IMBUED,SU_CAPTURE_GOLD,SU_CANDLE,SU_REWARD_INDEX,SU_TEMP};
enum {SP_REMOVE=1,SP_UPGRADE,SP_TRANSFORM,SP_COPY,SP_FORGE,SP_REWARD,SP_OPENING,SP_ANCIENT_STARTER};
#define SB(g,k) ((g)->revision186.battle[k])
#define SU(g,k) ((g)->revision186.run[k])
#define SC(g,id) ((g)->revision186.relic_counter[id])
static int sr_is_starter(int raw);
static int sr_is_strike(int raw);
static int sr_is_defend(int raw);
static int sr_allowed(const Game*,int);
static int sr_cost(const Game*,int,int);
static int sr_can_play(const Game*);
static int sr_repeat(Game*,int);
static int sr_x(const Game*);
static void sr_paid(Game*,int,int);
static void sr_play_begin(Game*,int);
static void sr_play_finish(Game*,int);
static void sr_drawn(Game*,int*);
static void sr_created(Game*,int);
static void sr_shuffle(Game*);
static void sr_exhaust(Game*,int);
static int sr_hp_loss(Game*,int);
static void sr_lost_hp(Game*,int);
static void sr_after_strength(Game*);
static int sr_attack_bonus(const Game*,int);
static int sr_block_amount(Game*,int);
static void sr_kills(Game*);
static void sr_pickup(Game*,int);
static void sr_added(Game*,int);
static int sr_new_card(const Game*,int);
static void sr_battle(Game*);
static void sr_turn_before(Game*);
static void sr_turn_after(Game*);
static int sr_turn_energy(Game*);
static int sr_turn_draw(Game*);
static int sr_end_turn(Game*);
static void sr_victory(Game*);
static void sr_reward(Game*);
static void sr_skipped(Game*);
static void sr_room(Game*,int);
static void sr_rest(Game*);
static int sr_rest_action(Game*,int);
static int*sr_potion_slot(Game*,int);
static int sr_rest_action(Game*,int);
static int*sr_potion_slot(Game*,int);
static void sr_potion(Game*);
static void sr_gain_gold(Game*,int);
static int sr_special_relic(Game*,int,int);
static void sr_open_pending(Game*);
static int sr_action(Game*,int);
static int sr_eligible_relic(const Game*,int,int);
static void sr_offer_blessing(Game*);
