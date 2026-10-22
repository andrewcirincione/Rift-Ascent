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
#include <pspaudio.h>

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
static RiftSound music;
static short audio_buffer[1024] __attribute__((aligned(64)));
static int audio_thread_id=-1;
static int sound_worker(SceSize arglen,void *argp) {
    (void)arglen;(void)argp;
    int channel=sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO);
    if(channel<0)return 0;
    while(running) {
        rift_audio_generate(&music,audio_buffer,512,music_battle,0);
        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }
    sceAudioChRelease(channel);
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
    if (down & PSP_CTRL_START) return BTN_START;
    return 0;
}
static void step_action(int key,unsigned tick) {
    if(!key)return;
    int oldhp=game.hp;
    int oldscreen=game.screen;
    int fx=0;
    if(oldscreen==COMBAT) {
        if(key==BTN_OK && game.hand_count>0) {
            int raw=game.hand[game.selection];
            int type=cards[CARD_ID(raw)].type;
            if(cards[CARD_ID(raw)].cost!=-2 && 
                (cards[CARD_ID(raw)].cost<=game.energy ||
                  cards[CARD_ID(raw)].cost==-1)) fx=(type==0?1:type==1?2:3);
        } else if(key==BTN_POTION)fx=3;
    }
    game_action(&game,key);
    if(game.screen==VICTORY || game.screen==DEFEAT) {
        if(game.screen!=oldscreen && profile_record(&lifetime,&game))
            profile_save(&lifetime,"ms0:/PSP/GAME/RIFTASCENT/PROFILE.DAT");
    }
    if(oldscreen==COMBAT && key==BTN_BACK && game.hp<oldhp)fx=4;
    if(fx)game_effect_start(fx,tick);
}
static int run_game(SceSize bytes,void *argp) {
    (void)bytes;(void)argp;
    void *vram=sceGeEdramGetAddr();
    if(!vram) {sceKernelExitGame();return -1;}
    const unsigned frame_words=STRIDE*H;
    const unsigned frame_bytes=frame_words*4U;
    if(sceGeEdramGetSize()<2U*frame_bytes) {
        sceKernelExitGame();return -4;
    }
    uint32_t *uncached0=(uint32_t*)((uintptr_t)vram | (uintptr_t)0x40000000U);
    uint32_t *uncached1=(uint32_t*)(((uintptr_t)vram+frame_bytes) |
                                    (uintptr_t)0x40000000U);
    if(sceDisplaySetMode(PSP_DISPLAY_MODE_LCD,W,H)<0) {
        sceKernelExitGame();return -2;
    }
    /* Seed chosen without libc startup/time import; different in each new run. */
    game_new(&game,0x93ab6711U ^ sceKernelGetSystemTimeLow());
    game_load(&game,"ms0:/PSP/GAME/RIFTASCENT/SAVE.DAT");
    profile_init(&lifetime);
    profile_load(&lifetime,"ms0:/PSP/GAME/RIFTASCENT/PROFILE.DAT");
    game_render_set_profile(&lifetime);
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
    game_render_frame(&game,uncached0,STRIDE,0);
    if(sceDisplaySetFrameBuf(vram,STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,
                            PSP_DISPLAY_SETBUF_IMMEDIATE)<0) {
        sceKernelExitGame();return -3;
    }
    setup_home_callback();
    unsigned int last=0;
    unsigned int frame=1;
    int held=0;
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
        step_action(key,frame);
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
        music_battle=(game.screen==COMBAT);
        if(frame==12) {
            /* Audio starts only after the display has been rendered repeatedly. */
            audio_thread_id=sceKernelCreateThread("Rift audio",sound_worker,
                                                0x18,8192,PSP_THREAD_ATTR_USER,0);
            if(audio_thread_id>=0)sceKernelStartThread(audio_thread_id,0,0);
        }
        /* Present completed offscreen frame only during VBlank.
           Double buffering prevents partially drawn UI from appearing. */
        sceDisplayWaitVblankStart();
        sceDisplaySetFrameBuf((void*)((uintptr_t)vram+
                              (frame&1U)*frame_bytes),
                              STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,
                              PSP_DISPLAY_SETBUF_IMMEDIATE);
        last=down;
        frame++;
    }
    sceKernelExitGame();
    return 0;
}
int _start(SceSize args,void *argp) {
    (void)args;(void)argp;
    SceUID th=sceKernelCreateThread("Rift Bare Game",run_game,0x20,
                                  128*1024,PSP_THREAD_ATTR_USER,0);
    if(th>=0)sceKernelStartThread(th,0,0);
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
