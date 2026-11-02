/* Native PSP front-end: only PSPSDK-supported APIs. */
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <psppower.h>
#include <pspaudio.h>
#include "sound.h"
#include <stdint.h>
#include "game.h"

PSP_MODULE_INFO("Rift Ascent", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static Game game;
static RiftProfile lifetime;
static RiftSound music;
static short sound_buf[512*2] __attribute__((aligned(64)));
#define FRAME_STRIDE 512
#define FRAME_BYTES (FRAME_STRIDE*H*4)
#define SAVE_PATH "ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT"
#define PROFILE_PATH "ms0:/PSP/GAME/RIFTASCENT/PROFILE.DAT"
static volatile int running=1;
static volatile int music_battle=0;
static int audio_thread_id=-1;
/* Audio must be fed independently of display VBlank. Pumping a 512-sample
   buffer once per video frame would underrun at 44.1 kHz (512*60<44100). */
static int audio_worker(SceSize args,void *argp){
 (void)args;(void)argp;
 int channel=sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO);
 if(channel<0)return 0;
 while(running){
  rift_audio_generate(&music,sound_buf,512,music_battle,0);
  if(sceAudioOutputBlocking(channel,0x4a00,sound_buf)<0)break;
 }
 sceAudioChRelease(channel);
 return 0;
}
static int exit_callback(int arg1,int arg2,void*common){(void)arg1;(void)arg2;(void)common;running=0;return 0;}
static int callback_thread(SceSize args,void*argp){(void)args;(void)argp;
 int cb=sceKernelCreateCallback("Rift exit",exit_callback,NULL);
 if(cb>=0)sceKernelRegisterExitCallback(cb);
 sceKernelSleepThreadCB();return 0;
}
static void setup_callbacks(void){
 int th=sceKernelCreateThread("Rift callbacks",callback_thread,0x11,0xFA0,0,0);
 if(th>=0)sceKernelStartThread(th,0,0);
}
static int translate(unsigned int pressed){
 if(pressed&PSP_CTRL_CROSS)return BTN_OK;
 if(pressed&PSP_CTRL_CIRCLE)return BTN_BACK;
 if(pressed&PSP_CTRL_SQUARE)return BTN_ALT;
 if(pressed&PSP_CTRL_TRIANGLE)return BTN_POTION;
 if(pressed&PSP_CTRL_LEFT)return BTN_LEFT;
 if(pressed&PSP_CTRL_RIGHT)return BTN_RIGHT;
 if(pressed&PSP_CTRL_UP)return BTN_UP;
 if(pressed&PSP_CTRL_DOWN)return BTN_DOWN;
 if(pressed&PSP_CTRL_START)return BTN_START;
 return 0;
}
int main(int argc,char**argv){
 (void)argc;(void)argv;
 setup_callbacks();
 scePowerSetClockFrequency(333,333,166);
 sceCtrlSetSamplingCycle(0);
 sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
 /* The original PSP has exactly 2 MiB of VRAM. Verify that both RGBA8888
    framebuffers fit before touching EDRAM; abort cleanly on SDK failure. */
 if(sceGeEdramGetSize()<2u*FRAME_BYTES){sceKernelExitGame();return 1;}
 if(sceDisplaySetMode(PSP_DISPLAY_MODE_LCD,W,H)<0){sceKernelExitGame();return 1;}
 uint32_t*edram=(uint32_t*)sceGeEdramGetAddr();
 if(!edram){sceKernelExitGame();return 1;}
 if(sceDisplaySetFrameBuf((void*)edram,FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,
                         PSP_DISPLAY_SETBUF_IMMEDIATE)<0){sceKernelExitGame();return 1;}
 game_new(&game,(uint32_t)sceKernelGetSystemTimeLow());
 game_load(&game,SAVE_PATH);
 profile_init(&lifetime);
 profile_load(&lifetime,PROFILE_PATH);
 game_render_set_profile(&lifetime);
 /* Start audio only after the video subsystem and save state are valid. */
 audio_thread_id=sceKernelCreateThread("Rift audio",audio_worker,0x18,0x1800,THREAD_ATTR_USER,0);
 if(audio_thread_id>=0)sceKernelStartThread(audio_thread_id,0,NULL);
 SceCtrlData pad;
 unsigned int last=0;int last_screen=game.screen,last_act=game.act,last_floor=game.floor;
 int repeat=0,frame=0;
 while(running){
  sceCtrlPeekBufferPositive(&pad,1);
  unsigned int down=pad.Buttons;
  int input=translate(down & ~last);
  /* D-pad is repeatable so navigating longer decks is practical. */
  if((down&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT|PSP_CTRL_UP|PSP_CTRL_DOWN))){
   if((down&last)!=0){repeat++;if(repeat>15&&repeat%5==0)input=translate(down);}else repeat=0;
  }else repeat=0;
  if(input){
   int previous_hp=game.hp,previous_screen=game.screen;
   int fx_kind=0;
   if(previous_screen==COMBAT){
    if(input==BTN_OK&&game.hand_count>0){
     int raw=game.hand[game.selection];
     int type=cards[CARD_ID(raw)].type;
     if(cards[CARD_ID(raw)].cost!=-2&&(cards[CARD_ID(raw)].cost<=game.energy||cards[CARD_ID(raw)].cost==-1)){
      fx_kind=(type==0?1:type==1?2:3);
     }
    }else if(input==BTN_POTION)fx_kind=3;
   }
   game_action(&game,input);
   if((game.screen==VICTORY||game.screen==DEFEAT)&&
      game.screen!=previous_screen&&profile_record(&lifetime,&game)){
      profile_save(&lifetime,PROFILE_PATH);
   }
   if(previous_screen==COMBAT&&input==BTN_BACK&&game.hp<previous_hp)fx_kind=4;
   if(fx_kind)game_effect_start(fx_kind,(unsigned)frame);
   if(game.screen!=TITLE&&game.screen!=HELP&&game.screen!=COMBAT&&game.screen!=DEFEAT)game_save(&game,SAVE_PATH);}
  if(game.screen!=last_screen||game.act!=last_act||game.floor!=last_floor){
   if(game.screen==MAP||game.screen==VICTORY||game.screen==DEFEAT)game_save(&game,SAVE_PATH);
   last_screen=game.screen;last_act=game.act;last_floor=game.floor;
  }
  game_update(&game);
  int fb=frame&1;
  uint32_t*buffer=edram+(fb*FRAME_STRIDE*H);
  game_render_frame(&game,buffer,FRAME_STRIDE,(unsigned)frame);
  sceKernelDcacheWritebackAll();
  sceDisplayWaitVblankStart();
  sceDisplaySetFrameBuf((void*)(edram + (fb*FRAME_STRIDE*H)),FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,PSP_DISPLAY_SETBUF_NEXTFRAME);
  music_battle=game.screen==COMBAT;
  frame++;
  last=down;
 }
 game_save(&game,SAVE_PATH);
 profile_save(&lifetime,PROFILE_PATH);
 if(audio_thread_id>=0){
  sceKernelWaitThreadEnd(audio_thread_id,NULL);
  sceKernelDeleteThread(audio_thread_id);
 }
 sceKernelExitGame();
 return 0;
}
