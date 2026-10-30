#include "ascent_paths.h"
#include <string.h>
static char executable[320]="ms0:/PSP/GAME/RIFTASCENT_BETA/EBOOT.PBP";
static char directory[320]="ms0:/PSP/GAME/RIFTASCENT_BETA";
void ascent_launch_path(unsigned bytes,const void*args){
 const char*s=args;unsigned len=0,slash=0;
 if(!s||!bytes)return;
 while(len<bytes&&len<sizeof(executable)-1&&s[len]){if(s[len]=='/')slash=len;len++;}
 if(len==bytes||len>=sizeof(executable)-1||!slash||len<8)return;
 if(memcmp(s+len-4,".PBP",4)&&memcmp(s+len-4,".pbp",4))return;
 memcpy(executable,s,len);executable[len]=0;
 memcpy(directory,s,slash);directory[slash]=0;
}
const char*ascent_eboot_path(void){return executable;}
const char*ascent_directory(void){return directory;}
