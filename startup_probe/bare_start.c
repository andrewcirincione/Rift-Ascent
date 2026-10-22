/*
 * PSPSDK hardware-startup isolation: bypass crt0/newlib/libcglue entirely.
 * Diagnostic experiment ONLY. No save writes, no audio, no networking.
 * The 15s auto-exit is cooperative, NOT a safety watchdog on hardware.
 */
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <psploadexec.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <psptypes.h>

PSP_MODULE_INFO("Rift Bare Startup", PSP_MODULE_USER, 1, 1);

#define WIDTH 480
#define HEIGHT 272
#define STRIDE 512

static int run_display(SceSize bytes, void *argp) {
    (void)bytes;
    (void)argp;
    void *vram = sceGeEdramGetAddr();
    if (!vram) {
        sceKernelExitGame();
        return 1;
    }
    volatile unsigned int *pixels = (volatile unsigned int *)((unsigned int)vram | 0x40000000U);
    int y, x;
    for (y = 0; y < HEIGHT; ++y) {
        unsigned int color = (y < HEIGHT/3) ? 0xFF206030U :
                             (y < (HEIGHT*2)/3) ? 0xFF4040A0U : 0xFF40A0B0U;
        for (x = 0; x < STRIDE; ++x) {
            pixels[y * STRIDE + x] = (x < WIDTH) ? color : 0;
        }
    }
    if (sceDisplaySetMode(PSP_DISPLAY_MODE_LCD, WIDTH, HEIGHT) >= 0) {
        sceDisplaySetFrameBuf(vram, STRIDE, PSP_DISPLAY_PIXEL_FORMAT_8888,
                             PSP_DISPLAY_SETBUF_IMMEDIATE);
    }
    sceKernelDelayThread(15000000);
    sceKernelExitGame();
    return 0;
}

int _start(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    SceUID thread = sceKernelCreateThread("Rift Bare Worker", run_display,
                                          0x20, 8192, PSP_THREAD_ATTR_USER, 0);
    if (thread >= 0) {
        sceKernelStartThread(thread, 0, 0);
    }
    return 0;
}

/* Same module entry export ABI that PSPSDK crt0.c normally supplies. */
static const unsigned int rift_entries[4]
    __attribute__((section(".rodata.sceResident"), used)) = {
    0xD632ACDBU, 0xF01D73A7U, (unsigned int)&_start,
    (unsigned int)&module_info
};
static const struct {
    const char *name;
    unsigned short version;
    unsigned short attribute;
    unsigned char entry_size;
    unsigned char variable_count;
    unsigned short function_count;
    const void *entry_table;
} rift_export __attribute__((section(".lib.ent"), used)) = {
    0, 0, 0x8000, 4, 1, 1, rift_entries
};
