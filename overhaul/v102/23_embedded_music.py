#!/usr/bin/env python3
"""Replace v1.0.2 external PCM playback with embedded mono IMA-ADPCM playback.

The compressed soundtrack is appended to EBOOT.PBP after the executable.
No extra file is required on the PSP, and only a 4 KiB decode cache is used.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text()
def change(a,b):
 global s
 if a not in s:raise RuntimeError("Embedded CHURN patch anchor absent: "+a[:90])
 s=s.replace(a,b,1)

change('#include <pspaudio.h>', '#include <pspaudio.h>\n#include <pspiofilemgr.h>')
change('static int audio_thread_id=-1;', r'''static int audio_thread_id=-1;
/* IMA ADPCM: mono 11,025 Hz, 4 bits/sample; decoded to stereo 44,100 Hz.
   A 20-byte footer at end of EBOOT: "RIFTIMA1", bytes, samples, CRC32.
   No allocated buffers, dependencies, or changes to PSP boot/display code. */
#define RIFT_FOOTER_SIZE 20
static SceUID churn_fd=-1;
static unsigned churn_start=0,churn_size=0,churn_samples=0,churn_remaining=0;
static unsigned char churn_cache[4096] __attribute__((aligned(64)));
static int churn_pos=0,churn_length=0,churn_has_high=0,churn_byte=0;
static int churn_predictor=0,churn_step_index=0,churn_last=0;
static unsigned churn_resample_phase=0;
static int churn_active=0;
static const int churn_steps[89]={
 7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,
 34,37,41,45,50,55,60,66,73,80,88,97,107,118,
 130,143,157,173,190,209,230,253,279,307,337,371,
 408,449,494,544,598,658,724,796,876,963,1060,1166,
 1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,
 3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,
 8630,9493,10442,11487,12635,13899,15289,16818,
 18500,20350,22385,24623,27086,29794,32767
};
static const signed char churn_idx_delta[16]={
 -1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8
};
static unsigned churn_u32(const unsigned char *b){
 return (unsigned)b[0]|((unsigned)b[1]<<8)|((unsigned)b[2]<<16)|((unsigned)b[3]<<24);
}
static void churn_close(void) {
 if(churn_fd>=0)sceIoClose(churn_fd);
 churn_fd=-1;churn_pos=0;churn_length=0;churn_active=0;
}
static int churn_rewind(void){
 if(churn_fd<0 || sceIoLseek32(churn_fd,(int)churn_start,0)<0)return 0;
 churn_remaining=churn_samples;churn_pos=churn_length=0;
 churn_has_high=0;churn_predictor=churn_step_index=churn_last=0;
 churn_resample_phase=0;
 return 1;
}
static int churn_open(void) {
 churn_close();
 churn_fd=sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP",PSP_O_RDONLY,0);
 if(churn_fd<0)return 0;
 int end=sceIoLseek32(churn_fd,0,2); /* SEEK_END */
 if(end<RIFT_FOOTER_SIZE){churn_close();return 0;}
 if(sceIoLseek32(churn_fd,end-RIFT_FOOTER_SIZE,0)<0){
   churn_close();return 0;
 }
 unsigned char footer[RIFT_FOOTER_SIZE];
 if(sceIoRead(churn_fd,footer,sizeof(footer))!=(int)sizeof(footer)){
   churn_close();return 0;
 }
 const char magic[8]={'R','I','F','T','I','M','A','1'};
 for(int i=0;i<8;i++)if(footer[i]!=(unsigned char)magic[i]){
   churn_close();return 0;
 }
 churn_size=churn_u32(footer+8);churn_samples=churn_u32(footer+12);
 if(churn_size==0||churn_samples==0||
    churn_size!=(churn_samples+1u)/2u||
    churn_size>(unsigned)(end-RIFT_FOOTER_SIZE)){
   churn_close();return 0;
 }
 churn_start=(unsigned)end-RIFT_FOOTER_SIZE-churn_size;
 if(!churn_rewind()){churn_close();return 0;}
 churn_active=1;
 boot_trace("Embedded CHURN found", (int)churn_samples);
 return 1;
}
static int churn_next_sample(void) {
 if(churn_fd<0)return 0;
 if(churn_remaining==0){
   if(!churn_rewind())return 0;
 }
 int nib;
 if(!churn_has_high){
   if(churn_pos>=churn_length){
     churn_length=sceIoRead(churn_fd,churn_cache,sizeof(churn_cache));
     churn_pos=0;
     if(churn_length<=0)return 0;
   }
   churn_byte=churn_cache[churn_pos++];
   nib=churn_byte&15;churn_has_high=1;
 }else{
   nib=(churn_byte>>4)&15;churn_has_high=0;
 }
 int step=churn_steps[churn_step_index],delta=step>>3;
 if(nib&1)delta+=step>>2;
 if(nib&2)delta+=step>>1;
 if(nib&4)delta+=step;
 if(nib&8)churn_predictor-=delta;else churn_predictor+=delta;
 if(churn_predictor>32767)churn_predictor=32767;
 if(churn_predictor< -32768)churn_predictor=-32768;
 churn_step_index+=churn_idx_delta[nib];
 if(churn_step_index<0)churn_step_index=0;
 if(churn_step_index>88)churn_step_index=88;
 churn_remaining--;
 return churn_predictor;
}
static short churn_clip(int n){
 return (short)(n>32000?32000:(n< -32000?-32000:n));
}''')
change(r'''    while(running) {
        rift_audio_generate(&music,audio_buffer,512,music_battle,0);
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }''',r'''    while(running){
        int mode=music_battle;
        int battle=(mode&1)!=0;
        if(battle&&!churn_active) {
          if(!churn_open())churn_active=-1; /* Missing trailer: synth fallback. */
        }
        if(!battle&&churn_active){
          churn_close();
        }
        rift_audio_generate(&music,audio_buffer,512,mode,0);
        if(battle&&churn_fd>=0){
          for(int i=0;i<512;i++){
            if(churn_resample_phase==0)churn_last=churn_next_sample();
            churn_resample_phase=(churn_resample_phase+1u)&3u;
            /* Retain low-level synthesized SFX; no full-track PCM in RAM. */
            int l=churn_last*3/4+(int)audio_buffer[2*i]/3;
            int r=churn_last*3/4+(int)audio_buffer[2*i+1]/3;
            audio_buffer[2*i]=churn_clip(l);
            audio_buffer[2*i+1]=churn_clip(r);
          }
        }
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }
    churn_close();''')
p.write_text(s)
print("PASS: embedded 4-bit CHURN music decoder patched (no external file).")
