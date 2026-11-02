#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#ifdef BASELINE
#include "../../v184r2/source/game.c"
#else
#include "../source/game.c"
#endif
static void combat(Game*g,int character,int custom){
 game_new(g,18433);game_choose_character(g,character);g->revision184.custom=custom;g->relic_count=0;
 game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=9999;
 g->enemies[0].block=0;g->enemies[0].intent=1;g->enemies[0].intent_value=0;
 g->energy=99;g->block=0;g->hand_count=g->draw_count=g->discard_count=g->exhaust_count=0;
}
static void play(Game*g,int raw){g->hand_count=1;g->hand[0]=raw;g->selection=0;game_play_card(g,0,0);}
int main(void){Game g,h;int cases=0,missing=0;
 const int follow[]={N_STRIKE_SILENT,N_DEFEND_SILENT,N_FOOTWORK,N_SHIV,0,1,6,R_STRIKE_REGENT,R_DEFEND_REGENT,R_GENESIS,C_FLASH_OF_STEEL,C_FINESSE,C_PANACHE,334,354,374};
 for(int c=0;c<3;c++)for(int custom=0;custom<2;custom++)for(int up=0;up<2;up++){
  combat(&g,c,custom);play(&g,N_AFTERIMAGE|(up?UPBIT:0));assert(g.ninja.v[NS_AFTERIMAGE]==1&&g.block==0);
  for(unsigned i=0;i<sizeof(follow)/sizeof(*follow);i++){
   Game active=g,off=g;off.ninja.v[NS_AFTERIMAGE]=0;active.hand_count=1;active.hand[0]=follow[i];active.selection=0;
   Game preview_before=active;CardPreview p;game_card_preview(&active,follow[i],0,&p);assert(!memcmp(&active,&preview_before,sizeof(active)));
   play(&active,follow[i]);play(&off,follow[i]);assert(active.screen==COMBAT&&off.screen==COMBAT);
   if(active.block-off.block!=1)missing++;
#ifndef BASELINE
   assert(active.block-off.block==1);assert(game_validate(&active));
#endif
   cases++;
  }
 }
#ifdef BASELINE
 assert(missing>0);printf("REPRODUCED missing Afterimage Block in %d/%d mixed-family plays (all characters, normal/custom, base/upgraded).\n",missing,cases);return 0;
#else
 assert(!missing);printf("PASS %d Attack/Skill/Power/token plays across all families and all characters, normal/custom, base/upgraded Afterimage; previewing cards leaves the live combat unchanged.\n",cases);
 combat(&g,1,0);play(&g,N_AFTERIMAGE);play(&g,N_AFTERIMAGE|UPBIT);assert(g.block==1&&g.ninja.v[NS_AFTERIMAGE]==2);
 g.block=0;play(&g,0);assert(g.block==2);g.dexterity=7;g.block=0;play(&g,R_STRIKE_REGENT);assert(g.block==2);
 g.ninja.v[NS_SHADOWMELD]=1;g.block=0;play(&g,C_FLASH_OF_STEEL);assert(g.block==4);g.ninja.v[NS_SHADOWMELD]=0;
 g.block=998;play(&g,0);assert(g.block==999);g.block=0;g.energy=0;play(&g,0);assert(g.block==0&&g.hand_count==1);
 g.energy=99;play(&g,50);assert(g.block==0&&g.hand_count==1);
 g.hand_count=0;g.draw_count=1;g.draw[0]=0;draw_card(&g);assert(g.block==0);
 game_end_turn(&g);assert(g.screen==COMBAT&&g.ninja.v[NS_AFTERIMAGE]==2);g.energy=99;g.block=0;play(&g,N_STRIKE_SILENT);assert(g.block==2);
 char path[]="/tmp/rift-afterimage-save-XXXXXX";int fd=mkstemp(path);assert(fd>=0);close(fd);
 assert(game_save(&g,path)&&game_load(&h,path));h.block=0;h.energy=99;play(&h,R_STRIKE_REGENT);assert(h.block==2);
 puts("PASS first/second activation, stacking, later turns, combat save/load, no Dexterity scaling, Shadowmeld, Block cap, rejected plays and drawing without playing.");
 const int echoes[]={0,N_STRIKE_SILENT,R_STRIKE_REGENT,C_FLASH_OF_STEEL,354};
 for(unsigned i=0;i<sizeof(echoes)/sizeof(*echoes);i++){
  combat(&g,1,1);play(&g,N_AFTERIMAGE);play(&g,echoes[i]|(MOD_ECHO<<CARD_MOD_SHIFT));assert(g.block==2);
 }
 combat(&g,1,1);play(&g,N_AFTERIMAGE);g.exhaust_count=2;g.exhaust[0]=g.exhaust[1]=N_SHIV;play(&g,N_KNIFE_TRAP);assert(g.block==3);
 for(int initial=0;initial<=1;initial++){
  combat(&g,1,1);if(initial)play(&g,N_AFTERIMAGE);g.draw_count=2;g.draw[0]=g.draw[1]=N_AFTERIMAGE;
  play(&g,C_CATASTROPHE);assert(g.ninja.v[NS_AFTERIMAGE]==initial+2);assert(g.block==(initial?4:1));
 }
 puts("PASS Echo replays in every resolver, autoplayed Shivs and nested Afterimage activation: one trigger per play, captured before that card's effects.");
 combat(&g,1,0);play(&g,N_AFTERIMAGE);g.draw_count=2;g.draw[0]=0;g.draw[1]=N_STRIKE_SILENT;
 play(&g,C_SEEKER_STRIKE);assert(g.screen==REGENT_CHOICE&&g.block==0&&g.regent.task_count==2);
 assert(g.regent.tasks[0].kind==RT_AFTERIMAGE&&game_validate(&g));assert(game_save(&g,path)&&game_load(&h,path));
 game_action(&h,BTN_OK);assert(h.screen==COMBAT&&h.block==1&&!h.regent.task_count);unlink(path);
 puts("PASS pending Afterimage survives a colorless card's selection screen and save/load, then awards Block exactly once.");return 0;
#endif
}
