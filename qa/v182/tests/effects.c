#include <assert.h>
#include "../source/game.c"
static void clean(Game*g){game_new(g,123);game_choose_character(g,0);g->relic_count=0;g->screen=MAP;game_start_battle(g,NODE_FIGHT);g->hp=50;g->maxhp=100;g->gold=0;g->block=g->strength=g->dexterity=0;g->energy=3;g->target=0;g->hand_count=g->discard_count=0;g->draw_count=20;for(int i=0;i<20;i++)g->draw[i]=0;g->regent.stars=g->regent.forge=0;for(int i=0;i<g->enemy_count;i++){g->enemies[i].hp=g->enemies[i].maxhp=10000;g->enemies[i].poison=g->enemies[i].weak=g->enemies[i].vulnerable=g->enemies[i].block=0;}}
int main(void){Game g,base;
 for(int id=6;id<26;id++){
  clean(&g);g.potions[0]=id;g.potion_count=1;
  if(id==13)g.weak=g.vulnerable=g.poison=3;
  if(id==23)for(int i=0;i<g.enemy_count;i++)g.enemies[i].block=20;
  if(id==24)g.strength=3;
  if(id==25)g.enemies[0].poison=7;
  game_use_potion(&g,0);assert(g.potion_count==0);
  switch(id){
   case 6:assert(g.hp==62&&g.strength==1);break;
   case 7:assert(g.block==28&&g.enemies[0].weak==1);break;
   case 8:assert(g.enemies[0].poison==9);break;
   case 9:assert(g.regent.stars==7&&g.energy==4);break;
   case 10:assert(g.regent.forge==12&&g.block==10);break;
   case 11:assert(g.hp==74);break;
   case 12:assert(g.enemies[0].hp==9968);break;
   case 13:assert(!g.weak&&!g.vulnerable&&!g.poison&&g.block==8);break;
   case 14:assert(g.hand_count==4&&g.block==4);break;
   case 15:assert(g.hp==46&&g.energy==6);break;
   case 16:assert(g.hand_count==4&&CARD_ID(g.hand[0])==N_SHIV&&CARD_UP(g.hand[0]));break;
   case 17:assert(g.strength==3&&g.dexterity==1);break;
   case 18:assert(g.enemies[0].weak==3&&g.enemies[0].vulnerable==3);break;
   case 19:assert(g.gold==40);break;
   case 20:assert(g.enemies[0].hp==9986&&g.block==14);break;
   case 21:assert(g.maxhp==104&&g.hp==60);break;
   case 22:assert(g.hand_count==2&&g.energy==5);break;
   case 23:assert(g.enemies[0].block==0&&g.enemies[0].hp==9990);break;
   case 24:assert(g.block==36&&g.strength==2);break;
   case 25:assert(g.enemies[0].poison==14&&g.enemies[0].vulnerable==2);break;
  }
 }
 for(int id=64;id<74;id++){
  clean(&g);add_relic(&g,id);
  if(id==72){assert(g.maxhp==110&&g.hp==60);continue;}
  game_start_battle(&g,NODE_FIGHT);
  switch(id){
   case 64:assert(g.block==6);break;
   case 65:assert(g.enemies[0].poison==2);break;
   case 66:assert(g.regent.stars==3);break;
   case 67:case 68:base=g;base.relic_count=0;for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=base.enemies[i].hp=0;check_win(&g);check_win(&base);assert(g.gold==base.gold+(id==67?8:0)&&g.hp==base.hp+(id==68?3:0));break;
   case 69:assert(g.hand_count==6);break;
   case 70:assert(g.strength==1);break;
   case 71:assert(g.dexterity==1);break;
   case 73:g.turn=2;game_start_turn(&g);assert(g.turn==3&&g.energy==g.max_energy+1);break;
  }
 }
 clean(&g);assert(card_base_damage(&g,343,0)==24);assert(card_base_damage(&g,372|UPBIT,0)==55);
 puts("PASS all 20 new potion effects, all 10 relic effects, and cross-card base damage");return 0;
}
