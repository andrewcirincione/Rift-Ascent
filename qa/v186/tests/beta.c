#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
int main(void){char root[]="/tmp/rift185-beta-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g;int arts[]={6,15,24,8,44,23,22,50,35};
for(int a=0;a<9;a++)for(int seed=1;seed<=1000;seed++){game_new(&g,seed);g.battle_type=NODE_BOSS;g.enemy_count=1;g.act=arts[a]==15?1:arts[a]==24?2:3;g.floor=5;init_enemy(&g,&g.enemies[0],arts[a]);for(int turn=0;turn<8;turn++){g.turn=turn;enemy_intent(&g,&g.enemies[0]);assert(g.enemies[0].intent==enemy_defs[arts[a]].moves[turn%4]);}g.enemies[0].hp=g.enemies[0].maxhp/2;g.turn=3;enemy_intent(&g,&g.enemies[0]);assert(g.enemies[0].intent==enemy_defs[arts[a]].moves[3]);}
puts("PASS retained beta elite/boss cycles: 9 species, 1,000 seeds, phase stability.");
assert(ascent_begin(&g,0,0,185));game_start_battle(&g,NODE_BOSS);g.relic_count=0;g.hp=g.maxhp=80;g.enemies[0].intent=31;g.enemies[0].hp=g.enemies[0].maxhp=999;g.enemies[0].block=0;game_end_turn(&g);assert(g.hp==80&&g.enemies[0].vulnerable==1&&g.enemies[0].block==0);
g.hp=1;g.enemies[0].intent=10;game_end_turn(&g);assert(g.screen==DEFEAT);assert(ascent_checkpoint(&g));Game h;assert(!ascent_resume(&h,0));game_action(&g,BTN_ALT);assert(g.scroll==1);game_action(&g,BTN_POTION);assert(g.scroll==2);game_action(&g,BTN_BACK);assert(!g.scroll);game_action(&g,BTN_OK);assert(g.screen==TITLE);
puts("PASS Recover window, death retirement and recap tabs.");return 0;}
