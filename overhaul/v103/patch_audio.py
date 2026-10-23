#!/usr/bin/env python3
"""Rift Ascent 1.0.3: ten embedded IMA tracks, no synthesized background music.

Music index is appended as a 96-byte trailer at end of EBOOT.PBP:
 b'RIFTM103' + 10*(u32 payload_offset,u32 sample_count) + u32 total_bytes + u32 crc32
 Each track's payload is 4-bit signed IMA ADPCM, MONO 11025 Hz.
 PSP decodes small chunks directly from its own EBOOT into 44100 Hz stereo.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def replace(a,b):
 global s
 if a not in s:raise RuntimeError("1.0.3 music anchor not found "+a[:90])
 s=s.replace(a,b,1)
replace("#include <pspaudio.h>", "#include <pspaudio.h>\n#include <pspiofilemgr.h>")
replace("static int audio_thread_id=-1;",'''static int audio_thread_id=-1;
#define RIFT_TRACK_COUNT 10
#define RIFT_MUSIC_FOOTER 96
static SceUID soundtrack_fd=-1;
static unsigned soundtrack_offset[10],soundtrack_samples[10];
static unsigned soundtrack_base=0,soundtrack_total=0;
static int soundtrack_current=-1,soundtrack_active=0;
static unsigned left_samples=0,track_seek=0;
static unsigned char audio_cache[4096] __attribute__((aligned(64)));
static int audio_pos=0,audio_length=0,audio_half=0,audio_byte=0;
static int audio_predictor=0,audio_step_idx=0,audio_last=0;
static unsigned audio_phase=0;
static const int adpcm_steps[89]={
 7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,
 34,37,41,45,50,55,60,66,73,80,88,97,107,118,
 130,143,157,173,190,209,230,253,279,307,337,371,
 408,449,494,544,598,658,724,796,876,963,1060,1166,
 1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,
 3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,
 8630,9493,10442,11487,12635,13899,15289,16818,
 18500,20350,22385,24623,27086,29794,32767
};
static const signed char adpcm_index_delta[16]={
 -1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8
};
static unsigned read_u32(const unsigned char *b){
 return (unsigned)b[0]|((unsigned)b[1]<<8)|((unsigned)b[2]<<16)|((unsigned)b[3]<<24);
}
static void soundtrack_close(void){
 if(soundtrack_fd>=0)sceIoClose(soundtrack_fd);
 soundtrack_fd=-1;soundtrack_active=0;soundtrack_current=-1;
}
static int soundtrack_open(void){
 soundtrack_close();
 soundtrack_fd=sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP",PSP_O_RDONLY,0);
 if(soundtrack_fd<0)return 0;
 int end=sceIoLseek32(soundtrack_fd,0,2);
 if(end<RIFT_MUSIC_FOOTER){soundtrack_close();return 0;}
 if(sceIoLseek32(soundtrack_fd,end-RIFT_MUSIC_FOOTER,0)<0){
   soundtrack_close();return 0;
 }
 unsigned char footer[RIFT_MUSIC_FOOTER];
 if(sceIoRead(soundtrack_fd,footer,sizeof(footer))!=(int)sizeof(footer)){
   soundtrack_close();return 0;
 }
 const char magic[8]={'R','I','F','T','M','1','0','3'};
 for(int i=0;i<8;i++)if(footer[i]!=(unsigned char)magic[i]){
   soundtrack_close();return 0;
 }
 soundtrack_total=read_u32(footer+88);
 if(soundtrack_total==0||soundtrack_total>(unsigned)(end-RIFT_MUSIC_FOOTER)){
   soundtrack_close();return 0;
 }
 soundtrack_base=(unsigned)end-RIFT_MUSIC_FOOTER-soundtrack_total;
 for(int i=0;i<10;i++){
   soundtrack_offset[i]=read_u32(footer+8+i*8);
   soundtrack_samples[i]=read_u32(footer+12+i*8);
   if(soundtrack_samples[i]==0||
      soundtrack_offset[i]>=soundtrack_total||
      (soundtrack_samples[i]+1u)/2u>soundtrack_total-soundtrack_offset[i]){
      soundtrack_close();return 0;
   }
 }
 soundtrack_active=1;
 boot_trace("Rift 1.0.3 embedded music loaded",10);
 return 1;
}
static int soundtrack_select(int track){
 if(!soundtrack_active||track<0||track>=10)return 0;
 track_seek=soundtrack_base+soundtrack_offset[track];
 if(sceIoLseek32(soundtrack_fd,(int)track_seek,0)<0)return 0;
 soundtrack_current=track;
 left_samples=soundtrack_samples[track];
 audio_pos=audio_length=audio_half=audio_byte=0;
 audio_predictor=audio_step_idx=audio_last=0;
 audio_phase=0;
 return 1;
}
static int soundtrack_sample(void){
 if(!soundtrack_active||soundtrack_current<0)return 0;
 if(left_samples==0){
   if(!soundtrack_select(soundtrack_current))return 0;
 }
 int nibble;
 if(!audio_half){
   if(audio_pos>=audio_length){
     audio_length=sceIoRead(soundtrack_fd,audio_cache,sizeof(audio_cache));
     audio_pos=0;
     if(audio_length<=0)return 0;
   }
   audio_byte=audio_cache[audio_pos++];
   nibble=audio_byte&15;
   audio_half=1;
 }else{
   nibble=(audio_byte>>4)&15;
   audio_half=0;
 }
 int step=adpcm_steps[audio_step_idx];
 int delta=step>>3;
 if(nibble&1)delta+=step>>2;
 if(nibble&2)delta+=step>>1;
 if(nibble&4)delta+=step;
 if(nibble&8)audio_predictor-=delta;else audio_predictor+=delta;
 if(audio_predictor>32767)audio_predictor=32767;
 if(audio_predictor< -32768)audio_predictor=-32768;
 audio_step_idx+=adpcm_index_delta[nibble];
 if(audio_step_idx<0)audio_step_idx=0;
 if(audio_step_idx>88)audio_step_idx=88;
 left_samples--;
 return audio_predictor;
}
static short audio_clip(int n){return (short)(n>31000?31000:(n< -31000?-31000:n));}''')
replace('''    while(running) {
        rift_audio_generate(&music,audio_buffer,512,music_battle,0);
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }''',
'''    soundtrack_open();
    while(running){
        int track=music_battle;
        if(soundtrack_active&&soundtrack_current!=track)
           soundtrack_select(track);
        /* No original synth music; accent=1 emits impact/SFX only. */
        rift_audio_generate(&music,audio_buffer,512,track,1);
        for(int i=0;i<512;i++){
           if(soundtrack_active){
              if(audio_phase==0)audio_last=soundtrack_sample();
              audio_phase=(audio_phase+1u)&3u;
           }else audio_last=0;
           int l=audio_last*4/5+(int)audio_buffer[i*2];
           int r=audio_last*4/5+(int)audio_buffer[i*2+1];
           audio_buffer[i*2]=audio_clip(l);
           audio_buffer[i*2+1]=audio_clip(r);
        }
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }
    soundtrack_close();''')
# Existing engine uses music_battle as previous procedural theme; it now
# selects one of ten tracks and persists during inspection and pause.
replace("music_battle=(game.act-1)*2+(game.screen==COMBAT?1:0);",
'''music_battle=(game.screen==TITLE?0:
   (game.screen==COMBAT||
    (game.screen==DECK_VIEW&&game.prior_screen==COMBAT)||
    (game.screen==PAUSE_MENU&&game_pause_source()==COMBAT))
    ?2+(int)((game.seed+(unsigned)(game.floor*3+game.act))%8u)
    :1);''')
p.write_text(s)
print("PASS 1.0.3 ten-track embedded music player.")
