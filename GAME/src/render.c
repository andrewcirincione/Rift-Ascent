#include "game.h"
#include "font_smooth.h"
#include "artwork.h"
#include "motion_art.h"
#include "rift112_scene_art.h"
#include "rift113_cards.h"
#include "rift120_cards.h"
#include "rift120_scenes.h"
#include "rift130_slots.h"
#include "rift133_art.h"
#include "rift134_art.h"
#include "rift134_fonts.h"
#include "rift135_art.h"
#include "rift137_art.h"
#include "rift138_art.h"
#include "rift1311_art.h"
#include "rift140_art.h"
#include "rift139_art.h"
#include "rift1310_art.h"
#include "sound.h"
#include "rift117_card_fonts.h"
#include "rift116_intent_icons.h"
#include "rift110_map_icons.h"
#include "enemy_art.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define RGB(r,g,b) ((uint32_t)(0xff000000u|((uint32_t)(b)<<16)|((uint32_t)(g)<<8)|(r)))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define CLAMP(x,a,b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define BG RGB(12,18,29)
#define PANEL RGB(25,35,49)
#define GOLD RGB(235,186,111)
#define WHITE RGB(231,229,210)
#define MUTED RGB(137,156,163)
#define RED RGB(223,91,79)
#define GREEN RGB(109,203,151)
#define BLUE RGB(89,171,210)
#define PURPLE RGB(161,112,186)
static uint32_t*pix;static int sw;
static const RiftProfile *profile_data;
void game_render_set_profile(const RiftProfile*p){profile_data=p;}
static unsigned animation_tick;
static char rift120_log[3][64];static int rift120_log_count=0;
static int rift133_play_x=35;
static int rift120_played_card=-1,rift120_auto_card=-1;static unsigned rift120_play_tick=0;
static Game r136_play_state;static int r136_play_state_valid;
static int r137_prepose;
void game_render_preplay(const Game *g,unsigned tick){
 if(g && g->screen==COMBAT && g->selection>=0 &&
    g->selection<g->hand_count){
   rift120_played_card=g->hand[g->selection];
   r136_play_state=*g;r136_play_state_valid=1;
   CardPreview preview;game_card_preview(g,rift120_played_card,g->target,&preview);
   int type=cards[CARD_ID(rift120_played_card)].type;
   r137_prepose=type==0?1:type==2?3:preview.block_count?2:0;
   int first=g->selection>=5?g->selection-4:0;
   rift133_play_x=56+(g->selection-first)*71;
   rift120_auto_card=-1;rift120_play_tick=tick;
 }
}
static void rift120_log_add(const char *s){
 if(!s||!*s)return;
 for(int i=0;i<2;i++)memcpy(rift120_log[i],rift120_log[i+1],64);
 snprintf(rift120_log[2],sizeof(rift120_log[2]),"%s",s);
 if(rift120_log_count<3)rift120_log_count++;
}
static int r140_potion_gained=-1;static unsigned r140_potion_tick;
static char r1310_potion_notice[72];static unsigned r1310_notice_tick;
static int overlay_kind,overlay_page,overlay_potion,overlay_status_scroll;
void game_render_set_status_scroll(int scroll){overlay_status_scroll=scroll;}
static int enemy_windup;
static const Game *card_combat_context;
void game_render_set_enemy_windup(int ticks){enemy_windup=ticks;}

static int fx_enemy_damage[2],fx_player_damage,fx_heal,fx_block,fx_target,fx_attack;
static Game victory_scene;
static int victory_replay=0;
static unsigned victory_tick=0;
static unsigned fx_tick,fx_transition_tick;
static int old_hand[10],old_hand_count=0;
static unsigned hand_exchange_tick=0;
static int hand_exchange=0;
static int fx_transition;
void game_render_set_overlay(int kind,int page,int potion){
 overlay_kind=kind;overlay_page=page;overlay_potion=potion;
}
static void r137_feedback(const Game*,const Game*,int,unsigned);
void game_render_feedback(const Game *before,const Game *after,int action,unsigned tick){
 r137_feedback(before,after,action,tick);
 if(after->potion_count>before->potion_count&&before->screen==SHOP){r140_potion_gained=after->potions[after->potion_count-1];r140_potion_tick=tick;}
 else if(before->screen!=after->screen||(before->screen==SHOP&&before->selection==5&&action==BTN_OK))r140_potion_gained=-1;
 if(action==BTN_POTION&&before->screen==COMBAT&&after->potion_count<before->potion_count){
  snprintf(r1310_potion_notice,sizeof(r1310_potion_notice),"%s",after->message);r1310_notice_tick=tick;
 }else if(before->screen!=COMBAT)r1310_potion_notice[0]=0;
 if(before->screen==COMBAT && action==BTN_OK &&
    before->selection>=0&&before->selection<before->hand_count){
   int raw=before->hand[before->selection];
   r136_play_state=*before;r136_play_state_valid=1;
   if(rift120_played_card!=raw || tick<rift120_play_tick ||
  tick-rift120_play_tick>40){
   rift120_played_card=raw;rift120_play_tick=tick;
 }
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"PLAY %s",cards[CARD_ID(raw)].name);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT&&after->ic[46]>0){
    rift120_auto_card=after->ic[46]-1;
    if(rift120_played_card<0)rift120_play_tick=tick;
    char abuf[64];snprintf(abuf,sizeof(abuf),"AUTO %s",cards[rift120_auto_card].name);
    rift120_log_add(abuf);
 }
 if(before->screen==COMBAT&&after->hp<before->hp){
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"FOE HIT: %d HP LOST",before->hp-after->hp);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT && after->weak>before->weak){
   char b[64];snprintf(b,sizeof(b),"ENEMY APPLIES WEAK +%d",after->weak-before->weak);rift120_log_add(b);
 }
 if(before->screen==COMBAT && after->vulnerable>before->vulnerable){
   char b[64];snprintf(b,sizeof(b),"ENEMY APPLIES VULNERABLE +%d",after->vulnerable-before->vulnerable);rift120_log_add(b);
 }
 if(before->screen==COMBAT && after->poison>before->poison){
   char b[64];snprintf(b,sizeof(b),"ENEMY APPLIES POISON +%d",after->poison-before->poison);rift120_log_add(b);
 }
 if(before->screen==COMBAT&&after->block>before->block){
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"SHIELD: +%d BLOCK",after->block-before->block);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT&&memcmp(before->message,after->message,sizeof(before->message))!=0)
   rift120_log_add(after->message);
 if(before->screen!=COMBAT&&after->screen==COMBAT){
   rift120_log_count=0;memset(rift120_log,0,sizeof(rift120_log));
   char encounter[64]="FOES: ";
   for(int i=0;i<after->enemy_count;i++){
      int used=(int)strlen(encounter);
      if(i>0)snprintf(encounter+used,sizeof(encounter)-used,"%s"," + ");
      used=(int)strlen(encounter);
      snprintf(encounter+used,sizeof(encounter)-used,"%s",
               game_enemy_name(after->enemies[i].art));
   }
   rift120_log_add(encounter);
 }
 if(before->screen==COMBAT&&after->screen==COMBAT&&
    before->turn!=after->turn){
   char turn[64];snprintf(turn,sizeof(turn),"TURN %d",after->turn);
   rift120_log_add(turn);
   old_hand_count=before->hand_count<10?before->hand_count:10;
   for(int j=0;j<old_hand_count;j++)old_hand[j]=before->hand[j];
   hand_exchange_tick=tick;hand_exchange=1;
 }
 fx_tick=tick;fx_player_damage=before->hp>after->hp?before->hp-after->hp:0;
 fx_heal=after->hp>before->hp?after->hp-before->hp:0;
 fx_block=after->block>before->block?after->block-before->block:0;
 fx_target=before->target;fx_attack=(action==BTN_OK && before->screen==COMBAT);
 for(int i=0;i<2;i++)
  fx_enemy_damage[i]=(i<before->enemy_count&&i<after->enemy_count &&
                       before->enemies[i].hp>after->enemies[i].hp)
                   ?before->enemies[i].hp-after->enemies[i].hp:0;
 if(before->screen!=after->screen){fx_transition=1;fx_transition_tick=tick;}
 if(before->screen==COMBAT && (after->screen==REWARD||after->screen==RELIC_RESULT||after->screen==VICTORY||after->screen==DEFEAT)){
    victory_scene=*after;
    fx_transition=0; /* The finishing clip has its own fade after contact. */
    victory_tick=tick;
    victory_replay=1;
 }
}

static unsigned effect_start;
static int effect_kind;
/* Particle / idle animation uses integer math only. No timing clock, floats,
   textures or allocations are required inside the PSP frame loop. */
static int wave(unsigned tick,int period,int amplitude){
 unsigned phase=tick%(unsigned)period;
 unsigned half=(unsigned)period/2;
 return (int)((phase<half?phase:period-phase)*(unsigned)(amplitude*2)/half)-amplitude;
}
void game_effect_start(int kind,unsigned tick){effect_kind=kind;effect_start=tick;}

static void put(int x,int y,uint32_t c);
static void rect(int x,int y,int w,int h,uint32_t c);
static void border(int x,int y,int w,int h,uint32_t c);
/* RGB565 textures are stored in the EBOOT, so artwork has no memory-stick
   dependencies. Nearest-neighbor scaling is crisp on the PSP's 480x272 LCD. */
static void art_blit(int dx,int dy,int dw,int dh,const uint16_t *data,int iw,int ih){
 if(dw<=0||dh<=0)return;
 int x0=dx<0?0:dx,y0=dy<0?0:dy,x1=dx+dw>W?W:dx+dw,y1=dy+dh>H?H:dy+dh;
 for(int y=y0;y<y1;y++){
  const uint16_t *src=data+(size_t)(dh==ih?(y-dy):((y-dy)*ih/dh))*iw;
  uint32_t *dst=pix+(size_t)y*sw;
  for(int x=x0;x<x1;x++){
   uint16_t c=src[dw==iw?(x-dx):((x-dx)*iw/dw)];
   unsigned int r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   r=(r<<3)|(r>>2);g=(g<<2)|(g>>4);b=(b<<3)|(b>>2);
   dst[x]=RGB(r,g,b);
  }
 }
}
/* PSP-compatible masked 16-bit sprite blitter (magenta colorkey). */

/* Scale 104x110 RGB565 creature frames with keyed transparency. */
static void sprite_scaled(int dx,int dy,int dw,int dh,const uint16_t *src,int iw,int ih){
 if(dw<1||dh<1)return;
 int x0=dx<0?0:dx,y0=dy<0?0:dy,x1=dx+dw>W?W:dx+dw,y1=dy+dh>H?H:dy+dh;
 for(int y=y0;y<y1;y++){
  const uint16_t *line=src+(size_t)((y-dy)*ih/dh)*iw;
  uint32_t *dst=pix+(size_t)y*sw;
  for(int x=x0;x<x1;x++){
   uint16_t c=line[(x-dx)*iw/dw];if(c==RIFT_TRANSPARENT_565)continue;
   unsigned int r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   dst[x]=RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
  }
 }
}
static void sprite_scaled_facing(int dx,int dy,int dw,int dh,
    const uint16_t *src,int iw,int ih,int flip){
 if(!flip){sprite_scaled(dx,dy,dw,dh,src,iw,ih);return;}
 if(dw<1||dh<1)return;
 int x0=dx<0?0:dx,y0=dy<0?0:dy,x1=dx+dw>W?W:dx+dw,y1=dy+dh>H?H:dy+dh;
 for(int y=y0;y<y1;y++){
  const uint16_t *line=src+(size_t)((y-dy)*ih/dh)*iw;
  uint32_t *dst=pix+(size_t)y*sw;
  for(int x=x0;x<x1;x++){
   int u=(dw-1-(x-dx))*iw/dw;
   if(u<0)u=0;if(u>=iw)u=iw-1;
   uint16_t c=line[u];if(c==RIFT_TRANSPARENT_565)continue;
   unsigned r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   dst[x]=RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
  }
 }
}
static void dark_rect(int x,int y,int w,int h,int strength){
 if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}
 if(x+w>W)w=W-x;
 if(y+h>H)h=H-y;
 if(w<=0||h<=0)return;
 for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
  uint32_t *c=pix+(size_t)yy*sw+xx;
  unsigned int r=(*c&255)*(256-strength)/256;
  unsigned int g=((*c>>8)&255)*(256-strength)/256;
  unsigned int b=((*c>>16)&255)*(256-strength)/256;
  *c=RGB(r,g,b);
 }
}
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
static void r135_shadow_text(int x,int y,const char*s,uint32_t color,const RiftFont*f){
 r134_text(x+1,y,s,RGB(0,0,0),f);r134_text(x,y+1,s,RGB(0,0,0),f);
 r134_text(x+1,y+1,s,RGB(0,0,0),f);r134_text(x,y,s,color,f);
}
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
  if(ellipsis&&i==maxrows-1&&n>maxrows){int k=(int)strlen(rows[i]);while(k&&r134_width(rows[i],f)+r134_width("...",f)>width)rows[i][--k]=0;memcpy(rows[i]+k,"...",4);}
  r134_text(x,y+i*leading,rows[i],ink,f);
 }
 return n;
}
static uint32_t r134_565(uint16_t c){unsigned r=c>>11,g=(c>>5)&63,b=c&31;return RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));}
static void r134_card_art(int x,int y,int w,int h,int id){
 /* Setup Strike has its own high-resolution illustration. */
 if(id==80){
  const int iw=240,ih=135;int crop=CLAMP(iw*h/w,35,ih),top=(ih-crop)/5;
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
   int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;
   int sx=xx*iw/w,sy=top+yy*crop/h;pix[dy*sw+dx]=r1310_setup[sy*iw+sx]|0xff000000u;
  }return;
 }
 /* Bilinear reconstruction and an upper crop that preserves faces; never paint over the frame. */
 const uint16_t*src=rift120_cards[id];int crop_h=CLAMP(60*h/w,18,32),crop_y=2;
 for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
  int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;
  int u=w>1?xx*59*256/(w-1):0,v=crop_y*256+(h>1?yy*(crop_h-1)*256/(h-1):0);
  int ix=u>>8,iy=v>>8,ax=u&255,ay=v&255,nx=MIN(ix+1,59),ny=MIN(iy+1,39);
  uint32_t top=r134_mix(r134_565(src[iy*60+ix]),r134_565(src[iy*60+nx]),ax);
  uint32_t bot=r134_mix(r134_565(src[ny*60+ix]),r134_565(src[ny*60+nx]),ax);
  pix[dy*sw+dx]=r134_mix(top,bot,ay);
 }
}

static void gilded_panel(int x,int y,int w,int h,int selected){r134_panel(x,y,w,h,selected);}
static void put(int x,int y,uint32_t c){if((unsigned)x<W&&(unsigned)y<H)pix[(size_t)y*sw+x]=c;}
static void rect(int x,int y,int w,int h,uint32_t c){if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>W)w=W-x;if(y+h>H)h=H-y;if(w<1||h<1)return;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)pix[(size_t)j*sw+i]=c;}
static void line(int x0,int y0,int x1,int y1,uint32_t c){int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;for(;;){put(x0,y0,c);if(x0==x1&&y0==y1)break;int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
static void circle(int xc,int yc,int r,uint32_t c){for(int y=-r;y<=r;y++)for(int x=-r;x<=r;x++)if(x*x+y*y<=r*r)put(xc+x,yc+y,c);}
static void triangle(int x1,int y1,int x2,int y2,int x3,int y3,uint32_t c){int minx=x1<x2?(x1<x3?x1:x3):(x2<x3?x2:x3);int maxx=x1>x2?(x1>x3?x1:x3):(x2>x3?x2:x3);int miny=y1<y2?(y1<y3?y1:y3):(y2<y3?y2:y3);int maxy=y1>y2?(y1>y3?y1:y3):(y2>y3?y2:y3);minx=minx<0?0:minx;miny=miny<0?0:miny;maxx=maxx>=W?W-1:maxx;maxy=maxy>=H?H-1:maxy;int det=(y2-y3)*(x1-x3)+(x3-x2)*(y1-y3);if(det==0)return;for(int y=miny;y<=maxy;y++)for(int x=minx;x<=maxx;x++){int a=(y2-y3)*(x-x3)+(x3-x2)*(y-y3),b=(y3-y1)*(x-x3)+(x1-x3)*(y-y3),d=det-a-b;if(det>0?(a>=0&&b>=0&&d>=0):(a<=0&&b<=0&&d<=0))put(x,y,c);}}
static void border(int x,int y,int w,int h,uint32_t c){rect(x,y,w,1,c);rect(x,y+h-1,w,1,c);rect(x,y,1,h,c);rect(x+w-1,y,1,h,c);}

static void text(int x,int y,const char*s,uint32_t ink,int size){r134_text(x,y,s,ink,size==2?&r134_f18b:&r134_f10);}
static void center(int x,int y,const char*s,uint32_t ink,int size){r134_center(x,y,s,ink,size==2?&r134_f18b:&r134_f10);}
static void fittext(int x,int y,const char*s,int n,uint32_t c){char b[128];snprintf(b,sizeof(b),"%.*s",n,s);text(x,y,b,c,1);}
/* Identical symmetrical retro-Gothic furniture across title, shop, rest,
 * relic and combat menus. Ornament stays outside the text-safe center. */
static void button(int x,int y,int w,int h,const char*label,int selected){
 sprite_scaled(x,y,w,h,rift133_button,RIFT133_BUTTON_W,RIFT133_BUTTON_H);
 if(!selected)dark_rect(x+2,y+2,w-4,h-4,58);
 center(x+w/2,y+(h-11)/2,label,selected?RGB(255,231,179):WHITE,1);
}


static void rift117_small_text(int x,int y,const char*s,uint32_t ink,int cw,int ch){(void)ch;r134_text(x,y,s,ink,cw>=6?&r134_f9:&r134_f8);}
/* Returns the number of characters consumed; last visible row is explicitly
 * ellipsized if there are more characters than the compact card can show. */
static int rift117_wrapped_text(int x,int y,const char *source,
 int columns,int maxlines,uint32_t ink,int charw,int charh){
 int start=0,len=(int)strlen(source),row=0;
 if(columns<2||maxlines<1)return 0;
 while(start<len&&row<maxlines){
   while(start<len&&source[start]==' ')start++;
   if(start>=len)break;
   int stop=start+columns;
   if(stop>len)stop=len;
   if(stop<len && source[stop]!=' '){
     int end=stop;
     while(end>start && source[end]!=' ')end--;
     if(end>start)stop=end;
   }
   char piece[128];int n=stop-start;
   if(n>columns)n=columns;
   if(n>(int)sizeof(piece)-1)n=(int)sizeof(piece)-1;
   memcpy(piece,source+start,(size_t)n);piece[n]=0;
   if(row==maxlines-1 && stop<len){
      /* Avoid silently cutting a description off mid-effect. */
      if(n>2){piece[n-1]='.';piece[n-2]='.';}
   }
   rift117_small_text(x,y+row*(charh+1),piece,ink,charw,charh);
   row++;start=stop;
 }
 return start;
}

static void wrapped_text(int x,int y,const char *source,int linechars,int maxlines,uint32_t c){
 int start=0,len=(int)strlen(source),row=0;
 while(start<len&&row<maxlines){
  while(start<len&&source[start]==' ')start++;
  if(start>=len)break;
  int stop=start+linechars;
  if(stop>=len)stop=len;
  else if(source[stop]!=' '){
   int end=stop;
   while(end>start&&source[end]!=' ')end--;
   if(end>start)stop=end;
  }
  char part[70];int n=stop-start;
  if(n>69)n=69;
  memcpy(part,source+start,(size_t)n);part[n]=0;
  text(x,y+row*11,part,c,1);
  start=stop;row++;
 }
}

/* self-contained integer clamp */
/* Unified compact health capsules.  Block lives outside the capsule,
   exactly where it belongs for both player and enemies. */
/* One HP capsule shared by player and enemies, not two stacked textures. */
static void r1310_block_number(int cx,int y,const char*value){
 int x=cx-r134_width(value,&r134_f9b)/2;
 /* Opaque dark stroke separates every digit from the cyan shield. */
 for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(dx||dy)r134_text(x+dx,y+dy,value,RGB(3,8,14),&r134_f9b);
 r134_text(x,y,value,RGB(255,255,244),&r134_f9b);
}
static void rift131_shield(int x,int y){r134_blit(x-4,y-4,28,30,r139_shield,28,30,255);}
static void rift120_hp_meter(int x,int y,int w,int hp,int maxhp,int shield){
 char value[24];int inset=w*22/100,inner=w-2*inset;
 rect(x+inset,y+5,inner,14,RGB(35,17,22));
 if(maxhp>0)rect(x+inset,y+14,(int)((long long)inner*CLAMP(hp,0,maxhp)/maxhp),4,RGB(195,36,46));
 r134_blit(x,y,w,24,r134_health_frame,112,24,255);
 snprintf(value,sizeof(value),"%d/%d",hp,maxhp);r134_center(x+w/2,y+5,value,WHITE,w<100?&r134_f8:&r134_f9b);
 if(shield>0){rift131_shield(x+w+3,y+2);snprintf(value,sizeof(value),"%d",shield);r1310_block_number(x+w+13,y+7,value);}
}
static void r138_player_meter(const Game*g){
 const int x=32,y=36;char value[24];
 rect(x+36,y+13,89,9,RGB(40,17,23));
 if(g->maxhp>0)rect(x+36,y+13,(int)(89LL*CLAMP(g->hp,0,g->maxhp)/g->maxhp),9,RGB(160,30,43));
 r134_blit(x,y,140,30,r138_player_hp,140,30,255);
 snprintf(value,sizeof(value),"%d/%d",g->hp,g->maxhp);r135_shadow_text(x+82-r134_width(value,&r134_f9b)/2,y+13,value,WHITE,&r134_f9b);
 if(g->block>0){rift131_shield(x+143,y+5);snprintf(value,sizeof(value),"%d",g->block);r1310_block_number(x+153,y+10,value);}
}
static const uint16_t *rift130_fight(const Game *g){
 if(g->battle_type==NODE_BOSS)
   return g->act==1?rift130_boss1:g->act==2?rift130_boss2:rift130_boss3;
 if(g->battle_type==NODE_ELITE)
   return (g->floor&1)?rift130_elite1:rift130_elite2;
 return (g->floor&1)?rift130_normal1:rift130_normal2;
}
static void background(const Game*g){
 const uint16_t *src=g->act==1?rift130_map1:
                      g->act==2?rift130_map2:rift130_map3;
 if(g->screen==TITLE)src=r138_menu;
 else if(g->screen==HELP||g->screen==VICTORY||g->screen==DEFEAT)src=ra_title_bg;
 else if(g->screen==COMBAT)src=rift130_fight(g);
 else if(g->screen==REST||
   (g->screen==DECK_VIEW&&g->prior_screen==REST))src=rift130_camp;
 else if(g->screen==REWARD||g->screen==BOSS_REWARD||g->screen==RELIC_RESULT)
   src=rift130_fight(g);
 else if((g->screen==DECK_VIEW||g->screen==CARD_CHOICE||g->screen==PAUSE_MENU)
        &&(g->prior_screen==COMBAT||g->screen==CARD_CHOICE))
   src=rift130_fight(g);
 art_blit(0,0,W,H,src,W,H);
}
static unsigned particle_hash(unsigned x){
 x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);
}
static void ambient_particles(const Game*g){
 if(g->screen!=COMBAT&&g->screen!=TITLE&&g->screen!=MAP&&g->screen!=REWARD)return;
 /* Looping ember motion and an irregular intermittent shimmer. */
 for(unsigned i=0;i<18;i++){
  unsigned seed=particle_hash(i*76543u+g->act*121u);
  unsigned travel=(animation_tick*(1u+(seed&3u))+(seed>>12))%181u;
  int x=(int)((seed%W)+((animation_tick/8u)%(15u)))-8;
  int y=177-(int)travel;
  unsigned flicker=particle_hash(seed+animation_tick/3u);
  uint32_t color=(flicker&7u)==0?RGB(251,182,106):RGB(176,63,50);
  if(y>45&&y<172&&x>=0&&x<W){
   rect(x,y,((seed>>8)&3u)==0?2:1,((seed>>5)&3u)==0?2:1,color);
  }
 }
}
static void header(const Game*g,const char*caption){
 r134_blit(0,0,480,17,r134_header,480,17,255);
 char b[96];r134_text(8,3,"RIFT ASCENT",GOLD,&r134_f9b);
 snprintf(b,sizeof(b),"HP %d/%d",g->hp,g->maxhp);r134_text(120,3,b,WHITE,&r134_f9);
 snprintf(b,sizeof(b),"%d GOLD",g->gold);r134_text(237,3,b,GOLD,&r134_f9);
 snprintf(b,sizeof(b),"ACT %d   FLOOR %02d",g->act,g->floor+1);r134_text(353,3,b,WHITE,&r134_f9);
 dark_rect(0,17,480,13,180);r134_text(12,19,caption,MUTED,&r134_f9);
 if(g->screen==HELP||g->screen==DECK_VIEW||g->screen==CARD_CHOICE||g->screen==RELIC_VIEW||g->screen==POTION_VIEW||g->screen==REWARD||g->screen==EVENT_RESULT||g->screen==VICTORY||g->screen==DEFEAT)gilded_panel(10,39,460,213,1);
}


static int r135_button_token(const char*p,int*length){
 static const char*names[]={"SQUARE","CIRCLE","START","SQ","X","O","TRIANGLE","TRI"};
 static const int ids[]={2,1,3,2,0,1,4,4};
 for(int i=0;i<8;i++){int n=(int)strlen(names[i]);if((int)strlen(p)>=n&&!memcmp(p,names[i],(size_t)n)&&(!p[n]||p[n]==' '||p[n]=='/')){*length=n;return ids[i];}}
 *length=1;return -1;
}
static void footer(const char*help){
 r134_blit(0,258,480,14,r134_footer,480,14,255);
 const uint32_t*icons[]={r135_cross,r135_circle,r135_square,r135_start,r1311_triangle};
 const RiftFont*f=&r134_f9;int width=0;
 for(int pass=0;pass<2;pass++){
  width=0;
  for(const char*p=help;*p;){int n=1,id=-1;if(p==help||p[-1]==' '||p[-1]=='/')id=r135_button_token(p,&n);
   if(id>=0)width+=(id==3?22:14)+3;else {unsigned c=(unsigned char)*p;width+=f->advance[(c>=32&&c<=126?c:'?')-32];}p+=n;
  }
  if(width<=462)break;f=&r134_f8;
 }
 int x=(480-width)/2;if(x<5)x=5;
 for(const char*p=help;*p;){int n=1,id=-1;if(p==help||p[-1]==' '||p[-1]=='/')id=r135_button_token(p,&n);
  if(id>=0){int w=id==3?22:14;r134_blit(x,258,w,14,icons[id],w,14,255);x+=w+3;}
  else{char ch[2]={*p,0};r134_text(x,261,ch,WHITE,f);x+=r134_width(ch,f);}p+=n;
 }
}

static const char *rift131_clean_rules(const char *s){
 static char clean[320];
 int i=0,j=0;
 if(!s)return "";
 while(s[i]&&j<(int)sizeof(clean)-1){
   if(s[i]=='C'&&s[i+1]=='o'&&s[i+2]=='s'&&s[i+3]=='t'&&
      s[i+4]==' '&&s[i+5]>='0'&&s[i+5]<='9'&&
      s[i+6]=='.'&&(s[i+7]==' '||s[i+7]=='\0')){
     i+=7;
     if(s[i]==' ')i++;
     continue;
   }
   clean[j++]=s[i++];
 }
 clean[j]=0;return clean;
}
/* Cache derived descriptions until combat state changes. Rendering must not
   repeatedly run the probe at the PSP frame rate. */
static const char*r136_rules(int raw){
 static Game state;static int valid,count,keys[32],targets[32];static char values[32][320];
 const Game*g=card_combat_context;
 if(!g||g->screen!=COMBAT)return card_description(raw);
 if(!valid||memcmp(&state,g,sizeof(state))){state=*g;valid=1;count=0;}
 for(int i=0;i<count;i++)if(keys[i]==raw&&targets[i]==g->target)return values[i];
 if(count==32)count=0;
 int slot=count++;keys[slot]=raw;targets[slot]=g->target;
 game_card_description_now(g,raw,g->target,values[slot],sizeof(values[slot]));
 return values[slot];
}
typedef struct {int title_rows,art_y,art_h,body_y,leading,rows,height;const RiftFont*font;} R135CardLayout;
static R135CardLayout r135_card_layout(int raw,int w){
 R135CardLayout l;char title[64],lines[24][192];int id=CARD_ID(raw),inner=w-34;
 snprintf(title,sizeof(title),"%s%s",cards[id].name,CARD_UP(raw)?"+":"");
 l.title_rows=r134_lines(title,inner,&r138_title9,lines);
 l.art_y=20+MIN(l.title_rows,2)*11+2;l.art_h=29;l.body_y=l.art_y+l.art_h+3;
 l.font=&r134_f9;l.leading=11;l.rows=r134_lines(rift131_clean_rules(r136_rules(raw)),inner,l.font,lines);
 l.height=l.body_y+l.rows*l.leading+18;
 if(l.height>158){l.font=&r134_f8;l.leading=9;l.rows=r134_lines(rift131_clean_rules(r136_rules(raw)),inner,l.font,lines);l.height=l.body_y+l.rows*l.leading+18;}
 if(l.height<100)l.height=100;
 return l;
}
static int r135_status_values(const Game*g,int enemy,int values[7]){
 if(enemy<0){values[0]=g->strength;values[1]=g->dexterity;values[2]=g->weak;values[3]=g->vulnerable;values[4]=g->poison;values[5]=g->energy_penalty;values[6]=g->ic[IC_PLATING];}
 else{const Enemy*e=&g->enemies[enemy];values[0]=e->strength;values[1]=0;values[2]=e->weak;values[3]=e->vulnerable;values[4]=e->poison;values[5]=values[6]=0;}
 int count=0;for(int i=0;i<7;i++)if(values[i])count++;return count;
}
static void r135_status_icons(const Game*g,int enemy,int x){
 int values[7],count=r135_status_values(g,enemy,values);if(!count)return;
 const uint32_t*icons[]={r135_strength,r135_dexterity,r135_weak,r135_vulnerable,r135_poison,r135_energy_drain,r1311_plating};
 int size=count>5?16:20,step=count>5?14:count>4?18:23,y=count>4?174-count*step:80,slot=0;
 for(int i=0;i<7;i++)if(values[i]){
  int yy=y+slot*step;char b[16];r134_blit(x,yy,size,size,icons[i],22,22,255);
  snprintf(b,sizeof(b),"%d",values[i]);int tx=x+22-r134_width(b,&r134_f9b);
  r135_shadow_text(tx,yy+(count>5?5:10),b,values[i]<0?RED:WHITE,&r134_f9b);slot++;
 }
}
static void r134_card_frame(int x,int y,int w,int h,int type){
 /* Preserve the generated sculpture but reserve a rectangular text interior.
    Nine-slicing keeps the corner ornaments proportional on hand thumbnails. */
 int small=w<105&&h<105;int side=w<100?7:12,top=small?11:20,base=small?9:16;
 const uint32_t*src=r134_card_frames[type];
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
static void card_render(int x,int y,int w,int h,int raw,int selected,int active){
 int id=CARD_ID(raw);if(id<0||id>=CARD_COUNT)return;
 int small=w<105&&h<105,type=CLAMP(cards[id].type,0,3),inset=small?10:17,inner=w-2*inset;
 const uint32_t face[4]={RGB(25,19,22),RGB(16,25,33),RGB(26,19,33),RGB(20,27,22)};
 int grid=small&&w>=98;
 if(grid&&selected)y-=5;
 rect(x+4,y+5,w-8,h-10,grid&&selected?RGB(74,20,30):face[type]);
 char title[64];snprintf(title,sizeof(title),"%s%s",cards[id].name,CARD_UP(raw)?"+":"");
 int title_inset=small?8:inset,title_inner=w-2*title_inset;
 const RiftFont *titlefont=small?&r138_title8:&r138_title9;
 char fitlines[24][192];if(small&&r134_lines(title,title_inner,titlefont,fitlines)>2)titlefont=&r138_title7;
 R135CardLayout layout={0};if(!small)layout=r135_card_layout(raw,w);
 int ty=y+(small?14:20),titlelead=small?9:11;
 char titlelines[24][192];int titles=r134_lines(title,title_inner,titlefont,titlelines);
 int arty=small?ty+MIN(2,titles)*9+3:y+layout.art_y,arth=small?16:layout.art_h;
 int art_inset=small?6:11,art_width=w-2*art_inset;
 /* The requested fine picture outline sits beneath the existing sculpture. */
 rect(x+art_inset-1,arty-1,art_width+2,arth+2,RGB(132,104,67));
 r134_card_art(x+art_inset,arty,art_width,arth,id);
 if(!active)dark_rect(x+art_inset,arty,art_width,arth,96);
 /* Frame is composited after the image. Its alpha is never overwritten by art. */
 r134_card_frame(x,y,w,h,type);
 if(!active&&card_combat_context){
  for(int yy=CLAMP(y+4,0,H);yy<MIN(H,y+h-4);yy++)for(int xx=CLAMP(x+4,0,W);xx<MIN(W,x+w-4);xx++){
   uint32_t c=pix[yy*sw+xx];int grey=((c&255)*30+((c>>8)&255)*59+((c>>16)&255)*11)/100;
   pix[yy*sw+xx]=r134_mix(c,RGB(grey/2,grey/2,grey/2),160);
  }
 }
 r134_wrap(x+title_inset,ty,title_inner,2,titlelead,title,selected?RGB(255,230,180):WHITE,titlefont,0);
 char cost[12];int energy=card_combat_context?game_card_cost_now(card_combat_context,raw):game_card_cost(raw);
 if(energy==-1)snprintf(cost,sizeof(cost),"X");else if(energy==-2)snprintf(cost,sizeof(cost),"-");else snprintf(cost,sizeof(cost),"%d",energy);
 int orb=small?12:18;r134_blit(x+inset,arty,orb,orb,!active&&card_combat_context?r140_energy_empty:r134_energy,46,46,255);
 r134_center(x+inset+orb/2,arty+(small?1:4),cost,!active&&card_combat_context?RGB(255,135,110):WHITE,&r134_f9b);
 int by=arty+arth+3,bottom=y+h-(small?10:18),lead=small?9:layout.leading;
 const RiftFont*body=small?&r134_f8:layout.font;const char*rules=rift131_clean_rules(r136_rules(raw));
 char check[24][192];int rows=(bottom-by)/lead;
 if(!small&&r134_lines(rules,inner,body,check)>rows){body=&r134_f8;lead=9;rows=(bottom-by)/lead;}
 if(rows>0)r134_wrap(x+inset,by,inner,rows,lead,rules,WHITE,body,small);
 if(selected&&!active&&card_combat_context&&!small&&energy>card_combat_context->energy){
  char missing[40];snprintf(missing,sizeof(missing),"NEED %d MORE ENERGY",energy-card_combat_context->energy);
  r134_blit(x,y-16,w,17,r140_selection,208,28,255);
  r134_center(x+w/2,y-12,missing,RGB(255,160,130),&r134_f8);
 }
 if(grid){if(selected)r134_blit(x+w/2-20,y-9,40,23,r138_choice,42,28,255);else dark_rect(x+3,y+4,w-6,h-8,70);}

}



/* Compact, shadowed, ICON-ONLY enemy telegraphs.
 * The enemy's full intention remains accessible through Start / Info.
 * Attack damage uses game's actual calculated post-modifier value.
 */
static void rift116_draw_enemy_intent(const Game*g,int index,int cx){
 const Enemy *e=&g->enemies[index];
 const uint32_t *glyph[3];
 char value[3][32];
 int count=0;
 int hits=game_enemy_intent_hits(g,index);
 int damage=game_enemy_intent_damage(g,index);
 if(hits>0){
  glyph[count]=r134_map_icons[0];
  if(hits>1)snprintf(value[count],sizeof(value[count]),"%dx%d",hits,damage/hits);
  else snprintf(value[count],sizeof(value[count]),"%d",damage);
  count++;
 }
 int block=-1,buff=-1,debuff=-1;
 switch(e->intent){
  case 1:block=e->intent_value;break;
  case 2:case 3:debuff=2;break; /* Weak or Vulnerable */
  case 4:buff=2;block=8;break;
  case 5:case 8:case 10:debuff=1;break; /* Burn, energy drain, Daze */
  case 6:debuff=2;break; /* poison */
  case 9:buff=-2;block=e->intent_value/2;break; /* Heal and shield */
  case 11:debuff=1;block=e->intent_value/2;break; /* Dread plus shield */
  case 12:buff=-2;break; /* leech heals enemy */
  case 13:buff=1;block=e->intent_value;break;
 }
 if(block>=0&&count<3){
  glyph[count]=r134_map_icons[7];
  snprintf(value[count],sizeof(value[count]),"%d",block);
  count++;
 }
 if(buff!=-1&&count<3){
  glyph[count]=r134_utility_icons[4];
  if(buff==-2)value[count][0]=0; /* Heal: numeric amount varies */
  else snprintf(value[count],sizeof(value[count]),"%d",buff);
  count++;
 }
 if(debuff>=0&&count<3){
  glyph[count]=r134_utility_icons[5];
  if(e->intent==2||e->intent==3||e->intent==6)
   snprintf(value[count],sizeof(value[count]),"%d",debuff);
  else value[count][0]=0; /* curse card or energy penalty */
  count++;
 }
 if(!count)return;
 int widths[3],total=0;
 for(int j=0;j<count;j++){
   widths[j]=17+r134_width(value[j],&r134_f9b)+4;
   total+=widths[j];
 }
 int x=cx-total/2;
 for(int j=0;j<count;j++){
   int dim=glyph[j]==r134_map_icons[0]||glyph[j]==r134_map_icons[7]?36:40;
   r134_blit(x,61,17,17,glyph[j],dim,dim,255);
   if(value[j][0])r135_shadow_text(x+18,64,value[j],WHITE,&r134_f9b);
   x+=widths[j];
 }
}


static int rift133_hand_start(const Game*g){
 int start=0;if(g->selection>=5&&g->selection<g->hand_count)start=g->selection-4;
 else if(g->selection>=g->hand_count&&g->hand_count>5)start=g->hand_count-5;
 return start;
}


static int r134_playing(void){return (rift120_played_card>=0||rift120_auto_card>=0)&&animation_tick>=rift120_play_tick&&animation_tick-rift120_play_tick<38;}
static int r134_enemy_x(const Game*g,int i){return g->enemy_count==1?356:(i?409:283);}
#include "combat_fx.inc"
static void combat(const Game*g){
 card_combat_context=g;unsigned age=animation_tick>=fx_tick?animation_tick-fx_tick:999;
 r137_scene(g);
 if(fx_heal>0&&age<28){char number[24];snprintf(number,sizeof(number),"+%d",fx_heal);r135_shadow_text(98,108-(int)age/2,number,GREEN,&r134_f18b);}
 header(g,"");
 /* Both living enemies get a complete, fixed row; no scrolling or turn log. */
 rect(0,17,480,19,RGB(11,12,16));int row=0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){
  char action[112],line[160];game_enemy_intent_text(g,i,action,sizeof(action));
  snprintf(line,sizeof(line),"%s: %s",g->enemies[i].name,action);
  r134_text(8,18+row*9,line,i==g->target?WHITE:MUTED,&r134_f8);row++;
 }
 r138_player_meter(g);
 r135_status_icons(g,-1,5);
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){
  const Enemy*e=&g->enemies[i];int ex=r134_enemy_x(g,i),w=g->enemy_count==1?112:88;
  rift120_hp_meter(ex-w/2,36,w,e->hp,e->maxhp,e->block);rift116_draw_enemy_intent(g,i,ex);
  r135_status_icons(g,i,MIN(456,ex+(g->enemy_count==1?60:49)));
 }
 /* Opaque native-resolution surface covers every pixel below the battlefield. */
 r134_blit(0,174,480,98,r134_hand_hud,480,98,255);
 r134_blit(4,199,46,46,g->energy==0?r140_energy_empty:r134_energy,46,46,255);
 char b[16];snprintf(b,sizeof(b),"%d",g->energy);r134_center(27,212,b,WHITE,&r134_f18b);
 int start=rift133_hand_start(g),playing=r134_playing();
 for(int i=start;i<g->hand_count&&i<start+5;i++){
  int selected=i==g->selection,cy=174,cost=game_card_cost_now(g,g->hand[i]);
  if(hand_exchange&&animation_tick>=hand_exchange_tick){unsigned a=animation_tick-hand_exchange_tick;if(a<10)cy=280;else if(a<23)cy+=(23-(int)a)*6;else hand_exchange=0;}
  if((!selected||playing)&&!(playing&&selected&&animation_tick-rift120_play_tick<18))card_render(56+(i-start)*71,cy,70,84,g->hand[i],0,cost!=-2&&(cost==-1||cost<=g->energy));
 }
 if(!playing&&g->selection>=start&&g->selection<g->hand_count&&g->selection<start+5){
  int cost=game_card_cost_now(g,g->hand[g->selection]);
  int preview_h=r135_card_layout(g->hand[g->selection],126).height;
  card_render(CLAMP(56+(g->selection-start)*71-28,35,289),254-preview_h,126,preview_h,g->hand[g->selection],1,cost!=-2&&(cost==-1||cost<=g->energy));
 }
 if(hand_exchange&&animation_tick>=hand_exchange_tick){unsigned a=animation_tick-hand_exchange_tick;if(a<15)for(int i=0;i<old_hand_count&&i<5;i++)card_render(56+i*71,174+(int)a*7,70,84,old_hand[i],0,1);}
 if(start>0){
  for(int yy=0;yy<28;yy++)for(int xx=0;xx<36;xx++){uint32_t c=r1311_arrow_glow[yy*36+35-xx];int a=c>>24;if(a)pix[(186+yy)*sw+28+xx]=r134_mix(pix[(186+yy)*sw+28+xx],c,a);}
 }
 if(start+5<g->hand_count)r134_blit(402,186,36,28,r1311_arrow_glow,36,28,255);
 int end_selected=g->selection>=g->hand_count;
 r134_blit(410,end_selected?213:215,68,29,r1310_end,68,29,end_selected?255:145);
 r135_shadow_text(444-r134_width("END",&r134_f9b)/2,end_selected?224:226,"END",end_selected?RGB(255,245,222):MUTED,&r134_f9b);
 if(end_selected)r134_blit(398,223,13,10,r1310_arrow,36,28,255);
 if(r1310_potion_notice[0]&&animation_tick>=r1310_notice_tick&&animation_tick-r1310_notice_tick<360){
  int width=r134_width(r1310_potion_notice,&r134_f9)+14;dark_rect(25,65,width,15,225);r135_shadow_text(32,68,r1310_potion_notice,WHITE,&r134_f9);
 }
 int need_energy=g->selection<g->hand_count&&game_card_cost_now(g,g->hand[g->selection])>g->energy;
 card_combat_context=0;footer(need_energy?"NOT ENOUGH ENERGY   O END   SQ DECK   START STATUS / INVENTORY":"X PLAY   O END   SQ DECK   START STATUS / INVENTORY");
}

static uint32_t node_color(int n){switch(n){case NODE_FIGHT:return RED;case NODE_ELITE:return PURPLE;case NODE_EVENT:return BLUE;case NODE_REST:return GREEN;case NODE_SHOP:return GOLD;case NODE_TREASURE:return RGB(209,160,72);default:return WHITE;}}

static int r138_map_x(const Game*g,int row,int lane){return 73+lane*94+(int)(particle_hash(g->seed+(unsigned)(row*31+lane*997))%15u)-7;}
static void room_icon(int n,int x,int y){if(n>=0&&n<=6)r134_blit(x-13,y-13,26,26,n==NODE_TREASURE?r140_chest:r134_map_icons[n],36,36,255);}
static void map_screen(const Game*g){
 header(g,"FATE'S PATH");gilded_panel(15,41,299,209,0);gilded_panel(315,41,160,209,0);
 int focus=g->floor+g->scroll,first=CLAMP(focus-2,0,MAP_ROWS-4);
 for(int r=first;r<first+4&&r<MAP_ROWS;r++){
  int y=222-(r-first)*38;
  for(int c=0;c<3;c++)if(game_route_row_mask(g,r)&(1u<<c)){
   int x=r138_map_x(g,r,c);unsigned routes=r==MAP_ROWS-1?2u:game_route_mask(g,r,c);
   for(int d=0;d<3;d++)if(routes&(1u<<d)){
    int nx=r==MAP_ROWS-1?167:r138_map_x(g,r+1,d),ny=y-38;
    int travelled=r<g->floor-1&&g->map_selected[r]==c&&g->map_selected[r+1]==d;
    int from_current=r==g->floor-1&&c==g->lane;
    uint32_t ink=travelled?GOLD:from_current?RGB(239,211,157):r<g->floor?RGB(67,59,53):RGB(131,111,88);
    line(x,y-12,nx,ny+12,ink);
   }
  }
 }
 for(int r=first;r<first+5&&r<=MAP_ROWS;r++){
  int y=222-(r-first)*38;char num[12];snprintf(num,sizeof(num),"%02d",r+1);r134_text(36,y-4,num,MUTED,&r134_f8);
  if(r==MAP_ROWS){room_icon(NODE_BOSS,167,y);if(g->floor==MAP_ROWS)r134_blit(151,y+8,32,18,r138_choice,42,28,255);continue;}
  for(int c=0;c<3;c++)if(game_route_row_mask(g,r)&(1u<<c)){
   int x=r138_map_x(g,r,c),n=g->map[r][c],opacity=255;
   if((r<g->floor&&g->map_selected[r]!=c)||(r==g->floor&&!game_route_available(g,c)))opacity=70;
   r134_blit(x-13,y-13,26,26,n==NODE_TREASURE?r140_chest:r134_map_icons[n],36,36,opacity);
   if(r==g->floor-1&&c==g->lane)r134_blit(x-17,y-18,34,30,r138_pawn,34,30,255);
   else if(r<g->floor&&g->map_selected[r]==c)r134_text(x+11,y+7,"+",GOLD,&r134_f8);
   if(r==focus&&c==g->selection)r134_blit(x-14,y+7,28,13,r138_choice,42,28,255);
  }
 }
 r134_blit(330,60,34,30,r138_pawn,34,30,255);r134_text(370,62,"CURRENT",GOLD,&r134_f9b);
 char b[70];snprintf(b,sizeof(b),g->floor?"FLOOR %02d":"ENTRANCE",g->floor);r134_text(370,77,b,WHITE,&r134_f8);
 snprintf(b,sizeof(b),g->endless?"ACT %d / ENDLESS":"ACT %d / 3",g->act);r134_text(335,103,b,GOLD,&r134_f10);
 r134_text(335,132,"NEXT",MUTED,&r134_f9);
 int n=focus>=MAP_ROWS?NODE_BOSS:g->map[focus][g->selection];r134_text(335,150,node_name(n),node_color(n),&r134_f10);
 const char*reward=n==NODE_FIGHT?"CARD REWARD":n==NODE_ELITE?"CARD + RELIC":n==NODE_SHOP?"BUY / REMOVE":n==NODE_REST?"HEAL / UPGRADE":n==NODE_EVENT?"MYSTERY":n==NODE_TREASURE?"RELIC":"BOSS";
 r134_text(335,174,reward,WHITE,&r134_f8);
 if(!g->scroll)r134_text(335,204,"X TAKE PATH",GOLD,&r134_f9);
 r134_text(335,224,"SQUARE DECK",MUTED,&r134_f8);
 footer("UP/DOWN SCROLL  LEFT/RIGHT PATH  X ENTER  SQ DECK  TRI POTIONS");
}
static void relic_result_screen(const Game*g){
 header(g,"RELIC");
 gilded_panel(39,47,401,193,1);
 int id=g->reward[0];
 center(240,58,g->relic_pending?"A RELIC AWAITS":"RELIC ACQUIRED",GOLD,2);
 r134_blit(211,86,58,58,r134_utility_icons[1],40,40,255);
 if(id>=0&&id<RELIC_COUNT){
    center(240,150,relic_defs[id].name,GOLD,1);
    wrapped_text(62,171,relic_defs[id].desc,51,3,WHITE);
 }
 if(g->relic_pending){button(73,210,145,27,"TAKE",g->selection==0);button(263,210,145,27,"SKIP",g->selection==1);}
 footer(g->relic_pending?"LEFT/RIGHT SELECT   X CONFIRM   O SKIP":"X CONTINUE");
}
static void event_result_screen(const Game*g){
 header(g,g->effect_turn==2?"THE ANCIENT LESSON / CARD RECEIVED":"THE ARMORY'S SPOILS");
 if(g->effect_turn==2){
  int height=r135_card_layout(g->reward[0],162).height;
  card_render(159,58+(158-height)/2,162,height,g->reward[0],1,1);
  r134_center(240,229,"Added to your deck",GOLD,&r134_f9);
  footer("X CONTINUE  O CONTINUE");return;
 }
 for(int i=0;i<2;i++){
   int x=103+i*148;
   card_render(x,53,128,166,g->reward[i],0,1);

 }
 center(240,230,"-10 HP FROM THE ARMORY",RED,1);
 footer("X CONTINUE  O CONTINUE");
}
static void r140_potion_card(const Game*g,int id,int x,int y,int w){
 r134_blit(x,y,w,43,r140_selection,208,28,255);
 r134_blit(x+15,y+8,27,27,r134_utility_icons[0],40,40,255);
 r134_text(x+49,y+7,game_potion_name(id),GOLD,&r134_f9b);
 r134_wrap(x+49,y+20,w-70,2,10,game_potion_description_now(g,id),WHITE,&r134_f8,0);
}
static void reward_screen(const Game*g){
 header(g,g->battle_type==NODE_BOSS?"BOSS SPOILS / CHOOSE ONE RARE CARD":"SPOILS OF BATTLE / CHOOSE ONE CARD");
 for(int i=0;i<3;i++){
  int chosen=g->selection==i,x=33+i*144,y=chosen?49:56;if(g->ic[37])y-=9;
  card_render(x,y,126,166,g->reward[i],chosen,1);
  int rarity=cards[CARD_ID(g->reward[i])].rarity;
  r134_center(x+63,y+148,rarity==2?"RARE":rarity==1?"UNCOMMON":"COMMON",rarity==2?GOLD:rarity==1?BLUE:WHITE,&r138_title7);
  if(chosen)r134_blit(x+42,32,42,28,r138_choice,42,28,255);
  else dark_rect(x+6,y+8,114,150,38);
 }
 if(g->ic[37]>0&&g->ic[37]<=6){
  r140_potion_card(g,g->ic[37]-1,20,211,366);
  button(394,228,68,23,"SKIP",g->selection==3);
 }else button(198,234,84,20,"SKIP",g->selection==3);
 footer("LEFT/RIGHT CHOOSE  X TAKE  TRI VIEW  SQ DECK");
}
static void boss_reward_screen(const Game*g){
 header(g,"GUARDIAN DEFEATED / SELECT A RELIC");
 for(int i=0;i<3;i++){
  int x=18+i*150,y=47,selected=g->selection==i;
  gilded_panel(x,y,144,178,selected);
  r134_blit(x+47,y+12,50,50,r134_utility_icons[1],40,40,selected?255:180);
  if(g->reward[i]>=0){
   r134_wrap(x+13,y+69,118,2,12,relic_defs[g->reward[i]].name,GOLD,&r134_f10,0);
   r134_wrap(x+13,y+98,118,5,12,relic_defs[g->reward[i]].desc,WHITE,&r134_f9,0);
  }
 }
 button(195,229,90,25,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT  X CONFIRM  O SKIP");
}
static void rest_screen(const Game*g){
 header(g,"REST SITE / A MOMENT OF PEACE");
 for(int i=0;i<2;i++){
  int x=30+i*230;gilded_panel(x,47,190,199,g->selection==i);
  r134_blit(x+55,69,80,80,i?r134_event_icons[10]:r134_campfire,80,80,255);
  r134_center(x+95,158,i?"TEMPER YOUR STEEL":"THE EMBERS REMAIN",GOLD,&r134_f9b);
  int chosen=g->selection==i;
  if(!chosen)dark_rect(x+6,53,178,187,125);
  r134_blit(x+7,191,176,43,r140_selection,208,28,chosen?255:100);
  r134_center(x+95,208,i?"UPGRADE A CARD":"HEAL 30% HP",chosen?WHITE:MUTED,&r134_f10);
  if(chosen)r134_blit(x+74,42,42,28,r138_choice,42,28,255);
 }
 footer("LEFT/RIGHT SELECT    X CONFIRM");
}
static const char*event_titles[]={"CURSED PURSE","MOON SPRING","FORGOTTEN TOME","ANCIENT SHRINE","TRAPPED CHEST","LOST TRAVELER","FORGOTTEN ARMORY","LIVING ALTAR","BLACK WATER","CINDER GAMBIT","RUSTED FORGE","LAST SENTINEL"};
static const char *rift_event_lore[12]={
 "A scorched purse lies beneath an abandoned watchtower.",
 "Pale moonwater ripples where no sky is visible.",
 "A grimoire rests on a lectern, its pages still warm.",
 "A forgotten monument vibrates with dormant power.",
 "An ornate chest waits behind a nest of tripwires.",
 "A weary pilgrim offers the last of their provisions.",
 "Steel hangs from racks in a broken underground forge.",
 "A stone altar asks blood in exchange for endurance.",
 "A black spring promises relief and a permanent scar.",
 "Coins shimmer in a fissure where the air feels thin.",
 "Two empty rune sockets demand precious gold.",
 "An unmoving sentinel watches an abandoned shrine."
};
static const char *rift113_event_details[12]={
 "Gain 75 gold now. Lose 8 HP immediately; may be fatal.",
 "Pay 2 Gold per HP restored, up to 20 HP. UP/DOWN adjusts the amount.",
 "Receive one random upgraded card.",
 "Gain 5 maximum HP and heal 5 HP immediately.",
 "Receive one random relic, then lose 7 HP. Relic revealed afterward.",
 "Restore 8 HP and gain 30 gold. No lasting penalty.",
 "Receive two random cards. Lose 10 HP.",
 "Gain 10 maximum HP. Lose 17 current HP; this can be fatal.",
 "Restore 28 HP. Add Injury: an unplayable card diluting future draws.",
 "Gain 110 gold, lose 16 HP. This can be fatal.",
 "Pay 12 gold to upgrade two eligible cards.",
 "Gain an unknown relic. Add Dread curse: unplayable; costs 1 energy next turn if held."
};
static void event_screen(const Game*g){
 header(g,"A CHOICE IN THE DARK");
 int id=CLAMP(g->effect_turn,0,11);gilded_panel(20,42,440,210,1);
 r134_center(240,57,event_titles[id],GOLD,&r134_f18b);
 r134_blit(38,91,80,80,r134_event_icons[id],80,80,255);
 r134_wrap(137,96,294,3,15,rift_event_lore[id],WHITE,&r134_f10,0);
 if(id==1){char quote[140];int amount=game_moon_heal_now(g);snprintf(quote,sizeof(quote),"Heal %d HP for %d Gold.\nUP/DOWN adjusts (2 Gold per HP, up to 20 HP).",amount,amount*2);r134_wrap(137,145,294,5,12,quote,GOLD,&r134_f9,0);}
 else r134_wrap(137,145,294,5,12,rift113_event_details[id],GOLD,&r134_f9,0);
 button(51,214,174,29,"ACCEPT",g->selection==0);
 button(255,214,174,29,"LEAVE",g->selection==1);
 footer("LEFT/RIGHT CHOOSE    X CONFIRM");
}
static void shop_screen(const Game*g){
 sprite_scaled(0,0,480,272,rift112_shop,240,136);
 dark_rect(0,28,480,224,165);

 header(g,"BLACK MARKET / INSPECT BEFORE YOU BUY");
 int discount=0;
 for(int i=0;i<g->relic_count;i++)if(g->relics[i]==6)discount=30;
 char label[90],caption[140],title[52],cost[24];
 int selected=g->selection,price=0;
 gilded_panel(10,44,230,204,0);
 gilded_panel(246,44,224,204,1);
 for(int i=0;i<7;i++){
   int y=54+i*25,chosen=(i==selected);
   if(chosen)r134_blit(27,y+1,204,22,r1311_shop,204,22,255);
   label[0]=0;
   if(i<3){
     if(g->shop_cards[i]>=0)
       snprintf(label,sizeof(label),"CARD  %s",cards[g->shop_cards[i]].name);
     else snprintf(label,sizeof(label),"CARD  SOLD OUT");
   }else if(i==3)snprintf(label,sizeof(label),"RELIC %s",
        g->shop_relic>=0?relic_defs[g->shop_relic].name:"SOLD OUT");
   else if(i==4)snprintf(label,sizeof(label),"SERVICE REMOVE CARD");
   else if(i==5)snprintf(label,sizeof(label),"POTION RANDOM  %d/%d",g->potion_count,MAX_POTIONS);
   else if(i==6)snprintf(label,sizeof(label),"LEAVE THE STORE");
   r134_wrap(36,y+8,195,1,10,label,chosen?WHITE:MUTED,&r134_f9,0);
 }
 caption[0]=0;title[0]=0;
 if(selected<3){
    int id=g->shop_cards[selected];
    if(id<0){snprintf(title,sizeof(title),"SOLD OUT");snprintf(caption,sizeof(caption),"This card has already been purchased.");}
    else{
      snprintf(title,sizeof(title),"%s",cards[id].name);
      snprintf(caption,sizeof(caption),"%s",cards[id].desc);
      price=50+25*cards[id].rarity-discount;
    }
 }else if(selected==3){
    if(g->shop_relic<0){snprintf(title,sizeof(title),"SOLD OUT");snprintf(caption,sizeof(caption),"The relic has been purchased.");}
    else{
      snprintf(title,sizeof(title),"%s",relic_defs[g->shop_relic].name);
      snprintf(caption,sizeof(caption),"%s",relic_defs[g->shop_relic].desc);
      price=155-discount;
    }
 }else if(selected==4){
    snprintf(title,sizeof(title),"CARD REMOVAL");
    snprintf(caption,sizeof(caption),"Permanently remove one card from the deck. You pay only upon removal.");
    price=60;
 }else if(selected==5){
    snprintf(title,sizeof(title),"RANDOM POTION");
    snprintf(caption,sizeof(caption),g->potion_count==MAX_POTIONS?"Belt full (3/3). Use a potion to make room.":"Gain one random potion. Belt: %d/3 slots.",g->potion_count);
    price=50;
 }else if(selected==6){
    snprintf(title,sizeof(title),"LEAVE SHOP");
    snprintf(caption,sizeof(caption),"Keep your purchases and continue along the connected route.");
 }else{
    snprintf(title,sizeof(title),"RANDOM POTION");
    snprintf(caption,sizeof(caption),"Receive one random potion if your three-slot belt has space.");
    price=50;
 }
 if(selected<3&&g->shop_cards[selected]>=0){
   card_render(295,51,126,166,g->shop_cards[selected],1,1);
 }else{
   r134_center(358,112,title,GOLD,&r134_f10);
   r134_blit(337,65,40,40,r134_utility_icons[selected==3?1:selected==5?0:2],40,40,255);
   r134_wrap(265,134,186,6,13,caption,WHITE,&r134_f10,0);
 }
 if(price){
   if(selected==5&&g->potion_count==MAX_POTIONS)snprintf(cost,sizeof(cost),"BELT FULL");
   else snprintf(cost,sizeof(cost),"PRICE %d GOLD",price);
   text(270,227,cost,g->gold>=price&&!(selected==5&&g->potion_count==MAX_POTIONS)?GREEN:RED,1);
 }
 if(r140_potion_gained>=0&&animation_tick-r140_potion_tick<720)r140_potion_card(g,r140_potion_gained,28,209,424);
 footer("X BUY  O LEAVE  SQ DECK  TRI POTIONS");
}
static void potion_screen(const Game*g){
 char title[32];snprintf(title,sizeof(title),"POTION BELT / %d OF %d",g->potion_count,MAX_POTIONS);header(g,title);
 for(int i=0;i<g->potion_count;i++){
  int y=53+i*56;if(i==g->selection)r134_blit(27,y,426,54,r140_selection,208,28,255);
  r134_blit(42,y+8,36,36,r134_utility_icons[0],40,40,255);
  r134_text(89,y+10,game_potion_name(g->potions[i]),GOLD,&r134_f10);
  r134_wrap(89,y+27,341,2,12,game_potion_description_now(g,g->potions[i]),WHITE,&r134_f9,0);
 }
 if(!g->potion_count)center(240,110,"NO POTIONS",MUTED,1);
 const char*hint=g->message[0]?g->message:g->potion_count&&g->potions[g->selection]!=0?"Use this potion during combat.":"Healing potions can be used here.";
 r134_center(240,228,hint,WHITE,&r134_f8);
 footer("UP/DOWN BROWSE  X USE  O RETURN");
}
static void relic_screen(const Game*g){
 header(g,"RELICS / ITEMS DISCOVERED");gilded_panel(16,43,225,207,0);gilded_panel(244,43,220,207,1);
 int first=CLAMP(g->selection-2,0,g->relic_count>6?g->relic_count-6:0);
 for(int i=first;i<g->relic_count&&i<first+6;i++){int y=56+(i-first)*30,chosen=i==g->selection;
  if(chosen)r134_blit(23,y-4,208,28,r140_selection,208,28,255);
  r134_text(46,y+5,relic_defs[g->relics[i]].name,chosen?WHITE:MUTED,&r134_f9);
 }
 if(g->relic_count){int id=g->relics[g->selection];r134_blit(332,59,40,40,r134_utility_icons[1],40,40,255);r134_wrap(264,111,178,2,13,relic_defs[id].name,GOLD,&r138_title10,0);r134_wrap(264,144,178,7,12,relic_defs[id].desc,WHITE,&r134_f9,0);}
 footer("UP/DOWN BROWSE   X/CIRCLE RETURN");
}
static void blessing_screen(const Game*g){
 char title[60];snprintf(title,sizeof(title),"ACT %d / CHOOSE YOUR BLESSING",g->act);header(g,title);
 for(int i=0;i<3;i++){int x=22+i*149,id=g->reward[i],chosen=i==g->selection;
  gilded_panel(x,48,140,196,chosen);r134_blit(x+49,69,42,42,r134_utility_icons[1],40,40,255);
  r134_wrap(x+13,119,114,2,13,relic_defs[id].name,GOLD,&r138_title9,0);
  r134_wrap(x+13,154,114,7,11,relic_defs[id].desc,WHITE,&r134_f9,0);
  if(chosen)r134_blit(x+49,39,42,28,r138_choice,42,28,255);else dark_rect(x+7,55,126,182,70);
 }
 footer("LEFT/RIGHT CHOOSE   X ACCEPT BLESSING");
}
static void reward_preview_screen(const Game*g){
 header(g,"CARD REWARD / INSPECT");int raw=g->reward[g->selection];
 R135CardLayout layout=r135_card_layout(raw,174);card_render(153,55,174,(layout.height>174?layout.height:174),raw,1,1);
 const char*rarity=cards[CARD_ID(raw)].rarity==2?"RARE":cards[CARD_ID(raw)].rarity==1?"UNCOMMON":"COMMON";
 r134_center(240,237,rarity,GOLD,&r134_f9b);footer("LEFT/RIGHT BROWSE   O RETURN");
}
static void battle_piles_screen(const Game*g){
 header(g,"COMBAT DECK / DRAW - HAND - DISCARD - EXHAUST");
 const char *names[]={"DRAW","HAND","DISCARD","EXHAUST"};
 const int *pile=g->scroll==0?g->draw:g->scroll==1?g->hand:
                  g->scroll==2?g->discard:g->exhaust;
 int count=g->scroll==0?g->draw_count:g->scroll==1?g->hand_count:
           g->scroll==2?g->discard_count:g->exhaust_count;
 char b[96];
 for(int i=0;i<4;i++){
   int x=12+i*115;
   button(x,43,109,23,"",g->scroll==i);
   int n=i==0?g->draw_count:i==1?g->hand_count:
         i==2?g->discard_count:g->exhaust_count;
   snprintf(b,sizeof(b),"%s %d",names[i],n);
   fittext(x+8,49,b,14,g->scroll==i?GOLD:WHITE);
 }
 int page=(g->selection/6)*6;
 for(int i=page;i<count&&i<page+6;i++){
   int off=i-page,x=18+(off%3)*108,y=72+(off/3)*82;
   card_render(x,y,98,77,pile[i],i==g->selection,1);
 }
 if(count)card_render(341,72,126,166,pile[g->selection],1,1);
 else center(402,137,"EMPTY PILE",MUTED,1);
 footer("LEFT/RIGHT PILES  UP/DOWN CARDS  O CLOSE");
}
static void card_choice_screen(const Game*g){
 header(g,"CHOOSE A CARD / RESOLVE THE EFFECT");
 int pending=g->next_screen;
 center(239,48,pending==23?"ARMAMENTS - UPGRADE ONE":
 pending==18?"BRAND - EXHAUST A CARD":
 pending==43?"BURNING PACT - EXHAUST":
 pending==17?"TRUE GRIT+ - EXHAUST": "HEADBUTT - CHOOSE DISCARDED CARD",GOLD,1);
 int page=(g->selection/6)*6;
 int count=pending==25?g->discard_count:g->hand_count;
 for(int i=page;i<count&&i<page+6;i++){
  int off=i-page,x=17+(off%3)*108,y=69+(off/3)*90;
  card_render(x,y,99,84,pending==25?g->discard[i]:g->hand[i],i==g->selection,1);
 }
 if(count){
  int c=pending==25?g->discard[g->selection]:g->hand[g->selection];
  if(pending==23&&game_card_is_upgradeable(c))c|=UPBIT;
  card_render(341,70,126,166,c,1,1);
 }
 footer("D-PAD CHOOSE  X CONFIRM  SELECT SAVE & QUIT");
}
static void deck_screen(const Game*g){
 if(g->prior_screen==COMBAT){battle_piles_screen(g);return;}
 header(g,g->prior_screen==REST||g->prior_screen==EVENT?
        "SELECT AN UPGRADE / BEFORE AND AFTER":"YOUR DECK / CARD COLLECTION");
 char b[128];
 int first=(g->selection/6)*6;
 for(int i=first;i<g->deck_count&&i<first+6;i++){
    int idx=i-first;
    int x=18+(idx%3)*108,y=66+(idx/3)*90;
    card_render(x,y,99,84,g->deck[i],i==g->selection,1);
 }
 if(g->deck_count>0){
   int raw=g->deck[g->selection];
   if(g->prior_screen==REST||g->prior_screen==EVENT){
     gilded_panel(339,48,133,197,1);
     r134_text(352,61,"CURRENT",GOLD,&r134_f9b);
     r134_wrap(352,78,105,6,11,card_description(raw),WHITE,&r134_f9,0);
     r134_text(352,147,"UPGRADED",GOLD,&r134_f9b);
     r134_wrap(352,164,105,6,11,game_card_is_upgradeable(raw)?card_description(raw|UPBIT):"Already upgraded",GREEN,&r134_f9,0);
   }else card_render(341,62,126,166,raw,1,1);
   snprintf(b,sizeof(b),"%d / %d",g->selection+1,g->deck_count);r134_center(404,238,b,GOLD,&r134_f9);
 }
 footer(g->prior_screen==REST||g->prior_screen==EVENT?
        "D-PAD CHOOSE  X UPGRADE  O RETURN":"D-PAD BROWSE  O RETURN");
}
static void title_screen(const Game*g){
 r134_blit(269,37,197,91,r134_logo,222,102,255);
 for(int i=0;i<3;i++){
  int y=132+i*35,selected=g->selection==i;
  r134_blit(279,y,176,32,r138_button_on,176,32,selected?255:145);
  r134_center(367,y+12,i==2?"HOW TO PLAY":i==1?"ENDLESS MODE":"BEGIN ASCENT",selected?RGB(255,234,184):WHITE,&r138_title9);
 }
 if(profile_data&&profile_data->runs){
  char b[60];snprintf(b,sizeof(b),"RUNS %lu    WINS %lu",(unsigned long)profile_data->runs,(unsigned long)profile_data->wins);r134_center(367,244,b,GOLD,&r134_f8);
 }else r134_center(367,244,g->selection==1?"UNLIMITED ACTS / RISING DIFFICULTY":"ONE WARRIOR / THREE ACTS",GOLD,&r134_f8);
 footer("UP/DOWN SELECT     X CONFIRM");
}

static void help_screen(const Game*g){
 header(g,"WALK THE SPIRE / CONTROLS");
 text(29,67,"D-PAD  Navigate cards, paths and menus",WHITE,1);
 text(29,87,"CROSS  Select or play a card",WHITE,1);
 text(29,107,"RIGHT  Move to END, CROSS ends turn",WHITE,1);
 text(29,127,"START  Inspect card, enemy, or status",WHITE,1);
 text(29,147,"TRIANGLE  Open potion belt / use any",WHITE,1);

 text(29,167,"Win fights, grow your deck, defeat 3 bosses.",GOLD,1);
 text(29,187,"Weak -25% dealt; Vulnerable +50% taken.",MUTED,1);
 text(29,215,"CROSS / CIRCLE   RETURN",GOLD,1);
 footer("RIFT ASCENT  /  ONE WARRIOR  /  THREE ACTS");
}
static void end_screen(const Game*g){
 header(g,g->screen==VICTORY?"VICTORY / THE HOLLOW KING FALLS":"DEFEAT / THE DARKNESS CLAIMS YOU");
 if(g->screen==VICTORY)r134_blit(110,48,260,80,r140_victory,324,100,255);
 else center(240,77,"THE END",RED,2);
 char b[80];snprintf(b,sizeof(b),"FOES VANQUISHED: %d",g->kill_count);center(240,136,b,WHITE,1);
 snprintf(b,sizeof(b),"DECK: %d CARDS",g->deck_count);center(240,155,b,WHITE,1);
 if(g->screen==VICTORY){
  for(int i=0;i<2;i++){int x=32+i*220,chosen=g->selection==i;
   r134_blit(x,196,196,38,r140_selection,208,28,chosen?255:100);
   r134_center(x+98,210,i?"RETURN TO MENU":"CONTINUE ENDLESS",chosen?WHITE:MUTED,&r134_f9b);
   if(chosen)r134_blit(x+77,174,42,28,r138_choice,42,28,255);
  }
  footer("LEFT/RIGHT CHOOSE   X CONFIRM   O MENU");
 }else{button(139,190,202,35,"RETURN TO MENU",1);footer("X CONTINUE");}
}
static void inspector(const Game*g){
 if(!overlay_kind)return;
 dark_rect(0,29,W,226,214);
 gilded_panel(20,30,440,215,1);
 r134_center(240,35,"STATUS / INVENTORY",GOLD,&r134_f9b);
 const char *tabs[]={"CARD","ENEMIES","PLAYER","POTIONS","RELICS"};
 for(int i=0;i<5;i++){
    int px=28+i*87;
    button(px,51,83,23,tabs[i],overlay_page==i);
 }
 char b[112];
 if(overlay_page==0){
   if(g->hand_count>0&&g->selection<g->hand_count){
      int raw=g->hand[g->selection];card_combat_context=g;
      card_render(32,77,126,r135_card_layout(raw,126).height,raw,1,1);
      char rules[320];game_card_description_now(g,raw,g->target,rules,sizeof(rules));
      r134_wrap(176,81,264,7,12,rules,WHITE,&r134_f9,0);
      CardPreview p;game_card_preview(g,raw,g->target,&p);
      int y=170;
      for(int e=0;e<g->enemy_count;e++)if(p.hits[e]){
       char value[32];if(p.low[e]==p.high[e])snprintf(value,sizeof(value),"%d",p.low[e]);else snprintf(value,sizeof(value),"%d-%d",p.low[e],p.high[e]);
       snprintf(b,sizeof(b),"%s: %s per hit",g->enemies[e].name,p.uncertain?"variable":value);
       r134_text(176,y,b,GOLD,&r134_f9);y+=14;
      }
      r134_text(176,207,"Damage is before enemy Block.",MUTED,&r134_f9);
      if(p.random||p.uncertain)r134_text(176,221,"Random effects can change the result.",MUTED,&r134_f8);
      card_combat_context=0;
   }else text(43,99,"END TURN selected. Press X to confirm.",WHITE,1);
 }else if(overlay_page==1){
   for(int e=0;e<g->enemy_count;e++){
    const Enemy*foe=&g->enemies[e];int x=37+e*216;
    r134_wrap(x,82,194,2,11,foe->name,e==g->target?GOLD:WHITE,&r134_f9b,0);
    snprintf(b,sizeof(b),"HP %d/%d   Block %d",(foe->hp>0?foe->hp:0),foe->maxhp,foe->block);
    r134_text(x,109,b,WHITE,&r134_f9);
    int count=game_enemy_status_count(g,e);
    for(int i=0;i<count;i++){game_enemy_status_line(g,e,i,b,sizeof(b));r134_wrap(x,127+i*15,194,1,11,b,WHITE,&r134_f8,0);}
    if(!count)r134_text(x,129,"No active status effects",MUTED,&r134_f9);
    int hits=game_enemy_intent_hits(g,e),damage=game_enemy_intent_damage(g,e);
    if(foe->hp<=0)snprintf(b,sizeof(b),"Defeated");
    else if(hits)snprintf(b,sizeof(b),"Intent: %d x %d damage",hits,damage/hits);
    else snprintf(b,sizeof(b),"Intent: %s",game_intent_title(foe->intent));
    r134_text(x,211,b,GOLD,&r134_f9);
   }
 }else if(overlay_page==2){
   snprintf(b,sizeof(b),"HP %d/%d   BLOCK %d   ENERGY %d",g->hp,g->maxhp,g->block,g->energy);
   r134_text(39,81,b,GOLD,&r134_f9b);
   r134_text(39,96,"STATUS EFFECTS",MUTED,&r134_f8);
   int count=game_status_count(g),first=CLAMP(overlay_status_scroll,0,(count>7?count-7:0));
   for(int i=0;i<7&&first+i<count;i++){
    game_status_line(g,first+i,b,sizeof(b));
    r134_text(39,111+i*15,b,(strlen(b)==6&&!memcmp(b,"POWERS",6))?GOLD:WHITE,(strlen(b)==6&&!memcmp(b,"POWERS",6))?&r134_f9b:&r134_f9);
   }
   if(!count)r134_text(39,119,"No active powers or status effects.",MUTED,&r134_f9);
   if(count>7){snprintf(b,sizeof(b),"%d-%d / %d",first+1,MIN(first+7,count),count);r134_text(370,225,b,GOLD,&r134_f8);}
 }else if(overlay_page==3) {
    for(int i=0;i<g->potion_count;i++){
       int y=82+i*44;int selected=(i==overlay_potion);
       if(selected)r134_blit(30,y-5,420,43,r140_selection,208,28,255);
       snprintf(b,sizeof(b),"%d. %s",i+1,game_potion_name(g->potions[i]));
       r134_text(66,y+1,b,selected?GOLD:WHITE,&r134_f9b);
       r134_blit(42,y+4,18,25,r134_utility_icons[0],40,40,255);
       r134_wrap(66,y+15,354,2,10,game_potion_description_now(g,g->potions[i]),WHITE,&r134_f8,0);
    }
    if(g->potion_count==0)center(240,113,"NO POTIONS IN YOUR BELT",MUTED,1);
 }else {
    int pick=CLAMP(overlay_status_scroll,0,g->relic_count?g->relic_count-1:0),first=CLAMP(pick-2,0,g->relic_count>5?g->relic_count-5:0);
    for(int i=first;i<g->relic_count&&i<first+5;i++){int y=91+(i-first)*27;
      if(i==pick)r134_blit(31,y-5,200,28,r140_selection,208,28,255);
      r134_text(55,y+3,relic_defs[g->relics[i]].name,i==pick?WHITE:MUTED,&r134_f9);
    }
    if(g->relic_count){int id=g->relics[pick];r134_wrap(251,95,186,2,14,relic_defs[id].name,GOLD,&r138_title10,0);r134_wrap(251,135,186,7,12,relic_defs[id].desc,WHITE,&r134_f9,0);}
    else r134_text(41,99,"No relics",MUTED,&r134_f9);
 }
 footer((overlay_page==2||overlay_page==4)?"LEFT/RIGHT TABS  UP/DOWN SCROLL  START CLOSE":overlay_page==3?"LEFT/RIGHT TABS  UP/DOWN POTION  X USE   START CLOSE":
                          "LEFT/RIGHT TABS  UP/DOWN TARGET  START CLOSE");
}
static void transition_overlay(void){
 if(!fx_transition||animation_tick<fx_transition_tick)return;
 unsigned age=animation_tick-fx_transition_tick;
 if(age>=24){fx_transition=0;return;}
 /* Fade a new event in rather than popping to it instantly. */
 int darkness=(int)((24-age)*205/24);
 dark_rect(0,28,W,227,darkness);
 for(int i=0;i<12;i++){
   unsigned seed=particle_hash(i*99991u+fx_transition_tick);
   int x=(int)(seed%480u);
   int y=49+(int)((seed>>12)%169u)-(int)age;
   put(x,y,GOLD);
 }
}
void game_render_frame(const Game*g,uint32_t*pixels,int stride,unsigned tick){
 animation_tick=tick;
 if((g->screen==REWARD||g->screen==RELIC_RESULT||g->screen==VICTORY||g->screen==DEFEAT)&&victory_replay&&tick>=victory_tick &&
    game_render_fx_busy(tick)){
   static Game shot;
   shot=victory_scene;
   shot.screen=COMBAT;
   unsigned age=tick-victory_tick;
   game_render_frame(&shot,pixels,stride,victory_tick+age);
   if(r1311_finishing){
    int banner=r1311_banner_at();
    if(age>=(unsigned)banner){
     pix=pixels;sw=stride;
     r134_blit(78,71,324,100,r140_victory,324,100,255);
     int fade= (int)age-banner-R1311_VICTORY_HOLD;
     if(fade>0)dark_rect(0,28,W,227,MIN(255,fade*255/R1311_VICTORY_FADE));
    }
    return;
   }
   int fade=r137_duration>30?r137_duration:30;
   if(age>(unsigned)fade){
      pix=pixels;sw=stride;dark_rect(0,28,W,227,(int)(age-fade)*22);
   }
   return;
 }
 pix=pixels;sw=stride;
 if(g->screen==ACT_INTRO){
  rect(0,0,W,H,RGB(0,0,0));
  int t=g->scroll,alpha=t<10?0:t<40?(t-10)*255/30:t<85?255:t<115?(115-t)*255/30:0;
  r134_blit(0,0,480,272,r1311_act,480,272,alpha);
  char label[40];snprintf(label,sizeof(label),"ACT %d",g->act);
  r134_center(240,208,label,RGB(230,222,202),&r134_f18b);
  if(g->endless)r134_center(240,235,"ENDLESS ASCENT",GOLD,&r138_title9);
  dark_rect(0,198,480,74,255-alpha);return;
 }
 background(g);
 ambient_particles(g);
 switch(g->screen){
 case TITLE:title_screen(g);break;
 case MAP:map_screen(g);break;
 case COMBAT:combat(g);break;
 case EVENT_RESULT:event_result_screen(g);break;
 case RELIC_RESULT:relic_result_screen(g);break;
 case REWARD:victory_replay=0;reward_screen(g);break;
 case BLESSING:blessing_screen(g);break;
 case REWARD_PREVIEW:reward_preview_screen(g);break;
 case BOSS_REWARD:boss_reward_screen(g);break;
 case REST:rest_screen(g);break;
 case SHOP:shop_screen(g);break;
 case POTION_VIEW:potion_screen(g);break;
 case EVENT:event_screen(g);break;
 case DECK_VIEW:deck_screen(g);break;
 case CARD_CHOICE:card_choice_screen(g);break;
 case VICTORY:case DEFEAT:end_screen(g);break;
 case HELP:help_screen(g);break;
 case RELIC_VIEW:relic_screen(g);break;
 case PAUSE_MENU: {
   Game frame=*g;frame.screen=game_pause_source();
   game_render_frame(&frame,pixels,stride,tick);
   dark_rect(0,24,W,231,209);
   gilded_panel(76,59,328,158,1);
   center(240,77,"GAME PAUSED",GOLD,2);
   center(240,112,"X SAVE & QUIT",WHITE,1);
   center(240,137,"CIRCLE / SELECT RESUME",WHITE,1);
   center(240,173,"PROGRESS SAVED TO MEMORY STICK",MUTED,1);
   break;
 }
 }
 transition_overlay();
 if(g->screen==COMBAT){
   if(r134_playing()){
    int age=(int)(tick-rift120_play_tick),t=CLAMP(age,0,18),ease=t*(36-t);
    int shown=age>=18&&rift120_auto_card>=0?rift120_auto_card:rift120_played_card;
    if(shown<0)shown=rift120_auto_card;
    if(shown>=0){
     if(r136_play_state_valid&&rift120_played_card>=0&&CARD_ID(shown)==CARD_ID(rift120_played_card)){
      shown=rift120_played_card;card_combat_context=&r136_play_state;
     }else card_combat_context=g;
     int target_h=r135_card_layout(shown,126).height,target_y=(272-target_h)/2;
     int x=rift133_play_x+(177-rift133_play_x)*ease/324;
     int y=174+(target_y-174)*ease/324,w=70+56*ease/324,h=84+(target_h-84)*ease/324;
     static uint32_t backdrop[126*220];
     int opacity=age<18?255:age<26?(26-age)*96/8:0;
     if(opacity<255)for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)backdrop[yy*w+xx]=pix[(y+yy)*sw+x+xx];
     card_render(x,y,w,h,shown,1,1);card_combat_context=0;
     if(opacity<255)for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){uint32_t*d=&pix[(y+yy)*sw+x+xx];*d=r134_mix(backdrop[yy*w+xx],*d,opacity);}
    }
   }else if(tick>=rift120_play_tick&&tick-rift120_play_tick>=38){rift120_played_card=-1;rift120_auto_card=-1;}
   inspector(g);
 }

}

/* Existing callers get a deterministic still frame. PSP uses animated frames. */
void game_render(const Game*g,uint32_t*pixels,int stride){
 game_render_frame(g,pixels,stride,0);
}
