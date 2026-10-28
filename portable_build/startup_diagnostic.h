#ifndef RIFT_STARTUP_DIAGNOSTIC_H
#define RIFT_STARTUP_DIAGNOSTIC_H
/* Asset-independent diagnostics: still works when the texture bank cannot open. */
#include "font.h"
static void startup_text(uint32_t*p,int x,int y,const char*s,uint32_t ink){
 int ox=x;
 for(;*s;s++){
  if(*s=='\n'||x>W-20-7){x=ox;y+=14;if(*s=='\n')continue;}
  unsigned c=(unsigned char)*s;if(c<32||c>126)c='?';
  for(int yy=0;yy<11;yy++)for(int xx=0;xx<7;xx++)
   if((rift_font[c-32][yy]&(1u<<xx))&&(unsigned)(x+xx)<W&&(unsigned)(y+yy)<H)
    p[(y+yy)*STRIDE+x+xx]=ink;
  x+=7;
 }
}
static void startup_loading(uint32_t*p){
 /* Clean black backbuffer while assets open. Never expose developer text. */
 for(unsigned i=0;i<STRIDE*H;i++)p[i]=0xff000000U;
}
static int startup_failure(void*vram,uint32_t*p,const char*reason,int code){
 char number[64];boot_trace(reason,code);
 for(unsigned i=0;i<STRIDE*H;i++)p[i]=0xff16100dU;
 startup_text(p,22,18,"RIFT ASCENT - STARTUP ERROR",0xffc7deeeU);
 startup_text(p,22,49,reason,0xffddddffU);
 snprintf(number,sizeof(number),"Error: %d / last I/O: %d",code,rift_launch_last_io_error());
 startup_text(p,22,73,number,0xffdededeU);
 startup_text(p,22,99,"Package being opened:",0xffdededeU);
 startup_text(p,22,119,rift_launch_executable(),0xffeeeeeeU);
 startup_text(p,22,187,"Please send RIFT_BOOT.LOG from the storage root.",0xffdededeU);
 startup_text(p,22,225,"Press X or HOME to return to the PSP menu.",0xffc7deeeU);
 sceDisplaySetFrameBuf(vram,STRIDE,PSP_DISPLAY_PIXEL_FORMAT_8888,PSP_DISPLAY_SETBUF_NEXTVSYNC);
 sceDisplayWaitVblankStart();SceCtrlData pad;int released=0;
 while(running){
  sceCtrlPeekBufferPositive(&pad,1);
  if(!(pad.Buttons&PSP_CTRL_CROSS))released=1;
  if(released&&(pad.Buttons&PSP_CTRL_CROSS))break;
  sceKernelDelayThread(20000);
 }
 sceKernelExitGame();return code;
}
#endif
