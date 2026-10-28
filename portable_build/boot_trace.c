/* PSP-3000 firmware 6.60 hardware-startup checkpoints.
   All records are synchronous, using only IoFileMgrForUser imports.
   Each checkpoint is independently closed, so even a later crash should
   leave the earliest successful records on the Memory Stick.
   Writing a trace may fail (read-only/full MS); lack of a file alone
   cannot identify the crashed stage. */
#include "boot_trace.h"
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>

#include "launch_path.h"
#define BOOT_LOG_PATH rift_launch_boot_log()
void boot_trace_reset(void) {
#ifdef RIFT_BOOT_TRACE
    SceUID f=sceIoOpen(BOOT_LOG_PATH, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    if(f>=0)sceIoClose(f);
#endif
}
void boot_trace(const char *stage,int result) {
#ifdef RIFT_BOOT_TRACE
    if(!stage)return;
    char buf[128];
    int n=snprintf(buf,sizeof(buf),"%s: %d\n",stage,result);
    if(n<=0)return;
    if(n>=(int)sizeof(buf))n=(int)sizeof(buf)-1;
    SceUID f=sceIoOpen(BOOT_LOG_PATH,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND,0777);
    if(f<0)return;
    sceIoWrite(f,buf,(SceSize)n);
    sceIoClose(f);
#else
    (void)stage;(void)result;
#endif
}

void boot_trace_text(const char *stage,const char *value) {
#ifdef RIFT_BOOT_TRACE
    if(!stage||!value)return;
    char buf[384];int n=snprintf(buf,sizeof(buf),"%s: %s\n",stage,value);
    if(n<=0)return;if(n>=(int)sizeof(buf))n=(int)sizeof(buf)-1;
    SceUID f=sceIoOpen(BOOT_LOG_PATH,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND,0777);
    if(f<0)return;sceIoWrite(f,buf,(SceSize)n);sceIoClose(f);
#else
    (void)stage;(void)value;
#endif
}
