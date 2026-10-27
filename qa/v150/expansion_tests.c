#include "source/game.c"
#include <assert.h>
static void arena(Game*g,int raw){game_new(g,150);game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->hand_count=1;g->hand[0]=raw;g->energy=5;g->hp=50;g->block=g->strength=g->dexterity=g->vulnerable=g->weak=0;g->relic_count=0;g->draw_count=g->discard_count=g->exhaust_count=0;Enemy*e=&g->enemies[0];e->hp=e->maxhp=999;e->block=e->strength=e->weak=e->vulnerable=0;e->intent=0;e->intent_value=12;}
int main(void){Game g;int cases=0;
 for(int intent=14;intent<INTENT_COUNT;intent++)for(int block=0;block<25;block+=3)for(int str=0;str<4;str++)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++){
  arena(&g,0);g.block=block;g.vulnerable=vul;Enemy*e=&g.enemies[0];e->intent=intent;e->intent_value=12;e->strength=str;e->weak=weak;
  int per=intent==14||intent==15?6+str:intent==16?4+str:0;if(weak)per=per*3/4;if(vul)per=per*3/2;
  int total=per*(intent==16?3:1),absorb=intent==14?0:intent==15?MAX(0,block-4):block;
  assert(game_enemy_intent_damage(&g,0)==total);
  int before=g.hp;game_end_turn(&g);assert(g.hp==before-MAX(0,total-absorb));
  if(intent==17){int injury=0;for(int i=0;i<g.hand_count;i++)injury+=g.hand[i]==52;for(int i=0;i<g.draw_count;i++)injury+=g.draw[i]==52;assert(injury==1);assert(g.deck_count==10);}
  cases++;
 }
 for(int up=0;up<2;up++){
  arena(&g,92|(up?UPBIT:0));g.enemies[0].block=20;game_play_card(&g,0,0);int dmg=14+5*up,remaining=20-6-3*up;assert(g.enemies[0].hp==999-MAX(0,dmg-remaining)&&g.enemies[0].block==MAX(0,remaining-dmg));
  for(int attack=0;attack<2;attack++){arena(&g,93|(up?UPBIT:0));g.enemies[0].intent=attack?0:1;game_play_card(&g,0,0);assert(g.block==6+3*up+attack*4);}
  for(int weak=0;weak<2;weak++){arena(&g,94|(up?UPBIT:0));g.enemies[0].weak=weak;game_play_card(&g,0,0);assert(g.hp==50+weak*(3+up));}
  for(int energy=0;energy<5;energy++){
   arena(&g,95|(up?UPBIT:0));g.strength=20;g.weak=2;g.enemies[0].vulnerable=3;g.enemies[0].block=3;
   game_play_card(&g,0,0);assert(g.ic[IC_COVENANT]==4+2*up&&power_present(&g,95));g.energy=energy;int old=g.enemies[0].hp;game_prepare_end_turn(&g);assert(g.enemies[0].hp==old-MAX(0,(4+2*up)*MIN(energy,2)-3));int hp=g.enemies[0].hp;game_prepare_end_turn(&g);assert(g.enemies[0].hp==hp);
  }
 }
 int seen[44]={0};for(int seed=1;seed<5000;seed++)for(int act=1;act<=3;act++)for(int mode=NODE_FIGHT;mode<=NODE_ELITE;mode++){
  game_new(&g,seed);g.act=act;g.floor=8;game_start_battle(&g,mode);for(int i=0;i<g.enemy_count;i++)seen[g.enemies[i].art]++;
 }
 for(int id=36;id<44;id++)assert(seen[id]>50);
 printf("PASS %d exact new-intent damage/block/status cases; four new base/upgraded cards, Covenant modifiers/cap/once-only end step, all eight new species spawn naturally\n",cases);
 // Preserve old maps and in-flight combat while extending the map by four rows.
 const char*files[]={"state-map-chest.dat","state-combat.dat","state-endless-choice.dat","state-upgrade-short-row.dat"};
 for(int i=0;i<4;i++){char path[150];snprintf(path,sizeof(path),"qa/v150/fixtures/%s",files[i]);assert(game_load(&g,path));assert(game_validate(&g));assert(game_save(&g,"qa/v150/evidence/migrated.dat"));Game loaded;assert(game_load(&loaded,"qa/v150/evidence/migrated.dat"));assert(!memcmp(&g,&loaded,sizeof(g)));}
 puts("PASS four legacy map/combat/ending/picker migrations into 19-floor acts and save roundtrip");
}
