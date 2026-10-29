/* Rift Ascent PSP-3000 CRT-free integration.
 * Keeps the actual game engine, UI, and all embedded sprite assets.
 * Built with -nostdlib -nostartfiles and explicit *user-mode* firmware imports.
 * Audio uses a deferred user-mode PSP thread; saving uses user-mode PSP I/O.
 * Hardware PSP-3000/6.60/ARK-5 launch has NOT been verified.
 */
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <psploadexec.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspctrl.h>
#include <psptypes.h>
#include <stdint.h>
#include <stdio.h>
#include "game.h"
#include "sound.h"
#include "asset_stream.h"
void game_render_set_overlay(int kind,int tab,int potion);
void game_render_set_enemy_windup(int ticks);
void game_render_feedback(const Game *before,const Game *after,int key,unsigned tick);
void game_render_preplay(const Game *g,unsigned tick);

#include <pspaudio.h>
#include <pspiofilemgr.h>
#include "boot_trace.h"
#include "ascent_paths.h"

/* PSPSDK crt0 usually declares linker boundary symbols. */
__asm__(
 ".section .lib.ent.top, \"a\", @progbits\n"
 ".balign 4\n.word 0\n.globl __lib_ent_top\n__lib_ent_top:\n"
 ".section .lib.ent.btm, \"a\", @progbits\n"
 ".balign 4\n.globl __lib_ent_bottom\n__lib_ent_bottom:\n.word 0\n"
 ".section .lib.stub.top, \"a\", @progbits\n"
 ".balign 4\n.word 0\n.globl __lib_stub_top\n__lib_stub_top:\n"
 ".section .lib.stub.btm, \"a\", @progbits\n"
 ".balign 4\n.globl __lib_stub_bottom\n__lib_stub_bottom:\n.word 0\n"
 ".text\n"
);
PSP_MODULE_INFO("Rift Ascent Bare", PSP_MODULE_USER, 1, 1);

#define STRIDE 512
static Game game;
static RiftProfile lifetime;
static char legacy_profile_path[360];
static volatile int running=1;
static volatile int music_battle=0;
static volatile int music_paused=0;
/* The audio thread reads track and mute state together, never a stale pair. */
static volatile unsigned music_route=0;
static Game before_input;
static int inspect_open=0,inspect_tab=0,inspect_potion=0,inspect_scroll=0;
static int enemy_turn_delay=0,action_cooldown=0;
static int plating_delay=0;

static RiftSound music;
static short audio_buffer[1024] __attribute__((aligned(64)));
static int audio_thread_id=-1;
#define RIFT_TRACK_COUNT 10
#define RIFT_MUSIC_FOOTER 136
static SceUID soundtrack_fd=-1;
static unsigned soundtrack_offset[RIFT_TRACK_COUNT],soundtrack_samples[RIFT_TRACK_COUNT],soundtrack_rate[RIFT_TRACK_COUNT];
static unsigned soundtrack_base=0,soundtrack_total=0;
static int soundtrack_current=-1,soundtrack_active=0;
static unsigned left_samples=0,track_seek=0;
static unsigned char audio_cache[4096] __attribute__((aligned(64)));
static int audio_pos=0,audio_length=0,audio_byte=0;
static int audio_predictor[2],audio_step_idx[2],audio_last[2];
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
 soundtrack_fd=sceIoOpen(ascent_eboot_path(),PSP_O_RDONLY,0);
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
 const char magic[8]={'R','I','F','T','A','U','1','8'};
 for(int i=0;i<8;i++)if(footer[i]!=(unsigned char)magic[i]){
   soundtrack_close();return 0;
 }
 soundtrack_total=read_u32(footer+128);
 if(soundtrack_total==0||soundtrack_total>(unsigned)(end-RIFT_MUSIC_FOOTER)){
   soundtrack_close();return 0;
 }
 soundtrack_base=(unsigned)end-RIFT_MUSIC_FOOTER-soundtrack_total;
 for(int i=0;i<RIFT_TRACK_COUNT;i++){
   soundtrack_offset[i]=read_u32(footer+8+i*12);
   soundtrack_samples[i]=read_u32(footer+12+i*12);
   soundtrack_rate[i]=read_u32(footer+16+i*12);
   if((soundtrack_rate[i]!=22050&&soundtrack_rate[i]!=44100)||soundtrack_samples[i]==0||
      soundtrack_offset[i]>=soundtrack_total||
      soundtrack_samples[i]>soundtrack_total-soundtrack_offset[i]){
      soundtrack_close();return 0;
   }
 }
 soundtrack_active=1;
 boot_trace("Rift 1.8 ten stereo music tracks loaded",RIFT_TRACK_COUNT);
 return 1;
}
static int soundtrack_select(int track){
 if(!soundtrack_active||track<0||track>=RIFT_TRACK_COUNT)return 0;
 track_seek=soundtrack_base+soundtrack_offset[track];
 if(sceIoLseek32(soundtrack_fd,(int)track_seek,0)<0)return 0;
 soundtrack_current=track;
 left_samples=soundtrack_samples[track];
 audio_pos=audio_length=audio_byte=0;
 for(int c=0;c<2;c++)audio_predictor[c]=audio_step_idx[c]=audio_last[c]=0;
 audio_phase=0;
 return 1;
}
static void soundtrack_sample(void){
 if(!soundtrack_active||soundtrack_current<0)return;
 if(!left_samples&&!soundtrack_select(soundtrack_current))return;
 if(audio_pos>=audio_length){
  unsigned want=left_samples<sizeof(audio_cache)?left_samples:sizeof(audio_cache);
  audio_length=sceIoRead(soundtrack_fd,audio_cache,want);audio_pos=0;
  if(audio_length<=0){soundtrack_close();audio_last[0]=audio_last[1]=0;return;}
 }
 unsigned byte=audio_cache[audio_pos++];
 for(int c=0;c<2;c++){
  int nibble=(byte>>(4*c))&15,step=adpcm_steps[audio_step_idx[c]],delta=step>>3;
  if(nibble&1)delta+=step>>2;if(nibble&2)delta+=step>>1;if(nibble&4)delta+=step;
  audio_predictor[c]+=(nibble&8)?-delta:delta;
  if(audio_predictor[c]>32767)audio_predictor[c]=32767;if(audio_predictor[c]<-32768)audio_predictor[c]=-32768;
  audio_step_idx[c]+=adpcm_index_delta[nibble];
  if(audio_step_idx[c]<0)audio_step_idx[c]=0;if(audio_step_idx[c]>88)audio_step_idx[c]=88;
  audio_last[c]=audio_predictor[c];
 }
 left_samples--;
}
static short audio_clip(int n){return (short)(n>31000?31000:(n< -31000?-31000:n));}
static int sound_worker(SceSize arglen,void *argp) {
    (void)arglen;(void)argp;
    int channel=sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO);
    if(channel<0)return 0;
    soundtrack_open();
    unsigned reopen_counter=0;
    while(running){
        /* Memory stick I/O may be invalidated by PSP suspend/resume. */
        if(!soundtrack_active && (++reopen_counter%160u)==0u)
            soundtrack_open();
        if(channel<0) {
            channel=sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO);
            if(channel<0){sceKernelDelayThread(50000);continue;}
        }
        unsigned route=music_route;int track=route&255,paused=(route>>8)&1;
        if(soundtrack_active&&soundtrack_current!=track)
           soundtrack_select(track);
        /* No original synth music; accent=1 emits impact/SFX only. */
        /* SFX only. NEVER enable a synthesized music fallback. The
           final EBOOT is packaged with the user's ten supplied WAV tracks. */
        rift_audio_generate(&music,audio_buffer,512,track,1);
        int gain=rift_sound_music_gain(&music);
        for(int i=0;i<512;i++){
           if(soundtrack_active&&!paused){
              if(audio_phase==0)soundtrack_sample();
              audio_phase=(audio_phase+1u)%(44100u/soundtrack_rate[track]);
           }else if(!soundtrack_active)audio_last[0]=audio_last[1]=0;
           int l=soundtrack_active&&!paused?audio_last[0]*gain/256+(int)audio_buffer[i*2]:
              (int)audio_buffer[i*2];
           int r=soundtrack_active&&!paused?audio_last[1]*gain/256+(int)audio_buffer[i*2+1]:
              (int)audio_buffer[i*2+1];
           audio_buffer[i*2]=audio_clip(l);
           audio_buffer[i*2+1]=audio_clip(r);
        }
        if(sceAudioOutputBlocking(channel,0x8000,audio_buffer)<0){
            /* A failed blocking output must not kill the sole music worker. */
            sceAudioChRelease(channel);
            channel=-1;
            soundtrack_close();
            rift_sound_reset(&music);
            reopen_counter=159u; /* retry stream immediately */
        }
    }
    soundtrack_close();
    if(channel>=0)sceAudioChRelease(channel);
    return 0;
}

/* OS exit callback: HOME can close the app when game threads are alive. */
static int exit_callback(int a,int b,void *ctx) {
    (void)a;(void)b;(void)ctx;
    running=0;
    sceKernelExitGame();
    return 0;
}
static int callbacks_worker(SceSize count,void *data) {
    (void)count;(void)data;
    int callback=sceKernelCreateCallback("Rift HOME Exit",exit_callback,0);
    if(callback>=0)sceKernelRegisterExitCallback(callback);
    sceKernelSleepThreadCB();
    return 0;
}
static void setup_home_callback(void) {
    SceUID th=sceKernelCreateThread("Rift HOME monitor",callbacks_worker,
                                   0x11,4096,PSP_THREAD_ATTR_USER,0);
    if(th>=0)sceKernelStartThread(th,0,0);
}
static int button(unsigned int down) {
    if (down & PSP_CTRL_CROSS) return BTN_OK;
    if (down & PSP_CTRL_CIRCLE) return BTN_BACK;
    if (down & PSP_CTRL_SQUARE) return BTN_ALT;
    if (down & PSP_CTRL_TRIANGLE) return BTN_POTION;
    if (down & PSP_CTRL_LEFT) return BTN_LEFT;
    if (down & PSP_CTRL_RIGHT) return BTN_RIGHT;
    if (down & PSP_CTRL_UP) return BTN_UP;
    if (down & PSP_CTRL_DOWN) return BTN_DOWN;
    if (down & PSP_CTRL_SELECT) return BTN_PAUSE;
    if (down & PSP_CTRL_START) return BTN_START;
    if (down & PSP_CTRL_LTRIGGER) return BTN_TAB_LEFT;
    if (down & PSP_CTRL_RTRIGGER) return BTN_TAB_RIGHT;
    return 0;
}
extern void game_render_set_status_scroll(int scroll);
static void step_action(int key,unsigned tick) {
    if(!key)return;
    before_input=game;
    int oldhp=game.hp;
    int oldscreen=game.screen;
    int fx=0;
    if(oldscreen==COMBAT) {
        if(key==BTN_OK && game.hand_count>0 &&
           game.selection>=0 && game.selection<game.hand_count) {
            int raw=game.hand[game.selection];
            int type=cards[CARD_ID(raw)].type;
            if(game_card_is_playable(raw) &&
                (game_card_cost_now(&game,raw)<=game.energy ||
                  game_card_cost_now(&game,raw)==-1)&&game_card_star_cost(&game,raw)<=game.regent.stars) fx=(type==0?1:type==2?5:0);
        } else if(key==BTN_POTION)fx=3;
    }
    game_combat_fx_begin();
    if(key==2048)game_update(&game);
    else if(key==1024)game_prepare_end_turn(&game);
    else game_action(&game,key);
    game_combat_fx_end();
    if(game.screen==VICTORY || game.screen==DEFEAT) {
        game_set_resume_available(1);
        if(game.screen!=oldscreen && profile_record(&lifetime,&game))
            profile_save(&lifetime,legacy_profile_path);
    }
    if(oldscreen==COMBAT && key==BTN_BACK && game.hp<oldhp)fx=4;
    if(fx)game_effect_start(fx,tick);
    int moved=before_input.selection!=game.selection||before_input.scroll!=game.scroll||before_input.target!=game.target||(game.screen==PAUSE_MENU&&(key==BTN_UP||key==BTN_DOWN));
    int sound=0,events=0;game_combat_fx_events(&events);
    if((key==BTN_UP||key==BTN_DOWN||key==BTN_LEFT||key==BTN_RIGHT)&&before_input.screen==game.screen&&moved)sound=6;
    else if(key==BTN_OK&&before_input.screen!=COMBAT)sound=7;
    else if(before_input.screen!=game.screen&&oldscreen!=COMBAT)sound=5;
    if((game_story_kind(&before_input)||game_story_kind(&game)||game_music_silent(&before_input)||game_music_silent(&game))&&key!=BTN_OK)sound=0;
    else if(fx==3&&!events)sound=3;
    if(sound)rift_sound_effect(sound);
    if(before_input.screen!=game.screen ||
       before_input.hp!=game.hp||before_input.block!=game.block||
       before_input.hand_count!=game.hand_count||before_input.potion_count!=game.potion_count||fx || events ||
       (oldscreen==COMBAT&&key==BTN_OK&&before_input.turn_played!=game.turn_played)||
       (key==BTN_BACK&&before_input.screen==COMBAT)||
       (key==BTN_OK&&before_input.screen==SHOP))
        game_render_feedback(&before_input,&game,key,tick);
}
static int run_game(SceSize bytes,void *argp) {
    (void)bytes;(void)argp;
    ascent_open(ascent_directory());
    snprintf(legacy_profile_path,sizeof(legacy_profile_path),"%s/PROFILE.DAT",ascent_directory());
    boot_trace("worker entered",1);
    int assets_ready=rift_assets_open();
    boot_trace("streamed artwork bank",assets_ready);
    if(!assets_ready){boot_trace("FATAL artwork bank",-8);sceKernelExitGame();return -8;}
    void *vram=sceGeEdramGetAddr();
    boot_trace("EDRAM base", (int)(uintptr_t)vram);
    if(!vram) {boot_trace("FATAL EDRAM", -1);sceKernelExitGame();return -1;}
    const unsigned frame_words=STRIDE*H;
    const unsigned frame_bytes=frame_words*4U;
    unsigned vram_size=sceGeEdramGetSize();
    boot_trace("VRAM size bytes",(int)vram_size);
    if(vram_size<2U*frame_bytes) {
        boot_trace("FATAL small VRAM", (int)vram_size);
        sceKernelExitGame();return -4;
    }
    uint32_t *uncached0=(uint32_t*)((uintptr_t)vram | (uintptr_t)0x40000000U);
    uint32_t *uncached1=(uint32_t*)(((uintptr_t)vram+frame_bytes) |
                                    (uintptr_t)0x40000000U);
    int mode_result=sceDisplaySetMode(PSP_DISPLAY_MODE_LCD,W,H);
    boot_trace("sceDisplaySetMode",mode_result);
    if(mode_result<0) {
        sceKernelExitGame();return -2;
    }
    /* Seed chosen without libc startup/time import; different in each new run. */
    boot_trace("game_new entering",1);
    game_new(&game,0x93ab6711U ^ sceKernelGetSystemTimeLow());
    boot_trace("game_new finished",game.screen);
    {int found=0;for(int i=0;i<ASCENT_SLOTS;i++)found|=ascent_slot(i)->present;
     game_set_resume_available(found);game.selection=found?0:1;}
    profile_init(&lifetime);
    boot_trace("profile load result",profile_load(&lifetime,legacy_profile_path));
    game_render_set_profile(&lifetime);
    boot_trace("profile set",1);
    rift_sound_reset(&music);
    game_render_set_overlay(0,0,0);
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
    boot_trace("render first entering",1);
    game_render_frame(&game,uncached0,STRIDE,0);
    boot_trace("render first finished",1);
    /* Physical PSP-3000: sync=0 was rejected as 0x80000107.
       Queue framebuffer for next vertical sync (sync=1). */
    boot_trace("initial framebuffer sync",PSP_DISPLAY_SETBUF_NEXTVSYNC);
    int fb_result=sceDisplaySetFrameBuf(vram,STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,
                            PSP_DISPLAY_SETBUF_NEXTVSYNC);
    boot_trace("first framebuffer result",fb_result);
    if(fb_result<0) {
        sceKernelExitGame();return -3;
    }
    /* sync=NEXTVSYNC queues the image for a FUTURE vertical blank.
       Let the display actually latch framebuffer 0 before drawing
       anything into the alternate framebuffer. */
    int initial_vblank=sceDisplayWaitVblankStart();
    boot_trace("initial frame committed at vblank",initial_vblank);
    if(initial_vblank<0) {
        boot_trace("FATAL initial vblank",initial_vblank);
        sceKernelExitGame();return -6;
    }
    setup_home_callback();
    boot_trace("exit callback setup",1);
    unsigned int last=0;
    unsigned int frame=1;
    unsigned int presentation_tick=4;
    int held=0;
    int pending_card_ticks=0;
    int last_screen=game.screen, last_act=game.act, last_floor=game.floor;
    SceCtrlData pad;
    while(running) {
        sceCtrlPeekBufferPositive(&pad,1);
        unsigned int down=pad.Buttons;
        int key=button(down & ~last);
        if(down&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT|PSP_CTRL_UP|PSP_CTRL_DOWN)) {
            if(down&last) {
                held++;
                if(held>15 && held%6==0)key=button(down);
            } else held=0;
        } else held=0;
#ifdef RIFT_SMOKE_DEMO
        /* CI-only deterministic virtual input, not included in release builds. */
        if(frame==4)key=BTN_OK;
        if(frame==9)key=BTN_OK;
        if(frame==14)key=BTN_OK;
        if(frame==19)key=BTN_OK;
#endif
        if(ascent_debug()&&!inspect_open&&(game.screen==COMBAT||game.screen==MAP||game.screen==REST||game.screen==SHOP||game.screen==EVENT||game.screen==REWARD)){
         if(key==BTN_TAB_LEFT)key=BTN_DEBUG_KILL;
         if(key==BTN_TAB_RIGHT)key=BTN_DEBUG_ACT;
        }
        key=game_render_story_input(&game,key,presentation_tick);
        /* Save a validated snapshot from the original screen. NEVER store PAUSE_MENU
           and never exit if the memory-stick write failed. */
        if(game.screen==PAUSE_MENU && key==BTN_OK && game_pause_selection()==0) {
            Game leaving=game;
            Game snapshot=game;
            snapshot.screen=game_pause_source();
            if(ascent_checkpoint(&snapshot)){
                boot_trace("save and quit result",1);
                game_new(&game,0x93ab6711U ^ sceKernelGetSystemTimeLow());
                game_set_resume_available(1);game.selection=0;key=0;
                inspect_open=0;pending_card_ticks=enemy_turn_delay=plating_delay=0;
                game_render_reset_fx();
                game_render_begin_transition(&leaving,presentation_tick);
            }else {
                boot_trace("save and quit FAILED",-1);
                key=0;
            }
        }
        music_paused=game_render_music_paused(presentation_tick);
        game_render_update_fx(presentation_tick);
        /* Finish the short action clip before accepting another combat action.
           The OS, audio worker and controller polling continue during hitstop. */
        if((game_render_fx_busy(presentation_tick)||plating_delay)&&key!=BTN_PAUSE)key=0;
        /* Shared overlay controller is exercised by native control audits. */
        if(key || inspect_open){
           int using_potion=inspect_open&&inspect_tab==3&&key==BTN_OK;
           if(using_potion){before_input=game;game_combat_fx_begin();}
           int handled=game_inventory_action(&game,key,&inspect_open,&inspect_tab,&inspect_scroll,&inspect_potion);
           if(using_potion)game_combat_fx_end();
           if(handled==2){game_render_feedback(&before_input,&game,BTN_POTION,presentation_tick);game_effect_start(3,presentation_tick);rift_sound_effect(3);}
           else if(handled&&key)rift_sound_effect(6);
           if(handled>=2&&game.screen!=COMBAT){
              if((game.screen==VICTORY||game.screen==DEFEAT)&&game.screen!=before_input.screen&&profile_record(&lifetime,&game))profile_save(&lifetime,legacy_profile_path);
              ascent_checkpoint(&game);
           }
           if(handled)key=0;
        }
        /* Pre-resolution card presentation: game state is deliberately
           untouched for six rendered frames, then the action resolves.
           This covers Strike, Defend, and cards that trigger autoplay. */
        if(game.screen==COMBAT && !inspect_open && !enemy_turn_delay && !plating_delay &&
           !pending_card_ticks && !action_cooldown && key==BTN_OK &&
           game.selection>=0 && game.selection<game.hand_count){
            int raw=game.hand[game.selection];
            int cost=game_card_cost_now(&game,raw);
            if(game_card_is_playable(raw)&&(cost==-1||cost<=game.energy)&&game_card_star_cost(&game,raw)<=game.regent.stars){
                game_render_preplay(&game,presentation_tick);
                pending_card_ticks=ascent_fast()?2:5;
                key=0;
            }
        }
        if(pending_card_ticks>0){
            pending_card_ticks--;
            key=0;
            if(pending_card_ticks==0){
                step_action(BTN_OK,presentation_tick);
                action_cooldown=ascent_fast()?1:4;
            }
        }
        /* Separate "end turn" selection from engine resolution so enemies
           telegraph their attack, then damage numbers remain visible. */
        if(game.screen==COMBAT&&!inspect_open&&!enemy_turn_delay&&!plating_delay &&
           (key==BTN_BACK || (key==BTN_OK&&game.selection>=game.hand_count))){
            step_action(1024,presentation_tick);
            if(game.screen==COMBAT)plating_delay=game.ic[IC_PLATING]>0?(ascent_fast()?5:14):1;
            action_cooldown=0;
            rift_sound_effect(7);
            key=0;
        }
        /* Resume a saved mid-turn Plating phase without granting it twice. */
        if(!game_render_transition_busy(presentation_tick)&&game.screen==COMBAT&&game.ic[38]&&!plating_delay&&!enemy_turn_delay)plating_delay=1;
        if(!game_render_transition_busy(presentation_tick)&&game.screen==COMBAT&&plating_delay>0){
            plating_delay--;
            if(!plating_delay&&!game_render_fx_busy(presentation_tick))enemy_turn_delay=ascent_fast()?2:6;
            else if(!plating_delay)plating_delay=1;
            key=0;
        }
        if(!game_render_transition_busy(presentation_tick)&&game.screen==COMBAT&&enemy_turn_delay>0){
            enemy_turn_delay--;
            if(enemy_turn_delay==0){
               step_action(BTN_BACK,presentation_tick);
               action_cooldown=ascent_fast()?1:4;
            }
            key=0;
        }
        if(action_cooldown>0){
           action_cooldown--;
           key=0;
        }
        if(key) {
           int prevscreen=game.screen;
           step_action(key,presentation_tick);
           if(prevscreen==COMBAT&&game.screen==REWARD)action_cooldown=15;
           else if(prevscreen!=game.screen)action_cooldown=5;
           else if(prevscreen==COMBAT&&key==BTN_OK)action_cooldown=ascent_fast()?1:4;
        }
        if(game.screen==REWARD&&action_cooldown>0)key=0;
        music_paused=game_render_music_paused(presentation_tick);
        game_render_update_fx(presentation_tick);
        game_render_set_enemy_windup(enemy_turn_delay*4);
        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);
        game_render_set_status_scroll(inspect_scroll);
        if((key==BTN_OK||key==BTN_BACK||key==BTN_DEBUG_KILL||key==BTN_DEBUG_ACT) && game.screen!=TITLE && game.screen!=HELP &&
           game.screen!=COMBAT && game.screen!=DEFEAT && game.screen!=PAUSE_MENU && game.screen!=CHARACTER_SELECT && game.screen!=NEW_RUN_CONFIRM)
            ascent_checkpoint(&game);
        if(game.screen!=last_screen || game.act!=last_act || game.floor!=last_floor) {
            if(game.screen==MAP || game.screen==VICTORY || game.screen==DEFEAT)
                ascent_checkpoint(&game);
            last_screen=game.screen;last_act=game.act;last_floor=game.floor;
        }
        if(!game_render_transition_busy(presentation_tick)&&(game.screen==ACT_INTRO||(game.screen==COMBAT&&(game.regent.task_count||game.regent.shuffle_pending||game.ninja.task_count))))step_action(2048,presentation_tick);
        uint32_t *next_frame=(frame&1U)?uncached1:uncached0;
        game_render_frame(&game,next_frame,STRIDE,presentation_tick);
        /* Keep its stream selected but paused after the killing contact. */
        const Game*audio_scene=game_render_audio_state(&game,presentation_tick);
        int story_audio=game_music_silent(audio_scene)?1:game_story_kind(audio_scene);
        rift_sound_scene_mode(story_audio);
        if(!game_render_fx_busy(presentation_tick))music_battle=game_music_track(audio_scene);
        music_paused=story_audio==1||game_render_transition_silent(&game,presentation_tick)||game_render_music_paused(presentation_tick);
        music_route=(music_battle&255)|(music_paused?256u:0u);
        if(frame==1)boot_trace("frame 1 rendered",game.screen);
        if(frame==12) {
            boot_trace("audio thread start",1);
            /* Audio starts only after the display has been rendered repeatedly. */
            audio_thread_id=sceKernelCreateThread("Rift audio",sound_worker,
                                                0x18,8192,PSP_THREAD_ATTR_USER,0);
            boot_trace("audio thread ID",audio_thread_id);
            if(audio_thread_id>=0)boot_trace("audio launch result",sceKernelStartThread(audio_thread_id,0,0));
        }
        /* PSP_DISPLAY_SETBUF_NEXTVSYNC queues a future swap.
           Queue -> wait for VBlank -> reuse the old backbuffer.
           Previously we waited BEFORE scheduling the switch, then
           immediately rendered into the still-visible buffer on
           real PSP hardware; that produces alternating UI flicker. */
        int flip_result=sceDisplaySetFrameBuf((void*)((uintptr_t)vram+
                              (frame&1U)*frame_bytes),
                              STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,
                              PSP_DISPLAY_SETBUF_NEXTVSYNC);
        if(frame==1 || frame==60)boot_trace("frame flip result",flip_result);
        if(flip_result<0) {
            boot_trace("FATAL framebuffer flip",flip_result);
            sceKernelExitGame();
            return -5;
        }
        int vblank_result=sceDisplayWaitVblankStart();
        if(frame==1 || frame==60)boot_trace("frame present vblank result",vblank_result);
        if(vblank_result<0) {
            boot_trace("FATAL frame vblank",vblank_result);
            sceKernelExitGame();
            return -7;
        }
        if(frame==60)boot_trace("frame 60 reached",game.screen);
        if(frame==180)boot_trace("frame 180 reached",game.screen);
        last=down;
        presentation_tick+=(ascent_fast()&&!game_render_transition_busy(presentation_tick)&&(game.screen==COMBAT||game_render_fx_busy(presentation_tick)))?12u:4u;
        frame++;
    }
    sceKernelExitGame();
    return 0;
}
int _start(SceSize args,void *argp) {
    ascent_launch_path(args,argp);
    boot_trace_reset();
    boot_trace("PSP module start",1);
    SceUID th=sceKernelCreateThread("Rift Bare Game",run_game,0x20,
                                  192*1024,PSP_THREAD_ATTR_USER,0);
    boot_trace("main thread ID",th);
    if(th>=0)boot_trace("main start result",sceKernelStartThread(th,0,0));
    else boot_trace("FATAL could not create thread",th);
    return 0;
}
static const unsigned int rift_entries[4]
 __attribute__((section(".rodata.sceResident"),used)) = {
 0xD632ACDBU, 0xF01D73A7U, (unsigned int)&_start,
 (unsigned int)&module_info
};
static const struct {
 const char *name;unsigned short version;unsigned short attribute;
 unsigned char entry_size;unsigned char variable_count;
 unsigned short function_count;const void *entry_table;
} rift_export __attribute__((section(".lib.ent"),used)) = {
 0,0,0x8000,4,1,1,rift_entries
};
