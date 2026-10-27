#include "game.h"
#include "render.c"
#include <assert.h>
static uint32_t frame[480*272],hud[480*76];
static void save_fixture(Game*g,const char*name){
 char path[240];snprintf(path,sizeof(path),"qa/v150r2/evidence/state-height-%s.dat",name);assert(game_save(g,path));
 game_render_frame(g,frame,480,300);snprintf(path,sizeof(path),"qa/v150r2/evidence/native-height-%s.ppm",name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int i=0;i<480*272;i++){fputc(frame[i]&255,f);fputc(frame[i]>>8&255,f);fputc(frame[i]>>16&255,f);}fclose(f);
}
static void arena(Game*g,int raw,int context){
 game_new(g,150);game_start_battle(g,NODE_FIGHT);g->enemy_count=2;g->enemies[1]=g->enemies[0];
 for(int i=0;i<2;i++){g->enemies[i].hp=g->enemies[i].maxhp=100000000;g->enemies[i].block=0;}
 g->hand_count=3;g->hand[0]=raw;g->hand[1]=0;g->hand[2]=1;g->selection=g->target=0;g->energy=6;
 if(context>=2){g->strength=context==2?20:context==3?200:context==4?9999:100000000;g->dexterity=context==2?10:context==3?50:9999;g->weak=1;
  g->enemies[0].vulnerable=context==2?4:context==3?100:context==4?99999:100000000;g->enemies[1].vulnerable=0;
  g->block=999;g->exhaust_count=128;for(int i=0;i<128;i++)g->exhaust[i]=1;
  g->ic[IC_HPLOSS_COMBAT]=20;g->ic[IC_HPLOSS_TURN]=1;g->ic[IC_NEXT_ATTACK_REPEAT]=context>=3;}
 if(context==5)g->energy=0;
 if(context>=6)g->ic[IC_UNMOVABLE]=1;
 if(context==7)g->energy=1000;
}
int main(void){setvbuf(stdout,0,_IOLBF,0);Game g,highest[8],widest;int worst[8]={0},min=999,maxwidth=0,cases=0,clipped=0,badge_overlap=0;char lines[24][192];
 for(int mod=0;mod<MOD_COUNT;mod++)for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++){
  int raw=id|(up?UPBIT:0);if(mod&&!game_can_modify(raw,mod))continue;raw|=mod<<CARD_MOD_SHIFT;
  for(int context=0;context<8;context++){
   arena(&g,raw,context);card_combat_context=context?&g:0;
   const char*rules=r136_rules(raw);R135CardLayout l=r150_hand_preview_layout(raw);
   int rows=r134_lines(rift131_clean_rules(rules),l.width-34,l.font,lines);
   if(rows*l.leading>l.height-l.body_y-18||l.title_rows>2)clipped++;
   if(context==5&&game_card_cost_now(&g,raw)>g.energy&&254-l.height-16<76)badge_overlap++;
   if(l.height>worst[context]){worst[context]=l.height;highest[context]=g;}
   if(l.width>maxwidth){maxwidth=l.width;widest=g;}
   if(l.height<min)min=l.height;
   assert(l.height<=162&&l.width<=160&&l.art_h>=24);assert(strlen(rules)<319);if(mod)assert(strstr(rules,game_mod_name(mod)));
   cases++;
  }
 }
 for(int c=0;c<8;c++){int raw=highest[c].hand[0];card_combat_context=c?&highest[c]:0;R135CardLayout l=r150_hand_preview_layout(raw);printf("context=%d max_height=%d width=%d card=%s upgraded=%d mod=%s\n",c,worst[c],l.width,cards[CARD_ID(raw)].name,CARD_UP(raw),game_mod_name(CARD_MOD(raw)));}
 printf("AUDIT %d layouts: min=%d, max_width=%d, clipped=%d, insufficient-energy badge/HUD collisions=%d\n",cases,min,maxwidth,clipped,badge_overlap);
 // Representative longest normal and extreme previews, plus short Strike.
 card_combat_context=0;for(int c=1;c<8;c++){char name[32];snprintf(name,sizeof(name),"long-%d",c);save_fixture(&highest[c],name);}
 save_fixture(&widest,"wide");
 // An ordinary late-run example for a readable combat screenshot.
 g=highest[3];for(int i=0;i<2;i++)g.enemies[i].hp=g.enemies[i].maxhp=5000;
 {int raw=g.hand[0];g.hand_count=5;g.hand[0]=0;g.hand[1]=1;g.hand[2]=raw;g.hand[3]=93|(MOD_WARD<<CARD_MOD_SHIFT);g.hand[4]=94|(MOD_KEEN<<CARD_MOD_SHIFT);g.selection=2;g.exhaust_count=16;}
 save_fixture(&g,"combat");
 arena(&g,0,1);save_fixture(&g,"short");
 int rendered=0;for(int c=1;c<8;c++)for(int slot=0;slot<10;slot++){
  g=highest[c];int raw=g.hand[0];g.hand_count=10;for(int i=0;i<10;i++)g.hand[i]=0;g.hand[slot]=raw;
  g.selection=10;rift120_played_card=rift120_auto_card=-1;game_render_frame(&g,frame,480,300);memcpy(hud,frame,sizeof(hud));
  g.selection=slot;game_render_frame(&g,frame,480,300);assert(!memcmp(hud,frame,sizeof(hud)));rendered++;
  for(int age=0;age<=38;age++){
   rift120_played_card=rift120_auto_card=-1;game_render_frame(&g,frame,480,300+age);memcpy(hud,frame,sizeof(hud));
   rift120_played_card=raw;rift120_play_tick=300;rift133_play_x=56+(slot%5)*71;r136_play_state_valid=0;
   game_render_frame(&g,frame,480,300+age);assert(!memcmp(hud,frame,sizeof(hud)));rendered++;
  }
 }
 rift120_played_card=rift120_auto_card=-1;
 assert(!clipped);assert(!badge_overlap);
 printf("PASS all complete descriptions fit; %d rendered hand/animation frames preserve every HUD pixel, including the Energy warning\n",rendered);
}
