/* Pixel-level regression: real combat actions and the complete renderer.
   Sound output is stubbed; gameplay, artwork and timing code are unchanged. */
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#ifdef RENDER_SOURCE
#include RENDER_SOURCE
#else
#include "../v186/source/render.c"
#endif
void rift_sound_effect(int kind){(void)kind;}
void rift_sound_impact(int amount,int blocked){(void)amount;(void)blocked;}
static uint32_t frame[480*272],expected[480*272];
static void setup(Game*g,int act){assert(ascent_begin(g,0,0,18602));g->act=act;g->endless=act>3;g->floor=MAP_ROWS;g->ascent.secret=act==4;game_start_battle(g,NODE_BOSS);g->relic_count=0;g->hp=g->maxhp=1000;g->hand_count=3;g->hand[0]=g->hand[1]=g->hand[2]=0;g->energy=10;g->selection=g->target=0;for(int i=0;i<g->enemy_count;i++){g->enemies[i].hp=1;g->enemies[i].block=0;}}
static void kill(Game*g,int target,unsigned tick){Game before=*g;g->target=target;before.target=target;game_combat_fx_begin();game_play_card(g,0,target);game_combat_fx_end();game_render_feedback(&before,g,BTN_OK,tick);}
static unsigned brightness(const uint32_t*p){unsigned sum=0;for(int i=0;i<480*272;i++)sum+=(p[i]&255)+((p[i]>>8)&255)+((p[i]>>16)&255);return sum;}
static void check_sequence(Game*g,unsigned start,int act){
 assert(g->screen==(act==3?ASCENT_ENDING:act==4?ASCENT_TRUE_END:ABOMINATION_AFTER));
 assert(victory_replay&&r1311_finishing);assert(!game_render_transition_busy(start));
 int end=r1311_finish_end(),fade_start=end-R1311_VICTORY_FADE;unsigned last=~0u;
 for(int age=0;age<=end+12;age++){
  game_render_frame(g,frame,480,start+age);
  assert(!r161_transition_active);
  if(age<fade_start){/* A persistent HUD strip cannot go black before the finish fade. */
   unsigned sum=0;for(int i=0;i<480*17;i++)sum+=frame[i]&0xffffffu;assert(sum);
  }
  if(age>=fade_start&&age<end){unsigned now=brightness(frame);assert(now<=last);last=now;}
  if(age>=end){
   /* Compare actual early-return cutscene pixels with that same scene and
      the expected reveal. This catches a missing reveal, not just flags. */
   int replay=victory_replay;victory_replay=0;game_render_frame(g,expected,480,start+age);victory_replay=replay;
   pix=expected;sw=480;if(age<end+12)dark_rect(0,0,480,272,(12-(age-end))*255/12);
   assert(!memcmp(frame,expected,sizeof(frame)));
   if(age==end)assert(!brightness(frame));
  }
  assert(!rift_asset_errors);
 }
 assert(!victory_replay&&!game_render_fx_busy(start+end+12));
 printf("PASS Act %d: one finishing fade, black midpoint, exact cutscene reveal, no combat return, all %d frames\n",act,end+13);
}
int main(void){char root[]="/tmp/r186r2-timeline-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));rift_assets_set_file("qa/v186/evidence/EBOOT.PBP");assert(rift_assets_open());Game g;
 for(int act=3;act<=7;act+=act==3?1:3)for(int leader_first=0;leader_first<2;leader_first++){
  game_render_reset_fx();setup(&g,act);
  if(leader_first&&g.enemy_count>1){kill(&g,0,100);assert(g.screen==COMBAT&&!r1311_finishing&&!victory_replay&&!game_render_transition_busy(100));for(int i=1;i+1<g.enemy_count;i++)g.enemies[i].hp=0;kill(&g,g.enemy_count-1,200);}
  else{for(int i=1;i<g.enemy_count;i++)g.enemies[i].hp=0;kill(&g,0,200);}
  check_sequence(&g,200,act);
 }
 game_render_reset_fx();setup(&g,3);Game before=g;before.screen=ASCENT_ENDING;g.screen=ASCENT_REVELATION;game_render_feedback(&before,&g,BTN_OK,500);assert(game_render_transition_busy(500));game_render_frame(&g,frame,480,516);assert(!brightness(frame));game_render_frame(&g,frame,480,532);assert(!game_render_transition_busy(532));
 puts("PASS ordinary story-to-story fade retained; leader death alone never starts ending; complete renderer, real combat effects, zero streamed-art errors.");return 0;}
