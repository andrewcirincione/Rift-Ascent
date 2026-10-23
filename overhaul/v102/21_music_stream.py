#!/usr/bin/env python3
"""1.0.2 item 21: play user-provided CHURN.PCM in battles via PSP user-mode I/O.
Format: signed little-endian, 22050 Hz, stereo, headerless PCM.
Stream from memory stick in 16 KB chunks; never load 16 MB into PSP RAM.
Outside battle retain procedurally synthesized music. Missing music file
falls back to the old soundtrack without preventing launch."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("CHURN PSP anchor missing "+a[:110])
 s=s.replace(a,b,1)
sub("#include <pspaudio.h>", "#include <pspaudio.h>\n#include <pspiofilemgr.h>")
sub("static int audio_thread_id=-1;",'''static int audio_thread_id=-1;
static SceUID churn_fd=-1;
static unsigned char churn_cache[16384] __attribute__((aligned(64)));
static int churn_pos=0,churn_length=0;
static int16_t churn_last_l=0,churn_last_r=0;
static int churn_active=0;
static void churn_close(void){
 if(churn_fd>=0)sceIoClose(churn_fd);
 churn_fd=-1;churn_pos=0;churn_length=0;
}
static void churn_open(void){
 churn_close();
 churn_fd=sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/CHURN.PCM",PSP_O_RDONLY,0);
 churn_active=1;
}
static int churn_sample(int16_t *left,int16_t *right){
 if(churn_fd<0)return 0;
 if(churn_pos+4>churn_length){
   churn_pos=0;churn_length=sceIoRead(churn_fd,churn_cache,sizeof(churn_cache));
   if(churn_length<4){
     /* Stream continuously through the original track; no RAM allocation. */
     churn_open();
     if(churn_fd<0)return 0;
     churn_length=sceIoRead(churn_fd,churn_cache,sizeof(churn_cache));
     if(churn_length<4)return 0;
   }
 }
 const unsigned char *p=churn_cache+churn_pos;
 *left=(int16_t)((unsigned)p[0]|((unsigned)p[1]<<8));
 *right=(int16_t)((unsigned)p[2]|((unsigned)p[3]<<8));
 churn_pos+=4;
 return 1;
}
static short clip_pcm(int n){return n>32000?32000:n< -32000?-32000:(short)n;}''')
sub('''    while(running) {
        rift_audio_generate(&music,audio_buffer,512,music_battle,0);
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }''',
'''    while(running) {
        int mode=music_battle;
        int battle=(mode&1)!=0;
        if(battle&&!churn_active)churn_open();
        if(!battle&&churn_active){churn_close();churn_active=0;}
        rift_audio_generate(&music,audio_buffer,512,mode,0);
        if(battle&&churn_fd>=0){
          for(int i=0;i<512;i++){
            if((i&1)==0){
               if(!churn_sample(&churn_last_l,&churn_last_r))
                  churn_last_l=churn_last_r=0;
            }
            int left=(int)churn_last_l*3/4+(int)audio_buffer[2*i]/2;
            int right=(int)churn_last_r*3/4+(int)audio_buffer[2*i+1]/2;
            audio_buffer[2*i]=clip_pcm(left);
            audio_buffer[2*i+1]=clip_pcm(right);
          }
        }
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }
    churn_close();''')
p.write_text(s)
print("PASS 21: streamed CHURN battle music with layered SFX and fallback.")
