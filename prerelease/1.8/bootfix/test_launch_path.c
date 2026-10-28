#include "launch_path.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 const char *a="ms0:/PSP/GAME/RIFTASCENT_A18/EBOOT.PBP";
 assert(rift_launch_capture(strlen(a)+1,a));
 assert(!strcmp(rift_launch_executable(),a));
 assert(!strcmp(rift_launch_directory(),"ms0:/PSP/GAME/RIFTASCENT_A18"));
 const char *b="ms0:/PSP/GAME/Rift Ascent Testing/EBOOT.PBP";
 assert(rift_launch_capture(strlen(b)+1,b));assert(!strcmp(rift_launch_executable(),b));
 assert(!strcmp(rift_launch_directory(),"ms0:/PSP/GAME/Rift Ascent Testing"));
 const char *c="ef0:/PSP/GAME/RIFTASCENT/EBOOT.PBP";
 assert(rift_launch_capture(strlen(c)+1,c));assert(!strcmp(rift_launch_boot_log(),"ef0:/RIFT_BOOT.LOG"));
 const char multi[]="host0:/games/test.pbp\0second argument\0";
 assert(rift_launch_capture(sizeof(multi),multi));assert(!strcmp(rift_launch_executable(),"host0:/games/test.pbp"));
 assert(rift_launch_capture(10,"EBOOT.PBP"));assert(!strcmp(rift_launch_directory(),"."));
 const char slash[]="/EBOOT.PBP";assert(rift_launch_capture(sizeof(slash),slash));assert(!strcmp(rift_launch_directory(),"/"));
 assert(!rift_launch_capture(0,0));assert(!strcmp(rift_launch_executable(),"EBOOT.PBP"));
 assert(!rift_launch_capture(6,0));assert(!rift_launch_capture(2,"EBOOT.PBP"));
 assert(!rift_launch_capture(strlen(a),a));
 assert(!rift_launch_capture(9,"DATA.ELF"));assert(!rift_launch_capture(4,"x\n\0"));
 char longpath[300];memset(longpath,'a',sizeof(longpath));longpath[299]=0;
 assert(!rift_launch_capture(sizeof(longpath),longpath));
 memset(longpath,'a',190);memcpy(longpath+190,"/EBOOT.PBP",11);assert(rift_launch_capture(201,longpath));
 memset(longpath,'a',192);memcpy(longpath+192,"/EBOOT.PBP",11);assert(!rift_launch_capture(203,longpath));
 puts("PASS launch paths: stock/renamed/spaced/ef0/host0, multiple args, relative and root paths; bounded malformed/unterminated/oversized input rejection");
}
