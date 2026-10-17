/* Minimal native PSP hardware/ARK-5 compatibility test.
   No game resources, save IO, audio, clock changes, or direct EDRAM access.
   Automatically exits after 30 seconds for safety. */
#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdio.h>

PSP_MODULE_INFO("Rift Hardware Test", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_MAIN_THREAD_STACK_SIZE_KB(64);

static volatile int running = 1;

static int exit_callback(int arg1, int arg2, void *common) {
    (void)arg1; (void)arg2; (void)common;
    running = 0;
    sceKernelExitGame();
    return 0;
}

static int callback_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    int cb = sceKernelCreateCallback("Rift safe exit", exit_callback, NULL);
    if (cb >= 0) sceKernelRegisterExitCallback(cb);
    sceKernelSleepThreadCB();
    return 0;
}

int main(void) {
    int th = sceKernelCreateThread("Rift exit handler", callback_thread, 0x11, 0xFA0, 0, NULL);
    if (th >= 0) sceKernelStartThread(th, 0, NULL);

    pspDebugScreenInit();
    pspDebugScreenSetTextColor(0xFF00FFFF);
    pspDebugScreenPrintf("RIFT HARDWARE TEST\n");
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenPrintf("Native PSP SDK test - NOT the game\n");
    pspDebugScreenPrintf("No music, save files or game assets loaded.\n");
    pspDebugScreenPrintf("If you see this, startup and display work.\n");
    pspDebugScreenPrintf("HOME or CROSS exits early.\n");
    pspDebugScreenPrintf("Automatic exit after 30 seconds.\n\n");

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
    for (int ticks=0; running && ticks<300; ++ticks) {
        SceCtrlData p = {0};
        sceCtrlPeekBufferPositive(&p, 1);
        if (p.Buttons & PSP_CTRL_CROSS) break;
        if ((ticks % 10) == 0) {
            pspDebugScreenSetXY(0, 9);
            pspDebugScreenPrintf("Hardware heartbeat: %3d seconds   ", ticks/10);
        }
        sceKernelDelayThread(100000);
    }
    sceKernelExitGame();
    return 0;
}
