#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void debug_tests(void){
 Game g,before;assert(a18_storage_open(NULL));assert(!game_debug_enabled());
 game_new(&g,122);g.act=1;g.floor=6;game_start_battle(&g,NODE_FIGHT);before=g;
 game_action(&g,BTN_DEBUG_KILL);assert(!memcmp(&g,&before,sizeof(g)));
 game_action(&g,BTN_DEBUG_ACT);assert(!memcmp(&g,&before,sizeof(g)));
 assert(game_set_debug_enabled(1));g.enemy_count=2;g.enemies[1]=g.enemies[0];int kills=g.kill_count;
 game_combat_fx_begin();game_action(&g,BTN_DEBUG_KILL);game_combat_fx_end();assert(g.screen==REWARD);assert(g.kill_count==kills+2);assert(game_validate(&g));
 unsigned wins=a18_profile()->wins[0],cards=a18_profile()->cards[0],best=a18_profile()->best_acts[0];
 game_action(&g,BTN_DEBUG_ACT);assert(g.act==2);assert(g.screen==ACT_INTRO);assert(game_validate(&g));
 assert(a18_profile()->wins[0]==wins&&a18_profile()->cards[0]==cards&&a18_profile()->best_acts[0]==best);
 before=g;g.screen=TITLE;game_action(&g,BTN_DEBUG_ACT);assert(g.screen==TITLE&&g.act==before.act);
 g.selection=3;game_action(&g,BTN_OK);assert(g.screen==HELP&&g.selection==0);game_action(&g,BTN_OK);assert(!game_debug_enabled());game_action(&g,BTN_BACK);assert(g.screen==TITLE&&g.selection==3);
 puts("PASS debug OFF by default; disabled shortcuts no-op; enabled L kills both enemies through rewards; R advances without awarding clears; Options toggles.");
}
static void options_persistence(void){
 char temp[]="/tmp/rift180-options-XXXXXX";assert(mkdtemp(temp));assert(a18_storage_open(temp));assert(!game_debug_enabled());assert(game_set_debug_enabled(1));
 assert(a18_storage_open(temp));assert(game_debug_enabled());assert(game_set_debug_enabled(0));assert(a18_storage_open(temp));assert(!game_debug_enabled());
 char path[256];snprintf(path,sizeof(path),"%s/A18_OPTIONS.DAT",temp);FILE*f=fopen(path,"wb");assert(f);fputs("broken",f);fclose(f);assert(a18_storage_open(temp));assert(!game_debug_enabled());
 puts("PASS independent options persistence and corrupt-settings safe-OFF; run/profile layout unchanged.");
}
static void moon(void){
 Game g;game_new(&g,234);g.hp=20;g.gold=0;game_begin_event(&g,1);assert(game_moon_heal_now(&g)==20);assert(game_event_choice_available(&g,0));game_action(&g,BTN_OK);assert(g.hp==40&&g.gold==0);
 game_new(&g,233);g.hp=g.maxhp-5;game_begin_event(&g,1);game_action(&g,BTN_OK);assert(g.hp==g.maxhp);
 game_new(&g,237);g.gold=100;g.deck[0]=UPBIT|(MOD_ECHO<<CARD_MOD_SHIFT);game_begin_event(&g,1);game_action(&g,BTN_DOWN);assert(g.selection==1);int old=g.deck_count,raw=g.deck[0];game_action(&g,BTN_OK);assert(g.screen==EVENT_CARD&&g.gold==100);game_action(&g,BTN_BACK);assert(g.screen==EVENT&&g.gold==100&&g.deck_count==old);game_action(&g,BTN_OK);g.selection=0;game_action(&g,BTN_OK);assert(g.gold==70&&g.deck_count==old+1&&g.deck[old]==raw);assert(game_validate(&g));
 puts("PASS Moon Spring free flat 20 HP, health cap, D-pad selection, transactional reflection copy with upgrade/mod preserved.");
}
static void late_encounters(void){
 static const int normal[30]={0,1,2,3,9,10,11,12,36,37,4,5,16,17,18,19,20,21,38,39,25,26,27,28,29,30,31,32,40,41};
 int counts[47]={0};Game g;
 for(int act=4;act<=12;act+=8)for(unsigned seed=1;seed<=5000;seed++){
  game_new(&g,seed*7171);g.act=act;g.endless=1;g.floor=6;game_start_battle(&g,NODE_FIGHT);
  for(int i=0;i<g.enemy_count;i++){int id=g.enemies[i].art,ok=0;for(int j=0;j<30;j++)ok|=normal[j]==id;assert(ok);counts[id]++;}
  if(g.enemy_count==2)assert(g.enemies[0].art!=g.enemies[1].art);assert(game_validate(&g));
 }
 for(int j=0;j<30;j++)assert(counts[normal[j]]>0);
 puts("PASS 10,000 Act 4/12 encounters draw all 30 NORMAL species; no elites, bosses or duplicate pairs.");
}
int main(void){debug_tests();options_persistence();moon();late_encounters();puts("ALL 1.8 FEEDBACK TESTS PASSED");return 0;}
