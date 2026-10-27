#define main ui_fixture_main
#include "ui_fixtures.c"
#undef main
#include "sound.h"
static void frame_shot(Game*g,unsigned tick,const char*name){static uint32_t f[W*H];game_render_update_fx(tick);game_render_frame(g,f,W,tick);char path[256];snprintf(path,sizeof(path),"qa/v150b/evidence/animation-%s.ppm",name);FILE*out=fopen(path,"wb");assert(out);fprintf(out,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(f[i]&255,out);fputc(f[i]>>8&255,out);fputc(f[i]>>16&255,out);}fclose(out);}
int main(void){Game g,b;RiftSound sound;int16_t pcm[1024];int ids[]={N_STRIKE_SILENT,N_DEFEND_SILENT,N_ACCURACY};const char*names[]={"attack","block","power","death"};for(int a=0;a<4;a++){
 arena(&g);g.hand_count=1;g.hand[0]=ids[a<3?a:0];g.selection=0;g.strength=0;r137_count=0;r137_held_valid=0;r138_death_active=0;victory_replay=0;rift120_played_card=rift120_auto_card=-1;g.block=0;if(a==3){g.hp=1;g.enemies[0].intent=0;g.enemies[0].intent_value=20;}b=g;
 rift_sound_reset(&sound);game_combat_fx_begin();if(a<3)game_play_card(&g,0,0);else game_end_turn(&g);game_combat_fx_end();game_render_feedback(&b,&g,a<3?BTN_OK:BTN_BACK,1000);int count;const GameCombatFx*fx=game_combat_fx_events(&count);assert(count>0);int expected=0;for(int i=0;i<count;i++)expected+=fx[i].kind==GFX_ATTACK||fx[i].kind==GFX_PROC;
 int ages[]={0,4,8,12,16,24,36,48,65};for(unsigned i=0;i<sizeof(ages)/sizeof(*ages);i++){char name[64];snprintf(name,sizeof(name),"%s-%02d",names[a],ages[i]);frame_shot(&g,1000+ages[i],name);}game_render_update_fx(1200);rift_audio_generate(&sound,pcm,512,0,0);if(a==0)assert(sound.impact_seen>0);if(a==1)assert(sound.block_age>0);if(a==2)assert(sound.power_age>0);
 }
 puts("PASS native Nightshade attack, Block, power, death render timelines and contact audio");return 0;}
