/* Rift Ascent 1.1.3 — SFX ONLY.
 * The game's ten music tracks MUST come from the supplied WAV recordings
 * packaged into the EBOOT.PBP ADPCM trailer. No procedural/synthesized music
 * is generated here, even when the trailer cannot be opened.
 * 44100-Hz stereo output; game-independent native unit tests run this C file.
 */
#include "sound.h"
#include <stdint.h>

static volatile unsigned fx_serial=0;
static volatile int fx_request=0;
void rift_sound_effect(int kind){
 if(kind<=0||kind>5)return;
 fx_request=kind;fx_serial++;
}
void rift_sound_reset(RiftSound *s){
 if(!s)return;
 s->pos=s->lead=s->bass=0;
 s->noise=0xCA49FE12u;
 s->last_step=0;
 s->pad_phase1=s->pad_phase2=s->sfx_phase=0;
 s->delay_pos=0;
 s->observed_serial=fx_serial;
 s->fx_kind=0;s->fx_age=65536;
 for(unsigned i=0;i<4096;i++)s->ambience_delay[i]=0;
}
static int16_t limit(int v){return (int16_t)(v>30000?30000:(v< -30000?-30000:v));}
static int pulse(uint32_t phase,int duty){return (int)((phase>>8)&255)<duty?1:-1;}
void rift_audio_generate(RiftSound *s,int16_t *out,int frames,int battle,int accent){
 (void)battle;(void)accent;
 if(!s||!out||frames<=0)return;
 for(int i=0;i<frames;i++){
  if(s->observed_serial!=fx_serial){
   s->observed_serial=fx_serial;
   s->fx_kind=fx_request;
   s->fx_age=0;
   s->sfx_phase=0;
  }
  s->noise ^= s->noise<<13;
  s->noise ^= s->noise>>17;
  s->noise ^= s->noise<<5;
  if(!s->noise)s->noise=0x9e3779b9u;
  int sfx=0;
  if(s->fx_age<6000){
   int amp=(6000-s->fx_age)/4;
   switch(s->fx_kind){
    case 1: /* strike */
     sfx=((s->noise&1u)?amp*2:-amp*2)+pulse(s->sfx_phase,100)*amp*2;
     s->sfx_phase+=(unsigned)(1550-((s->fx_age*1300)/6000));break;
    case 2: /* guard */
     sfx=pulse(s->sfx_phase,70)*amp;s->sfx_phase+=850;break;
    case 3: /* potion */
     sfx=pulse(s->sfx_phase,80)*amp;s->sfx_phase+=640+s->fx_age/24;break;
    case 4: /* heavy enemy hit */
     sfx=((s->noise&1u)?amp*3:-amp*3)+pulse(s->sfx_phase,100)*amp*2;
     s->sfx_phase+=(unsigned)(650-s->fx_age/14);break;
    case 5: /* reward */
     sfx=pulse(s->sfx_phase,90)*amp;s->sfx_phase+=840+s->fx_age/28;break;
   }
   s->fx_age++;
  }
  out[i*2]=limit(sfx);
  out[i*2+1]=limit(sfx);
  s->pos++;
 }
}
