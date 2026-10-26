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
#include "game.h"
#include "sound.h"
void game_render_set_overlay(int kind,int tab,int potion);
void game_render_set_enemy_windup(int ticks);
void game_render_feedback(const Game *before,const Game *after,int key,unsigned tick);
void game_render_preplay(const Game *g,unsigned tick);

#include <pspaudio.h>
#include <pspiofilemgr.h>
#include "boot_trace.h"

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
static volatile int running=1;
static volatile int music_battle=0;
static Game before_input;
static int inspect_open=0,inspect_tab=0,inspect_potion=0,inspect_scroll=0;
static int enemy_turn_delay=0,action_cooldown=0;

static RiftSound music;
static short audio_buffer[1024] __attribute__((aligned(64)));
static int audio_thread_id=-1;
#define RIFT_TRACK_COUNT 3
#define RIFT_MUSIC_FOOTER 40
static SceUID soundtrack_fd=-1;
static unsigned soundtrack_offset[RIFT_TRACK_COUNT],soundtrack_samples[RIFT_TRACK_COUNT];
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
 const char magic[8]={'R','I','F','T','M','1','1','9'};
 for(int i=0;i<8;i++)if(footer[i]!=(unsigned char)magic[i]){
   soundtrack_close();return 0;
 }
 soundtrack_total=read_u32(footer+32);
 if(soundtrack_total==0||soundtrack_total>(unsigned)(end-RIFT_MUSIC_FOOTER)){
   soundtrack_close();return 0;
 }
 soundtrack_base=(unsigned)end-RIFT_MUSIC_FOOTER-soundtrack_total;
 for(int i=0;i<RIFT_TRACK_COUNT;i++){
   soundtrack_offset[i]=read_u32(footer+8+i*8);
   soundtrack_samples[i]=read_u32(footer+12+i*8);
   if(soundtrack_samples[i]==0||
      soundtrack_offset[i]>=soundtrack_total||
      (soundtrack_samples[i]+1u)/2u>soundtrack_total-soundtrack_offset[i]){
      soundtrack_close();return 0;
   }
 }
 soundtrack_active=1;
 boot_trace("Rift 1.1.9 original tracks loaded",RIFT_TRACK_COUNT);
 return 1;
}
static int soundtrack_select(int track){
 if(!soundtrack_active||track<0||track>=RIFT_TRACK_COUNT)return 0;
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
     if(audio_length<=0){soundtrack_close();return 0;}
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
        int track=music_battle;
        if(soundtrack_active&&soundtrack_current!=track)
           soundtrack_select(track);
        /* No original synth music; accent=1 emits impact/SFX only. */
        /* SFX only. NEVER enable a synthesized music fallback. The
           final EBOOT is packaged with the user's ten supplied WAV tracks. */
        rift_audio_generate(&music,audio_buffer,512,track,1);
        for(int i=0;i<512;i++){
           if(soundtrack_active){
              if(audio_phase==0)audio_last=soundtrack_sample();
              audio_phase=(audio_phase+1u)&3u;
           }else audio_last=0;
           int l=soundtrack_active?audio_last+(int)audio_buffer[i*2]*2/3:
              (int)audio_buffer[i*2];
           int r=soundtrack_active?audio_last+(int)audio_buffer[i*2+1]*2/3:
              (int)audio_buffer[i*2+1];
           audio_buffer[i*2]=audio_clip(l);
           audio_buffer[i*2+1]=audio_clip(r);
        }
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0){
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
                  game_card_cost_now(&game,raw)==-1)) fx=(type==0?1:type==2?5:0);
        } else if(key==BTN_POTION)fx=3;
    }
    game_combat_fx_begin();
    game_action(&game,key);
    game_combat_fx_end();
    if(game.screen==VICTORY || game.screen==DEFEAT) {
        if(game.screen!=oldscreen && profile_record(&lifetime,&game))
            profile_save(&lifetime,"ms0:/PSP/GAME/RIFTASCENT/PROFILE.DAT");
    }
    if(oldscreen==COMBAT && key==BTN_BACK && game.hp<oldhp)fx=4;
    if(fx)game_effect_start(fx,tick);
    int moved=before_input.selection!=game.selection||before_input.scroll!=game.scroll||before_input.target!=game.target;
    int sound=0,events=0;game_combat_fx_events(&events);
    if((key==BTN_UP||key==BTN_DOWN||key==BTN_LEFT||key==BTN_RIGHT)&&before_input.screen==game.screen&&moved)sound=6;
    else if(key==BTN_OK&&before_input.screen!=COMBAT)sound=7;
    else if(before_input.screen!=game.screen&&oldscreen!=COMBAT)sound=5;
    else if(fx==3&&!events)sound=3;
    if(sound)rift_sound_effect(sound);
    if(before_input.screen!=game.screen ||
       before_input.hp!=game.hp||before_input.block!=game.block||
       before_input.hand_count!=game.hand_count||fx || events ||
       (key==BTN_BACK&&before_input.screen==COMBAT))
        game_render_feedback(&before_input,&game,key,tick);
}
static int run_game(SceSize bytes,void *argp) {
    (void)bytes;(void)argp;
    boot_trace("worker entered",1);
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
    boot_trace("save load result",game_load(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT"));
    profile_init(&lifetime);
    boot_trace("profile load result",profile_load(&lifetime,"ms0:/PSP/GAME/RIFTASCENT/PROFILE.DAT"));
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
        /* Save a validated snapshot from the original screen. NEVER store PAUSE_MENU
           and never exit if the memory-stick write failed. */
        if(game.screen==PAUSE_MENU && key==BTN_OK) {
            Game snapshot=game;
            snapshot.screen=game_pause_source();
            if(game_save(&snapshot,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT")){
                boot_trace("save and quit result",1);
                sceKernelExitGame();
                return 0;
            }else {
                boot_trace("save and quit FAILED",-1);
                key=0;
            }
        }
        game_render_update_fx(frame);
        /* Finish the short action clip before accepting another combat action.
           The OS, audio worker and controller polling continue during hitstop. */
        if(game_render_fx_busy(frame)&&key!=BTN_PAUSE)key=0;
        /* START opens a readable battle glossary; Triangle opens potions.
           Inputs are consumed while the inspector is open. */
        if(game.screen!=COMBAT && inspect_open)inspect_open=0;
        if(game.screen==COMBAT && key==BTN_START) {
           inspect_open=!inspect_open;
           if(inspect_open){inspect_tab=2;inspect_scroll=0;}
           rift_sound_effect(2);key=0;
        }else if(game.screen==COMBAT && !inspect_open && key==BTN_POTION) {
           inspect_open=1;inspect_tab=3;
           inspect_potion=game.potion_count>0?game.potion_count-1:0;
           key=0;
        }else if(inspect_open && game.screen==COMBAT) {
           if(key==BTN_LEFT||key==BTN_RIGHT||key==BTN_UP||key==BTN_DOWN)
              rift_sound_effect(6);
           if(key==BTN_BACK){inspect_open=0;key=0;}
           else if(key==BTN_LEFT){inspect_tab=(inspect_tab+4)%5;key=0;}
           else if(key==BTN_RIGHT){inspect_tab=(inspect_tab+1)%5;key=0;}
           else if(key==BTN_UP){
             if(inspect_tab==2&&inspect_scroll>0)inspect_scroll--;
             else if(inspect_tab==3 && inspect_potion>0)inspect_potion--;
             else if(inspect_tab==1 && game.enemy_count>0)
               game.target=(game.target+game.enemy_count-1)%game.enemy_count;
             key=0;
           }else if(key==BTN_DOWN){
             if(inspect_tab==2&&inspect_scroll+7<game_status_count(&game))inspect_scroll++;
             else if(inspect_tab==3&&inspect_potion+1<game.potion_count)inspect_potion++;
             else if(inspect_tab==1 && game.enemy_count>0)
               game.target=(game.target+1)%game.enemy_count;
             key=0;
           }else if(key==BTN_OK){
             if(inspect_tab==3 && game.potion_count>0){
                before_input=game;
                game_combat_fx_begin();
                game_use_potion(&game,inspect_potion);
                game_combat_fx_end();
                game_render_feedback(&before_input,&game,BTN_POTION,frame);
                game_effect_start(3,frame);rift_sound_effect(3);
                if(inspect_potion>=game.potion_count)inspect_potion=game.potion_count-1;
                if(game.potion_count==0)inspect_open=0;
             }
             key=0;
           }else key=0;
        }
        /* Pre-resolution card presentation: game state is deliberately
           untouched for 18 rendered frames, then the action resolves.
           This covers Strike, Defend, and cards that trigger autoplay. */
        if(game.screen==COMBAT && !inspect_open && !enemy_turn_delay &&
           !pending_card_ticks && !action_cooldown && key==BTN_OK &&
           game.selection>=0 && game.selection<game.hand_count){
            int raw=game.hand[game.selection];
            int cost=game_card_cost_now(&game,raw);
            if(game_card_is_playable(raw)&&(cost==-1||cost<=game.energy)){
                game_render_preplay(&game,frame);
                pending_card_ticks=18;
                key=0;
            }
        }
        if(pending_card_ticks>0){
            pending_card_ticks--;
            key=0;
            if(pending_card_ticks==0){
                step_action(BTN_OK,frame);
                action_cooldown=21;
            }
        }
        /* Separate "end turn" selection from engine resolution so enemies
           telegraph their attack, then damage numbers remain visible. */
        if(game.screen==COMBAT&&!inspect_open&&!enemy_turn_delay &&
           (key==BTN_BACK || (key==BTN_OK&&game.selection>=game.hand_count))){
            enemy_turn_delay=23;
            action_cooldown=0;
            rift_sound_effect(7);
            key=0;
        }
        if(enemy_turn_delay>0){
            enemy_turn_delay--;
            if(enemy_turn_delay==0){
               step_action(BTN_BACK,frame);
               action_cooldown=20;
            }
            key=0;
        }
        if(action_cooldown>0){
           action_cooldown--;
           key=0;
        }
        if(key) {
           int prevscreen=game.screen;
           step_action(key,frame);
           if(prevscreen==COMBAT&&game.screen==REWARD)action_cooldown=44;
           else if(prevscreen!=game.screen)action_cooldown=14;
           else if(prevscreen==COMBAT&&key==BTN_OK)action_cooldown=10;
        }
        if(game.screen==REWARD&&action_cooldown>0)key=0;
        game_render_update_fx(frame);
        game_render_set_enemy_windup(enemy_turn_delay);
        game_render_set_overlay(inspect_open,inspect_tab,inspect_potion);
        game_render_set_status_scroll(inspect_scroll);
        if(key && game.screen!=TITLE && game.screen!=HELP &&
           game.screen!=COMBAT && game.screen!=DEFEAT)
            game_save(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT");
        if(game.screen!=last_screen || game.act!=last_act || game.floor!=last_floor) {
            if(game.screen==MAP || game.screen==VICTORY || game.screen==DEFEAT)
                game_save(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT");
            last_screen=game.screen;last_act=game.act;last_floor=game.floor;
        }
        game_update(&game);
        uint32_t *next_frame=(frame&1U)?uncached1:uncached0;
        game_render_frame(&game,next_frame,STRIDE,frame);
        music_battle=(game.screen==TITLE?0:
   (game.screen==COMBAT||game.screen==CARD_CHOICE||
    (game.screen==DECK_VIEW&&game.prior_screen==COMBAT)||
    (game.screen==PAUSE_MENU&&(game_pause_source()==COMBAT||game_pause_source()==CARD_CHOICE)))
    ?(game.battle_type==NODE_ELITE||game.battle_type==NODE_BOSS?0:2)
    :1);
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
        frame++;
    }
    sceKernelExitGame();
    return 0;
}
int _start(SceSize args,void *argp) {
    (void)args;(void)argp;
    boot_trace_reset();
    boot_trace("PSP module start",1);
    SceUID th=sceKernelCreateThread("Rift Bare Game",run_game,0x20,
                                  128*1024,PSP_THREAD_ATTR_USER,0);
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
