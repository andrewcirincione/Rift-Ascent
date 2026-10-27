#include "source/game.c"
#include <assert.h>
#include <stdio.h>
#define HAS(id) (a==(id)||b==(id))
static void setup(Game*g,int a,int b){
 game_new(g,140);g->relic_count=0;add_relic(g,a);add_relic(g,b);
 g->deck_count=40;for(int i=0;i<40;i++)g->deck[i]=0;
 game_start_battle(g,NODE_FIGHT);if(g->enemy_count<2)g->enemies[1]=g->enemies[0];g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i].hp=g->enemies[i].maxhp=30000;g->enemies[i].block=g->enemies[i].weak=0;}
}
static void play(Game*g,int id){g->screen=COMBAT;g->hand_count=1;g->hand[0]=id;g->energy=20;game_play_card(g,0,0);}
int main(void){Game g,t;int cases=0;
 for(int a=0;a<37;a++)if(a!=30)for(int b=a+1;b<37;b++)if(b!=30){
  setup(&g,a,b);setup(&t,b,a);
  assert(g.maxhp==80+14*HAS(4)+12*HAS(31)-12*HAS(11));
  assert(g.strength==HAS(28)&&g.dexterity==HAS(35));
  assert(g.energy==3+HAS(11)+HAS(25)+HAS(1)+HAS(33));
  assert(g.hand_count==5+HAS(18)+2*HAS(7)-HAS(25));
  assert(g.block==12*HAS(10)+10*HAS(29));
  assert(g.strength==t.strength&&g.energy==t.energy&&g.hand_count==t.hand_count&&g.block==t.block);
  for(int i=0;i<2;i++)assert(g.enemies[i].vulnerable==2*HAS(19));
  g.hp=20;g.turn=2;g.hand_count=0;game_start_turn(&g);
  assert(g.hp==20+4*HAS(20));assert(g.strength==HAS(28)+HAS(34)+3*HAS(14));
  assert(g.energy==3+HAS(11)+HAS(25));assert(g.hand_count==5+HAS(18)-HAS(25));
  g.weak=1;g.enemies[0].vulnerable=1;g.enemies[0].weak=3;
  int expected=((6+g.strength)*3/4)*3/2+5*HAS(3)+3*HAS(26);
  play(&g,0);assert(g.enemies[0].hp==30000-expected);assert(g.enemies[0].weak==3+2*HAS(9));
  int hp=g.hp,strength=g.strength;play(&g,0);play(&g,0);
  assert(g.strength==strength+HAS(5)&&g.hp==hp+3*HAS(24));
  g.block=0;g.dexterity=HAS(35);for(int i=0;i<3;i++)play(&g,1);
  assert(g.block==3*(5+HAS(35)+2*HAS(15)));assert(g.dexterity==HAS(35)+HAS(22));
  for(int id=0;id<6;id++){
   setup(&g,a,b);g.hp=20;g.strength=g.weak=g.block=0;g.energy=0;g.hand_count=0;g.potion_count=1;g.potions[0]=id;
   for(int j=0;j<2;j++){g.enemies[j].vulnerable=0;g.enemies[j].weak=3;}
   game_use_potion(&g,0);
   assert(g.block==(id==2?20+5*HAS(17):0)+12*HAS(21));
   assert(g.hp==20+(id==0?18+5*HAS(17):0));
   assert(g.strength==(id==1?2+HAS(17):0));assert(g.energy==(id==4?2+HAS(17):0));
   assert(g.hand_count==(id==5?3+HAS(17):0));
   for(int j=0;j<2;j++){assert(g.enemies[j].weak==3);assert(g.enemies[j].hp==30000-(id==3?20+5*HAS(17):0));}
  }
  setup(&g,a,b);g.hp=20;int gold=g.gold,reward=g.reward_gold;g.enemies[0].hp=g.enemies[1].hp=0;check_win(&g);
  assert(g.hp==20+6*HAS(0)+3*HAS(36));assert(g.gold==gold+reward+20*HAS(16));
  g.selection=3;game_action(&g,BTN_OK);assert(g.hp==20+6*HAS(0)+3*HAS(36)+6*HAS(27));
  cases++;
 }
 /* All distinct relics coexist, not just pairs. */
 game_new(&g,140);g.relic_count=0;for(int id=0;id<37;id++)if(id!=30)add_relic(&g,id);
 assert(g.relic_count==36);g.deck_count=40;for(int i=0;i<40;i++)g.deck[i]=0;game_start_battle(&g,NODE_FIGHT);
 assert(g.hand_count==7&&g.energy==7&&g.block==22&&g.strength==1&&g.dexterity==1);
 for(int i=0;i<37;i++)if(i!=30)add_relic(&g,i);assert(g.relic_count==36&&g.max_energy==5);
 /* Multi-hit Rust adds only once per target; existing Weak remains. */
 setup(&g,9,3);g.enemies[0].weak=4;play(&g,27);assert(g.enemies[0].weak==6);play(&g,0);assert(g.enemies[0].weak==6);
 setup(&g,9,19);g.enemies[0].weak=4;g.enemies[1].weak=7;play(&g,3);assert(g.enemies[0].weak==6&&g.enemies[1].weak==9);
 /* Legacy Seer's Oath survives as one Compass; no ID shift or pickup replay. */
 for(int both=0;both<2;both++){
  game_new(&g,140);g.relic_count=0;g.relics[g.relic_count++]=30;if(both)g.relics[g.relic_count++]=7;
  assert(game_save(&g,"qa/v140/evidence/legacy-compass.dat"));assert(game_load(&t,"qa/v140/evidence/legacy-compass.dat"));
  assert(t.relic_count==1&&t.relics[0]==7);game_start_battle(&t,NODE_FIGHT);assert(t.hand_count==7);
 }
 for(int seed=1;seed<=1000;seed++){
  game_new(&g,seed);offer_blessing(&g);for(int i=0;i<3;i++)assert(g.reward[i]!=30);
 }
 game_new(&g,140);g.screen=BLESSING;g.reward[0]=30;g.reward[1]=37;g.reward[2]=38;
 assert(game_save(&g,"qa/v140/evidence/legacy-offer.dat"));assert(game_load(&t,"qa/v140/evidence/legacy-offer.dat"));assert(t.reward[0]==39&&game_validate(&t));
 puts("PASS all 630 distinct relic pairs in both acquisition orders: first/third turn, damage, draw, Energy, Block, Strength/Dexterity, third-card counters, all six potions, victory and skip rewards; all 36 relics together; Rust multi-hit/AoE; 1000 blessing offers; legacy Compass migrations");
 return 0;
}
