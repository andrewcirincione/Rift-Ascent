#define main smoke_main
#include "engine_tests.c"
#undef main
static int checks;
static void finish_choices(Game*g){for(int i=0;i<100&&g->screen==NINJA_CHOICE;i++){g->selection=0;while(g->selection<g->hand_count&&!game_ninja_choice_eligible(g,g->selection))g->selection++;assert(g->selection<g->hand_count);game_action(g,BTN_OK);}assert(g->screen!=NINJA_CHOICE);}
static int damage_oracle(int n,int str,int keen,int weak,int vul,int track,int shadow){n=MAX(0,n+str+keen);if(weak)n=n*3/4;if(vul)n=n*3/2;if(track)n*=2;if(shadow)n*=2;return n;}
static void math_audit(void){
 struct Attack {int id,base,up,hits,all;} list[]={
 {N_ASSASSINATE,10,13,1,0},{N_BACKSTAB,11,15,1,0},{N_DAGGER_SPRAY,4,6,2,1},{N_DAGGER_THROW,9,12,1,0},{N_DASH,10,13,1,0},{N_ECHOING_SLASH,10,13,1,1},{N_FINISHER,6,8,3,0},{N_FLECHETTES,5,7,2,0},{N_FLICK_FLACK,6,8,1,1},{N_GRAND_FINALE,60,75,1,1},{N_LEADING_STRIKE,3,6,1,0},{N_MEMENTO_MORI,21,26,1,0},{N_MURDER,8,8,1,0},{N_NEUTRALIZE,3,4,1,0},{N_PINPOINT,15,19,1,0},{N_POISONED_STAB,6,8,1,0},{N_POUNCE,14,20,1,0},{N_PRECISE_CUT,9,12,1,0},{N_PREDATOR,15,20,1,0},{N_SHIV,4,6,1,0},{N_SKEWER,8,11,4,0},{N_SLICE,6,9,1,0},{N_SNAKEBITE,0,0,0,0},{N_STRANGLE,8,10,1,0},{N_STRIKE_SILENT,6,9,1,0},{N_SUCKER_PUNCH,8,10,1,0},{N_SUPPRESS,11,17,1,0},{N_THE_HUNT,10,15,1,0}};
 for(unsigned i=0;i<sizeof(list)/sizeof(*list);i++)for(int up=0;up<2;up++)for(int str=-8;str<=12;str+=2)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++)for(int mod=0;mod<3;mod++)for(int block=0;block<=13;block+=13){
  struct Attack a=list[i];if(!a.hits)continue;int modid=mod==1?MOD_KEEN:mod==2?MOD_ECHO:0;if(modid&&!game_can_modify(a.id,modid))continue;
  Game g=fixture();g.hand_count=2;g.hand[0]=N_DEFEND_SILENT;g.hand[1]=N_DEFLECT;g.strength=str;g.weak=weak;g.energy=4;g.ninja.v[NS_DISCARDS_TURN]=3;g.ninja.v[NS_DRAWS_COMBAT]=7;g.ic[IC_ATTACKS_THIS_TURN]=3;
  for(int e=0;e<2;e++){g.enemies[e].vulnerable=vul;g.enemies[e].block=block;}
  int raw=a.id|(up?UPBIT:0)|(modid<<CARD_MOD_SHIFT);g.hand[g.hand_count++]=raw;
  CardPreview pv;Game before=g;game_card_preview(&g,raw,0,&pv);assert(!memcmp(&g,&before,sizeof(g)));game_play_card(&g,2,0);finish_choices(&g);
  int total=0,hits=0;for(int replay=0;replay<1+(mod==2);replay++){
   int base=up?a.up:a.base,v=vul;
   if(a.id==N_ASSASSINATE&&replay)v=1;
   if(a.id==N_PRECISE_CUT&&replay)base=up?12:9;
   int h=a.hits;if(a.id==N_FINISHER)h+=replay;
   int hit=damage_oracle(base,str,modid==MOD_KEEN?2:0,weak,v,0,0);total+=hit*h;hits+=h;
  }
  for(int e=0;e<(a.all?2:1);e++){
   int expected=MAX(0,total-block);if(a.id==N_STRANGLE&&mod==2&&e==0)expected+=2+up;
   if(g.enemies[e].hp!=100000-expected){fprintf(stderr,"math card=%d up=%d str=%d weak=%d vul=%d mod=%d block=%d expected=%d actual=%d\n",a.id,up,str,weak,vul,mod,block,expected,100000-g.enemies[e].hp);abort();}
   if(!pv.uncertain&&(pv.total[e]!=total||pv.hits[e]!=hits)){fprintf(stderr,"preview id=%d up=%d str=%d weak=%d vul=%d mod=%d total %d/%d hits %d/%d\n",a.id,up,str,weak,vul,mod,pv.total[e],total,pv.hits[e],hits);abort();}
  }checks++;
 }
 struct Block{int id,base,up;} b[]={{N_BACKFLIP,5,8},{N_BLUR,5,8},{N_CLOAK_AND_DAGGER,6,6},{N_DASH,10,13},{N_DEFEND_SILENT,5,8},{N_DEFLECT,4,7},{N_DODGE_AND_ROLL,4,6},{N_ESCAPE_PLAN,3,5},{N_HAND_TRICK,7,10},{N_LEG_SWEEP,11,14},{N_MIRAGE,12,12},{N_SURVIVOR,8,11},{N_UNTOUCHABLE,6,9}};
 for(unsigned i=0;i<sizeof(b)/sizeof(*b);i++)for(int up=0;up<2;up++)for(int dex=-6;dex<=9;dex+=3)for(int ward=0;ward<2;ward++)for(int shadow=0;shadow<3;shadow++)for(int burst=0;burst<2;burst++){
  Game g=fixture();g.dexterity=dex;g.ninja.v[NS_SHADOWMELD]=shadow;g.ninja.v[NS_BURST]=burst;g.draw_count=4;for(int j=0;j<4;j++)g.draw[j]=N_DEFEND_SILENT;g.enemies[0].poison=5;g.enemies[1].poison=7;
  int raw=b[i].id|(up?UPBIT:0)|(ward?MOD_WARD<<CARD_MOD_SHIFT:0);int n=MAX(0,(up?b[i].up:b[i].base)+dex+ward*4)*(1<<shadow);int repeats=burst&&cards[b[i].id].type==1?2:1;play(&g,raw);finish_choices(&g);assert(g.block==MIN(999,n*repeats));checks++;
 }
 printf("PASS %d independent new-card damage / Block cases, including upgrades, Keen, Echo, Ward, Burst, Strength, Weak, Vulnerable and stacked Shadowmeld\n",checks);
}
static void interaction_audit(void){Game g,h;
 g=fixture();g.enemies[0].hp=1;play(&g,N_STRANGLE|(MOD_ECHO<<CARD_MOD_SHIFT));assert(g.enemies[1].hp==99992&&g.ninja.v[NS_STRANGLE1]==2);
 g=fixture();play(&g,N_AFTERIMAGE);assert(g.block==0);play(&g,N_AFTERIMAGE);assert(g.block==1);play(&g,N_SLICE);assert(g.block==3);
 g=fixture();g.ninja.v[NS_POUNCE]=1;ninja_discard_effects(&g,N_HAZE,0);ninja_process(&g);assert(!g.ninja.v[NS_POUNCE]);
 g=fixture();g.ic[IC_UNRELENTING]=1;ninja_discard_effects(&g,N_FLICK_FLACK,0);ninja_process(&g);assert(!g.ic[IC_UNRELENTING]);
 g=fixture();g.ninja.v[NS_BURST]=1;play(&g,N_UP_MY_SLEEVE);assert(g.discard_count==1&&CARD_COUNTER(g.discard[0])==2&&g.hand_count==6);assert(game_card_cost_now(&g,g.discard[0])==0);
 g=fixture();g.ninja.v[NS_PLANNER]=1;play(&g,N_DEFEND_SILENT);assert(g.discard[0]&CARD_SLY);g.hand[g.hand_count++]=g.discard[0];g.discard_count=0;play(&g,N_SURVIVOR);game_action(&g,BTN_OK);assert(g.block==18&&g.discard_count==2);
 g=fixture();g.hand[g.hand_count++]=N_MALAISE|CARD_SLY;g.energy=4;play(&g,N_SURVIVOR);game_action(&g,BTN_OK);assert(g.energy==3&&g.enemies[0].strength==-3&&g.enemies[0].weak==3);
 g=fixture();g.hand[g.hand_count++]=N_REFLEX;g.hand[g.hand_count++]=N_TACTICIAN;play(&g,N_HIDDEN_DAGGERS);assert(g.screen==NINJA_CHOICE);Game snapshot=g;for(int key=BTN_BACK;key<=BTN_START;key*=2){game_action(&g,key);assert(g.screen==NINJA_CHOICE&&g.hand_count==snapshot.hand_count&&g.energy==snapshot.energy);}game_action(&g,BTN_PAUSE);assert(g.screen==PAUSE_MENU);game_action(&g,BTN_BACK);assert(g.screen==NINJA_CHOICE);
 game_action(&g,BTN_OK);assert(g.screen==NINJA_CHOICE&&g.ninja.choice_count==1);assert(game_save(&g,"qa/v150b/evidence/pending-sly.dat"));assert(game_load(&h,"qa/v150b/evidence/pending-sly.dat"));assert(!memcmp(&g,&h,sizeof(g)));finish_choices(&g);finish_choices(&h);assert(!memcmp(&g,&h,sizeof(g)));assert(g.hand_count>=2&&g.energy==51);
 g=fixture();g.ninja.v[NS_PLANS]=2;g.hand[0]=N_REFLEX;g.hand[1]=N_SNAKEBITE;g.hand[2]=N_TACTICIAN;g.hand_count=3;game_end_turn(&g);assert(g.screen==NINJA_CHOICE);game_action(&g,BTN_OK);game_action(&g,BTN_BACK);assert(g.screen==COMBAT&&g.turn==2&&g.ninja.v[NS_DISCARDS_TURN]==0);
 g=fixture();g.exhaust_count=128;for(int i=0;i<128;i++)g.exhaust[i]=N_SHIV;play(&g,N_KNIFE_TRAP|UPBIT);assert(g.enemies[0].hp==100000-128*6&&g.exhaust_count==128&&g.ninja.task_count==0);
 g=fixture();g.relics[g.relic_count++]=NR_SKULL;play(&g,N_OUTBREAK);play(&g,N_DEADLY_POISON);play(&g,N_HAZE);assert(g.enemies[0].poison==11&&g.enemies[1].poison==5&&g.enemies[0].hp==99989&&g.enemies[1].hp==99989);
 play(&g,N_ACCELERANT|UPBIT);g.enemies[0].intent=g.enemies[1].intent=1;game_end_turn(&g);assert(g.enemies[0].hp==99959&&g.enemies[0].poison==8);
 g=fixture();g.ninja.v[NS_INTANGIBLE]=2;g.poison=9;g.hand[g.hand_count++]=50;game_end_turn(&g);assert(g.hp==498&&g.ninja.v[NS_INTANGIBLE]==1);
 g=fixture();play(&g,N_SHADOW_STEP);play(&g,N_SHADOW_STEP);game_end_turn(&g);assert(g.ninja.v[NS_SHADOW_NOW]==2);game_end_turn(&g);assert(g.ninja.v[NS_SHADOW_NOW]==1);game_end_turn(&g);assert(!g.ninja.v[NS_SHADOW_NOW]);
 g=fixture();g.hand[0]=N_UP_MY_SLEEVE|(2<<CARD_COUNTER_SHIFT);g.hand_count=1;play(&g,N_NIGHTMARE);game_action(&g,BTN_OK);game_end_turn(&g);int seen=0,serial[10]={0};for(int i=0;i<g.hand_count;i++)if(CARD_ID(g.hand[i])==N_UP_MY_SLEEVE){assert(CARD_COUNTER(g.hand[i])==2);serial[seen++]=CARD_INSTANCE(g.hand[i]);}assert(seen>=3);for(int i=0;i<seen;i++)for(int j=0;j<i;j++)assert(serial[i]!=serial[j]);
 for(int intent=0;intent<18;intent++)for(int strength=-25;strength<=5;strength+=5){g=fixture();g.enemy_count=1;g.enemies[0].intent=intent;g.enemies[0].intent_value=18;g.enemies[0].strength=strength;g.enemies[0].weak=2;g.relics[g.relic_count++]=NR_KRANE;int expected=game_enemy_intent_damage(&g,0);int hits=intent==7?2:intent==16?3:1;game_end_turn(&g);int poison=intent==6?2:0;assert(500-g.hp==expected+poison);}
 puts("PASS Sly chains, forced-choice controls, pending save/resume, Retain, 128-Shiv replay, poison/Outbreak, Intangible, Shadow Step duration, Nightmare copies and all 18 enemy intents");
}
static void pools(void){Game g;int bins[3][3]={{0}};for(int ch=0;ch<2;ch++)for(int mode=0;mode<3;mode++)for(int seed=1;seed<=1000;seed++){
 game_new(&g,seed);game_choose_character(&g,ch);game_start_battle(&g,mode==0?NODE_FIGHT:mode==1?NODE_ELITE:NODE_BOSS);for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;game_end_turn(&g);if(g.screen==RELIC_RESULT)game_action(&g,BTN_BACK);assert(g.screen==REWARD);for(int i=0;i<3;i++){int id=CARD_ID(g.reward[i]);assert(ch==CHAR_NIGHTSHADE?id>=96:id<96);assert(id!=N_FLANKING&&id!=N_SNEAKY&&cards[id].rarity<3);if(mode==2)assert(cards[id].rarity==2);if(mode==0)assert(cards[id].rarity<2);bins[mode][cards[id].rarity]++;}}
 printf("PASS 18,000 class-filtered reward draws; bosses are rare, ordinary fights exclude rare, Ancient, starter, token and multiplayer cards\n");
}
int main(void){setvbuf(stdout,0,_IOLBF,0);smoke_main();math_audit();interaction_audit();pools();return 0;}
