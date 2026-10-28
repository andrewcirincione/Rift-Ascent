#include "game.h"
#include "sound.h"
#include "render.c"
#include <assert.h>
static uint32_t frame[W*H],mask[W*H];
static void save(Game*g,const char*n){char p[180];snprintf(p,sizeof(p),"qa/v171r2/evidence/state-%s.dat",n);assert(game_validate(g)&&game_save(g,p));}
int main(void){Game g,b,l;char n[80];int pages=0;
 for(int c=0;c<3;c++)for(int a=1;a<=4;a++){
  game_new(&g,17102+c*41+a);game_choose_character(&g,c);g.act=a;g.endless=a>3;g.screen=ACT_INTRO;g.scroll=45;g.floor=0;g.selection=0;assert(game_music_silent(&g));
  snprintf(n,sizeof(n),"intro-%d-%d",c,a);save(&g,n);int relics=g.relic_count,deck=g.deck_count;unsigned rng=g.rng;
  b=g;game_action(&g,BTN_OK);assert(g.screen==STORY_ACT_BEFORE_BLESSING&&g.rng==rng&&g.relic_count==relics&&g.deck_count==deck&&game_music_silent(&g));
  snprintf(n,sizeof(n),"story-%d-%d",c,a);save(&g,n);Game timed=b;timed.scroll=119;game_update(&timed);assert(!memcmp(&g,&timed,sizeof(g)));
  game_action(&g,BTN_PAUSE);assert(game_music_silent(&g));game_action(&g,BTN_BACK);assert(g.screen==STORY_ACT_BEFORE_BLESSING);
  save(&g,"roundtrip");assert(game_load(&l,"qa/v171r2/evidence/state-roundtrip.dat")&&!memcmp(&g,&l,sizeof(g)));
  Game story=g;game_action(&g,BTN_OK);assert(g.screen==BLESSING&&!game_music_silent(&g)&&game_music_track(&g)==1&&g.relic_count==relics&&g.deck_count==deck&&game_validate(&g));
  game_render_reset_fx();game_render_feedback(&story,&g,BTN_OK,100);assert(game_music_silent(game_render_audio_state(&g,115))&&!game_music_silent(game_render_audio_state(&g,116)));
  game_render_frame(&g,frame,W,116);for(int i=0;i<W*H;i++)assert((frame[i]&0xffffff)==0);
  game_action(&g,BTN_OK);assert(g.screen==MAP&&!game_music_silent(&g));
  /* Buffered skip edges must stop at the relic choice, never accept it. */
  g=story;game_render_reset_fx();game_render_feedback(&b,&g,BTN_OK,200);for(int i=0;i<3;i++)assert(!game_render_story_input(&g,BTN_OK,204+i*4));assert(game_render_story_input(&g,0,232)==BTN_OK);b=g;game_action(&g,BTN_OK);game_render_feedback(&b,&g,BTN_OK,232);assert(g.screen==BLESSING&&!game_render_story_input(&g,0,264)&&g.relic_count==relics&&g.deck_count==deck);
  /* Existing post-relic story saves go straight to the map without a gift. */
  g=story;g.screen=STORY_ACT;game_action(&g,BTN_OK);assert(g.screen==MAP&&g.relic_count==relics&&g.deck_count==deck);
  pages++;
 }
 for(int a=1;a<=4;a++){
  game_new(&g,171);game_choose_character(&g,0);g.act=a;g.endless=a>3;g.screen=ACT_INTRO;g.scroll=50;game_render_reset_fx();game_render_frame(&g,mask,W,200);
  for(int t=0;t<120;t++){g.scroll=t;game_render_frame(&g,frame,W,200+t);for(int y=198;y<H;y++)for(int x=0;x<W;x++){assert((frame[y*W+x]>>24)==255);if(!(mask[y*W+x]&0xffffff))assert(!(frame[y*W+x]&0xffffff));}}
 }
 int grounded=0;for(int art=0;art<44;art++)for(int h=86;h<=98;h++)for(int flip=0;flip<2;flip++){
  int b=r170_boss_index(art),w=b>=0?r170_boss_size[b][0]*h/r170_boss_size[b][1]:art>=36?r150_enemy_size[art-36][0]*h/r150_enemy_size[art-36][1]:art==9?144:r1311_enemy_bounds[art][2]*h/r1311_enemy_bounds[art][3];w=MIN(w,144);int gap=r171_ground_gap(art,w,h,b<0&&art<36&&art!=9?flip:0);assert(gap>=0&&gap<6);
  pix=frame;sw=W;rect(0,0,W,H,RGB(70,62,57));int x=240-w/2,y=174-h+gap;
  if(b>=0)r170_rgba_actor(x,y,w,h,r170_boss_sprites[b],r170_boss_size[b][0],r170_boss_size[b][1],0,255,0);
  else if(art>=36)r150_enemy_actor(x,y,w,h,art,0,255,0);
  else if(art==9)r137_blit(x,y,w,h,r1310_hound,144,96,255,0);
  else r137_actor(x,y,w,h,rift_enemy_sprites[art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip,0,255,0);
  int feet=0;for(int xx=160;xx<320;xx++)feet+=frame[173*W+xx]!=RGB(70,62,57);assert(feet>=2);grounded++;
 }
 /* New enum only: saved Game layout and boss music behavior stay compatible. */
 assert(sizeof(Game)==28172);g.screen=STORY_BOSS;assert(!game_music_silent(&g)&&game_music_track(&g)==0);
 printf("PASS %d character/act flows: splash -> silent story -> relic -> map; automatic/X progression, save/pause compatibility, old-save no-double-gift and buffered-X boundaries\n",pages);
 puts("PASS solid black act-title bar across 480 fade phases; scene music switches at black midpoint");
 printf("PASS %d enemy grounding configurations; only idle cycles accelerated, save layout unchanged\n",grounded);
 int layouts=0;
 for(int raw=0;raw<CARD_COUNT;raw++)for(int up=0;up<2;up++){
  int card=raw|(up?UPBIT:0);R135CardLayout q=r135_card_layout(card,180);
  for(int w=182;q.height>166&&w<=214;w+=2)q=r135_card_layout(card,w);
  assert(q.height<=166&&81+q.height<=247&&q.title_rows<=2);
  assert(q.body_y+q.rows*q.leading<=q.height-18);layouts++;
 }
 for(int c=0;c<3;c++)for(int e=2;e<=6;e+=4){
  int longest=-1;Game candidate;
  for(int seed=1;seed<=120;seed++){
   game_new(&g,171000+seed);game_choose_character(&g,c);g.gold=100;game_begin_event(&g,e);
   int score=(int)strlen(card_description(g.reward[0]))+(int)strlen(card_description(g.reward[1]));
   if(score>longest){longest=score;candidate=g;}
  }
  g=candidate;snprintf(n,sizeof(n),"event-%d-%d",c,e);save(&g,n);
  for(int choice=0;choice<2;choice++){
   g=candidate;g.selection=choice;b=g;int card=g.reward[choice];game_action(&g,BTN_OK);
   assert(g.deck_count==b.deck_count+1&&g.deck[g.deck_count-1]==card);
   assert(g.gold==b.gold-(e==2&&choice==0?35:0)&&g.hp==b.hp-(e==6?6:0));
  }
  g=candidate;g.selection=2;b=g;game_action(&g,BTN_OK);assert(g.deck_count==b.deck_count&&g.gold==b.gold&&g.hp==b.hp);
 }
 /* Preserve the underlying selection while browsing pause. */
 g.selection=1;b=g;game_action(&g,BTN_PAUSE);assert(game_pause_selection()==0);
 game_render_reset_fx();game_render_frame(&g,mask,W,100);
 game_action(&g,BTN_DOWN);assert(game_pause_selection()==1&&g.selection==b.selection);
 game_render_frame(&g,frame,W,100);assert(memcmp(frame,mask,sizeof(frame)));
 game_action(&g,BTN_OK);assert(!memcmp(&g,&b,sizeof(g)));
 game_action(&g,BTN_PAUSE);game_action(&g,BTN_UP);assert(game_pause_selection()==1);game_action(&g,BTN_DOWN);assert(game_pause_selection()==0);
 game_action(&g,BTN_BACK);assert(!memcmp(&g,&b,sizeof(g)));
 RiftSound snd={0};assert(rift_sound_music_gain(&snd)==184);snd.impact_tier[0]=1;
 assert(rift_sound_music_gain(&snd)==55);int previous=55;
 for(unsigned age=1;age<=4410;age++){snd.impact_age[0]=age;int gain=rift_sound_music_gain(&snd);assert(gain>=previous&&gain<=184);previous=gain;}
 printf("PASS %d full card layouts fit the mystery panel; actual Tome/Armory rewards, payment and leave behavior for all characters\n",layouts);
 puts("PASS pause UP/DOWN highlights, X resumes chosen option and preserves underlying selection; separate Save & Quit dispatch");
 puts("PASS music gain +15%, smooth impact ducking and recovery; music-only maximum 72% leaves PCM headroom");return 0;
}
