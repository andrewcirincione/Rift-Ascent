/* One 5% story roll after a completed non-boss map room and its spoils. */
static int expedition_relic(Game*g){
 int id=random_relic(g);return id>=0?id:37+rnd(g,3);
}
static void expedition_roll(Game*g){
 if(preview_trace||g->revision182.room_type==NODE_BOSS||g->expedition.story_pending)return;
 if(rnd(g,100)>=5)return;
 unsigned seen=g->expedition.story_seen;if(seen==0xfffffu)seen=0;
 int pool[20],n=0;for(int i=0;i<20;i++)if(!(seen&(1u<<i)))pool[n++]=i;
 int id=pool[rnd(g,n)];g->expedition.story_seen=seen|(1u<<id);
 g->expedition.story_id=id;g->expedition.story_pending=1;
}
static const ExpeditionStory*expedition_current(const Game*g){
 return &expedition_stories[CLAMP(g->character,0,2)][CLAMP(g->expedition.story_id,0,19)];
}
static void expedition_prepare_gift(Game*g){
 /* Four of twenty story outcomes offer card work instead of a gift.
    The story bag makes this available throughout the run without a bonus
    Forge button on every other encounter. */
 int id=g->expedition.story_id;
 if(id==5||id==6||id==15||id==16){
  for(int i=0;i<g->deck_count;i++)for(int c=0;c<3;c++)if(game_beta_eligible(g,c,g->deck[i])){
   g->expedition.gift_kind=2;g->expedition.gift=0;return;
  }
 }
 const ExpeditionStory*s=expedition_current(g);g->expedition.gift_kind=s->reward_kind;
 if(s->reward_kind){int gift=journey_relic(g,s->relic,-1);g->expedition.gift=gift>=0?gift:39;}
 else if(g->deck_count<MAX_DECK)g->expedition.gift=journey_upgrade(g,reward_of_rarity(g,journey_rarity(g,s->rarity,(unsigned)id),-1,-1),(unsigned)id);
 else{g->expedition.gift_kind=1;g->expedition.gift=39;}
}
int expedition_scene(int s){return s==EXPEDITION_STORY||s==ABOMINATION_BEFORE||s==ABOMINATION_AFTER||s==MYSTERY_STORY;}
const char*expedition_title(const Game*g){
 if(g->screen==ABOMINATION_BEFORE)return "THE UNBEGOTTEN";
 if(g->screen==ABOMINATION_AFTER)return "THE SILENCE ABOVE GOD";
 if(g->screen==MYSTERY_STORY)return "THE NAMELESS EXCHANGE";
 return expedition_current(g)->title;
}
const char*expedition_line(const Game*g){
 static const char*before[3]={
  "Beyond the seventh ascent, the sky opens inward. Every ruined altar points toward the same impossible heart.",
  "It has no worshippers. The voices in its wings are the gods it swallowed.",
  "The Unbegotten unfolds. Creation remembers being prey."
 };
 static const char*after[3]={
  "The dead star inside it fractures. For the first time, the voices stop praying.",
  "Its halos fall through worlds that have not yet been born. Nothing waits behind them.",
  "The wound above the tower remains. Whatever made this thing now knows it can bleed."
 };
 static const char*mystery[3]={
  "A door opens into three lives you have never lived. A faceless trader lays their memories on a velvet table.",
  "TRADER: A blade, a shadow, a crown. You need not become them to learn what they know.",
  "Choose one rare lesson from another path, or leave the exchange untouched."
 };
 int page=CLAMP(g->selection,0,2);
 const char*memory=game_loop_event(g);if(memory)return memory;
 if(g->screen==EXPEDITION_STORY&&page==2&&g->expedition.gift_kind==2)return "A buried ember wakes. The encounter offers no keepsake: its fire can reshape one card before your journey continues.";
 return g->screen==ABOMINATION_BEFORE?before[page]:g->screen==ABOMINATION_AFTER?after[page]:g->screen==MYSTERY_STORY?mystery[page]:expedition_current(g)->lines[page];
}
int expedition_art(const Game*g){return expedition_current(g)->art;}
/* Deliberately offer self-contained rare cards. No inaccessible Stars cost or
 * character-only upkeep: the selected card actually works for its recipient. */
static const int expedition_foreign[3][8]={
 {9,36,38,42,47,74,75,76},
 {N_ADRENALINE,N_ASSASSINATE,N_BLADE_OF_INK,N_BULLET_TIME,N_ECHOING_SLASH,N_MALAISE,N_SHADOWMELD,N_STORM_OF_STEEL},
 {R_CRASH_LANDING,R_HEAVENLY_DRILL,R_HEIRLOOM_HAMMER,R_GUARDS,-1,-1,-1,-1}
};
static void expedition_mystery(Game*g){
 int first=(g->character+1+rnd(g,2))%3,second=3-g->character-first;
 for(int i=0;i<3;i++){
  int owner=i==0?first:i==1?second:(rnd(g,2)?first:second),pool[8],n=0;
  for(int j=0;j<8;j++){int id=expedition_foreign[owner][j],used=0;for(int k=0;k<i;k++)used|=CARD_ID(g->reward[k])==id;if(id>=0&&!used)pool[n++]=id;}
  g->reward[i]=journey_upgrade(g,pool[rnd(g,n)],(unsigned)(i+97));
 }
 g->screen=MYSTERY_STORY;g->selection=0;msg(g,"");
}
static void expedition_boss_spoils(Game*g){
 g->reward[0]=expedition_relic(g);g->relic_pending=1;g->relic_resume=REWARD;
 g->win_screen=1;g->screen=RELIC_RESULT;g->selection=0;msg(g,"A god has fallen. Your ascent continues.");
}
static int expedition_action(Game*g,int b){
 if(b==BTN_PAUSE)return 0;
 switch(g->screen){
 case EXPEDITION_STORY:
  if(b==BTN_OK&&++g->selection>=3){g->selection=0;g->screen=EXPEDITION_GIFT;}return 1;
 case EXPEDITION_GIFT:
  if(g->selection==2)g->selection=1; /* Legacy third choice is now Leave. */
  if(b==BTN_LEFT||b==BTN_UP)g->selection=(g->selection+1)%2;
  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%2;
  else if(b==BTN_OK||b==BTN_BACK){
   if(b==BTN_OK&&g->selection==0&&game_beta_offer(g)){g->screen=BETA_FORGE;g->selection=0;return 1;}
   if(b==BTN_OK&&g->selection==0){if(g->expedition.gift_kind==2){msg(g,"No eligible cards. You may leave the forge.");return 1;}else if(g->expedition.gift_kind==1)add_relic(g,g->expedition.gift);else{if(g->deck_count>=MAX_DECK){msg(g,"Deck full. Leave this gift.");return 1;}add_card(g,g->expedition.gift);}}
   enter_map(g);msg(g,"The encounter passes into memory.");
  }return 1;
 case ABOMINATION_BEFORE:
  if(b==BTN_OK&&++g->selection>=3)game_start_battle(g,NODE_BOSS);return 1;
 case ABOMINATION_AFTER:
  if(b==BTN_OK&&++g->selection>=3)expedition_boss_spoils(g);return 1;
 case MYSTERY_STORY:
  if(b==BTN_OK&&++g->selection>=3){g->screen=MYSTERY_CARDS;g->selection=0;}return 1;
 case MYSTERY_CARDS:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(3,g->selection+1);
  else if(b==BTN_DOWN)g->selection=3;
  else if(b==BTN_UP&&g->selection==3)g->selection=1;
  else if(b==BTN_OK||b==BTN_BACK){
   if(b==BTN_OK&&g->selection<3){if(g->deck_count>=MAX_DECK){msg(g,"Deck full. You may leave the exchange.");return 1;}add_card(g,g->reward[g->selection]);}
   enter_map(g);msg(g,"The door closes without a sound.");
  }return 1;
 default:return 0;
 }
}
