#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
static void combat(Game*g,int art,int intent){
 game_new(g,18434);g->act=art==50?1:2;game_start_battle(g,NODE_ELITE);g->relic_count=0;g->enemy_count=1;
 init_enemy(g,&g->enemies[0],art);g->enemies[0].intent=intent;g->enemies[0].intent_value=18;
 g->enemies[0].hp=g->enemies[0].maxhp=1000;g->enemies[0].strength=g->enemies[0].weak=0;
 g->hp=g->maxhp=500;g->block=g->vulnerable=g->poison=0;g->hand_count=g->discard_count=0;g->draw_count=12;for(int j=0;j<12;j++)g->draw[j]=1;
}
int main(void){Game g,h;long long old=0,now=0;int cases=0;
 for(int intent=0;intent<INTENT_COUNT;intent++)for(int v=1;v<=120;v++)for(int strength=-4;strength<=8;strength+=4){
  Enemy e={0};e.intent=intent;e.intent_value=v;e.strength=strength;
  int base=enemy_base_damage(&e),scaled=enemy_raw_damage(&e);assert(scaled==(base*106+50)/100&&scaled>=base);
  assert(scaled-base<=base*.06+.51);if(base<9)assert(scaled==base);if(!base)assert(!scaled);
  old+=base;now+=scaled;cases++;
 }
 printf("PASS %d intent/value/Strength combinations: damage-only +6%% rounded per hit; zero-damage actions and hits below 9 unchanged; sample aggregate +%.2f%%.\n",cases,100.0*(now-old)/old);
 for(int intent=0;intent<INTENT_COUNT;intent++)for(int weak=0;weak<2;weak++)for(int vulnerable=0;vulnerable<2;vulnerable++){
  combat(&g,23,intent);g.enemies[0].weak=weak;g.vulnerable=vulnerable;
  int shown=game_enemy_intent_damage(&g,0),hits=game_enemy_intent_hits(&g,0);
  for(int i=0;i<hits;i++)enemy_hit(&g,&g.enemies[0],enemy_attack_value(&g.enemies[0]));
  assert(g.hp==500-shown);
 }
 puts("PASS all 27 intent displays match actual direct damage, including multi-hits, Weak and Vulnerable.");
 for(int intent=25;intent<=26;intent++){
  int art=intent==25?50:23;combat(&g,art,intent);char text[160];game_enemy_intent_text(&g,0,text,sizeof(text));
  assert(strcmp(game_intent_title(intent),"UNKNOWN")&&strlen(game_intent_description(intent))>20);
  int damage=game_enemy_intent_damage(&g,0);assert(game_validate(&g));
  char path[]="/tmp/rift-intent-save-XXXXXX";int fd=mkstemp(path);assert(fd>=0);close(fd);assert(game_save(&g,path)&&game_load(&h,path));unlink(path);g=h;
  game_end_turn(&g);assert(g.screen==COMBAT&&g.hp==500-damage);
  if(intent==25){assert(g.enemies[0].block==9&&g.discard_count==1&&g.discard[0]==50&&g.energy==g.max_energy);assert(strstr(text,"Block")&&strstr(text,"Burn"));}
  else{assert(damage==12&&g.energy==g.max_energy-1);assert(strstr(text,"twice")&&strstr(text,"Energy"));}
  int appearances=0;for(int seed=1;seed<=1000;seed++){g.seed=seed;g.turn=2;g.enemies[0].phase=0;enemy_intent(&g,&g.enemies[0]);appearances+=g.enemies[0].intent==intent;}
  assert(appearances>900);printf("PASS %s: %s; save/load and actual turn effects; appears in %d/1000 scheduled turns.\n",game_enemy_name(art),text,appearances);
 }
 return 0;
}
