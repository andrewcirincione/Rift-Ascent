#include "game.h"
#include "render.c"
#include <assert.h>
static uint32_t pixels[480*272];
static void shot(Game*g,const char*name){
 r137_count=r1311_finishing=victory_replay=hand_exchange=fx_transition=0;rift120_played_card=rift120_auto_card=-1;
 game_render_frame(g,pixels,480,300);char file[256];snprintf(file,sizeof(file),"qa/v150/evidence/native-%s.ppm",name);FILE*f=fopen(file,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");for(int i=0;i<480*272;i++){fputc(pixels[i]&255,f);fputc(pixels[i]>>8&255,f);fputc(pixels[i]>>16&255,f);}fclose(f);
 snprintf(file,sizeof(file),"qa/v150/evidence/state-%s.dat",name);assert(game_save(g,file));
}
static void arena(Game*g){game_new(g,150);game_start_battle(g,NODE_FIGHT);g->enemy_count=2;g->enemies[1]=g->enemies[0];for(int i=0;i<2;i++){g->enemies[i].art=16+i*3;snprintf(g->enemies[i].name,24,"%s",game_enemy_name(g->enemies[i].art));g->enemies[i].hp=g->enemies[i].maxhp=150;}g->energy=5;g->selection=0;}
int main(void){Game g;char name[80];int ids[]={0,54,27,16,9,5};
 game_new(&g,150);shot(&g,"title");
 for(int id=0;id<EVENT_COUNT;id++)for(int option=0;option<3;option++){
  game_new(&g,150+id);g.gold=200;g.hp=45;game_begin_event(&g,id);g.selection=option;
  snprintf(name,sizeof(name),"event-%02d-%d",id,option);shot(&g,name);
  if((id>=12&&id<=14)||id==16||id==10){
   if(option<2){game_action(&g,BTN_OK);assert(g.screen==EVENT_CARD);snprintf(name,sizeof(name),"picker-%02d-%d",id,option);shot(&g,name);game_action(&g,BTN_OK);snprintf(name,sizeof(name),"result-%02d-%d",id,option);shot(&g,name);}
  }
 }
 for(int id=0;id<EVENT_COUNT;id++){
  game_new(&g,150+id);g.gold=200;g.hp=45;game_begin_event(&g,id);game_action(&g,BTN_OK);if(g.screen==EVENT_CARD)game_action(&g,BTN_OK);snprintf(name,sizeof(name),"outcome-%02d",id);shot(&g,name);
 }
 arena(&g);g.hand_count=7;for(int m=1;m<MOD_COUNT;m++)g.hand[m-1]=ids[m-1]|(m<<CARD_MOD_SHIFT);g.hand[6]=13;
 for(int i=0;i<7;i++){g.selection=i;snprintf(name,sizeof(name),"combat-mod-%d",i+1);shot(&g,name);}
 arena(&g);g.hand_count=1;g.hand[0]=0|(MOD_ECHO<<CARD_MOD_SHIFT);g.energy=2;g.ic[IC_NEXT_ATTACK_REPEAT]=1;shot(&g,"echo-ready");
 arena(&g);g.hand_count=3;g.hand[0]=0|(MOD_BOUND<<CARD_MOD_SHIFT);g.hand[1]=1|(MOD_WARD<<CARD_MOD_SHIFT);g.hand[2]=5|(MOD_KINDLED<<CARD_MOD_SHIFT);g.enemies[0].intent=g.enemies[1].intent=1;g.enemies[0].intent_value=g.enemies[1].intent_value=2;shot(&g,"retain-ready");
 arena(&g);g.relics[g.relic_count++]=40;g.relics[g.relic_count++]=41;g.relics[g.relic_count++]=42;g.relics[g.relic_count++]=43;g.hand_count=4;g.hand[0]=0;g.hand[1]=1;g.hand[2]=6;g.hand[3]=9;g.energy=7;shot(&g,"relic-combat-ready");
 game_render_set_overlay(1,2,0);shot(&g,"relic-status");game_render_set_overlay(0,0,0);
 for(int id=40;id<44;id++){g.screen=RELIC_RESULT;g.reward[0]=id;g.relic_pending=0;g.win_screen=0;snprintf(name,sizeof(name),"relic-%d",id);shot(&g,name);}

 for(int id=36;id<44;id++){
  arena(&g);g.enemy_count=1;g.enemies[0].art=id;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(id));g.selection=g.hand_count;snprintf(name,sizeof(name),"enemy-%02d",id);shot(&g,name);
 }
 for(int intent=14;intent<18;intent++){
  arena(&g);g.enemy_count=1;g.enemies[0].art=36+intent-14;g.enemies[0].intent=intent;g.enemies[0].intent_value=12;g.block=20;g.hand_count=0;g.selection=0;snprintf(name,sizeof(name),"intent-%02d",intent);shot(&g,name);
 }
 for(int id=92;id<96;id++){
  arena(&g);g.hand[0]=id;g.selection=0;g.enemies[0].weak=2;g.enemies[0].block=12;g.hp=55;snprintf(name,sizeof(name),"card-%02d",id);shot(&g,name);
 }
 for(int seed=1;seed<=4;seed++)for(int row=0;row<=15;row+=5){
  game_new(&g,seed);g.screen=MAP;g.scroll=row;unsigned visible=game_route_visible_mask(&g,row);while(!(visible&(1u<<g.selection)))g.selection++;snprintf(name,sizeof(name),"map-%d-%02d",seed,row);shot(&g,name);
 }
 arena(&g);g.hand_count=6;g.hand[0]=92|(MOD_ECHO<<CARD_MOD_SHIFT);g.hand[1]=93|(MOD_WARD<<CARD_MOD_SHIFT);g.hand[2]=94|(MOD_KEEN<<CARD_MOD_SHIFT);g.hand[3]=95|(MOD_BOUND<<CARD_MOD_SHIFT);g.hand[4]=5|(MOD_KINDLED<<CARD_MOD_SHIFT);g.hand[5]=9|(MOD_FLEETING<<CARD_MOD_SHIFT);g.selection=2;g.enemies[0].art=38;g.enemies[1].art=41;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(38));snprintf(g.enemies[1].name,24,"%s",game_enemy_name(41));g.enemies[0].intent=15;g.enemies[0].intent_value=12;g.enemies[1].intent=17;g.enemies[1].intent_value=12;shot(&g,"combat-release");
 int count=0,worst=0;for(int mod=1;mod<MOD_COUNT;mod++)for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++){
  int raw=id|(up?UPBIT:0);if(!game_can_modify(raw,mod))continue;raw|=mod<<CARD_MOD_SHIFT;
  card_combat_context=0;R135CardLayout l=r135_card_layout(raw,126);if(l.height>worst)worst=l.height;
  if(l.height>176)printf("TALL id=%d mod=%d up=%d h=%d\n",id,mod,up,l.height);
  count++;
 }
 card_combat_context=0;printf("PASS rendered 17 rooms, 10 card pickers/results, six modification seals, four relics; %d modified layouts, tallest %d\n",count,worst);
}
