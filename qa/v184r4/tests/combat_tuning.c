#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#ifdef BASELINE
#include "../../v184r3/source/game.c"
#else
#include "../source/game.c"
#endif
static int attacks_in_guards(void){Game g;int n=0;game_new(&g,1);g.act=1;g.floor=5;g.battle_type=NODE_FIGHT;g.turn=2;Enemy e={0};e.art=0;e.hp=e.maxhp=100;
 for(int seed=1;seed<=10000;seed++){g.seed=seed;enemy_intent(&g,&e);n+=enemy_raw_damage(&e)>0;}return n;}
static void combat(Game*g,int art,int intent){game_new(g,18440);game_start_battle(g,NODE_ELITE);g->relic_count=0;g->enemy_count=1;init_enemy(g,&g->enemies[0],art);g->enemies[0].hp=g->enemies[0].maxhp=1000;g->enemies[0].intent=intent;g->enemies[0].intent_value=18;g->enemies[0].strength=g->enemies[0].weak=0;g->hp=g->maxhp=500;g->block=g->vulnerable=g->poison=0;g->hand_count=g->discard_count=0;g->draw_count=20;for(int i=0;i<20;i++)g->draw[i]=1;}
int main(void){
#ifdef BASELINE
 for(int intent=0;intent<INTENT_COUNT;intent++)for(int value=1;value<=120;value++)for(int strength=-4;strength<=8;strength+=4){Enemy e={0};e.intent=intent;e.intent_value=value;e.strength=strength;printf("%d %d %d %d\n",intent,value,strength,enemy_raw_damage(&e));}
 printf("FREQUENCY %d\n",attacks_in_guards());return 0;
#else
 FILE*f=fopen("qa/v184r4/evidence/baseline-damage.txt","r");assert(f);int intent,value,strength,old,count=0;long long before=0,after=0;
 while(fscanf(f,"%d %d %d %d\n",&intent,&value,&strength,&old)==4){Enemy e={0};e.intent=intent;e.intent_value=value;e.strength=strength;int now=enemy_raw_damage(&e);assert(now==(old*130+50)/100);assert(!old?!now:now>=old);before+=old;after+=now;count++;}
 int old_frequency;assert(fscanf(f,"FREQUENCY %d",&old_frequency)==1);fclose(f);assert(count==12960);int current_frequency=attacks_in_guards();assert(current_frequency-old_frequency>350&&current_frequency-old_frequency<650);
 printf("PASS %d attacks compared with the published r3 resolver: exactly +30%% before nearest-integer rounding; sample aggregate +%.2f%%; no minimum damage on non-attacks.\n",count,100.0*(after-before)/before);
 printf("PASS small attack-frequency increase: %d -> %d attacks across 10,000 otherwise-defensive Ash Crawler turns.\n",old_frequency,current_frequency);
 Game g,h;int stone=0;
 for(int seed=1;seed<=1000;seed++){game_new(&g,seed);g.act=1;g.floor=5;g.battle_type=NODE_ELITE;Enemy e={0};e.art=6;e.hp=e.maxhp=1000;int attacks=0;
  for(int turn=0;turn<4;turn++){g.turn=turn;enemy_intent(&g,&e);attacks+=enemy_raw_damage(&e)>0;if(turn==0)assert(e.intent==24);if(turn==1)assert(e.intent==10);if(turn==3)assert(e.intent==0||e.intent==24);if(turn==2)assert(e.intent==21||e.intent==0||e.intent==24);}assert(attacks>=3);stone+=attacks;}
 printf("PASS Stone Giant attacks on at least three turns in every four-turn rotation across 1,000 seeds (%d/4,000 attacking turns), keeps Purge and braced Block, and does not open with Crush.\n",stone);
 for(int id=0;id<INTENT_COUNT;id++)for(int weak=0;weak<2;weak++)for(int vulnerable=0;vulnerable<2;vulnerable++){
  combat(&g,6,id);g.enemies[0].weak=weak;g.vulnerable=vulnerable;int shown=game_enemy_intent_damage(&g,0),hits=game_enemy_intent_hits(&g,0);for(int j=0;j<hits;j++)enemy_hit(&g,&g.enemies[0],enemy_attack_value(&g,&g.enemies[0]));assert(g.hp==500-shown);
 }
 puts("PASS all 31 displayed intentions match actual direct damage with Weak/Vulnerable and multi-hits.");
 for(int id=0;id<INTENT_COUNT;id++)for(int weak=0;weak<2;weak++)for(int crane=0;crane<2;crane++){
  combat(&g,6,id);g.enemy_count=2;g.enemies[1]=g.enemies[0];g.enemies[0].weak=weak;
  if(crane){g.relic_count=1;g.relics[0]=NR_KRANE;}
  int raw=enemy_raw_damage(&g.enemies[0]),scaled=enemy_encounter_damage(&g,&g.enemies[0]);assert(scaled==(raw*75+50)/100);
  int shown=game_enemy_intent_damage(&g,0);g.enemies[1].hp=0;assert(game_enemy_intent_damage(&g,0)==shown);
  int hits=game_enemy_intent_hits(&g,0);for(int j=0;j<hits;j++)enemy_hit(&g,&g.enemies[0],enemy_attack_value(&g,&g.enemies[0]));assert(g.hp==500-shown);
 }
 combat(&g,0,0);g.enemy_count=2;g.enemies[1]=g.enemies[0];g.enemies[1].intent=7;int together=game_enemy_intent_damage(&g,0)+game_enemy_intent_damage(&g,1);game_end_turn(&g);assert(g.hp==500-together);
 puts("PASS paired enemies each use 75% of single-enemy damage; all 31 intents, Weak/Crane, multi-hits and actual combined enemy turns agree with previews; survivor damage never jumps.");
 const int arts[]={14,20,43,35};
 for(int id=27;id<31;id++){
  combat(&g,arts[id-27],id);if(id==28)g.block=5;
  if(id==27||id==29){g.enemy_count=2;init_enemy(&g,&g.enemies[1],0);g.enemies[1].intent=1;g.enemies[1].intent_value=0;}
  char text[160],path[]="/tmp/rift-r4-intent-XXXXXX";game_enemy_intent_text(&g,0,text,sizeof(text));int damage=game_enemy_intent_damage(&g,0);int fd=mkstemp(path);assert(fd>=0);close(fd);assert(game_validate(&g)&&game_save(&g,path)&&game_load(&h,path));unlink(path);g=h;
  game_end_turn(&g);assert(g.screen==COMBAT);
  if(id==27){assert(g.hp==500-damage&&g.enemies[0].block==4&&g.enemies[1].block==4);}
  if(id==28){assert(g.hp==500-MAX(0,damage-2)&&g.energy==g.max_energy-1);}
  if(id==29){assert(g.hp==498&&g.poison==1&&g.enemies[0].block==5&&g.enemies[1].block==5);}
  if(id==30){assert(g.hp==500-damage&&g.vulnerable==1&&g.discard_count==1&&g.discard[0]==51);}
  printf("PASS %s / %s: %s; actual effects and combat save/load.\n",game_enemy_name(arts[id-27]),game_intent_title(id),text);
 }
 return 0;
#endif
}
