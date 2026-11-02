/* Small freestanding C helpers for Rift Ascent's CRT-free PSP build.
 * No newlib FILE, heap or global reentrancy state is initialized or accessed.
 * File operations are intentionally disabled in this FIRST bootable prototype.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void *memset(void *p, int value, size_t n) {
    unsigned char *d=(unsigned char *)p;
    for(size_t i=0;i<n;i++) d[i]=(unsigned char)value;
    return p;
}
void *memcpy(void *dest,const void *src,size_t n) {
    unsigned char *d=(unsigned char *)dest;
    const unsigned char *s=(const unsigned char *)src;
    for(size_t i=0;i<n;i++) d[i]=s[i];
    return dest;
}
void *memmove(void *dest,const void *src,size_t n) {
    unsigned char *d=(unsigned char *)dest;
    const unsigned char *s=(const unsigned char *)src;
    if (d>s && d<s+n) {
        for(size_t i=n;i>0;i--)d[i-1]=s[i-1];
    } else {
        for(size_t i=0;i<n;i++)d[i]=s[i];
    }
    return dest;
}
int memcmp(const void *a,const void *b,size_t n) {
    const unsigned char *x=a,*y=b;
    for(size_t i=0;i<n;i++)if(x[i]!=y[i])return (int)x[i]-(int)y[i];
    return 0;
}
size_t strlen(const char *s) {
    size_t n=0;while(s[n])n++;return n;
}
int abs(int i) {return i<0?-i:i;}

typedef struct {
    char *dest;
    size_t cap;
    size_t length;
} Output;
static void append(Output *o,char ch) {
    if(o->cap && o->length+1<o->cap) o->dest[o->length]=ch;
    o->length++;
}
static void append_str(Output *o, const char *s, int max_chars) {
    if(!s)s="(null)";
    while(*s && (max_chars<0 || max_chars-->0))append(o,*s++);
}
static void append_num(Output *o,unsigned long n,int negative,int width,int zero) {
    char buf[32];int k=0;
    do {buf[k++]=(char)('0'+n%10);n/=10;}while(n&&k<31);
    int pad=width-k-negative;
    if(!zero)while(pad-->0)append(o,' ');
    if(negative)append(o,'-');
    if(zero)while(pad-->0)append(o,'0');
    while(k>0)append(o,buf[--k]);
}
int vsnprintf(char *s,size_t cap,const char *fmt,va_list va) {
    Output out={s,cap,0};
    while(*fmt){
        if(*fmt!='%'){append(&out,*fmt++);continue;}
        fmt++;
        if(*fmt=='%'){append(&out,'%');fmt++;continue;}
        int zero=0,width=0,precision=-1,lflag=0;
        if(*fmt=='0'){zero=1;fmt++;}
        while(*fmt>='0'&&*fmt<='9'){width=width*10+(*fmt++-'0');}
        if(*fmt=='.'){
            fmt++;
            if(*fmt=='*'){precision=va_arg(va,int);fmt++;}
            else{precision=0;while(*fmt>='0'&&*fmt<='9')precision=precision*10+(*fmt++-'0');}
        }
        if(*fmt=='l'){lflag=1;fmt++;if(*fmt=='l'){lflag=2;fmt++;}}
        char type=*fmt;if(type)fmt++;
        if(type=='s'){append_str(&out,va_arg(va,const char*),precision);}
        else if(type=='c'){append(&out,(char)va_arg(va,int));}
        else if(type=='d'||type=='i'){
            long n=lflag?va_arg(va,long):va_arg(va,int);
            int neg=n<0;
            unsigned long u=neg?(unsigned long)(-(n+1))+1UL:(unsigned long)n;
            append_num(&out,u,neg,width,zero);
        } else if(type=='u') {
            unsigned long n=lflag?va_arg(va,unsigned long):va_arg(va,unsigned int);
            append_num(&out,n,0,width,zero);
        } else if(type)append(&out,'?');
    }
    if(cap){size_t stop=out.length<cap?out.length:cap-1;out.dest[stop]=0;}
    return (int)out.length;
}
int snprintf(char *s,size_t cap,const char *fmt,...) {
    va_list va;va_start(va,fmt);
    int n=vsnprintf(s,cap,fmt,va);va_end(va);return n;
}

/* Portable file backend implemented with PSP's user-mode IoFileMgrForUser
 * syscalls. No newlib startup, errno or FILE internals are required.
 * "FILE *" is only an opaque token to this game's own fread/fwrite calls.
 */
#include <pspiofilemgr.h>
typedef struct {
    SceUID handle;
    int in_use;
} FileHandle;
static FileHandle open_handles[4];

FILE *fopen(const char *path,const char *mode) {
    int flags;
    if(!path || !mode)return 0;
    if(mode[0]=='r')flags=PSP_O_RDONLY;
    else if(mode[0]=='w')flags=PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC;
    else return 0;
    int slot=-1;
    for(int i=0;i<4;i++)if(!open_handles[i].in_use){slot=i;break;}
    if(slot<0)return 0;
    SceUID fd=sceIoOpen(path,flags,0777);
    if(fd<0)return 0;
    open_handles[slot].handle=fd;
    open_handles[slot].in_use=1;
    return (FILE *)&open_handles[slot];
}
static FileHandle *validate_file(FILE *f) {
    for(int i=0;i<4;i++)if((FILE*)&open_handles[i]==f && open_handles[i].in_use)
        return &open_handles[i];
    return 0;
}
size_t fread(void *dst,size_t size,size_t n,FILE *stream) {
    FileHandle *f=validate_file(stream);
    if(!f || !size || !dst || n>(size_t)(0x7fffffffu/size))return 0;
    size_t count=size*n;
    int read=sceIoRead(f->handle,dst,(SceSize)count);
    return read>=0?(size_t)read/size:0;
}
size_t fwrite(const void *src,size_t size,size_t n,FILE *stream) {
    FileHandle *f=validate_file(stream);
    if(!f || !size || !src || n>(size_t)(0x7fffffffu/size))return 0;
    size_t count=size*n;
    int written=sceIoWrite(f->handle,src,(SceSize)count);
    return written>=0?(size_t)written/size:0;
}
int fclose(FILE *stream) {
    FileHandle *f=validate_file(stream);
    if(!f)return -1;
    int result=sceIoClose(f->handle);
    f->in_use=0;
    return result>=0?0:-1;
}

int remove(const char*path){return sceIoRemove(path)<0?-1:0;}
