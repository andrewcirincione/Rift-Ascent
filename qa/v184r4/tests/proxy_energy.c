#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
static void combat(Game*g){
 game_new(g,18444);game_choose_character(g,CHAR_REGENT);game_start_battle(g,NODE_FIGHT);
 g->relic_count=0;g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=9999;
 g->hp=g->maxhp=500;g->hand_count=g->draw_count=g->discard_count=g->exhaust_count=0;
 g->energy=3;
}
static void play(Game*g,int raw){g->hand_count=1;g->hand[0]=raw;game_play_card(g,0,0);assert(g->screen==COMBAT);}
int main(void){Game g,h,before;char path[]="/tmp/rift-proxy-energy-XXXXXX";int fd=mkstemp(path);assert(fd>=0);close(fd);
 const int ids[]={R_HEGEMONY,R_REFINE_BLADE,R_CONVERGENCE};
 for(int i=0;i<3;i++)for(int up=0;up<2;up++){
  combat(&g);int raw=ids[i]|(up?UPBIT:0),bonus=i==0?2+up:1;
  g.hand_count=1;g.hand[0]=raw;before=g;
  for(int j=0;j<100;j++){CardPreview p;game_card_preview(&g,raw,0,&p);assert(!memcmp(&g,&before,sizeof(g)));}
  game_play_card(&g,0,0);assert(g.regent.next_energy==bonus);
  assert(game_save(&g,path)&&game_load(&h,path));assert(h.regent.next_energy==bonus);
  game_end_turn(&h);assert(h.energy==3+bonus&&h.regent.next_energy==0);
  assert(game_save(&h,path)&&game_load(&g,path));assert(g.energy==3+bonus&&g.regent.next_energy==0);
  for(int j=0;j<3;j++){game_end_turn(&g);assert(g.energy==3&&g.regent.next_energy==0);}
 }
 puts("PASS Hegemony/Refine Blade/Convergence, base/upgraded: 600 previews leave Energy unchanged; delayed Energy applies once, survives save/load, and does not repeat on subsequent turns or after resuming.");
 combat(&g);play(&g,R_HEGEMONY|UPBIT);play(&g,R_REFINE_BLADE|UPBIT);assert(g.energy==0&&g.regent.next_energy==4);
 game_end_turn(&g);assert(g.energy==7&&!g.regent.next_energy);game_end_turn(&g);assert(g.energy==3);
 g.regent.next_energy=4;game_start_battle(&g,NODE_FIGHT);assert(g.energy==3&&!g.regent.next_energy);
 puts("PASS Hegemony+ and Refine Blade+ correctly produce 7 Energy for one turn only; pending Energy never leaks into the next combat.");
 combat(&g);g.relic_count=1;g.relics[0]=62;g.energy=10;
 play(&g,R_STRIKE_REGENT);play(&g,R_DEFEND_REGENT);play(&g,R_FURNACE);assert(g.energy==9&&(g.v151.relic[9]&8));
 for(int j=0;j<3;j++){game_end_turn(&g);assert(g.energy==3);play(&g,R_STRIKE_REGENT);play(&g,R_DEFEND_REGENT);assert(g.energy==1);}
 puts("PASS Prism Bone grants its 2 Energy once per combat, not each turn; Furnace grants Forge, not Energy.");
 /* Optional private reproduction input stays outside the repository. Never
    publish the player's account/save data or modify the supplied directory. */
 const char*root=getenv("RIFT_PRIVATE_SAVE_ROOT");if(root){
  snprintf(ascent_root,sizeof(ascent_root),"%s",root);unsigned generation;
  assert(ascent_read("ASCENT18_RUN_1",0,&g,sizeof(g),&generation)&&game_validate(&g));
  assert(g.character==CHAR_REGENT&&g.energy==6&&g.max_energy==3&&g.regent.next_energy==0);
  for(int j=0;j<3;j++){game_end_turn(&g);assert(g.screen==COMBAT&&g.energy==3&&!g.regent.next_energy);}
  puts("PASS supplied Proxy save resumes with 6 Energy, then returns to 3 for three consecutive turns without playing cards; no persistent unexplained Energy reproduced.");
 }
 unlink(path);return 0;
}
