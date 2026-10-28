#include "asset_stream.h"
#include <stdint.h>
#include <string.h>
unsigned rift_asset_reads,rift_asset_errors,rift_asset_recoveries;
#ifdef RIFT_STREAMED_ART
#ifdef __PSP__
#include <pspiofilemgr.h>
static int asset_fd=-1;
static int disk_open(void){asset_fd=sceIoOpen("ms0:/PSP/GAME/RIFTASCENT/EBOOT.PBP",PSP_O_RDONLY,0);return asset_fd>=0;}
static int disk_seek(int offset,int origin){return sceIoLseek32(asset_fd,offset,origin);}
static int disk_read(void*dst,unsigned size){return sceIoRead(asset_fd,dst,size);}
static void disk_close(void){if(asset_fd>=0)sceIoClose(asset_fd);asset_fd=-1;}
#else
#include <stdio.h>
static FILE *asset_file;
static const char*asset_path="qa/v171/evidence/EBOOT.PBP";
void rift_assets_set_file(const char*path){rift_assets_close();asset_path=path;}
static int disk_open(void){asset_file=fopen(asset_path,"rb");return asset_file!=0;}
static int disk_seek(int offset,int origin){if(!asset_file||fseek(asset_file,offset,origin))return -1;return (int)ftell(asset_file);}
static int disk_read(void*dst,unsigned size){return (int)fread(dst,1,size,asset_file);}
static void disk_close(void){if(asset_file)fclose(asset_file);asset_file=0;}
#endif
#define PAGE_BYTES 65536u
#define CACHE_PAGES 64u
/* A bounded 4 MiB cache replaces roughly 16 MiB of resident texture arrays.
   No heap allocations, no decompression and no changes to any art pixels. */
static unsigned char texture_cache[PAGE_BYTES*CACHE_PAGES] __attribute__((aligned(64)));
typedef struct{unsigned offset,size,first,pages;int valid;} CacheEntry;
static CacheEntry entries[CACHE_PAGES];
static unsigned char owners[CACHE_PAGES];
static unsigned cursor,bank_base,bank_size;
static int opened;
static unsigned rd32(const unsigned char*p){return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static unsigned crc_table[256];static int crc_ready;
static unsigned checksum(const unsigned char*p,unsigned size){
 if(!crc_ready){for(unsigned i=0;i<256;i++){unsigned v=i;for(int k=0;k<8;k++)v=(v>>1)^((v&1)?0xedb88320u:0);crc_table[i]=v;}crc_ready=1;}
 unsigned v=~0u;for(unsigned i=0;i<size;i++)v=crc_table[(v^p[i])&255]^(v>>8);return ~v;
}
static int read_at(unsigned offset,void*dst,unsigned bytes){
 if(disk_seek((int)offset,0)!=(int)offset)return 0;
 unsigned done=0;while(done<bytes){int n=disk_read((unsigned char*)dst+done,bytes-done);if(n<=0)return 0;done+=(unsigned)n;}return 1;
}
int rift_assets_open(void){
 if(opened)return 1;
 unsigned char footer[56],bank[16];
 if(!disk_open())goto fail;
 int end=disk_seek(0,2);if(end<72||!read_at((unsigned)end-56,footer,56))goto fail;
 if(memcmp(footer,"RIFTM311",8))goto fail;
 unsigned music=rd32(footer+48);if(music>(unsigned)end-72)goto fail;
 unsigned bank_end=(unsigned)end-56-music;
 if(!read_at(bank_end-16,bank,16)||memcmp(bank,"RIFTART1",8))goto fail;
 bank_size=rd32(bank+8);if(!bank_size||bank_size>bank_end-16)goto fail;
 bank_base=bank_end-16-bank_size;opened=1;return 1;
fail: disk_close();bank_base=bank_size=0;rift_asset_errors++;return 0;
}
void rift_assets_close(void){disk_close();opened=0;cursor=0;memset(entries,0,sizeof(entries));memset(owners,0,sizeof(owners));}
#ifdef RIFT_ASSET_TEST
void rift_assets_test_break_io(void){disk_close();}
#endif
const void *rift_asset_resolve(const void*source,unsigned expected_bytes){
 const unsigned char*d=source;
 if(!source)return 0;
 if(memcmp(d,"RIFTSIMG",8))return source;
 unsigned offset=rd32(d+8),size=rd32(d+12),pixel_bytes=rd32(d+16),crc=rd32(d+20);
 if(size!=expected_bytes||(pixel_bytes!=2&&pixel_bytes!=4)||size%pixel_bytes||size>sizeof(texture_cache)){rift_asset_errors++;return 0;}
 if(!opened&&!rift_assets_open())return 0;
 if(offset>bank_size||size>bank_size-offset){rift_asset_errors++;return 0;}
 for(unsigned i=0;i<CACHE_PAGES;i++)if(entries[i].valid&&entries[i].offset==offset&&entries[i].size==size)return texture_cache+entries[i].first*PAGE_BYTES;
 unsigned pages=(size+PAGE_BYTES-1)/PAGE_BYTES;if(cursor+pages>CACHE_PAGES)cursor=0;
 for(unsigned i=cursor;i<cursor+pages;i++)if(owners[i]){
  CacheEntry*e=&entries[owners[i]-1];for(unsigned k=e->first;k<e->first+e->pages;k++)owners[k]=0;e->valid=0;
 }
 unsigned slot=0;while(slot<CACHE_PAGES&&entries[slot].valid)slot++;
 if(slot==CACHE_PAGES){rift_asset_errors++;return 0;}
 unsigned char*dest=texture_cache+cursor*PAGE_BYTES;
 if(!read_at(bank_base+offset,dest,size)||checksum(dest,size)!=crc){
  /* Suspend/transient Memory Stick faults invalidate descriptors. Reopen once,
     preserving good cache entries; failed partial data is never published. */
  disk_close();opened=0;
  if(!rift_assets_open()||offset>bank_size||size>bank_size-offset||
     !read_at(bank_base+offset,dest,size)||checksum(dest,size)!=crc){
   disk_close();opened=0;rift_asset_errors++;return 0;
  }
  rift_asset_recoveries++;
 }
 entries[slot]=(CacheEntry){offset,size,cursor,pages,1};
 for(unsigned i=cursor;i<cursor+pages;i++)owners[i]=(unsigned char)(slot+1);
 cursor+=pages;rift_asset_reads++;return dest;
}
#else
int rift_assets_open(void){return 1;}
void rift_assets_close(void){}
#ifdef RIFT_ASSET_TEST
void rift_assets_test_break_io(void){disk_close();}
#endif
const void *rift_asset_resolve(const void*source,unsigned expected_bytes){(void)expected_bytes;return source;}
#ifndef __PSP__
void rift_assets_set_file(const char*path){(void)path;}
#endif
#endif
