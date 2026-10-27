#include "game.h"
#include "render.c"
#include <assert.h>
#include <stddef.h>
static uint32_t frame[W*H];
static void snap(Game*g,const char*name){char p[256];game_render_reset_fx();game_render_frame(g,frame,W,400);snprintf(p,sizeof(p),"qa/v170/evidence/native-%s.ppm",name);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(frame[i]&255,f);fputc(frame[i]>>8&255,f);fputc(frame[i]>>16&255,f);}assert(!fclose(f));snprintf(p,sizeof(p),"qa/v170/evidence/state-%s.dat",name);assert(game_save(g,p));}
static void setup(Game*g,int c,int act){game_new(g,170170u+c*31u);game_choose_character(g,c);g->act=act;g->endless=act>3;g->screen=MAP;g->floor=act<=3?MAP_ROWS:0;g->selection=1;}
static int silent(const short*p,int n){for(int i=0;i<n;i++)if(p[i])return 0;return 1;}
int main(void){Game g,b,loaded;char name[80],rows[24][192];int pages=0;
 for(int c=0;c<3;c++)for(int act=1;act<=4;act++)for(int v=0;v<10;v++){
  setup(&g,c,act);g.floor=0;unsigned rng=g.rng;game_story_start(&g,0);g.scroll=v;assert(g.screen==STORY_ACT&&g.rng==rng&&game_validate(&g));assert(r134_lines(game_story_text(&g),336,&r134_f9,rows)<=4);assert(game_story_kind(&g)==1);
  b=g;int ignored[]={BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT,BTN_BACK,BTN_POTION,BTN_ALT,BTN_START,BTN_TAB_LEFT,BTN_TAB_RIGHT};for(int k=0;k<10;k++){game_action(&g,ignored[k]);assert(!memcmp(&g,&b,sizeof(g)));}
  snprintf(name,sizeof(name),"entry-%d-%d-%d",c,act,v);snap(&g,name);pages++;
  game_action(&g,BTN_PAUSE);assert(g.screen==PAUSE_MENU&&game_story_kind(&g)==1);game_action(&g,BTN_BACK);assert(!memcmp(&g,&b,sizeof(g)));game_action(&g,BTN_OK);assert(g.screen==MAP&&g.rng==rng&&game_music_track(&g)==1);
 }
 int ids[]={15,24,8};
 for(int c=0;c<3;c++)for(int act=1;act<=3;act++)for(int v=0;v<5;v++){
  setup(&g,c,act);Game plain=g;unsigned rng=g.rng;game_action(&g,BTN_OK);assert(g.screen==STORY_BOSS&&g.rng==rng);g.scroll=v;assert(game_story_kind(&g)==2&&game_music_track(&g)==0);
  for(int page=0;page<3;page++){assert(g.selection==page&&game_validate(&g));assert(r134_lines(game_story_text(&g),336,&r134_f9,rows)<=4);assert(page!=1||!strcmp(game_story_speaker(&g),game_enemy_name(ids[act-1])));snprintf(name,sizeof(name),"boss-%d-%d-%d-%d",c,act,v,page);snap(&g,name);pages++;assert(game_load(&loaded,"qa/v170/evidence/save-test.dat")==0);assert(game_save(&g,"qa/v170/evidence/roundtrip.dat"));assert(game_load(&loaded,"qa/v170/evidence/roundtrip.dat")&&!memcmp(&g,&loaded,sizeof(g)));game_action(&g,BTN_OK);}
  assert(g.screen==COMBAT&&g.enemies[0].art==ids[act-1]&&game_music_track(&g)==0);game_start_battle(&plain,NODE_BOSS);assert(g.rng==plain.rng&&g.hp==plain.hp&&g.hand_count==plain.hand_count&&!memcmp(g.hand,plain.hand,sizeof(g.hand))&&!memcmp(g.enemies,plain.enemies,sizeof(g.enemies)));
  if(!v){g.selection=g.hand_count;snprintf(name,sizeof(name),"fight-%d-%d",c,act);snap(&g,name);}
 }
 assert(pages==255);
 for(int c=0;c<3;c++)for(int act=4;act<=30;act++){setup(&g,c,act);g.floor=MAP_ROWS;game_action(&g,BTN_OK);assert(g.screen==COMBAT&&game_music_track(&g)==0);}
 int coverage[3][4][10]={0};for(int seed=1;seed<=1000;seed++)for(int c=0;c<3;c++)for(int a=1;a<=4;a++){setup(&g,c,a);g.seed=seed;unsigned rng=g.rng;game_story_start(&g,0);assert(g.rng==rng);coverage[c][a-1][g.scroll]++;}
 for(int c=0;c<3;c++)for(int a=0;a<4;a++)for(int v=0;v<10;v++)assert(coverage[c][a][v]>45&&coverage[c][a][v]<160);
 /* Full black midpoint, audio matches the visible half, and buffered X never
    leaks from a scene into the following map/combat. */
 setup(&b,0,1);b.floor=0;b.screen=BLESSING;g=b;game_story_start(&g,0);game_render_reset_fx();game_render_feedback(&b,&g,BTN_OK,100);
 assert(game_render_transition_busy(100)&&game_render_audio_state(&g,115)->screen==BLESSING&&game_render_audio_state(&g,116)->screen==STORY_ACT);
 game_render_frame(&g,frame,W,116);for(int i=0;i<W*H;i++)assert((frame[i]&0xffffff)==0);
 for(int i=0;i<3;i++)assert(game_render_story_input(&g,BTN_OK,110+i)==0);assert(game_render_story_input(&g,0,132)==BTN_OK);Game before=g;game_action(&g,BTN_OK);game_render_feedback(&before,&g,BTN_OK,132);assert(g.screen==MAP&&game_render_story_input(&g,0,164)==0&&g.floor==0);
 setup(&b,2,2);g=b;game_story_start(&g,1);game_render_reset_fx();game_render_feedback(&b,&g,BTN_OK,200);assert(game_music_track(game_render_audio_state(&g,215))==1&&game_music_track(game_render_audio_state(&g,216))==0);
 for(int page=0;page<3;page++){before=g;game_action(&g,BTN_OK);game_render_feedback(&before,&g,BTN_OK,240+page*40);}
 assert(g.screen==COMBAT);assert(game_music_track(game_render_audio_state(&g,320))==0&&game_music_track(game_render_audio_state(&g,336))==0);
 /* Title has exactly three choices, and an old Endless run stays resumable. */
 game_new(&g,170);g.selection=0;for(int i=0;i<3;i++)game_action(&g,BTN_DOWN);assert(g.selection==0);game_action(&g,BTN_UP);assert(g.selection==2);game_action(&g,BTN_OK);assert(g.screen==HELP);game_action(&g,BTN_BACK);g.selection=1;g.endless=1;game_set_resume_available(0);game_action(&g,BTN_OK);assert(g.screen==CHARACTER_SELECT&&!g.endless);game_action(&g,BTN_OK);assert(g.screen==ACT_INTRO);game_action(&g,BTN_OK);assert(g.screen==BLESSING);game_action(&g,BTN_OK);assert(g.screen==STORY_ACT);game_action(&g,BTN_OK);assert(g.screen==MAP);
 setup(&g,2,8);assert(game_save(&g,"qa/v170/evidence/roundtrip.dat")&&game_load(&loaded,"qa/v170/evidence/roundtrip.dat")&&loaded.endless&&loaded.act==8);
 for(int c=0;c<3;c++)for(int a=1;a<=4;a++){setup(&g,c,a);g.floor=0;g.screen=ACT_INTRO;g.scroll=119;game_update(&g);assert(g.screen==BLESSING);snprintf(name,sizeof(name),"blessing-%d-%d",c,a);snap(&g,name);if(a<=3){setup(&g,c,a);snprintf(name,sizeof(name),"map-boss-%d-%d",c,a);snap(&g,name);}}
 game_new(&g,170);snap(&g,"title");
 /* Scene mode cancels all prior sounds; only X's confirm sound is accepted. */
 RiftSound sound;static short audio[4096];rift_sound_reset(&sound);rift_sound_effect(9);rift_sound_effect(11);rift_sound_impact(40,1);rift_audio_generate(&sound,audio,512,1,1);assert(!silent(audio,1024));rift_sound_scene_mode(1);rift_audio_generate(&sound,audio,512,1,1);assert(silent(audio,1024));for(int k=1;k<=11;k++)if(k!=7)rift_sound_effect(k);rift_sound_impact(30,1);rift_audio_generate(&sound,audio,512,1,1);assert(silent(audio,1024));rift_sound_effect(7);rift_audio_generate(&sound,audio,512,1,1);assert(!silent(audio,1024));rift_sound_scene_mode(2);rift_audio_generate(&sound,audio,512,0,1);assert(silent(audio,1024));rift_sound_effect(7);rift_audio_generate(&sound,audio,512,0,1);assert(!silent(audio,1024));rift_sound_scene_mode(0);
 FILE*f=fopen("qa/v170/evidence/state-layout.json","w");fprintf(f,"{\"size\":%zu",sizeof(Game));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(character);OFF(selection);OFF(scroll);OFF(act);OFF(floor);OFF(rng);OFF(hand);OFF(hand_count);OFF(energy);OFF(hp);OFF(potion_count);OFF(deck_count);OFF(enemies);OFF(regent);OFF(turn_played);OFF(turn);
 fprintf(f,"}\n");fclose(f);
 puts("PASS 255 dialogue pages fit; 120 entry variants, 45 exchanges, all boss identities, X/shortcut/save controls, 12000 seeded choices and Endless bypass");
 puts("PASS black-midpoint fades, audio-state handoffs, bounded queued X, three-option title, Endless save compatibility and scene SFX isolation");return 0;}
