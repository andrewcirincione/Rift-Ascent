static void reference_r134_blit(int x,int y,int w,int h,const uint32_t*src,int iw,int ih,int opacity){
 if(w<=0||h<=0)return;
 src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if((unsigned)dy>=H)continue;
  const uint32_t*row=src+(yy*ih/h)*iw;
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if((unsigned)dx>=W)continue;
   uint32_t c=row[xx*iw/w];int a=(int)(c>>24)*opacity/255;if(!a)continue;
   uint32_t*d=&pix[dy*sw+dx];*d=a==255?c:r134_mix(*d,c,a);
  }
 }
}
static void reference_r134_panel(int x,int y,int w,int h,int selected){
 /* Nine-slice sculpted art: corner shapes never stretch into content. */
 const uint32_t*panel=rift_asset_resolve(r134_menu_panel,480u*240*4);if(!panel)return;
 int side=MIN(22,MIN(w/3,h/3));
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if((unsigned)dy>=H)continue;
  int sy=yy<side?yy*30/side:yy>=h-side?240-(h-yy)*30/side:30+(yy-side)*180/(h-2*side);
  sy=CLAMP(sy,0,239);
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if((unsigned)dx>=W)continue;
   int sx=xx<side?xx*30/side:xx>=w-side?480-(w-xx)*30/side:30+(xx-side)*420/(w-2*side);
   uint32_t c=panel[sy*480+CLAMP(sx,0,479)];
   if(!selected)c=r134_mix(c,RGB(8,12,18),27);
   pix[dy*sw+dx]=c|0xff000000u;
  }
 }
}
static void reference_r134_card_frame(int x,int y,int w,int h,int type){
 /* Preserve the generated sculpture but reserve a rectangular text interior.
    Nine-slicing keeps the corner ornaments proportional on hand thumbnails. */
 int small=w<105&&h<105;int side=w<100?7:12,top=small?11:20,base=small?9:16;
 const uint32_t*src=rift_asset_resolve(r134_card_frames[type],126u*166*4);if(!src)return;
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if((unsigned)dy>=H)continue;
  int sy=yy<top?yy*43/top:yy>=h-base?134+(yy-h+base)*32/base:43+(yy-top)*91/(h-top-base);
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if((unsigned)dx>=W)continue;
   int sx=xx<side?xx*24/side:xx>=w-side?102+(xx-w+side)*24/side:24+(xx-side)*78/(w-2*side);
   uint32_t c=src[sy*126+sx];int a=c>>24;if(a){uint32_t*d=&pix[dy*sw+dx];*d=r134_mix(*d,c,a);}
  }
 }
}
