#include "launch_path.h"
#include <string.h>
#ifdef __PSP__
#include <pspiofilemgr.h>
#include "boot_trace.h"
#endif
static char executable[RIFT_LAUNCH_PATH_MAX];
static char directory[RIFT_LAUNCH_DIR_MAX]=".";
static int io_error;
static int assign_path(const char *path,unsigned length){
 unsigned last=0;int slash=0;
 if(!path||!length||length>=sizeof(executable))return 0;
 for(unsigned i=0;i<length;i++){
  unsigned char c=(unsigned char)path[i];
  if(c<32||c==127||c=='\\')return 0;
  if(c=='/'){last=i;slash=1;}
 }
 if(slash&&last>=sizeof(directory))return 0;
 if(length<4||path[length-4]!='.'||
   (path[length-3]!='p'&&path[length-3]!='P')||
   (path[length-2]!='b'&&path[length-2]!='B')||
   (path[length-1]!='p'&&path[length-1]!='P'))return 0;
 memcpy(executable,path,length);executable[length]=0;
 if(slash){if(!last){directory[0]='/';directory[1]=0;}
  else{memcpy(directory,path,last);directory[last]=0;}}
 else{directory[0]='.';directory[1]=0;}
 return 1;
}
int rift_launch_capture(unsigned bytes,const void *argp){
 executable[0]=0;directory[0]='.';directory[1]=0;io_error=0;
 if(!argp||!bytes)return 0;
 const char *p=(const char*)argp;unsigned n=0;
 while(n<bytes&&n<sizeof(executable)&&p[n])n++;
 if(n==bytes||n>=sizeof(executable))return 0;
 return assign_path(p,n);
}
const char *rift_launch_executable(void){return executable[0]?executable:"EBOOT.PBP";}
const char *rift_launch_directory(void){return directory;}
const char *rift_launch_boot_log(void){
 return !memcmp(executable,"ef0:",4)?"ef0:/RIFT_BOOT.LOG":"ms0:/RIFT_BOOT.LOG";
}
int rift_launch_last_io_error(void){return io_error;}
#ifdef __PSP__
static unsigned rlp_u32(const unsigned char*p){return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static int read_exact(int fd,void*dst,unsigned bytes){
 unsigned done=0;while(done<bytes){int n=sceIoRead(fd,(char*)dst+done,bytes-done);
  if(n<=0){io_error=n;return 0;}done+=(unsigned)n;}return 1;
}
static int validate_package(int fd){
 unsigned char hdr[40],sfo[1024],tail[56],bank[16];
 const char title[]="RIFT ASCENT 1.8 PRE-RELEASE";int found=0;
 if(!read_exact(fd,hdr,sizeof(hdr))||memcmp(hdr,"\0PBP",4))return 0;
 unsigned start=rlp_u32(hdr+8),stop=rlp_u32(hdr+12);
 if(start<40||stop<=start||stop-start>sizeof(sfo))return 0;
 if(sceIoLseek32(fd,(int)start,0)!=(int)start||!read_exact(fd,sfo,stop-start))return 0;
 for(unsigned i=0;i+sizeof(title)<=stop-start;i++)
  if(!memcmp(sfo+i,title,sizeof(title))){found=1;break;}
 if(!found)return 0; /* Never silently borrow another release's assets. */
 int end=sceIoLseek32(fd,0,2);
 if(end<72||sceIoLseek32(fd,end-56,0)!=end-56||!read_exact(fd,tail,56)||memcmp(tail,"RIFTM311",8))return 0;
 unsigned music=rlp_u32(tail+48);if(music>(unsigned)end-72)return 0;
 unsigned bankend=(unsigned)end-56-music;
 if(sceIoLseek32(fd,(int)bankend-16,0)!=(int)bankend-16||!read_exact(fd,bank,16)||memcmp(bank,"RIFTART1",8))return 0;
 if(!rlp_u32(bank+8)||rlp_u32(bank+8)>bankend-16||bankend-16-rlp_u32(bank+8)<rlp_u32(hdr+36))return 0;
 return sceIoLseek32(fd,0,0)==0;
}
static int try_package(const char*path){
 int fd=sceIoOpen(path,PSP_O_RDONLY,0);io_error=fd;
 if(fd<0){boot_trace_text("package open failed",path);boot_trace("package I/O result",fd);return fd;}
 if(!validate_package(fd)){sceIoClose(fd);io_error=-9;boot_trace_text("package invalid",path);return -1;}
 char copy[RIFT_LAUNCH_PATH_MAX];unsigned n=(unsigned)strlen(path);
 if(n>=sizeof(copy)){sceIoClose(fd);return -1;}
 memcpy(copy,path,n+1);if(!assign_path(copy,n)){sceIoClose(fd);io_error=-10;return -1;}
 io_error=0;boot_trace_text("package opened",executable);return fd;
}
int rift_launch_open_package(void){
 /* A valid launch argument is authoritative. Do not hide damaged packages
    by selecting a different installation when the active one is invalid. */
 if(executable[0])return try_package(executable);
 const char*const fallback[]={"EBOOT.PBP","ms0:/PSP/GAME/RIFTASCENT_A18/EBOOT.PBP","ef0:/PSP/GAME/RIFTASCENT_A18/EBOOT.PBP"};
 for(unsigned i=0;i<sizeof(fallback)/sizeof(fallback[0]);i++){
  int fd=try_package(fallback[i]);if(fd>=0)return fd;
 }
 return -1;
}
#endif
