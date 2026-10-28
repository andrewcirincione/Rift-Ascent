#include "game.h"
#include "render.c"
#include <assert.h>
#include <stdio.h>
static uint32_t one[512*H],two[512*H],good[512*H];
int main(void){
 Game g;game_new(&g,16212);game_choose_character(&g,CHAR_WARDEN);game_start_battle(&g,NODE_FIGHT);g.selection=g.hand_count;game_render_reset_fx();
 rift_assets_set_file("qa/v171r2/evidence/EBOOT.PBP");assert(rift_assets_open());game_render_frame(&g,good,512,500);
 /* Persistent unavailable artwork cannot retain pixels from either prior buffer. */
 rift_assets_set_file("qa/v171r2/evidence/does-not-exist.PBP");
 for(int i=0;i<512*H;i++){one[i]=0xff123456;two[i]=0xffabcdef;}
 game_render_frame(&g,one,512,500);game_render_frame(&g,two,512,500);
 for(int y=0;y<H;y++){for(int x=0;x<W;x++)assert(one[y*512+x]==two[y*512+x]);for(int x=W;x<512;x++)assert(one[y*512+x]==0xff123456&&two[y*512+x]==0xffabcdef);}
 assert(rift_asset_errors>0);rift_assets_set_file("qa/v171r2/evidence/EBOOT.PBP");game_render_frame(&g,one,512,500);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++)assert(one[y*512+x]==good[y*512+x]);
 puts("PASS missing artwork cannot ghost prior framebuffer; stride padding untouched; full original scene recovers without game reload");
 RiftSound sound;rift_sound_reset(&sound);static short pcm[16000*2];rift_sound_effect(11);rift_audio_generate(&sound,pcm,16000,2,1);
 long long energy=0;int peak=0;for(int i=0;i<16000;i++){assert(pcm[i*2]==pcm[i*2+1]);int a=abs(pcm[i*2]);energy+=(long long)a*a;if(a>peak)peak=a;}
 assert(energy>10000000&&peak>1000&&peak<8000);for(int i=15000;i<16000;i++)assert(pcm[i*2]==0);
 FILE*f=fopen("qa/v171r2/evidence/stars-glimmer.s16le","wb");assert(f);assert(fwrite(pcm,sizeof(pcm),1,f)==1);assert(!fclose(f));
 rift_sound_effect(11);rift_sound_effect(6);rift_audio_generate(&sound,pcm,1500,2,1);assert(sound.stars_age==1500);
 printf("PASS dedicated Star glimmer: nonzero stereo, peak %d, bounded 0.34-second tail; cursor sound does not cancel it\n",peak);
 return 0;
}
