/* Host-side audio regression: guard buffers and exercise every act/mode. */
#include "sound.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define FRAMES 512
static int16_t pcm[FRAMES*2+4];
int main(void){
 uint64_t signatures[6]={0};
 for(int mode=0;mode<6;mode++){
   RiftSound state={0};
   rift_sound_reset(&state);
   uint64_t sum=0,energy=0;
   for(int block=0;block<96;block++){
     pcm[0]=12345;pcm[1]=-12345;
     pcm[FRAMES*2+2]=23456;pcm[FRAMES*2+3]=-23456;
     rift_audio_generate(&state,pcm+2,FRAMES,mode,0);
     if(pcm[0]!=12345||pcm[1]!=-12345||pcm[FRAMES*2+2]!=23456||pcm[FRAMES*2+3]!=-23456){
       fprintf(stderr,"Audio buffer overrun mode %d block %d\n",mode,block);return 1;
     }
     for(int i=2;i<FRAMES*2+2;i++){
       int v=pcm[i];
       energy+=(uint64_t)(v<0?-v:v);
       sum=sum*33u+(uint16_t)v;
     }
     if(block==4)rift_sound_effect(1);
     if(block==8)rift_sound_effect(3);
   }
   if(energy<1000000u){fprintf(stderr,"Audio silent in mode %d\n",mode);return 1;}
   signatures[mode]=sum;
 }
 for(int i=0;i<6;i++)for(int j=i+1;j<6;j++)
   if(signatures[i]==signatures[j]){fprintf(stderr,"Audio themes identical %d %d\n",i,j);return 1;}
 puts("PASS: 6 act/mode tracks, non-silent PCM, distinct signals, bounded buffers, SFX");
 return 0;
}
