/* Rift Ascent 1.1.3 — SFX ONLY.
 * The game's ten music tracks MUST come from the supplied WAV recordings
 * packaged into the EBOOT.PBP ADPCM trailer. No procedural/synthesized music
 * is generated here, even when the trailer cannot be opened.
 * 44100-Hz stereo output; game-independent native unit tests run this C file.
 */
#include "sound.h"
#include <stdint.h>

static volatile unsigned fx_serial=0,scene_serial=0;
static volatile int scene_mode=0;
void rift_sound_scene_mode(int mode){if(scene_mode!=mode){scene_mode=mode;scene_serial++;}}
static volatile int fx_request=0;
static volatile unsigned block_serial=0;
static volatile unsigned impact_serial=0,power_serial=0,victory_serial=0,stars_serial=0;
static volatile int impact_requests[32];
void rift_sound_impact(int damage,int blocked){
 if(scene_mode)return;
 int tier=damage>20?3:damage>10?2:damage>0?1:0;
 if(tier){impact_requests[impact_serial&31u]=tier;impact_serial++;}
 if(blocked)block_serial++;
}
void rift_sound_effect(int kind){
 if(scene_mode&&kind!=7)return;
 if(kind<=0||kind>11)return;
 if(kind==11){stars_serial++;return;}
 if(kind==10){victory_serial++;return;}
 if(kind==9){power_serial++;return;}
 if(kind==8){block_serial++;return;}
 fx_request=kind;fx_serial++;
}
void rift_sound_reset(RiftSound *s){
 if(!s)return;
 s->scene_seen=scene_serial;
 s->pos=s->lead=s->bass=0;
 s->noise=0xCA49FE12u;
 s->last_step=0;
 s->pad_phase1=s->pad_phase2=s->sfx_phase=0;
 s->delay_pos=0;
 s->observed_serial=fx_serial;
 s->fx_kind=0;s->fx_age=65536;
 s->block_serial=block_serial;s->block_age=12000;s->block_phase=0;
 s->impact_seen=impact_serial;s->impact_voice=0;
 for(int i=0;i<4;i++){s->impact_age[i]=20000;s->impact_phase[i]=0;s->impact_tier[i]=0;s->impact_low[i]=0;}
 s->stars_seen=stars_serial;s->stars_age=20000;for(int i=0;i<3;i++)s->stars_phase[i]=0;
 s->victory_seen=victory_serial;s->victory_age=40000;for(int i=0;i<3;i++)s->victory_phase[i]=0;
 s->power_seen=power_serial;s->power_age=20000;s->power_phase=0;
 for(unsigned i=0;i<4096;i++)s->ambience_delay[i]=0;
}
static int16_t limit(int v){int sign=v<0?-1:1,a=v*sign;if(a>20000)a=20000+(a-20000)*9000/(a-20000+9000);return (int16_t)(a*sign);}
static int triangle(uint32_t phase){unsigned q=phase>>16;return q<32768u?(int)q*2-32768:98303-(int)q*2;}
static int pulse(uint32_t phase,int duty){return (int)((phase>>8)&255)<duty?1:-1;}
void rift_audio_generate(RiftSound *s,int16_t *out,int frames,int battle,int accent){
 (void)battle;(void)accent;
 if(!s||!out||frames<=0)return;
 if(s->scene_seen!=scene_serial)rift_sound_reset(s);
 for(int i=0;i<frames;i++){
  if(s->observed_serial!=fx_serial){
   s->observed_serial=fx_serial;
   /* Cursor clicks cannot truncate a damage transient. */
   if(!(fx_request==6 && (s->fx_kind==1||s->fx_kind==4) && s->fx_age<4000)){
    s->fx_kind=fx_request;s->fx_age=0;s->sfx_phase=0;
   }
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
    case 6: /* brief cursor click */
     if(s->fx_age<1100){
       int env=(1100-s->fx_age)*amp/1100;
       sfx=pulse(s->sfx_phase,64)*env/2;s->sfx_phase+=2200;
     }
     break;
    case 7: /* map/selection confirm */
     if(s->fx_age<2600){
       int env=(2600-s->fx_age)*amp/2600;
       sfx=pulse(s->sfx_phase,125)*env;
       s->sfx_phase+=1550+(s->fx_age/10);
     }
     break;

   }
   s->fx_age++;
  }
  /* Separate metallic voice: blocked hits and shield gain are audible even
     when an attack resolves in the same frame. Cursor sounds cannot cut it. */
  if(s->block_serial!=block_serial){s->block_serial=block_serial;s->block_age=0;s->block_phase=0;}
  if(s->block_age<6500){
   int a=(int)s->block_age,env=(6500-a)*1800/6500;
   int ring=pulse(s->block_phase,128)*env+pulse(s->block_phase*7u/3u,128)*env/2;
   int transient=a<700?((s->noise&1u)?1:-1)*(700-a)*2:0;
   sfx+=ring+transient;s->block_phase+=2350;s->block_age++;
  }
  /* Four independently decaying impact voices keep multi-hit attacks crisp.
     Larger hits add a longer low-frequency body, not just louder white noise. */
  if(impact_serial-s->impact_seen>32u)s->impact_seen=impact_serial-32u;
  while(s->impact_seen!=impact_serial){
   int v=(int)(s->impact_voice++&3u);
   s->impact_tier[v]=impact_requests[s->impact_seen++&31u];
   s->impact_age[v]=s->impact_phase[v]=0;s->impact_low[v]=0;
  }
  for(int v=0;v<4;v++){
   int tier=s->impact_tier[v],duration=tier==3?13200:tier==2?9000:6000,a=(int)s->impact_age[v];
   if(!tier||a>=duration)continue;
   int env=(duration-a)*4096/duration;env=env*env/4096;
   if(a<50)env=env*a/50;
   int noise=(int)((s->noise>>(v*3))&65535u)-32768;
   s->impact_low[v]+=(noise-s->impact_low[v])/3;
   int amp=tier==3?17000:tier==2?15000:13000;
   int tone=triangle(s->impact_phase[v]);
   int hit=(s->impact_low[v]*env/4096)*amp/32768;
   hit+=(tone*env/4096)*(amp*(tier+1)/3)/32768;
   if(a<1400)hit+=(triangle(s->impact_phase[v]*11u)*((1400-a)*env/1400)/4096)*amp/65536;
   /* Dry, bit-crushed crack above the bass body: audible on PSP speakers. */
   if(a<3000){int grit=(noise/4096)*4096;hit+=(grit*((3000-a)*env/3000)/4096)*amp/32768;}
   sfx+=hit;
   unsigned base=tier==3?8500000u:tier==2?12000000u:19000000u;
   s->impact_phase[v]+=base-(base/2u/(unsigned)duration)*(unsigned)a;
   s->impact_age[v]++;
  }
  if(s->power_seen!=power_serial){s->power_seen=power_serial;s->power_age=s->power_phase=0;}
  if(s->power_age<16000){
   int a=(int)s->power_age,env=a<1800?a*3000/1800:(16000-a)*3000/14200;
   int tone=triangle(s->power_phase)+triangle(s->power_phase*2u)/2;
   sfx+=tone*env/49152;s->power_phase+=29000000u+(unsigned)a*900u;s->power_age++;
  }
  /* Short three-note bell flourish, separate from attacks and cursor SFX. */
  if(s->victory_seen!=victory_serial){s->victory_seen=victory_serial;s->victory_age=0;for(int n=0;n<3;n++)s->victory_phase[n]=0;}
  if(s->victory_age<28665u){
   static const unsigned step[3]={50960076u,64188931u,76352335u};
   for(int n=0;n<3;n++){
    int a=(int)s->victory_age-n*3528,duration=21609;
    if(a>=0&&a<duration){
     int env=(duration-a)*4096/duration;if(a<160)env=env*a/160;
     int bell=triangle(s->victory_phase[n])/2+triangle(s->victory_phase[n]*2u)/4+triangle(s->victory_phase[n]*3u)/8;
     sfx+=(bell*env/4096)*4000/32768;s->victory_phase[n]+=step[n];
    }
   }
   s->victory_age++;
  }
  if(s->stars_seen!=stars_serial){s->stars_seen=stars_serial;s->stars_age=0;for(int n=0;n<3;n++)s->stars_phase[n]=0;}
  if(s->stars_age<15000u){
   static const unsigned step[3]={101920152u,128377862u,152704670u};
   for(int n=0;n<3;n++){int a=(int)s->stars_age-n*1323,duration=11025;
    if(a>=0&&a<duration){int env=(duration-a)*4096/duration;env=env*env/4096;if(a<80)env=env*a/80;
     int bell=triangle(s->stars_phase[n])/2+triangle(s->stars_phase[n]*3u)/6;
     sfx+=(bell*env/4096)*3800/32768;s->stars_phase[n]+=step[n];}}
   s->stars_age++;
  }
  out[i*2]=limit(sfx);
  out[i*2+1]=limit(sfx);
  s->pos++;
 }
}

int rift_sound_music_gain(const RiftSound*s){
 int gain=160;
 for(int i=0;i<4;i++)if(s->impact_tier[i]&&s->impact_age[i]<4410u){int g=48+(int)s->impact_age[i]*112/4410;if(g<gain)gain=g;}
 return gain;
}
