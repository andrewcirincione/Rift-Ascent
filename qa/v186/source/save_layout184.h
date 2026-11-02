/* Frozen 1.8.4 layout. Keep old array sizes and offsets for migration. */
typedef struct {
 int stars,forge,blade_created,created,stars_gained,skills,plays_combat;
 int shuffle_pending,shuffle_draws;
 int powers[EXTRA_LAST-REGENT_FIRST],played[EXTRA_LAST-REGENT_FIRST];
 int hit_count[2],conqueror[2];
 int vigor,attack_vigor,monologue,next_energy,next_stars,next_draw,next_block,next_seek;
 int retain_all,orbit_spent,draws,nostalgia_used,void_used,panic,reflect,gambit;
 int instance_damage[REGENT_INSTANCE_MAX],instance_cost[REGENT_INSTANCE_MAX],instance_replay[REGENT_INSTANCE_MAX];
 int return_cards[MAX_PILE],return_count;
 int bomb_ticks[16],bomb_damage[16];
 int choice_kind,choice_remaining,choice_raw,choice_target,choice_aux,choice_depth;
 int options[MAX_PILE],option_count;
 int task_count,resolving;RegentTask tasks[REGENT_TASK_MAX];
 int automation_phase[10],panache_phase[5];
} RegentState184;
typedef struct {
 uint32_t magic,version,seed,rng;
 int screen,prior_screen,act,floor,lane,selection,target,scroll;
 int hp,maxhp,block,energy,max_energy,gold,turn,energy_penalty,strength,dexterity,weak,vulnerable,poison;
 int potion_count,potions[MAX_POTIONS],relic_count,relics[LEGACY_RELIC_CAP];
 int deck_count,deck[MAX_DECK],master_index;
 int draw_count,draw[MAX_PILE],discard_count,discard[MAX_PILE],exhaust_count,exhaust[MAX_PILE],hand_count,hand[MAX_HAND];
 int map[MAP_ROWS][MAP_LANES],map_selected[MAP_ROWS],enemy_count,reward[3],reward_gold;
 Enemy enemies[2];
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
 RegentState184 regent;
 AscentRun ascent;
 Expedition181 expedition;
 /* Append-only 1.8.2 state: leave every older save offset intact. */
 struct {int rare_offset,room_pending,room_type;unsigned event_seen_hi;int next_combat[5];} revision182;
 /* Append-only: old slots migrate with their original route topology. */
 struct {int custom,building,tab,quantity,item,return_tab,return_selection,route_style,seen_act;unsigned seen[2];} revision184;
} Game184;
typedef char layout184_size[(sizeof(Game184)==28500)?1:-1];
