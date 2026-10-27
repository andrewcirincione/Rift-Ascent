#define main engine_main
#include "../engine_tests.c"
#undef main
static unsigned rng=0x1a5b;
static unsigned roll(unsigned n){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng%n;}
static void valid(Game*g){if(!game_validate(g)){fprintf(stderr,"invalid screen=%d selection=%d hp=%d maxhp=%d deck=%d hand=%d target=%d tasks=%d\n",g->screen,g->selection,g->hp,g->maxhp,g->deck_count,g->hand_count,g->target,g->ninja.task_count);abort();}}
static void settle(Game*g){for(int n=0;n<1000&&g->screen==NINJA_CHOICE;n++){int i=0;while(i<g->hand_count&&!game_ninja_choice_eligible(g,i))i++;assert(i<g->hand_count);g->selection=i;game_action(g,BTN_OK);}assert(g->screen!=NINJA_CHOICE);}
static void class_cards(Game*g){for(int i=0;i<g->deck_count;i++){int id=CARD_ID(g->deck[i]);assert((id>=50&&id<=53)||(id>=NINJA_FIRST)==g->character);}}
static void events_and_shop(void){int cases=0;
 for(int ch=0;ch<2;ch++)for(int seed=1;seed<=200;seed++)for(int id=0;id<EVENT_COUNT;id++)for(int option=0;option<3;option++){
  Game g;game_new(&g,seed);game_choose_character(&g,ch);g.hp=40;g.gold=1000;g.potion_count=0;
  game_begin_event(&g,id);valid(&g);g.selection=option;Game before=g;game_action(&g,BTN_OK);
  if(g.screen==EVENT_CARD){Game picker=g;game_action(&g,BTN_BACK);assert(g.gold==before.gold&&g.hp==before.hp&&g.deck_count==before.deck_count&&!memcmp(g.deck,before.deck,sizeof(g.deck)));g=picker;game_action(&g,BTN_OK);}
  valid(&g);class_cards(&g);cases++;
 }
 printf("PASS %d event choices, class pools, picker cancellation and commits\n",cases);
 for(int ch=0;ch<2;ch++)for(int seed=1;seed<=500;seed++){
  Game g;game_new(&g,seed);game_choose_character(&g,ch);g.gold=1000;g.screen=MAP;int lane=0;while(lane<3&&!game_route_available(&g,lane))lane++;assert(lane<3);g.map[0][lane]=NODE_SHOP;game_choose_node(&g,lane);assert(g.screen==SHOP);
  for(int i=0;i<3;i++){g.selection=i;game_action(&g,BTN_OK);valid(&g);class_cards(&g);}
  int olddeck=g.deck_count,oldgold=g.gold;game_action(&g,BTN_ALT);assert(g.screen==DECK_VIEW&&g.deck_readonly);game_action(&g,BTN_OK);game_action(&g,BTN_BACK);assert(g.deck_count==olddeck&&g.gold==oldgold);
  g.potion_count=0;g.selection=5;for(int i=0;i<3;i++){int prior[3];memcpy(prior,g.potions,sizeof(prior));game_action(&g,BTN_OK);assert(g.potion_count==i+1&&!memcmp(prior,g.potions,i*sizeof(int)));}
  int gold=g.gold;int prior[3];memcpy(prior,g.potions,sizeof(prior));game_action(&g,BTN_OK);assert(g.gold==gold&&g.potion_count==3&&!memcmp(prior,g.potions,sizeof(prior)));
  g.screen=MAP;g.selection=g.lane;g.potions[0]=0;g.hp=1;game_use_potion(&g,0);assert(g.hp==19&&g.potion_count==2);valid(&g);
 }
 puts("PASS 1,000 shops: class-correct purchases, read-only browsing, full potion belts and map healing");
}
static Game equivalent(int ch,int r1,int r2){Game g=fixture();g.character=ch;g.hp=400;g.deck_count=12;for(int i=0;i<12;i++)g.deck[i]=ch?N_STRIKE_SILENT:0;g.relic_count=0;add_relic(&g,r1);if(r2!=r1)add_relic(&g,r2);return g;}
static void same_stats(const Game*a,const Game*b){assert(a->hp==b->hp&&a->maxhp==b->maxhp&&a->gold==b->gold&&a->strength==b->strength&&a->dexterity==b->dexterity&&a->energy==b->energy&&a->block==b->block);for(int i=0;i<2;i++)assert(a->enemies[i].hp==b->enemies[i].hp&&a->enemies[i].weak==b->enemies[i].weak&&a->enemies[i].vulnerable==b->enemies[i].vulnerable);}
static void shared_relics(void){
 for(int a=0;a<44;a++)for(int b=a;b<44;b++){
  Game w=equivalent(0,a,b),n=equivalent(1,a,b);same_stats(&w,&n);
  for(int i=0;i<6;i++){int skill=i%2;play(&w,skill?1:0);play(&n,skill?N_DEFEND_SILENT:N_STRIKE_SILENT);same_stats(&w,&n);}
  for(int p=0;p<6;p++){w.potion_count=n.potion_count=1;w.potions[0]=n.potions[0]=p;game_use_potion(&w,0);game_use_potion(&n,0);same_stats(&w,&n);}
  game_end_turn(&w);game_end_turn(&n);same_stats(&w,&n);valid(&w);valid(&n);
 }
 puts("PASS 990 shared relic singles/pairs: equivalent Strike/Defend triggers, all six potions and turn transitions");
}
static void replacement_pool(void){
 for(int seed=1;seed<=1000;seed++){Game g=equivalent(0,0,0);g.rng=seed;g.hand_count=9;for(int i=0;i<9;i++)g.hand[i]=0;play(&g,83);for(int i=0;i<g.hand_count;i++)assert(CARD_ID(g.hand[i])<NINJA_FIRST&&CARD_ID(g.hand[i])!=63&&CARD_ID(g.hand[i])!=85);valid(&g);}
 puts("PASS 1,000 random-hand replacements stay in the Warden pool");
}
static void combinations(void){int cases=0;int counts[256]={0};
 for(int run=0;run<2000;run++){
  Game g=fixture();g.hp=g.maxhp=5000;g.energy=99;
  for(int j=0;j<5;j++)add_relic(&g,roll(RELIC_COUNT));
  for(int j=0;j<24;j++){int id;do{id=NINJA_FIRST+roll(NINJA_COUNT);}while(id==N_FLANKING||id==N_SNEAKY);int raw=id|(roll(2)?UPBIT:0),mod=1+roll(MOD_COUNT-1);if(game_can_modify(raw,mod))raw|=mod<<CARD_MOD_SHIFT;g.draw[g.draw_count++]=raw;}
  for(int j=0;j<7;j++)draw_card(&g);
  for(int action=0;action<80&&g.screen==COMBAT;action++){
   valid(&g);if(g.hand_count&&roll(5)){int i=roll(g.hand_count),raw=g.hand[i];Game before=g;CardPreview pv;game_card_preview(&g,raw,0,&pv);assert(!memcmp(&before,&g,sizeof(g)));int n=g.turn_played;game_play_card(&g,i,0);settle(&g);if(g.turn_played>n)counts[CARD_ID(raw)]++;}
   else{game_end_turn(&g);settle(&g);}
   valid(&g);cases++;
  }
 }
 int reached=0;for(int id=NINJA_FIRST;id<NINJA_LAST;id++)if(counts[id])reached++;
 printf("PASS %d mixed-deck actions across 2,000 encounters: %d solo cards exercised, preview purity, modifications, choices and valid state\n",cases,reached);assert(reached==87);
}
static void progression(void){
 for(int ch=0;ch<2;ch++){
  Game g;game_new(&g,150);game_action(&g,BTN_OK);g.selection=ch;game_action(&g,BTN_OK);
  for(int act=1;act<=100;act++){
   while(g.screen==ACT_INTRO)game_update(&g);assert(g.screen==BLESSING&&g.act==act&&g.hp==g.maxhp);valid(&g);g.selection=act%3;game_action(&g,BTN_OK);assert(g.screen==MAP);class_cards(&g);
   game_start_battle(&g,NODE_BOSS);assert(g.screen==COMBAT&&g.ninja.task_count==0);g.hp=1;
   for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;
   game_end_turn(&g);
   if(act==3){assert(g.screen==VICTORY);Game menu=g;menu.selection=1;game_action(&menu,BTN_OK);assert(menu.screen==TITLE);g.selection=0;game_action(&g,BTN_OK);assert(g.endless&&g.act==4);}
   else {assert(g.screen==RELIC_RESULT);game_action(&g,BTN_BACK);assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);}
   valid(&g);
  }
 }
 puts("PASS both characters: normal victory/menu/endless choices and 100-act progression each (controlled boss defeats)");
}
static void choice_controls(void){
 for(int kind=1;kind<=4;kind++){
  Game base=fixture();base.hand_count=2;base.hand[0]=N_DEFEND_SILENT;base.hand[1]=N_TACTICIAN;
  if(kind==4){base.ninja.v[NS_PLANS]=1;game_end_turn(&base);}else play(&base,kind==1?N_SURVIVOR:kind==2?N_HAND_TRICK:N_NIGHTMARE);
  assert(base.screen==NINJA_CHOICE&&base.ninja.choice_kind==kind);valid(&base);
  int buttons[]={BTN_ALT,BTN_START,BTN_POTION,BTN_BACK};
  for(int j=0;j<4;j++){if(kind==4&&buttons[j]==BTN_BACK)continue;Game g=base;game_action(&g,buttons[j]);assert(g.screen==NINJA_CHOICE&&g.hand_count==base.hand_count&&g.energy==base.energy&&g.ninja.task_count==base.ninja.task_count);valid(&g);}
  Game g=base;game_action(&g,BTN_PAUSE);assert(g.screen==PAUSE_MENU);game_action(&g,BTN_BACK);assert(g.screen==NINJA_CHOICE);valid(&g);
  Game loaded;assert(game_save(&g,"/tmp/rift-choice-parity.dat"));assert(game_load(&loaded,"/tmp/rift-choice-parity.dat"));settle(&g);settle(&loaded);assert(!memcmp(&g,&loaded,sizeof(g)));valid(&g);
 }
 remove("/tmp/rift-choice-parity.dat");puts("PASS all four Nightshade choices: shortcut guards, pause/return and exact save/resume");
}
int main(void){setvbuf(stdout,0,_IOLBF,0);events_and_shop();shared_relics();replacement_pool();combinations();progression();choice_controls();return 0;}
