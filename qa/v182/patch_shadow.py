from pathlib import Path
p=Path(__file__).parent/'source';f=p/'render.c';s=f.read_text();mark='static void r160_sprite('
func='''/* A small, soft silhouette shadow uses the live sprite alpha. No extra art,
   asset reads or persistent state; a five-tap kernel keeps the PSP cost small. */
static int r182_shadows=1;
static void r182_shadow(int x,int y,int w,int h,const void*src,int format,const uint32_t*palette,int iw,int ih,int left,int top,int cw,int ch,int flip,int lean,int opacity){
 if(!r182_shadows||w<1||h<1||w>192||h>128||opacity<1)return;
 unsigned char mask[192*128];
 for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
  int sx=xx*cw/w;if(flip)sx=cw-1-sx;int at=(top+yy*ch/h)*iw+left+sx;
  mask[yy*w+xx]=format==4?((const uint32_t*)src)[at]>>24:format==2?(((const uint16_t*)src)[at]!=RIFT_TRANSPARENT_565?255:0):palette[((const unsigned char*)src)[at]]>>24;
 }
 for(int yy=-1;yy<=h;yy++)for(int xx=-1;xx<=w;xx++){
  int a=0;
  if(xx>=0&&xx<w){if(yy>=0&&yy<h)a+=mask[yy*w+xx]*2;if(yy>0&&yy<=h)a+=mask[(yy-1)*w+xx];if(yy>=-1&&yy<h-1)a+=mask[(yy+1)*w+xx];}
  if(yy>=0&&yy<h){if(xx>0&&xx<=w)a+=mask[yy*w+xx-1];if(xx>=-1&&xx<w-1)a+=mask[yy*w+xx+1];}
  a=a*opacity/4250; /* 36% at the opaque center, feathered at the edge. */
  int dx=x+xx+2+lean*(h-CLAMP(yy,0,h))/h,dy=y+yy+2;
  if(a&&dx>=22&&dx<W-10&&dy>=54&&dy<174)pix[dy*sw+dx]=r134_mix(pix[dy*sw+dx],RGB(3,3,6),a);
 }
}
'''
s=s.replace(mark,func+mark,1).replace('const Rift160Sprite*src,int combat){\n for','const Rift160Sprite*src,int combat){\n if(combat)r182_shadow(x,y,w,h,src->pixels,1,src->palette,144,96,0,0,144,96,0,0,255);\n for',1);f.write_text(s)
f=p/'combat_fx.inc';s=f.read_text();s=s.replace('src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n for', 'src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n if(r138_hero_clip||src==rift_asset_resolve(r1310_hound,144*96*4))r182_shadow(x,y,w,h,src,4,0,iw,ih,0,0,iw,ih,flip,0,opacity);\n for',1)
# Keep the hound comparison on its descriptor; do not resolve another asset for every FX.
s=s.replace('src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n if(r138_hero_clip||src==rift_asset_resolve(r1310_hound,144*96*4))','int actor_shadow=r138_hero_clip||src==r1310_hound;\n src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n if(actor_shadow)')
s=s.replace('src=rift_asset_resolve(src,(unsigned)iw*ih*2);if(!src)return;\n for','src=rift_asset_resolve(src,(unsigned)iw*ih*2);if(!src)return;\n if(opacity>45)r182_shadow(x,y,w,h,src,2,0,iw,ih,left,top,cw,ch,flip,lean,opacity);\n for',1)
start=s.index('static void r170_rgba_actor');a=s[:start];b=s[start:].replace('src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n for','src=rift_asset_resolve(src,(unsigned)iw*ih*4);if(!src)return;\n r182_shadow(x,y,w,h,src,4,0,iw,ih,0,0,iw,ih,0,lean,opacity);\n for',1);f.write_text(a+b)
