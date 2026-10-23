/* Rift Ascent audio v2: three original procedural themes plus layered SFX.
 * 44.1 kHz integer synthesis, no streamed files, malloc or math library.
 * Phrases vary over 64 steps and across acts instead of a 16-note loop.
 */
#include "sound.h"
#include <stdint.h>

static volatile unsigned fx_serial=0;
static volatile int fx_request=0;
void rift_sound_effect(int kind) {
 fx_request=kind; fx_serial++;
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

static const unsigned short lead_calm[64]={
 220,0,262,0,330,0,392,349, 330,0,294,262,0,220,196,0,
 196,0,247,294,330,0,392,0, 440,392,330,294,0,262,0,0,
 220,0,262,330,392,0,440,392,349,330,294,0,262,294,330,0,
 247,0,220,0,196,247,294,0, 330,294,262,220,196,0,0,0
};
static const unsigned short lead_battle[64]={
 175,175,0,233,262,0,311,233, 175,196,233,0,294,262,233,0,
 196,196,0,262,349,311,294,262, 233,0,175,196,233,0,0,0,
 175,0,233,262,311,0,349,392, 349,311,262,233,196,175,0,0,
 220,220,262,0,311,349,392,311, 262,233,220,175,0,233,175,0
};
static const unsigned short bass_calm[16]={
 55,0,55,0,65,0,55,0, 49,0,49,0,44,0,49,0
};
static const unsigned short bass_battle[16]={
 55,55,0,73,55,55,0,65, 49,49,0,65,49,55,0,73
};
static int16_t limit(int v){return (int16_t)(v>30000?30000:(v< -30000?-30000:v));}
static int pulse(uint32_t phase,int duty) {
 return (int)((phase>>8)&255)<duty?1:-1;
}
void rift_audio_generate(RiftSound*s,int16_t*out,int frames,int battle,int accent){
 if(!s||!out||frames<=0)return;
 /* Persistent audio event envelope does not need dynamically allocated voices. */
 /* Per-instance state is reset by rift_sound_reset(). */
 unsigned theme=(unsigned)(battle<0?0:battle);
 unsigned act=theme/2u;
 battle=(int)(theme&1u);
 const unsigned step_samples=battle?5880u:9180u;  /* 112 or 188 BPM-feel subdivisions */
 for(int i=0;i<frames;i++){
  if(s->observed_serial!=fx_serial){
   s->observed_serial=fx_serial;s->fx_kind=fx_request;s->fx_age=0;s->sfx_phase=0;
  }
  unsigned step=(s->pos/step_samples)&63u;
  unsigned elapsed=s->pos%step_samples;
  unsigned phrase=(s->pos/(step_samples*64u));
  unsigned tune=step;
  unsigned short note=(battle?lead_battle:lead_calm)[tune];
  if(act==1 && note)note=(unsigned short)(note*6u/5u);
  if(act>=2 && note)note=(unsigned short)(note*4u/3u);
  if(phrase&1u && note)note=(unsigned short)(note*9u/8u);
  if(phrase&2u && note)note=(unsigned short)(note*3u/4u);
  unsigned short root=(battle?bass_battle:bass_calm)[(step/4u)&15u];
  if(act==1 && root)root=(unsigned short)(root*6u/5u);
  if(act>=2 && root)root=(unsigned short)(root*4u/3u);
  static const unsigned short pad_chords[8]={110,131,98,147,110,123,92,131};
  unsigned pad_root=pad_chords[(step/8u)&7u];
  if(act==1)pad_root=pad_root*6u/5u;
  if(act>=2)pad_root=pad_root*4u/3u;
  s->lead += (uint32_t)(((uint64_t)note*65536u)/44100u);
  s->bass += (uint32_t)(((uint64_t)root*65536u)/44100u);
  s->pad_phase1+=(uint32_t)(((uint64_t)pad_root*65536u)/44100u);
  s->pad_phase2+=(uint32_t)(((uint64_t)pad_root*3u*65536u/2u)/44100u);
  s->noise ^= s->noise<<13;s->noise ^= s->noise>>17;s->noise ^= s->noise<<5;
  if(!s->noise)s->noise=0x9e3779b9u;
  int env=elapsed<260u?(int)(elapsed/2u): (elapsed<step_samples*2u/3u?130:70);
  int voice=note?pulse(s->lead,battle?91:65)*env*3:0;
  int a=(int)(s->bass&65535u);
  int tri=(a<32768?a:65535-a)-16384;
  int low=root?tri*(battle?920:720)/16384:0;
  int bassharm=root?pulse(s->bass,120)*(battle?130:100):0;
  int t1=(int)(s->pad_phase1&65535u);t1=t1<32768?t1:65535-t1;
  int t2=(int)(s->pad_phase2&65535u);t2=t2<32768?t2:65535-t2;
  int pad=((t1-16384)*180+(t2-16384)*130)/16384;
  int kick=step%4u==0&&elapsed<1000u?
       ((s->noise&1u)?1:-1)*(int)(1000u-elapsed)*2:0;
  int snare=step%8u==4&&elapsed<950u?
       ((s->noise&1u)?1:-1)*(int)(950u-elapsed):0;
  int hat=elapsed<320u&&(step%2u==0)?
       ((s->noise&1u)?1:-1)*(battle?460:240):0;
  int sfx=0;
  if(s->fx_age<6000){
    int amp=(6000-s->fx_age)/4;
    switch(s->fx_kind){
      case 1: /* strike: falling pitch, noise transient */
        sfx=((s->noise&1u)?amp:-amp)+pulse(s->sfx_phase,100)*amp;
        s->sfx_phase+=(unsigned)(1550-((s->fx_age*1300)/6000));break;
      case 2: /* guard: high bell */
        sfx=pulse(s->sfx_phase,70)*amp;s->sfx_phase+=850;break;
      case 3: /* potion: sparkling rising trill */
        sfx=pulse(s->sfx_phase,80)*amp;s->sfx_phase+=640+s->fx_age/24;break;
      case 4: /* enemy hit: heavy low impact */
        sfx=((s->noise&1u)?amp:-amp)*2;s->sfx_phase+=200;break;
      case 5: /* reward */
        sfx=pulse(s->sfx_phase,90)*amp;s->sfx_phase+=840+s->fx_age/28;break;
    }
    s->fx_age++;
  }
  int l=voice+low+bassharm+pad+kick+snare+hat+sfx;
  int r=voice+low+bassharm+pad+kick-snare+hat+sfx;
  int echo=s->ambience_delay[s->delay_pos];
  s->ambience_delay[s->delay_pos]=(int16_t)((l+r)/4+echo/3);
  s->delay_pos=(s->delay_pos+1u)&4095u;
  l+=echo/4;
  r+=echo/3;
  out[i*2]=limit(l);
  out[i*2+1]=limit(r);
  s->pos++;
 }
 (void)accent;
}
