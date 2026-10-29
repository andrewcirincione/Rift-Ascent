#include <assert.h>
#include <time.h>
#include <stdio.h>
#include "../source/game.c"
#include "../source/render.c"
#include "blit_reference.inc"
static uint32_t fast[W*H],reference[W*H];
static void draw(int mode,int n){int x=(n%3==0?-9:n%3==1?17:355),y=n%5==0?-12:41,w=70+n%410,h=70+n%200;
 if(n%3==0){if(mode)reference_r134_panel(x,y,w,h,n%2);else r134_panel(x,y,w,h,n%2);}
 if(n%3==1){if(mode)reference_r134_blit(x,y,w,h,r134_menu_panel,480,240,n%256);else r134_blit(x,y,w,h,r134_menu_panel,480,240,n%256);}
 if(n%3==2){if(mode)reference_r134_card_frame(x,y,w,h,n%4);else r134_card_frame(x,y,w,h,n%4);}
}
int main(void){rift_assets_set_file("qa/v184/evidence/EBOOT.PBP");assert(rift_assets_open());sw=W;
 for(int n=0;n<500;n++){for(int i=0;i<W*H;i++)fast[i]=reference[i]=RGB(i%256,(i/3)%256,(i/7)%256);pix=fast;draw(0,n);pix=reference;draw(1,n);assert(!memcmp(fast,reference,sizeof(fast)));}
 puts("PASS 500 pixel-identical scaled alpha/panel/card frame cases, including off-screen clipping, multiple sizes, selected/dim states and opacity");
 double ms[2];for(int m=0;m<2;m++){pix=fast;clock_t start=clock();for(int n=0;n<3000;n++)draw(m,n);ms[m]=1000.0*(clock()-start)/CLOCKS_PER_SEC;}
 printf("Native representative drawing CPU: reference %.2f ms, optimized %.2f ms (%.1f%% less). PSP speed must be assessed separately.\n",ms[1],ms[0],100*(1-ms[0]/ms[1]));assert(!rift_asset_errors);return 0;}
