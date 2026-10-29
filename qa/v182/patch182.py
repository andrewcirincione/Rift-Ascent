"""Apply the 1.8.2 source changes to a fresh copy of qa/v181/source."""
from pathlib import Path
r=Path(__file__).resolve().parent/'source'
def edit(name,old,new):
 p=r/name;s=p.read_text();assert old in s,(name,old[:90]);p.write_text(s.replace(old,new))
edit('game.h','#define GAME_VERSION 181','#define GAME_VERSION 182')
edit('game.h','#define ENEMY_COUNT 46','#define ENEMY_COUNT 47')
edit('game.h','#define INTENT_COUNT 18','#define INTENT_COUNT 24')
edit('game.h','#define EVENT_COUNT 17','#define EVENT_COUNT 24')
edit('game.h',' Expedition181 expedition;',' Expedition181 expedition;\n /* Append-only 1.8.2 state: leave every older save offset intact. */\n struct {int rare_offset,room_pending,room_type;} revision182;')
edit('game.h','int game_event_choice_available','int game_event_choice_count(const Game *g);\nint game_event_choice_available')
edit('game.h','const Game*game_render_audio_state','int game_render_transition_silent(const Game*g,unsigned tick);\nconst Game*game_render_audio_state')
edit('Makefile','RIFT ASCENT 1.8.1','RIFT ASCENT 1.8.2')
edit('Makefile','render.o: expedition.h','render.o: aesthetic_art.h expedition.h')
edit('render.c','#include "expedition_art.h"','#include "expedition_art.h"\n#include "aesthetic_art.h"')
edit('render.c','static unsigned r161_transition_length=48;','static unsigned r161_transition_length=48;\nstatic int r182_combat_transition;')
edit('render.c','r161_transition_active=1;r161_transition_length=48;','r161_transition_active=1;r161_transition_length=48;r182_combat_transition=0;')
edit('render.c','const Game*game_render_audio_state(const Game*g,unsigned tick){','int game_render_transition_silent(const Game*g,unsigned tick){\n return r182_combat_transition&&g->screen==COMBAT&&game_render_transition_busy(tick)&&tick-r161_transition_tick<32;\n}\nconst Game*game_render_audio_state(const Game*g,unsigned tick){')
edit('render.c','if(before->screen!=after->screen){fx_transition=1;fx_transition_tick=tick;}','''if(before->screen!=after->screen){
  /* One full-frame transition; never lay a rectangular partial-screen veil
     over the new screen. Pause and card-choice overlays remain immediate. */
  fx_transition=0;
  if(before->screen!=COMBAT&&after->screen!=PAUSE_MENU&&before->screen!=PAUSE_MENU){
   game_render_begin_transition(before,tick);r161_transition_length=24;
   if(before->screen==MAP&&after->screen==COMBAT&&after->battle_type!=NODE_BOSS){
    r161_transition_length=48;r182_combat_transition=1;
   }
  }
 }''')
# Retain entry/exit cinematics, but generic fade no longer overlays their frames.
p=r/'render.c';s=p.read_text();start=s.index('static void transition_overlay(void){');end=s.index('void game_render_frame(',start);s=s[:start]+s[end:];s=s.replace(' transition_overlay();\n','');s=s.replace('r140_potion_gained=-1;r1310_potion_notice[0]=0;}','r140_potion_gained=-1;r1310_potion_notice[0]=0;r182_combat_transition=0;}');p.write_text(s)
edit('psp_start.c','music_paused=story_audio==1||game_render_music_paused(frame*4u);','music_paused=story_audio==1||game_render_transition_silent(&game,frame*4u)||game_render_music_paused(frame*4u);')
# Menu panel shading and lettering clearance.
edit('render.c','else dark_rect(x+7,74,130,164,65);','else dark_rect(x+7,54,130,180,65);')
p=r/'render.c';s=p.read_text();start=s.index(' const AscentAccount*a=ascent_profile();char b[96];',s.index('static void title_screen'));end=s.index('\n footer(',start);s=s[:start]+' r135_shadow_text(25,236,"RIFT ASCENT 1.8.2",MUTED,&r134_f8);\n'+s[end:];p.write_text(s)
edit('ascent_ui.inc','r134_blit(27,y,426,42,r140_selection,208,28,255);','r134_blit(27,y-2,426,42,r151_selection,420,60,255);')
edit('ascent_ui.inc','r134_text(44,y+7,b','r134_text(57,y+5,b')
edit('ascent_ui.inc','r134_text(44,y+23,b','r134_text(57,y+23,b')
edit('expedition_ui.inc','card_render(20+i*156,46,128,169,g->reward[i],g->selection==i,1);','r162_card_choice=1;card_render(20+i*156,50,128,166,g->reward[i],g->selection==i,1);r162_card_choice=0;')
edit('render.c','int raw=g->reward[g->selection];R135CardLayout l=r135_card_layout(raw,180);\n  for(int w=182;l.height>166&&w<=214;w+=2)l=r135_card_layout(raw,w);\n  card_render(335-l.width/2,81,l.width,l.height,raw,1,1);','int raw=g->reward[g->selection];\n  card_render(265,81,140,165,raw,1,1);')
# Music identities: 5=RiftFight4, 6=RiftFight5.
edit('game.c','mode==NODE_ELITE?(music_hash&1?3:5):(music_hash%3==0?2:music_hash%3==1?4:6)','mode==NODE_ELITE?(music_hash&1?3:6):(music_hash%3==0?2:music_hash%3==1?4:5)')
edit('game.c','return g->combat_music==5?5:3;','return g->combat_music==6?6:3;')
edit('game.c','g->combat_music==6?6:2;','g->combat_music==5?5:2;')
# Completion is consumed once after all room spoils, never by opening inventory.
edit('game.c','static void enter_map(Game*g){','static void enter_map(Game*g){\n if(g->revision182.room_pending){g->revision182.room_pending=0;expedition_roll(g);}')
edit('game.c','static void encounter(Game*g,int node){\n g->selection=0;','static void encounter(Game*g,int node){\n g->revision182.room_type=node;g->revision182.room_pending=node!=NODE_BOSS;\n g->selection=0;')
edit('game.c','g->screen=COMBAT;g->battle_type=mode;','g->screen=COMBAT;g->battle_type=mode;\n g->revision182.room_type=mode;g->revision182.room_pending=mode!=NODE_BOSS;')
edit('game.c','static void begin_act(Game*g){g->screen','static void begin_act(Game*g){g->revision182.room_pending=0;g->revision182.rare_offset=-5;g->screen')
edit('game.c','g->v151.potion_offer=-1;g->seed=seed?','g->revision182.rare_offset=-5;g->v151.potion_offer=-1;g->seed=seed?')
edit('expedition_flow.inc','/* All post-fight stories are queued once, then shown after ordinary spoils. */','/* One 1% story roll after a completed non-boss map room and its spoils. */')
edit('expedition_flow.inc','if(preview_trace||(g->battle_type!=NODE_FIGHT&&g->battle_type!=NODE_ELITE))return;\n if(rnd(g,100)>=5)return;','if(preview_trace||g->revision182.room_type==NODE_BOSS||g->expedition.story_pending)return;\n if(rnd(g,100)!=0)return;')
edit('game.c',' expedition_roll(g);\n fill_rewards(g);g->screen=REWARD;g->selection=0;',' g->screen=REWARD;g->selection=0;')
edit('game.c','}else msg(g,"Victory! Choose one reward.");','}else{fill_rewards(g);msg(g,"Victory! Choose one reward.");}')
# Encounter rarity follows the STS2 base bands and per-card non-rare offset.
p=r/'game.c';s=p.read_text();start=s.index('/* Events and shops keep their rarity rates;');end=s.index('static void add_relic(',start);s=s[:start]+'''/* STS2 base bands, with a persisted -5..40 rare offset. Ancient cards are
   reserved for character story gifts and explicit mystery rewards. */
static int roll_rarity(Game*g,int rare_base,int uncommon,int use_offset,int advance){
 int rare=rare_base+(use_offset?g->revision182.rare_offset:0),roll=rnd(g,100);
 int result=roll<rare?2:roll<rare+uncommon?1:0;
 if(advance)g->revision182.rare_offset=result==2?-5:MIN(40,g->revision182.rare_offset+1);
 return result;
}
static int card_reward_except(Game*g,int a,int b){return reward_of_rarity(g,roll_rarity(g,9,37,1,0),a,b);}
static int card_reward(Game*g){return reward_of_rarity(g,roll_rarity(g,3,34,0,0),-1,-1);}
static void fill_rewards(Game*g){
 for(int i=0;i<3;i++){
  int rare=g->battle_type==NODE_BOSS?2:roll_rarity(g,g->battle_type==NODE_ELITE?10:3,g->battle_type==NODE_ELITE?40:37,1,1);
  if(g->battle_type==NODE_BOSS)g->revision182.rare_offset=-5;
  int c=reward_of_rarity(g,rare,i>0?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1);
  int upgraded=relic(g,13)||(rare<2&&rnd(g,100)<MIN(2,g->act-1)*25);
  g->reward[i]=c|(upgraded?UPBIT:0);
 }
}
''' +s[end:];p.write_text(s)
# The first 28,420 bytes retain 1.8.1 offsets. Both alternating generations load.
edit('game.c','else if(version==180){','else if(version==181){if(n!=offsetof(Game,revision182))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}\n else if(version==180){')
edit('game.c','if(version<23&&!migrate_raw22(&temp))return 0;','if(version<23&&!migrate_raw22(&temp))return 0;\n if(version<182){temp.revision182.rare_offset=-5;revision182_migrate_room(&temp);}')
edit('game.c','static int expedition_relic(Game*);','static int expedition_relic(Game*);\nstatic void revision182_migrate_room(Game*);')
edit('game.c','if(g->act<1||g->act>100000','if(g->revision182.rare_offset< -5||g->revision182.rare_offset>40||g->revision182.room_pending<0||g->revision182.room_pending>1||g->revision182.room_type<0||g->revision182.room_type>NODE_BOSS)return 0;\n if(g->act<1||g->act>100000')
p=r/'ascent_storage.inc';s=p.read_text();start=s.index('static int ascent_read_run(');end=s.index('int ascent_resume(',start);s=s[:start]+'''static void revision182_migrate_room(Game*g){
 int s=g->screen;if(s==DECK_VIEW||s==POTION_VIEW||s==RELIC_VIEW)s=g->prior_screen;
 int in_room=s==COMBAT||s==REWARD||s==RELIC_RESULT||s==EVENT||s==EVENT_CARD||s==EVENT_RESULT||s==SHOP||s==REST||s==MYSTERY_STORY||s==MYSTERY_CARDS;
 if(in_room&&g->floor>0){int lane=CLAMP(g->map_selected[g->floor-1],0,2);int node=g->map[g->floor-1][lane];
  g->revision182.room_type=node;g->revision182.room_pending=node!=NODE_BOSS&&!g->win_screen&&!g->expedition.story_pending;
 }
 if(g->battle_type==NODE_ELITE&&g->combat_music==5)g->combat_music=6;
 else if(g->battle_type==NODE_FIGHT&&g->combat_music==6)g->combat_music=5;
}
static int ascent_read_run(const char*stem,unsigned side,Game*g,unsigned*generation){
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,sizeof(*g),generation))return game_validate(g);
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,offsetof(Game,revision182),generation)&&g->version==181){
  g->version=GAME_VERSION;g->revision182.rare_offset=-5;revision182_migrate_room(g);return game_validate(g);
 }
 memset(g,0,sizeof(*g));
 if(!ascent_read(stem,side,g,offsetof(Game,expedition),generation)||g->version!=180||g->relic_count>64)return 0;
 g->version=GAME_VERSION;g->revision182.rare_offset=-5;revision182_migrate_room(g);
 if(g->screen==RELIC_RESULT&&g->relic_pending&&g->reward[0]<0)g->reward[0]=37;
 return game_validate(g);
}
''' +s[end:];p.write_text(s)
print('Applied UI, audio routing, rarity, room stories and append-only save migration')
