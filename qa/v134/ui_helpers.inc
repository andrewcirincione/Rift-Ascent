/* New UI artwork uses full alpha, preserving antialiased edges at native size. */
static uint32_t r134_mix(uint32_t bg,uint32_t fg,int a){
 int r=((bg&255)*(255-a)+(fg&255)*a+127)/255;
 int g=(((bg>>8)&255)*(255-a)+((fg>>8)&255)*a+127)/255;
 int b=(((bg>>16)&255)*(255-a)+((fg>>16)&255)*a+127)/255;
 return RGB(r,g,b);
}
static void r134_blit(int x,int y,int w,int h,const uint32_t*src,int iw,int ih,int opacity){
 if(w<=0||h<=0)return;
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
static void r134_panel(int x,int y,int w,int h,int selected){
 /* Nine-slice sculpted art: corner shapes never stretch into content. */
 int side=MIN(22,MIN(w/3,h/3));
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if((unsigned)dy>=H)continue;
  int sy=yy<side?yy*30/side:yy>=h-side?240-(h-yy)*30/side:30+(yy-side)*180/(h-2*side);
  sy=CLAMP(sy,0,239);
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if((unsigned)dx>=W)continue;
   int sx=xx<side?xx*30/side:xx>=w-side?480-(w-xx)*30/side:30+(xx-side)*420/(w-2*side);
   uint32_t c=r134_menu_panel[sy*480+CLAMP(sx,0,479)];
   if(!selected)c=r134_mix(c,RGB(8,12,18),27);
   pix[dy*sw+dx]=c|0xff000000u;
  }
 }
}
static int r134_width(const char*s,const RiftFont*f){int w=0;for(;*s;s++){unsigned c=(unsigned char)*s;if(c<32||c>126)c='?';w+=f->advance[c-32];}return w;}
static void r134_text(int x,int y,const char*s,uint32_t ink,const RiftFont*f){
 for(;s&&*s;s++){
  unsigned c=(unsigned char)*s;if(c<32||c>126)c='?';
  const uint8_t*g=f->alpha+(c-32)*f->w*f->h;
  for(int yy=0;yy<f->h;yy++)for(int xx=0;xx<f->w;xx++){
   int a=g[yy*f->w+xx],dx=x+xx,dy=y+yy;
   if(a&&(unsigned)dx<W&&(unsigned)dy<H){uint32_t*d=&pix[dy*sw+dx];*d=r134_mix(*d,ink,a);}
  }
  x+=f->advance[c-32];
 }
}
static void r134_center(int x,int y,const char*s,uint32_t ink,const RiftFont*f){r134_text(x-r134_width(s,f)/2,y,s,ink,f);}
static int r134_lines(const char*s,int width,const RiftFont*f,char rows[24][192]){
 int count=0;
 while(*s&&count<24){
  while(*s==' '||*s=='\n')s++;if(!*s)break;
  int k=0,last=-1,pixels=0;
  while(s[k]&&s[k]!='\n'&&k<190){unsigned c=(unsigned char)s[k];if(c<32||c>126)c='?';int adv=f->advance[c-32];if(pixels+adv>width&&k)break;pixels+=adv;if(s[k]==' ')last=k;k++;}
  if(s[k]&&s[k]!='\n'&&last>0)k=last;
  if(k<1)k=1;memcpy(rows[count],s,(size_t)k);rows[count][k]=0;count++;s+=k;
 }
 return count;
}
static int r134_wrap(int x,int y,int width,int maxrows,int leading,const char*s,uint32_t ink,const RiftFont*f,int ellipsis){
 char rows[24][192];int n=r134_lines(s,width,f,rows);
 for(int i=0;i<n&&i<maxrows;i++){
  if(ellipsis&&i==maxrows-1&&n>maxrows){int k=(int)strlen(rows[i]);while(k&&r134_width(rows[i],f)+r134_width("...",f)>width)rows[i][--k]=0;strcat(rows[i],"...");}
  r134_text(x,y+i*leading,rows[i],ink,f);
 }
 return n;
}
static uint32_t r134_565(uint16_t c){unsigned r=c>>11,g=(c>>5)&63,b=c&31;return RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));}
static void r134_card_art(int x,int y,int w,int h,int id){
 /* Bilinear reconstruction and a central crop; never paint over the frame. */
 const uint16_t*src=rift120_cards[id];int crop_h=CLAMP(60*h/w,18,32),crop_y=(40-crop_h)/2;
 for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
  int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;
  int u=w>1?xx*59*256/(w-1):0,v=crop_y*256+(h>1?yy*(crop_h-1)*256/(h-1):0);
  int ix=u>>8,iy=v>>8,ax=u&255,ay=v&255,nx=MIN(ix+1,59),ny=MIN(iy+1,39);
  uint32_t top=r134_mix(r134_565(src[iy*60+ix]),r134_565(src[iy*60+nx]),ax);
  uint32_t bot=r134_mix(r134_565(src[ny*60+ix]),r134_565(src[ny*60+nx]),ax);
  pix[dy*sw+dx]=r134_mix(top,bot,ay);
 }
}
