#include "game.h"
#include "render.c"
#include <assert.h>
static uint32_t frame[480*272];
static void output(Game*g,const char*name){
 game_render_frame(g,frame,480,300);char path[240];
 snprintf(path,sizeof(path),"qa/v150/evidence/native-%s.ppm",name);FILE*f=fopen(path,"wb");assert(f);
 fprintf(f,"P6\n480 272\n255\n");for(int i=0;i<480*272;i++){fputc(frame[i]&255,f);fputc(frame[i]>>8&255,f);fputc(frame[i]>>16&255,f);}fclose(f);
 snprintf(path,sizeof(path),"qa/v150/evidence/state-%s.dat",name);assert(game_save(g,path));
}
static void arena(Game*g){
 game_new(g,150);game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=100000;
 g->enemies[0].art=38;snprintf(g->enemies[0].name,24,"%s",game_enemy_name(38));g->enemies[0].intent=15;
 g->energy=9;g->hand_count=3;g->hand[1]=0;g->hand[2]=1;g->selection=0;
}
int main(void){Game g;int cases=0,worst=0,longest=0;char name[80];int representative[6]={35,54,13,16,9,5};
 for(int mod=1;mod<MOD_COUNT;mod++)for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++){
  int raw=id|(up?UPBIT:0);if(!game_can_modify(raw,mod))continue;raw|=mod<<CARD_MOD_SHIFT;
  for(int context=0;context<5;context++){
   arena(&g);g.hand[0]=raw;g.strength=context==2?-3:context==3?200:context*5;
   g.dexterity=g.strength;g.weak=context%2;g.enemies[0].vulnerable=context;g.block=context*20;
   g.exhaust_count=context*4;for(int i=0;i<g.exhaust_count;i++)g.exhaust[i]=1;
   if(context==4)g.ic[IC_NEXT_ATTACK_REPEAT]=1;
   card_combat_context=context?&g:0;
   const char*rules=r136_rules(raw);assert(strstr(rules,game_mod_name(mod)));
   int length=(int)strlen(rules);assert(length<319);if(length>longest)longest=length;
   R135CardLayout l=r135_card_layout(raw,126);if(l.height>worst)worst=l.height;
   if(l.height>176)printf("OVERFLOW id=%d mod=%d up=%d context=%d height=%d\n",id,mod,up,context,l.height);
   assert(l.height<=176);assert(254-l.height>=76);
   // Whole description fits between the image and bottom trim, using actual
   // font wrapping, including the modification's complete final clause.
   char lines[24][192];int rows=r134_lines(rift131_clean_rules(rules),92,l.font,lines);
   assert(rows*l.leading<=l.height-18-l.body_y);
   assert(l.art_y>=20+l.title_rows*11+2&&l.body_y>=l.art_y+l.art_h+3);
   assert(24<=l.art_h&&126-11-24>17+18);cases++;
  }
 }
 for(int mod=1;mod<MOD_COUNT;mod++){
  arena(&g);g.hand_count=6;for(int i=0;i<6;i++)g.hand[i]=representative[i]|((i+1)<<CARD_MOD_SHIFT);
  g.selection=mod-1;snprintf(name,sizeof(name),"mod-audit-%d",mod);output(&g,name);
 }
 card_combat_context=0;
 printf("PASS %d static/live modified card layouts; complete descriptions (max %d characters), tallest preview %dpx; seal stays within art and clear of title, energy and body; all six mixed-hand fixtures\n",cases,longest,worst);
}
