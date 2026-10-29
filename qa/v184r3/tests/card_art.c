#include <assert.h>
#include <stdio.h>
#ifdef BASELINE
#include "../../v184r2/source/game.c"
#include "../../v184r2/source/render.c"
#else
#include "../source/game.c"
#include "../source/render.c"
#endif
static uint32_t pixels[W*H],reference[W*H];
int main(void){rift_assets_set_file("versions/1.8.4-r2/EBOOT.PBP");assert(rift_assets_open());pix=pixels;sw=W;int cases=0,missing=0;
 const int flags[]={UPBIT,UPBIT|CARD_FREE,UPBIT|CARD_FORCE_EXHAUST,UPBIT|(2<<CARD_MOD_SHIFT)};
 for(int id=0;id<CARD_COUNT;id++){
  for(int i=0;i<W*H;i++)pixels[i]=0x5a5a5a5a;
  r134_card_art(0,0,69,32,id);assert(pixels[0]!=0x5a5a5a5a);memcpy(reference,pixels,sizeof(pixels));
  for(unsigned f=0;f<sizeof(flags)/sizeof(*flags);f++){
   for(int i=0;i<W*H;i++)pixels[i]=0x5a5a5a5a;
   r134_card_art(0,0,69,32,id|flags[f]);cases++;missing+=pixels[0]==0x5a5a5a5a;
#ifndef BASELINE
   assert(!memcmp(reference,pixels,sizeof(pixels)));
#endif
  }
 }
 assert(!rift_asset_errors);
#ifdef BASELINE
 assert(missing==cases);printf("CONFIRMED previous build: %d/%d flagged/upgraded thumbnails draw no artwork.\n",missing,cases);
#else
 assert(!missing);printf("PASS %d flagged/upgraded thumbnails exactly match their base artwork across all %d cards; zero missing images or streamed-art errors.\n",cases,CARD_COUNT);
#endif
 return 0;
}
