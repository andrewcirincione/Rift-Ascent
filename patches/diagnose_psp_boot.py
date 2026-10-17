#!/usr/bin/env python3
"""Patch the packaged PSP game for hardware-visible boot diagnostics."""
from pathlib import Path
import sys
p = Path(sys.argv[1])
s = p.read_text()
def replace(old, new):
    global s
    if s.count(old) != 1:
        raise SystemExit("Unexpected source layout: "+repr(old[:75]))
    s = s.replace(old, new, 1)
replace('#include <stdint.h>', '#include <stdint.h>\n#include <pspiofilemgr.h>\n#include <stdio.h>')
replace('static volatile int running=1;', """
/* Diagnostic trace on memory stick ROOT: ms0:/RIFT_BOOT.LOG */
#define RIFT_BOOT_PATH "ms0:/RIFT_BOOT.LOG"
static void boot_mark(const char *stage, int code){
 SceUID fd=sceIoOpen(RIFT_BOOT_PATH, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
 if(fd<0)return;
 char msg[100];
 int n=snprintf(msg,sizeof(msg),"%s: %d\\n",stage,code);
 if(n>0)sceIoWrite(fd,msg,(SceSize)(n<(int)sizeof(msg)?n:(int)sizeof(msg)-1));
 sceIoClose(fd);
}
static volatile int running=1;""")
replace(' setup_callbacks();\n scePowerSetClockFrequency(333,333,166);', """
 sceIoRemove(RIFT_BOOT_PATH);
 boot_mark("entered main",1);
 setup_callbacks();
 boot_mark("callbacks registered",1);
 boot_mark("CPU clock result",scePowerSetClockFrequency(333,333,166));""")
replace(' if(sceGeEdramGetSize()<2u*FRAME_BYTES){sceKernelExitGame();return 1;}', """
 int video_memory=sceGeEdramGetSize();
 boot_mark("VRAM bytes",video_memory);
 if(video_memory<(int)(2u*FRAME_BYTES)){boot_mark("FATAL VRAM",video_memory);sceKernelExitGame();return 1;}""")
replace(' if(sceDisplaySetMode(PSP_DISPLAY_MODE_LCD,W,H)<0){sceKernelExitGame();return 1;}', """
 int display_result=sceDisplaySetMode(PSP_DISPLAY_MODE_LCD,W,H);
 boot_mark("display mode result",display_result);
 if(display_result<0){sceKernelExitGame();return 1;}""")
replace(' if(!edram){sceKernelExitGame();return 1;}', """
 boot_mark("EDRAM base",(int)(uintptr_t)edram);
 if(!edram){sceKernelExitGame();return 1;}""")
replace(' if(sceDisplaySetFrameBuf((void*)edram,FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,\n                         PSP_DISPLAY_SETBUF_IMMEDIATE)<0){sceKernelExitGame();return 1;}', """
 int framebuf_result=sceDisplaySetFrameBuf((void*)edram,FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,PSP_DISPLAY_SETBUF_IMMEDIATE);
 boot_mark("framebuffer init result",framebuf_result);
 if(framebuf_result<0){sceKernelExitGame();return 1;}
 for(int i=0;i<FRAME_STRIDE*H;i++)edram[i]=0xff10104bu;
 sceKernelDcacheWritebackAll();
 boot_mark("red diagnostic screen drawn",1);""")
replace(' game_new(&game,(uint32_t)sceKernelGetSystemTimeLow());', """
 game_new(&game,(uint32_t)sceKernelGetSystemTimeLow());
 boot_mark("game initialized",1);""")
replace(' game_load(&game,SAVE_PATH);', """
 boot_mark("save loading",1);
 boot_mark("save load result",game_load(&game,SAVE_PATH));""")
replace(' profile_load(&lifetime,PROFILE_PATH);', """
 boot_mark("profile load result",profile_load(&lifetime,PROFILE_PATH));""")
replace(' audio_thread_id=sceKernelCreateThread("Rift audio",audio_worker,0x18,0x1800,THREAD_ATTR_USER,0);\n if(audio_thread_id>=0)sceKernelStartThread(audio_thread_id,0,NULL);', """
 /* Delay audio until display has produced multiple frames. */
 boot_mark("entering render loop",1);""")
replace('  game_render_frame(&game,buffer,FRAME_STRIDE,(unsigned)frame);', """
  unsigned render_started=sceKernelGetSystemTimeLow();
  if(frame==0)boot_mark("first frame render started",1);
  game_render_frame(&game,buffer,FRAME_STRIDE,(unsigned)frame);
  if(frame==0)boot_mark("first frame rendering microseconds",(int)(sceKernelGetSystemTimeLow()-render_started));""")
replace('  sceDisplaySetFrameBuf((void*)(edram + (fb*FRAME_STRIDE*H)),FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,PSP_DISPLAY_SETBUF_NEXTFRAME);', """
  int flip_result=sceDisplaySetFrameBuf((void*)(edram + (fb*FRAME_STRIDE*H)),FRAME_STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,PSP_DISPLAY_SETBUF_NEXTFRAME);
  if(frame==0)boot_mark("first frame display flip result",flip_result);
  if(frame==3){
    boot_mark("four frames reached",1);
    audio_thread_id=sceKernelCreateThread("Rift audio",audio_worker,0x18,0x1800,THREAD_ATTR_USER,0);
    boot_mark("audio thread created",audio_thread_id);
    if(audio_thread_id>=0)boot_mark("audio thread start result",sceKernelStartThread(audio_thread_id,0,NULL));
  }
  if(frame==120)boot_mark("120 frames reached",1);""")
p.write_text(s)
print("Applied PSP memory stick startup diagnostics")
