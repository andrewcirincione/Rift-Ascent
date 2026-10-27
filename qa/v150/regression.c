#include "game.h"
#include "sound.h"
#include "render.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern void game_render_set_overlay(int,int,int);
extern void game_render_feedback(const Game*,const Game*,int,unsigned);
extern void game_render_set_status_scroll(int);
static void shot(Game*g,const char*name){
 static uint32_t pixels[W*H];char path[250];
 r138_death_active=0;r137_count=0;hand_exchange=0;fx_transition=0;victory_replay=0;fx_player_damage=0;fx_heal=fx_block=0;fx_enemy_damage[0]=fx_enemy_damage[1]=0;effect_kind=0;rift120_played_card=rift120_auto_card=-1;
 game_render_frame(g,pixels,W,300);
 snprintf(path,sizeof(path),"qa/v150/evidence/native-%s.ppm",name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int i=0;i<W*H;i++){fputc(pixels[i]&255,f);fputc((pixels[i]>>8)&255,f);fputc((pixels[i]>>16)&255,f);}fclose(f);
 snprintf(path,sizeof(path),"qa/v150/evidence/state-%s.dat",name);assert(game_save(g,path));
}
static void battle(Game*g){game_new(g,133);game_start_battle(g,NODE_FIGHT);g->hand_count=5;g->hand[0]=0;g->hand[1]=1;g->hand[2]=25;g->hand[3]=54;g->hand[4]=28;g->energy=5;g->block=12;g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i]=g->enemies[0];g->enemies[i].art=i?19:16;snprintf(g->enemies[i].name,24,"%s",game_enemy_name(g->enemies[i].art));g->enemies[i].block=8;}}
static void tests(void){Game g,t;
 for(unsigned seed=1;seed<2001;seed++){
  game_new(&g,seed);assert(MAP_ROWS+1==19);
  int forks=0,edges=0;
  for(int r=0;r<MAP_ROWS;r++){
   unsigned live=game_route_row_mask(&g,r);assert(__builtin_popcount(live)>=2);
   if(r==MAP_ROWS-1)continue;
   for(int a=0;a<3;a++)if(live&(1u<<a)){
    unsigned ma=game_route_mask(&g,r,a);assert(ma&&ma<8);edges+=__builtin_popcount(ma);forks+=__builtin_popcount(ma)>1;
    for(int b=a+1;b<3;b++)if(live&(1u<<b))for(int da=0;da<3;da++)for(int db=0;db<3;db++)assert(!(ma&(1u<<da))||!(game_route_mask(&g,r,b)&(1u<<db))||da<=db);
    for(int dest=0;dest<3;dest++)if(ma&(1u<<dest)){int x=g.map[r][a],y=g.map[r+1][dest];int ua=x==NODE_EVENT||x==NODE_SHOP||x==NODE_REST||x==NODE_TREASURE;int ub=y==NODE_EVENT||y==NODE_SHOP||y==NODE_REST||y==NODE_TREASURE;assert(!(ua&&ub));}
   }
  }
  assert(forks==4&&edges<55);
  for(int c=0;c<3;c++){int shops=0;for(int r=0;r<MAP_ROWS;r++)shops+=g.map[r][c]==NODE_SHOP;assert(shops>=1&&shops<=2&&g.map[17][c]==NODE_REST);}
 }
 battle(&g);g.screen=SHOP;g.selection=2;g.gold=200;int count=g.deck_count;
 game_action(&g,BTN_ALT);assert(g.screen==DECK_VIEW&&g.deck_readonly);game_action(&g,BTN_OK);assert(g.deck_count==count&&g.gold==200);game_action(&g,BTN_BACK);assert(g.screen==SHOP&&g.selection==2);
 game_action(&g,BTN_POTION);assert(g.screen==POTION_VIEW);game_action(&g,BTN_OK);assert(g.potion_count==2);game_action(&g,BTN_BACK);assert(g.screen==SHOP&&g.selection==2);
 g.selection=4;game_action(&g,BTN_OK);assert(g.screen==DECK_VIEW&&!g.deck_readonly);game_action(&g,BTN_OK);assert(g.deck_count==count-1&&g.gold==140);
 for(int skip=0;skip<2;skip++){
  game_new(&g,133);g.screen=MAP;g.map[0][0]=NODE_TREASURE;int old=g.relic_count;game_choose_node(&g,0);assert(g.screen==RELIC_RESULT&&g.relic_count==old);g.selection=skip;game_action(&g,BTN_OK);assert(g.relic_count==old+!skip&&g.screen==MAP);
 }
 for(int mode=NODE_ELITE;mode<=NODE_BOSS;mode+=NODE_BOSS-NODE_ELITE){
  game_new(&g,133);game_start_battle(&g,mode);int old=g.relic_count;for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;game_end_turn(&g);
  assert(g.screen==RELIC_RESULT&&g.relic_count==old);game_action(&g,BTN_BACK);assert(g.screen==REWARD&&g.relic_count==old);g.selection=3;game_action(&g,BTN_OK);assert(g.screen==(mode==NODE_BOSS?ACT_INTRO:MAP));
 }
 /* A declined cursed relic must not change max HP or Energy. */
 for(int skip=0;skip<2;skip++){
  game_new(&g,133);g.screen=RELIC_RESULT;g.reward[0]=11;g.relic_pending=1;g.relic_resume=MAP;g.selection=skip;
  int hp=g.maxhp,energy=g.max_energy,count=g.relic_count;game_action(&g,BTN_OK);
  assert(g.maxhp==hp-(skip?0:12)&&g.max_energy==energy+!skip&&g.relic_count==count+!skip);
 }
 /* Traverse all 45 floors with deterministic won combats. */
 game_new(&g,133);g.screen=MAP;
 for(int act=1;act<=3;act++){
  assert(g.act==act);
  for(int floor=0;floor<MAP_ROWS;floor++){
   if(g.screen==BLESSING)game_action(&g,BTN_OK);assert(g.floor==floor);g.screen=MAP;g.selection=g.lane;
   int lane=0;while(lane<3&&!game_route_available(&g,lane))lane++;assert(lane<3);g.map[floor][lane]=NODE_FIGHT;game_choose_node(&g,lane);
   assert(g.screen==COMBAT);for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;
   game_end_turn(&g);assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);
  }
  assert(g.floor==MAP_ROWS&&g.screen==MAP);game_action(&g,BTN_OK);
  assert(g.screen==COMBAT&&g.battle_type==NODE_BOSS);
  for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;
  game_end_turn(&g);if(act==3){assert(g.screen==VICTORY);}else{assert(g.screen==RELIC_RESULT);game_action(&g,BTN_BACK);
  assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);}
 }
 assert(g.screen==VICTORY);RiftProfile profile;profile_init(&profile);assert(profile_record(&profile,&g));assert(profile.furthest_floor==57);
 battle(&g);assert(strstr(card_description(25),"discarded card")&&strstr(card_description(25|UPBIT),"draw pile"));
 g.ic[IC_BARRICADE]=1;g.ic[IC_PYRE]=2;g.ic[IC_DARK_EMBRACE]=1;assert(game_status_count(&g)>=3);
 assert(game_save(&g,"qa/v150/evidence/test-save.dat"));assert(game_load(&t,"qa/v150/evidence/test-save.dat"));assert(!memcmp(&g,&t,sizeof(g)));
 RiftSound sound;int16_t pcm[2048];rift_sound_reset(&sound);rift_sound_effect(4);rift_audio_generate(&sound,pcm,1024,0,0);long power=0;for(int i=0;i<2048;i++)power+=abs(pcm[i]);assert(power>500000);rift_sound_effect(6);rift_audio_generate(&sound,pcm,512,0,0);assert(sound.fx_kind==4);
 puts("PASS: 2,000 planar maps, utility spacing, shop browsing/removal, potion viewing, relic take/skip, elite/boss progression, 57-floor campaign, cursed-relic side effects, descriptions, powers, save roundtrip and damage audio");
}
#include "math_tests.h"
#include "visual_tests.h"
#include "animation_tests.h"
#include "patch_tests.h"
#include "audit_tests.h"
#include "inherited_1311_tests.h"
#include "finisher_tests.h"
int main(void){setvbuf(stdout,0,_IOLBF,0);math_tests();dynamic_visual_tests();animation_tests();patch138_tests();tests();patch139_tests();audit1310_tests();patch1311_tests();followup1311_tests();return 0;}
