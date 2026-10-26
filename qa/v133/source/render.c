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
void game_render_preplay(const Game *g,unsigned tick){
 if(g && g->screen==COMBAT && g->selection>=0 &&
    g->selection<g->hand_count){
   rift120_played_card=g->hand[g->selection];
   int first=g->selection>=5?g->selection-4:0;
   rift133_play_x=CLAMP(56+(g->selection-first)*73-21,35,300);
   rift120_auto_card=-1;rift120_play_tick=tick;
 }
}
static void rift120_log_add(const char *s){
 if(!s||!*s)return;
 for(int i=0;i<2;i++)memcpy(rift120_log[i],rift120_log[i+1],64);
 snprintf(rift120_log[2],sizeof(rift120_log[2]),"%s",s);
 if(rift120_log_count<3)rift120_log_count++;
}
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
void game_render_feedback(const Game *before,const Game *after,int action,unsigned tick){
 if(before->screen==COMBAT && action==BTN_OK &&
    before->selection>=0&&before->selection<before->hand_count){
   int raw=before->hand[before->selection];
   if(rift120_played_card!=raw || tick<rift120_play_tick ||
  tick-rift120_play_tick>26){
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
 if(before->screen==COMBAT && after->screen==REWARD){
    victory_scene=*before;
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
static void sprite(int dx,int dy,int w,int h,const uint16_t *data){
 int x0=dx<0?0:dx,y0=dy<0?0:dy,x1=dx+w>W?W:dx+w,y1=dy+h>H?H:dy+h;
 for(int y=y0;y<y1;y++){
  const uint16_t *src=data+(size_t)(y-dy)*w;
  uint32_t *dst=pix+(size_t)y*sw;
  for(int x=x0;x<x1;x++){
   uint16_t c=src[x-dx];if(c==RIFT_TRANSPARENT_565)continue;
   unsigned int r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   dst[x]=RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
  }
 }
}
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
static void gilded_panel(int x,int y,int w,int h,int selected){
 rect(x+2,y+2,w,h,RGB(5,10,17));
 rect(x,y,w,h,RGB(15,25,38));
 border(x,y,w,h,selected?GOLD:RGB(121,97,69));
 rect(x+2,y+2,w-4,1,selected?GOLD:RGB(59,77,91));
 rect(x+2,y+h-3,w-4,1,RGB(44,54,64));
 put(x+2,y+2,GOLD);put(x+w-3,y+2,GOLD);
 put(x+2,y+h-3,GOLD);put(x+w-3,y+h-3,GOLD);
}
static void put(int x,int y,uint32_t c){if((unsigned)x<W&&(unsigned)y<H)pix[(size_t)y*sw+x]=c;}
static void rect(int x,int y,int w,int h,uint32_t c){if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>W)w=W-x;if(y+h>H)h=H-y;if(w<1||h<1)return;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)pix[(size_t)j*sw+i]=c;}
static void line(int x0,int y0,int x1,int y1,uint32_t c){int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy;for(;;){put(x0,y0,c);if(x0==x1&&y0==y1)break;int e=2*err;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
static void circle(int xc,int yc,int r,uint32_t c){for(int y=-r;y<=r;y++)for(int x=-r;x<=r;x++)if(x*x+y*y<=r*r)put(xc+x,yc+y,c);}
static void triangle(int x1,int y1,int x2,int y2,int x3,int y3,uint32_t c){int minx=x1<x2?(x1<x3?x1:x3):(x2<x3?x2:x3);int maxx=x1>x2?(x1>x3?x1:x3):(x2>x3?x2:x3);int miny=y1<y2?(y1<y3?y1:y3):(y2<y3?y2:y3);int maxy=y1>y2?(y1>y3?y1:y3):(y2>y3?y2:y3);minx=minx<0?0:minx;miny=miny<0?0:miny;maxx=maxx>=W?W-1:maxx;maxy=maxy>=H?H-1:maxy;int det=(y2-y3)*(x1-x3)+(x3-x2)*(y1-y3);if(det==0)return;for(int y=miny;y<=maxy;y++)for(int x=minx;x<=maxx;x++){int a=(y2-y3)*(x-x3)+(x3-x2)*(y-y3),b=(y3-y1)*(x-x3)+(x1-x3)*(y-y3),d=det-a-b;if(det>0?(a>=0&&b>=0&&d>=0):(a<=0&&b<=0&&d<=0))put(x,y,c);}}
static void border(int x,int y,int w,int h,uint32_t c){rect(x,y,w,1,c);rect(x,y+h-1,w,1,c);rect(x,y,1,h,c);rect(x+w-1,y,1,h,c);}
static uint32_t blend_text(uint32_t bg,uint32_t ink,int opacity) {
 int br=bg&255,bg_=bg>>8&255,bb=bg>>16&255;
 int ir=ink&255,ig=ink>>8&255,ib=ink>>16&255;
 return RGB((br*(255-opacity)+ir*opacity)/255,
            (bg_*(255-opacity)+ig*opacity)/255,
            (bb*(255-opacity)+ib*opacity)/255);
}
static void text(int x,int y,const char *s,uint32_t ink,int size) {
 if(!s)return;
 for(int n=0;s[n]&&x<W;n++) {
    unsigned chr=(unsigned char)s[n];
    if(chr<32||chr>126)chr='?';
    const uint8_t *glyph=rift_smooth_font[chr-32];
    for(int yy=0;yy<11;yy++)
     for(int xx=0;xx<7;xx++){
       unsigned alpha=glyph[yy*7+xx];
       if(!alpha)continue;
       int px=x+xx*size,py=y+yy*size;
       for(int sy=0;sy<size;sy++)
        for(int sx=0;sx<size;sx++){
          int tx=px+sx,ty=py+sy;
          if((unsigned)tx<W&&(unsigned)ty<H) {
             uint32_t *dst=&pix[(size_t)ty*sw+tx];
             *dst=blend_text(*dst,ink,(int)alpha);
          }
        }
     }
    x+=7*size;
 }
}
static void center(int x,int y,const char*s,uint32_t c,int scale){int len=(int)strlen(s);text(x-len*7*scale/2,y,s,c,scale);}
static void fittext(int x,int y,const char*s,int n,uint32_t c){char b[128];snprintf(b,sizeof(b),"%.*s",n,s);text(x,y,b,c,1);}
/* Identical symmetrical retro-Gothic furniture across title, shop, rest,
 * relic and combat menus. Ornament stays outside the text-safe center. */
static void button(int x,int y,int w,int h,const char*label,int selected){
 sprite_scaled(x,y,w,h,rift133_button,RIFT133_BUTTON_W,RIFT133_BUTTON_H);
 if(!selected)dark_rect(x+2,y+2,w-4,h-4,58);
 center(x+w/2,y+(h-11)/2,label,selected?RGB(255,231,179):WHITE,1);
}


static void rift117_small_text(int x,int y,const char *s,uint32_t ink,int cw,int ch){
 if(!s)return;
 for(int n=0;s[n];n++){
  unsigned chr=(unsigned char)s[n];
  if(chr<32||chr>126)chr='?';
  const uint8_t *glyph=(cw==6?rift117_font_6x9[chr-32]:
                       cw==5?rift117_font_5x8[chr-32]:
                             rift117_font_4x6[chr-32]);
  for(int yy=0;yy<ch;yy++)for(int xx=0;xx<cw;xx++){
    int alpha=glyph[yy*cw+xx];
    if(alpha>20&&(unsigned)(x+xx)<W&&(unsigned)(y+yy)<H){
      uint32_t *dst=&pix[(size_t)(y+yy)*sw+(x+xx)];
      *dst=blend_text(*dst,ink,alpha);
    }
  }
  x+=cw;
 }
}
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
static void bar(int x,int y,int w,int h,int current,int max,uint32_t c){rect(x,y,w,h,RGB(56,53,58));if(max>0)rect(x,y,(w*CLAMP(current,0,max))/max,h,c);border(x,y,w,h,RGB(176,151,124));}
/* self-contained integer clamp */
/* Unified compact health capsules.  Block lives outside the capsule,
   exactly where it belongs for both player and enemies. */
/* One HP capsule shared by player and enemies, not two stacked textures. */
static void rift131_shield(int x,int y){
 const uint32_t blue=RGB(114,212,255),silver=RGB(180,226,254);
 rect(x+2,y+3,11,8,RGB(22,77,119));
 line(x+1,y+2,x+13,y+2,silver);
 line(x+1,y+2,x+1,y+12,blue);line(x+13,y+2,x+13,y+12,blue);
 line(x+1,y+12,x+7,y+17,blue);line(x+13,y+12,x+7,y+17,blue);
 line(x+4,y+5,x+4,y+11,silver);line(x+4,y+11,x+7,y+14,silver);
}
static void rift120_hp_meter(int x,int y,int w,int hp,int maxhp,int shield){
 char b[32];int bw=w+4;
 const uint32_t rim=RGB(155,111,80),inner=RGB(9,13,22);
 rect(x+3,y+1,bw-6,24,inner);
 rect(x+1,y+4,bw-2,18,inner);
 rect(x,y+7,bw,12,inner);
 line(x+4,y,x+bw-5,y,rim);line(x+4,y+25,x+bw-5,y+25,rim);
 line(x,y+7,x,y+18,rim);line(x+bw-1,y+7,x+bw-1,y+18,rim);
 put(x+1,y+5,rim);put(x+bw-2,y+5,rim);
 put(x+1,y+20,rim);put(x+bw-2,y+20,rim);
 rect(x+5,y+16,bw-10,5,RGB(55,35,39));
 if(maxhp>0){
   int fill=(bw-10)*CLAMP(hp,0,maxhp)/maxhp;
   if(fill>0)rect(x+5,y+16,fill,5,RGB(194,55,62));
 }
 snprintf(b,sizeof(b),"%d/%d",hp,maxhp);
 center(x+bw/2,y+2,b,WHITE,1);
 if(shield>0){
   border(x+5,y+15,bw-10,7,BLUE);
   rift131_shield(x+bw+4,y+3);
   snprintf(b,sizeof(b),"%d",shield);
   fittext(x+bw+20,y+8,b,3,WHITE);
 }
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
 if(g->screen==TITLE||g->screen==HELP||g->screen==VICTORY||g->screen==DEFEAT)
   src=ra_title_bg;
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
 rect(0,0,W,29,RGB(9,16,27));
 rect(0,27,W,1,RGB(149,106,63));
 text(11,7,"RIFT ASCENT",GOLD,1);
 char linebuf[128];snprintf(linebuf,sizeof(linebuf),"HP %d/%d",g->hp,g->maxhp);text(124,7,linebuf,RED,1);
 snprintf(linebuf,sizeof(linebuf),"$%d",g->gold);text(220,7,linebuf,GOLD,1);
 snprintf(linebuf,sizeof(linebuf),"ACT %d  FL %02d",g->act,g->floor+1);text(337,7,linebuf,WHITE,1);
 dark_rect(0,29,W,15,164);
 fittext(14,31,caption,63,MUTED);
}
static int footer_button(const char *p,int n){
 if(n==1&&p[0]=='X')return 1;
 if(n==1&&p[0]=='O')return 2;
 if(n==2&&p[0]=='S'&&p[1]=='Q')return 3;
 if(n==6&&!memcmp(p,"SQUARE",6))return 3;
 if(n==6&&!memcmp(p,"CIRCLE",6))return 2;
 if(n==3&&!memcmp(p,"TRI",3))return 4;
 if(n==8&&!memcmp(p,"TRIANGLE",8))return 4;
 if(n==5&&!memcmp(p,"START",5))return 5;
 return 0;
}
static void footer_glyph(int x,int y,int kind){
 if(kind==5){
   rect(x-7,y-4,19,9,RGB(40,53,75));
   border(x-7,y-4,19,9,WHITE);
   /* Tiny 'ST', visually distinct from the outlined Square glyph. */
   text(x-5,y-4,"ST",WHITE,1);
   return;
 }
 circle(x,y,7,RGB(47,57,76));circle(x,y,6,RGB(95,115,137));
 if(kind==1){line(x-3,y-3,x+3,y+3,WHITE);line(x+3,y-3,x-3,y+3,WHITE);}
 else if(kind==2){circle(x,y,4,WHITE);circle(x,y,2,RGB(95,115,137));}
 else if(kind==3){border(x-4,y-4,9,9,WHITE);}
 else if(kind==4){triangle(x,y-4,x-4,y+4,x+4,y+4,WHITE);}
}
static void footer(const char*help){
 rect(0,255,W,17,RGB(9,16,27));rect(0,255,W,1,RGB(115,85,60));
 int x=8;
 for(const char *p=help;*p&&x<470;){
  if(*p==' '){x+=3;p++;continue;}
  if(*p=='/'||*p=='+'){text(x,258,"/",MUTED,1);x+=9;p++;continue;}
  const char*q=p;while(*q&&*q!=' '&&*q!='/'&&*q!='+')q++;
  int n=(int)(q-p),kind=footer_button(p,n);
  if(kind){footer_glyph(x+7,264,kind);x+=18;}
  else if(n<44){char word[46];memcpy(word,p,(size_t)n);word[n]=0;
    text(x,258,word,MUTED,1);x+=n*7+3;}
  p=q;
 }
}
static void enemy_art(int x,int y,const Enemy*e){
 int id=e->art;uint32_t cloak=id<3?RGB(63,87,86):id<6?RGB(115,59,65):RGB(100,92,91);
 circle(x,y+40,30,RGB(23,32,41));
 if(id==0){
  for(int k=-1;k<=1;k++)line(x+k*6,y+39,x+k*11,y+64,cloak);
  circle(x,y+12,19,cloak);circle(x,y+10,11,RGB(90,115,95));
  circle(x-6,y+5,3,RED);circle(x+6,y+5,3,RED);
  triangle(x-21,y+19,x-17,y-8,x-3,y+10,cloak);triangle(x+3,y+10,x+17,y-8,x+21,y+19,cloak);
 }else if(id==1||id==6){
  rect(x-19,y+15,38,43,cloak);triangle(x-22,y+20,x,y-17,x+22,y+20,RGB(127,126,113));
  rect(x-15,y+14,30,15,RGB(47,58,61));rect(x-10,y+16,6,3,RED);rect(x+4,y+16,6,3,RED);
  rect(x-24,y+33,13,20,RGB(104,108,103));rect(x+11,y+33,13,20,RGB(104,108,103));
  rect(x-15,y+58,12,9,RGB(112,111,97));rect(x+4,y+58,12,9,RGB(112,111,97));
 }else if(id==2||id==5){
  triangle(x-28,y+65,x,y-13,x+28,y+65,cloak);
  circle(x,y+3,18,RGB(25,30,37));circle(x,y+6,6,id==2?BLUE:RED);
  triangle(x-13,y+5,x,y-23,x+13,y+5,RGB(151,91,75));
  line(x+18,y+26,x+38,y-14,RGB(127,91,55));circle(x+39,y-15,8,id==2?BLUE:RED);
 }else{
  triangle(x-27,y+65,x,y+4,x+29,y+65,cloak);
  rect(x-20,y+8,40,42,RGB(85,79,85));
  circle(x,y+5,19,RGB(115,106,106));rect(x-14,y+6,28,10,RGB(32,41,46));
  rect(x-11,y+9,7,4,RED);rect(x+4,y+9,7,4,RED);
  triangle(x-14,y+5,x-19,y-25,x-2,y-4,RGB(123,121,119));
  triangle(x+2,y-4,x+19,y-25,x+14,y+5,RGB(123,121,119));
  if(id>=7){line(x+25,y+21,x+42,y-13,GOLD);triangle(x+35,y-15,x+52,y-30,x+47,y-5,GOLD);}
 }
}
static const char*intent_name(int n){static const char*s[]={"STRIKE","GUARD","WEAKEN","EXPOSE","EMPOWER","HEX","VENOM","DOUBLE","DRAIN","MEND","CRUSH","DREAD","LEECH","FORTIFY"};return n>=0&&n<14?s[n]:"???";}

static int rift118_corner_inset(int row,int h,int radius){
 int dist=row<h-1-row?row:h-1-row;
 if(dist>=radius)return 0;
 if(dist==0)return radius-1;
 if(dist==1)return radius-2;
 return 1;
}
static void rift118_round_outline(int x,int y,int w,int h,uint32_t ink,int radius){
 if(w<radius*3||h<radius*3)return;
 for(int j=0;j<h;j++){
   int ins=rift118_corner_inset(j,h,radius);
   if(j==0||j==h-1)rect(x+ins,y+j,w-ins*2,1,ink);
   else {
     put(x+ins,y+j,ink);
     put(x+w-ins-1,y+j,ink);
   }
 }
}
static void rift118_card_panel(int x,int y,int w,int h,int selected){
 const int radius=4;
 uint32_t face=RGB(15,25,38);
 uint32_t edge=selected?GOLD:RGB(121,97,69);
 /* Each row begins/ends inside its rounded contour: no square fragments. */
 for(int j=0;j<h;j++){
   int ins=rift118_corner_inset(j,h,radius);
   rect(x+ins,y+j,w-2*ins,1,face);
 }
 rift118_round_outline(x,y,w,h,edge,radius);
 rect(x+4,y+3,w-8,1,selected?GOLD:RGB(59,77,91));
 rect(x+4,y+h-4,w-8,1,RGB(44,54,64));
}

static void rift121_rules(int x,int y,const char *src,int cols,int rows,
                          uint32_t ink,int cw,int ch){
 if(cols<4||rows<1||!src)return;
 char lines[36][66];int n=0;
 const char *p=src;
 while(*p&&n<36){
   while(*p==' ')p++;
   if(!*p)break;
   int k=0,last_space=-1;
   while(p[k]&&p[k]!='\n'&&k<cols&&k<63){
      if(p[k]==' ')last_space=k;k++;
   }
   if(!k&&*p=='\n'){p++;continue;}
   if(p[k] && p[k]!='\n' && k>=cols && last_space>0)
      k=last_space;
   memcpy(lines[n],p,(size_t)k);lines[n][k]=0;
   while(k>0&&lines[n][k-1]==' ')lines[n][--k]=0;
   n++;p+=k;
   while(*p==' '||*p=='\n')p++;
 }
 int scroll=n>rows?((int)(animation_tick/125u)%(n-rows+1)):0;
 for(int i=0;i<rows&&i+scroll<n;i++){
   rift117_wrapped_text(x,y+i*(ch+1),lines[i+scroll],cols,1,ink,cw,ch);
 }
 if(n>rows)text(x+cols*cw-9,y+(rows-1)*(ch+1),"v",GOLD,1);
}
static const char *rift131_clean_rules(const char *s){
 static char clean[192];
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
/* This engine routine already applies Strength, Weak, Vulnerable, relic
 * modifiers and repeat counts; sum over enemies only for ALL attacks. */
static int rift132_preview_damage(const Game *g,int raw){
 if(!g||g->screen!=COMBAT)return 0;
 int id=CARD_ID(raw),total=0;
 int all=(id==3||id==10||id==14||id==19||id==36||id==37||id==70);
 if(all){
  for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)
    total+=game_effective_damage(g,raw,i);
 }else{
  int target=g->target;
  if(target<0||target>=g->enemy_count||g->enemies[target].hp<=0){
   for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){target=i;break;}
  }
  total=game_effective_damage(g,raw,target);
 }
 return total;
}
static void card_render(int x,int y,int w,int h,int raw,int selected,int active){
 int id=CARD_ID(raw);if(id<0||id>=CARD_COUNT)return;
 const CardDef*d=&cards[id];int small=w<85||h<100;
 int inset=small?5:8,inner=w-2*inset;
 rect(x+inset,y+5,inner,h-10,RGB(15,19,27));
 sprite_scaled(x,y,w,h,rift133_card_frame,RIFT133_CARD_FRAME_W,RIFT133_CARD_FRAME_H);
 char title[48];snprintf(title,sizeof(title),"%s%s",d->name,CARD_UP(raw)?"+":"");
 int titleh=small?16:22;
 rect(x+inset,y+7,inner,titleh-4,RGB(25,19,26));
 rift117_wrapped_text(x+inset+2,y+7,title,(inner-4)/5,2,selected?GOLD:WHITE,5,7);
 int art_y=y+titleh+3,art_h=small?32:(h>=135?55:36);
 art_blit(x+inset,art_y,inner,art_h,rift120_cards[id],60,40);
 char cost[10];int energy=card_combat_context?game_card_cost_now(card_combat_context,raw):game_card_cost(raw);
 if(energy==-1)snprintf(cost,sizeof(cost),"X");else if(energy==-2)snprintf(cost,sizeof(cost),"-");else snprintf(cost,sizeof(cost),"%d",energy);
 sprite_scaled(x+inset-1,art_y-1,19,19,rift133_energy,RIFT133_ENERGY_W,RIFT133_ENERGY_H);
 center(x+inset+8,art_y+3,cost,WHITE,1);
 if(card_combat_context&&d->type==0){
  int damage=rift132_preview_damage(card_combat_context,raw);
  if(damage>0){char v[12];snprintf(v,sizeof(v),"%d",damage);
   dark_rect(x+w-inset-28,art_y+1,28,14,220);
   center(x+w-inset-14,art_y+3,v,GOLD,1);
  }
 }
 int text_y=art_y+art_h+3,bottom=y+h-(small?6:9);
 const char*rules=rift131_clean_rules(card_description(raw));
 int cw=small?4:5,ch=small?6:8,rows=(bottom-text_y)/(ch+1),cols=(inner-2)/cw;
 if(!small&&(int)strlen(rules)>rows*cols-10){cw=4;ch=6;rows=(bottom-text_y)/7;cols=(inner-2)/4;}
 if(rows>0)rift121_rules(x+inset+1,text_y,rules,cols,rows,selected?WHITE:MUTED,cw,ch);
 if(!active)dark_rect(x+inset,y+6,inner,h-12,105);
}

static void rift_status_chip(int x,int y,const char *glyph,int stacks,uint32_t ink){
 char label[24];snprintf(label,sizeof(label),"%s %d",glyph,stacks);
 rect(x,y,61,15,RGB(21,30,44));border(x,y,61,15,ink);
 circle(x+7,y+7,5,ink);
 fittext(x+15,y+2,label,6,WHITE);
}

/* Compact, shadowed, ICON-ONLY enemy telegraphs.
 * The enemy's full intention remains accessible through Start / Info.
 * Attack damage uses game's actual calculated post-modifier value.
 */
static void rift116_draw_enemy_intent(const Game*g,int index,int cx){
 const Enemy *e=&g->enemies[index];
 const uint16_t *glyph[3];
 char value[3][16];
 int count=0;
 int hits=game_enemy_intent_hits(g,index);
 int damage=game_enemy_intent_damage(g,index);
 if(hits>0){
  glyph[count]=rift116_icon_swords;
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
  glyph[count]=rift116_icon_shield;
  snprintf(value[count],sizeof(value[count]),"%d",block);
  count++;
 }
 if(buff!=-1&&count<3){
  glyph[count]=rift116_icon_up;
  if(buff==-2)value[count][0]=0; /* Heal: numeric amount varies */
  else snprintf(value[count],sizeof(value[count]),"%d",buff);
  count++;
 }
 if(debuff>=0&&count<3){
  glyph[count]=rift116_icon_down;
  if(e->intent==2||e->intent==3||e->intent==6)
   snprintf(value[count],sizeof(value[count]),"%d",debuff);
  else value[count][0]=0; /* curse card or energy penalty */
  count++;
 }
 if(!count)return;
 int widths[3],total=0;
 for(int j=0;j<count;j++){
   widths[j]=24+(int)strlen(value[j])*7+3;
   total+=widths[j];
 }
 int x=cx-total/2;
 for(int j=0;j<count;j++){
   /* Transparent illustrated icons; no black rectangles or text panels. */
   sprite_scaled(x,50,23,23,glyph[j],24,24);
   if(value[j][0]){
     text(x+24,59,value[j],RGB(16,21,31),1);
     text(x+23,58,value[j],j==0&&hits?GOLD:WHITE,1);
   }
   x+=widths[j];
 }
}


static int rift133_hand_start(const Game*g){
 int start=0;if(g->selection>=5&&g->selection<g->hand_count)start=g->selection-4;
 else if(g->selection>=g->hand_count&&g->hand_count>5)start=g->hand_count-5;
 return start;
}
static int rift133_preview_x(const Game*g){return CLAMP(56+(g->selection-rift133_hand_start(g))*73-21,35,300);}
static int rift133_hp_y(const Game*g,int x,int w,int shield){
 int right=x+w+4+(shield>0?45:0);
 if(g->selection>=0&&g->selection<g->hand_count){int left=rift133_preview_x(g);
  if(x<left+112&&right>left)return 78;
 }
 if((rift120_played_card>=0||rift120_auto_card>=0)&&animation_tick>=rift120_play_tick&&animation_tick-rift120_play_tick<24){
  if(x<rift133_play_x+112&&right>rift133_play_x)return 78;
 }
 return 142;
}
static void combat(const Game*g){
 card_combat_context=g;
 /* Independent animated hero and foe layers are extracted from the scene.
    Different enemy species use procedural sprites, not an incorrect wight. */
 unsigned age=animation_tick>=fx_tick?animation_tick-fx_tick:999;
 int hero_bob=wave(animation_tick,110,2);
 int recoil=fx_player_damage>0&&age<14?((14-(int)age)*((age&1)?-1:1)/2):0;
 if(enemy_windup>0)recoil+=(enemy_windup&3)==0?1:0;
 sprite_scaled(RIFT_HERO_X+24+recoil,RIFT_HERO_Y+28+hero_bob,160,99,rift_hero,RIFT_HERO_W,RIFT_HERO_H);
 /* The claymore, its guard and hilt are painted into motion_art.c.
    Do not draw independent diagonal line primitives over the sprite. */

 for(int i=0;i<g->enemy_count;i++){
  const Enemy *e=&g->enemies[i];if(e->hp<=0)continue;
  int ex=g->enemy_count==1?337:(i==0?276:392);
  int bob=wave(animation_tick+i*24u,96,3);
  if(fx_enemy_damage[i]>0&&age<12)ex+=((int)(12-age)*((age&1)?1:-1))/2;
  if(fx_player_damage>0&&age<12)ex-=(int)((12-age)*2/3);
  if(enemy_windup>0)ex-=((24-enemy_windup)>6 && (24-enemy_windup)<19)?8:0;
  if(e->art>=0&&e->art<ENEMY_COUNT){
   int dw=(g->enemy_count==1?128:96)+wave(animation_tick+i*9u,140,1);
   int dh=(g->enemy_count==1?124:108)+wave(animation_tick+i*13u,130,1);
   int facing_wrong=(e->art==1||e->art==4||e->art==9||e->art==10||
                     e->art==14||e->art==15||e->art==16||e->art==19||e->art==20||
                     e->art==26||e->art==29||e->art==32||
                     e->art==2||e->art==7||e->art==11||
                     e->art==18||e->art==21||e->art==23||e->art==27||
                     e->art==30||e->art==35);
   sprite_scaled_facing(ex-dw/2,169-dh+bob,dw,dh,
     rift_enemy_sprites[e->art],RIFT_ENEMY_W,RIFT_ENEMY_H,facing_wrong);
  }else enemy_art(ex,84+bob,e);
 }
 /* Short battle event animation is drawn behind live card/UI information. */
 if(effect_kind&&animation_tick>=effect_start&&animation_tick-effect_start<14){
  unsigned age=animation_tick-effect_start;
  if(effect_kind==1){ /* attack streak travelling to the target */
   int x=155+(int)age*12;
   for(int j=0;j<3;j++)line(x+j*5,78+j*9,x+j*5+19,55+j*9,(j==1?GOLD:RED));
  }else if(effect_kind==2){ /* defensive sigil around the hero */
   int r=22+(int)age;
   for(int k=0;k<22;k++){
    int px=100+((k*17)%r)-r/2;
    int py=115+((k*29)%r)-r/2;
    put(px,py,(k&1)?BLUE:WHITE);
   }
   border(78-(int)age/2,81-(int)age/2,48+(int)age,59+(int)age,BLUE);
  }else if(effect_kind==3){ /* healing/potion sparks */
   for(int k=0;k<14;k++){
    int px=95+(int)((particle_hash((unsigned)k*91u)%67u))-33;
    int py=156-(int)(age*3)-(k%3)*4;
    rect(px,py,2,3,GREEN);
   }
  }else if(effect_kind==4){ /* hit feedback */
   for(int k=0;k<7;k++)line(72+k*11,84+((int)age%4),62+k*11,96+((int)age%4),RED);
  }
 }
 /* Fly-up damage and block text plus a small spray of combat sparks. */
 if(age<29) {
   char amount[28];
   if(fx_player_damage>0){
     snprintf(amount,sizeof(amount),"-%d",fx_player_damage);
     center(95,85-(int)(age/2),amount,RED,2);
   } else if(fx_heal>0){
     snprintf(amount,sizeof(amount),"+%d",fx_heal);
     center(95,85-(int)(age/2),amount,GREEN,1);
   } else if(fx_block>0){
     snprintf(amount,sizeof(amount),"+%d BLOCK",fx_block);
     center(95,78-(int)(age/2),amount,BLUE,1);
   }
   for(int i=0;i<g->enemy_count;i++)if(fx_enemy_damage[i]>0){
      int ex=g->enemy_count==1?337:(i==0?276:392);
      snprintf(amount,sizeof(amount),"-%d",fx_enemy_damage[i]);
      center(ex,86-(int)(age/2),amount,age<8?WHITE:RED,2);
      /* No unrelated decorative geometry around enemy silhouettes. */
   }
 }
 if(enemy_windup>0) {
    /* Windup is conveyed through enemy movement and flashing slash sparks. */
    int x=g->enemy_count==1?337:(g->target==0?276:392);
    int y=105-(24-enemy_windup)/3;
    if((enemy_windup/3)%2==0)circle(x,y,2,GOLD);
 }
 header(g,"");
 {
   int x=13;
   for(int i=2;i>=0;i--){
     if(!rift120_log[i][0])continue;
     if(x>13){text(x,31," | ",GOLD,1);x+=21;}
     const char *entry=rift120_log[i];
     int cols=MIN(28,(468-x)/7);
     if(cols<=0)break;
     fittext(x,31,entry,cols,(memcmp(entry,"TURN",4)==0)?GOLD:WHITE);
     x+=MIN((int)strlen(entry),cols)*7;
     if(x>454)break;
   }
 }
 if(!memcmp(g->message,"Random exhaust:",15)) {
   dark_rect(20,46,236,18,222);
   fittext(27,50,g->message,32,GOLD);
 }
 /* The illustrated reference contained placeholder HUD text. Occlude it
    so no static HP or intent labels compete with the live simulation. */
 /* No giant backdrop above enemies: distinct icon silhouettes render on scenery. */
 rect(0,167,W,8,RGB(27,25,28));
 /* The battlefield and character illustrations are native embedded artwork;
    all stats and selections below are live gameplay data. */
 char b[100];
 rift120_hp_meter(45,rift133_hp_y(g,45,70,g->block),70,g->hp,g->maxhp,g->block);
 int py=78;
 if(g->strength){rift_status_chip(10,py,"STR",g->strength,GOLD);py+=20;}
 if(g->weak){rift_status_chip(10,py,"WEAK",g->weak,RED);py+=20;}
 if(g->vulnerable){rift_status_chip(10,py,"VUL",g->vulnerable,PURPLE);py+=20;}
 if(g->poison){rift_status_chip(10,py,"PSN",g->poison,GREEN);}
 if(g->dexterity){snprintf(b,sizeof(b),"DEX %d",g->dexterity);text(13,119,b,BLUE,1);}

 for(int i=0;i<g->enemy_count;i++){
  const Enemy*e=&g->enemies[i];if(e->hp<=0)continue;
  int ex=g->enemy_count==1?337:i==0?276:392;
  rift116_draw_enemy_intent(g,i,ex);
  rift120_hp_meter(ex-(g->enemy_count==1?39:29),rift133_hp_y(g,ex-(g->enemy_count==1?39:29),g->enemy_count==1?70:53,e->block),g->enemy_count==1?70:53,e->hp,e->maxhp,e->block);

  if(i==g->target){
    int px=415,py=83;
    if(e->strength){rift_status_chip(px,py,"STR",e->strength,GOLD);py+=20;}
    if(e->weak){rift_status_chip(px,py,"WEAK",e->weak,RED);py+=20;}
    if(e->vulnerable){rift_status_chip(px,py,"VUL",e->vulnerable,PURPLE);py+=20;}
    if(e->poison){rift_status_chip(px,py,"PSN",e->poison,GREEN);}
    if(g->enemy_count>1)text(ex-4,rift133_hp_y(g,ex-29,53,e->block)-10,"v",GOLD,1);
  }
 }
 sprite_scaled(0,175,480,80,rift133_hand_tray,RIFT133_HAND_TRAY_W,RIFT133_HAND_TRAY_H);
 sprite_scaled(0,194,52,52,rift133_energy,RIFT133_ENERGY_W,RIFT133_ENERGY_H);
 if(g->energy==0){
  for(int yy=194;yy<246;yy++)for(int xx=0;xx<52;xx++){
   uint16_t a=rift133_energy[(yy-194)*52+xx];if(a==RIFT_TRANSPARENT_565)continue;
   uint32_t c=pix[yy*sw+xx];int v=((c&255)*30+((c>>8)&255)*59+((c>>16)&255)*11)/100;
   pix[yy*sw+xx]=RGB(v*3/4,v*3/4,v*3/4);
  }
 }
 snprintf(b,sizeof(b),"%d",g->energy);center(26,209,b,WHITE,2);
 int visible=5,start=0;
 if(g->selection>=visible&&g->selection<g->hand_count)start=g->selection-visible+1;
 else if(g->selection>=g->hand_count&&g->hand_count>visible)start=g->hand_count-visible;
 for(int i=start;i<g->hand_count&&i<start+visible;i++){
  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?168+wave(animation_tick,60,1):177;
  int id=CARD_ID(g->hand[i]);
  if(hand_exchange&&animation_tick>=hand_exchange_tick){
    unsigned exchange_age=animation_tick-hand_exchange_tick;
    if(exchange_age<10)y=270; /* discard leaves before drawing */
    else if(exchange_age<23)y+= (int)((23-exchange_age)*6);
  }
  if(!selected)card_render(x,y,70,79,g->hand[i],0,game_card_cost_now(g,g->hand[i])!=-2&&(game_card_cost_now(g,g->hand[i])<=g->energy||game_card_cost_now(g,g->hand[i])==-1));
 }
 if(g->selection>=start && g->selection<g->hand_count &&
    g->selection<start+visible){
    int idx=g->selection;
    int x=CLAMP(56+(idx-start)*73-21,52,292);
    int id=CARD_ID(g->hand[idx]);
    int active=game_card_cost_now(g,g->hand[idx])!=-2 &&
         (game_card_cost_now(g,g->hand[idx])<=g->energy||game_card_cost_now(g,g->hand[idx])==-1);
    card_render(rift133_preview_x(g),108,112,144,g->hand[idx],1,active);
 }

 if(hand_exchange&&animation_tick>=hand_exchange_tick){
   unsigned age=animation_tick-hand_exchange_tick;
   if(age<15){
      for(int i=0;i<old_hand_count&&i<5;i++){
        int x=56+i*73,y=177+(int)age*7;
        card_render(x,y,70,79,old_hand[i],0,0);
      }
   }else if(age>=23)hand_exchange=0;
 }
 if(start+visible<g->hand_count){
   gilded_panel(392,200,16,27,0);
   text(396,207,">",GOLD,1);
 }
 if(start>0)text(42,209,"<",GOLD,1);
 card_combat_context=0;
 /* Full card rules are printed directly on the selected card.
    No separate floating tooltip covers the player or battlefield. */
 button(416,196,62,35,"END",g->selection>=g->hand_count);
 footer("X PLAY   O END   SQ DECK   START INFO / INVENTORY");
}
static uint32_t node_color(int n){switch(n){case NODE_FIGHT:return RED;case NODE_ELITE:return PURPLE;case NODE_EVENT:return BLUE;case NODE_REST:return GREEN;case NODE_SHOP:return GOLD;case NODE_TREASURE:return RGB(209,160,72);default:return WHITE;}}
static char node_symbol(int n){return "FE?SR$B"[CLAMP(n,0,6)];}
static void room_icon(int n,int x,int y) {
 if(n<0||n>6)return;
 sprite_scaled(x-15,y-15,30,30,rift110_map_icons[n],30,30);
}
static void map_screen(const Game*g){
 header(g,"FATE'S PATH / CHOOSE A CONNECTED NODE");
 border(17,45,300,202,RGB(175,132,73));gilded_panel(319,47,156,200,0);
 int first=CLAMP(g->floor+g->scroll-2,0,MAP_ROWS-5);
 if(g->floor>=MAP_ROWS-4)first=MAP_ROWS-4;
 for(int r=first;r<first+5&&r<MAP_ROWS;r++){
  int y=220-(r-first)*37;
  for(int c=0;c<3;c++){
   int x=79+c*89;
   if(r<MAP_ROWS-1){
     if(!(game_route_row_mask(g,r)&(1u<<c)))continue;
     unsigned routes=game_route_mask(g,r,c);
     if(r+1<MAP_ROWS)routes&=game_route_row_mask(g,r+1);
     for(int d=0;d<3;d++)if(routes&(1u<<d))
       {
         uint32_t ink=(r==g->floor-1&&c==g->lane)?GOLD:RGB(154,117,77);
         line(x-1,y-12,79+d*89-1,y-25,RGB(63,53,49));
         line(x,y-12,79+d*89,y-25,ink);
         line(x+1,y-12,79+d*89+1,y-25,ink);
       }
    }
  }
 }
 for(int r=first;r<first+5&&r<MAP_ROWS;r++){
  int y=220-(r-first)*37;
  for(int c=0;c<3;c++){
   if(!(game_route_row_mask(g,r)&(1u<<c)))continue;
   int x=79+c*89,n=g->map[r][c],done=r<g->floor,
       available=r==g->floor&&game_route_available(g,c);
   uint32_t col=node_color(n);
   circle(x,y,16,done?RGB(45,50,57):available?RGB(180,125,61):RGB(35,47,62));
   circle(x,y,12,done?RGB(73,76,77):(r==g->floor&&!available)?RGB(71,66,64):col);
   room_icon(n,x,y);
   if(r==g->floor+g->scroll&&c==g->selection){
    border(x-18,y-18,36,36,((animation_tick/16u)&1u)?GOLD:RGB(252,226,168));
    if((animation_tick%42u)<22u)put(x+17,y-19,GOLD);
   }
   if(done&&g->map_selected[r]==c)circle(x,y+12,3,GREEN);
   char num[16];snprintf(num,sizeof(num),"%02d",r+1);text(26,y-5,num,MUTED,1);
  }
 }
 if(g->floor>=MAP_ROWS-4){
    const int bossx=168,bossy=220-(MAP_ROWS-first)*37;
    const int lasty=220-(MAP_ROWS-1-first)*37;
    unsigned last=game_route_visible_mask(g,MAP_ROWS-1);
    for(int c=0;c<3;c++)if(last&(1u<<c)){
      line(79+c*89,lasty-17,bossx,bossy+15,RGB(57,46,39));
      line(79+c*89,lasty-18,bossx,bossy+14,GOLD);
    }
    circle(bossx,bossy,19,RGB(96,27,42));
    room_icon(NODE_BOSS,bossx,bossy);
    if(g->floor==MAP_ROWS)border(bossx-21,bossy-21,42,42,GOLD);
    text(26,bossy-4,"BOSS",GOLD,1);
 }
 text(324,63,"THE ASCENT",GOLD,1);
 char b[60];snprintf(b,sizeof(b),"ACT %d / 3",g->act);text(325,85,b,WHITE,1);
 snprintf(b,sizeof(b),"FLOOR %d/%d",g->floor+1,MAP_ROWS+1);text(325,103,b,WHITE,1);
 text(325,130,"NEXT",MUTED,1);
 if(g->floor+g->scroll<MAP_ROWS){
    int node=g->map[g->floor+g->scroll][g->selection];
    text(325,147,node_name(node),node_color(node),1);
 }
 else text(325,147,"THE BOSS",RED,1);
 if(g->floor+g->scroll<MAP_ROWS) {
    int node=g->map[g->floor+g->scroll][g->selection];
    const char *risk=node==NODE_FIGHT?"STANDARD ENCOUNTER":
      node==NODE_ELITE?"DANGEROUS / RELIC":
      node==NODE_EVENT?"UNKNOWN OUTCOME":
      node==NODE_SHOP?"BUY / UPGRADE":
      node==NODE_REST?"HEAL / UPGRADE":
      node==NODE_TREASURE?"RARE TREASURE":"BOSS APPROACH";
    fittext(324,169,risk,21,node_color(node));
 }
 if(g->scroll==0)text(324,193,"X TAKE PATH",GOLD,1);
 text(324,213,"SQ DECK",MUTED,1);
 footer("UP PREVIEW   DOWN RETURN   X ENTER   SQUARE DECK");
}
static void relic_result_screen(const Game*g){
 header(g,"RELIC");
 gilded_panel(39,47,401,193,1);
 int id=g->reward[0];
 center(240,58,g->relic_pending?"A RELIC AWAITS":"RELIC ACQUIRED",GOLD,2);
 circle(240,113,29,RGB(77,61,85));
 triangle(240,85,213,133,267,133,PURPLE);
 triangle(240,89,223,130,257,130,GOLD);
 if(id>=0&&id<RELIC_COUNT){
    center(240,150,relic_defs[id].name,GOLD,1);
    wrapped_text(62,171,relic_defs[id].desc,51,3,WHITE);
 }
 if(g->relic_pending){button(73,210,145,27,"TAKE",g->selection==0);button(263,210,145,27,"SKIP",g->selection==1);}
 footer(g->relic_pending?"LEFT/RIGHT SELECT   X CONFIRM   O SKIP":"X CONTINUE");
}
static void event_result_screen(const Game*g){
 header(g,"EVENT RESULT / SEE EXACT REWARDS");
 center(240,49,"THE ARMORY'S SPOILS",GOLD,2);
 for(int i=0;i<2;i++){
   int x=103+i*148;
   card_render(x,75,128,142,g->reward[i],0,1);
   fittext(x,224,cards[CARD_ID(g->reward[i])].name,17,GOLD);
 }
 center(240,239,"-10 HP FROM THE ARMORY",RED,1);
 footer("X CONTINUE  O CONTINUE");
}
static void reward_screen(const Game*g){
 header(g,"SPOILS OF BATTLE / CHOOSE ONE CARD");
 for(int i=0;i<3;i++)card_render(57+i*132,68,102,152,g->reward[i],g->selection==i,1);
 button(198,225,84,23,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT   X TAKE CARD OR SKIP");
}
static void boss_reward_screen(const Game*g){
 rect(0,28,W,64,BG);
 header(g,"GUARDIAN DEFEATED / SELECT A RELIC");
 center(240,55,"CHOOSE YOUR SPOILS",GOLD,2);
 for(int i=0;i<3;i++){
  int x=30+i*148,y=83,selected=g->selection==i;
  rect(x,y,131,148,RGB(30,38,50));border(x,y,131,148,selected?GOLD:RGB(75,83,92));
  circle(x+65,y+29,22,RGB(65,49,67));
  triangle(x+65,y+10,x+46,y+39,x+65,y+55,PURPLE);
  triangle(x+65,y+10,x+84,y+39,x+65,y+55,GOLD);
  if(g->reward[i]>=0){
   fittext(x+5,y+64,relic_defs[g->reward[i]].name,17,GOLD);
   wrapped_text(x+7,y+85,relic_defs[g->reward[i]].desc,16,5,WHITE);
  }
 }
 rect(0,232,W,23,BG);
 button(195,230,90,23,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT  X CONFIRM  O SKIP");
}
static void rest_screen(const Game*g){
 header(g,"REST SITE / A MOMENT OF PEACE");
 dark_rect(34,44,413,43,110);
 center(240,54,"THE EMBERS REMAIN",GOLD,1);
 button(49,189,179,38,"REST - HEAL 30%",g->selection==0);
 button(254,189,175,38,"FORGE - UPGRADE",g->selection==1);
 footer("LEFT/RIGHT SELECT    X CONFIRM");
}
static const char*event_titles[]={"CURSED PURSE","MOON SPRING","FORGOTTEN TOME","ANCIENT SHRINE","TRAPPED CHEST","LOST TRAVELER","FORGOTTEN ARMORY","LIVING ALTAR","BLACK WATER","CINDER GAMBIT","RUSTED FORGE","LAST SENTINEL"};
static const char*event_choices[]={"75 GOLD / LOSE 8 HP","HEAL 20 / PAY 40 GOLD","GET UPGRADED CARD","GAIN 5 MAX HP","RELIC / LOSE 7 HP","HEAL 8 / GAIN 30 GOLD","GET 2 CARDS / LOSE 10 HP","GAIN 10 MAX HP / LOSE 17 HP","HEAL 28 / ADD INJURY","110 GOLD / LOSE 16 HP","UPGRADE 2 / PAY 12 GOLD","GET RELIC / ADD DREAD"};
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
 "Restore up to 20 HP. Pay up to 40 gold; partial payment allowed.",
 "Receive one random card already upgraded; identity is not known yet.",
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
 sprite_scaled(0,0,480,272,rift112_event,240,136);
 dark_rect(0,28,480,224,165);

 header(g,"UNKNOWN / A CHOICE IN THE DARK");
 center(240,62,event_titles[g->effect_turn],GOLD,2);
 int style=g->effect_turn%4;
 uint32_t event_ink=style==0?GOLD:style==1?BLUE:style==2?PURPLE:RED;
 circle(240,135,27,RGB(36,42,61));
 if(style==0) {
    rect(216,120,48,32,event_ink);
    border(217,120,46,30,WHITE);
    line(216,136,264,136,BG);
 }else if(style==1){
    for(int i=0;i<6;i++)circle(208+i*13,136+(i%2)*5,5,event_ink);
    triangle(240,106,226,145,254,145,WHITE);
 }else if(style==2){
    triangle(240,102,215,154,265,154,event_ink);
    circle(240,132,10,GOLD);
 }else{
    for(int i=0;i<6;i++)line(215+i*9,110,215+i*9,155,event_ink);
    circle(240,131,15,WHITE);
 }
 fittext(42,88,rift_event_lore[g->effect_turn],56,WHITE);
 dark_rect(23,165,434,43,226);
 border(23,165,434,43,RGB(110,90,73));
 wrapped_text(33,171,rift113_event_details[g->effect_turn],58,3,WHITE);
 center(240,210,event_choices[g->effect_turn],GOLD,1);
 button(60,224,170,27,"ACCEPT",g->selection==0);
 button(251,224,170,27,"LEAVE",g->selection==1);
 footer("LEFT/RIGHT CHOOSE    X CONFIRM");
}
static void shop_screen(const Game*g){
 sprite_scaled(0,0,480,272,rift112_shop,240,136);
 dark_rect(0,28,480,224,165);

 header(g,"BLACK MARKET / INSPECT BEFORE YOU BUY");
 int discount=0;
 for(int i=0;i<g->relic_count;i++)if(g->relics[i]==6)discount=15;
 char label[90],caption[140],title[52],cost[24];
 int selected=g->selection,price=0;
 gilded_panel(246,51,222,190,1);
 for(int i=0;i<7;i++){
   int y=57+i*23,chosen=(i==selected);
   rect(12,y,224,21,chosen?RGB(93,69,47):RGB(27,36,49));
   border(12,y,224,21,chosen?GOLD:RGB(53,67,79));
   label[0]=0;
   if(i<3){
     if(g->shop_cards[i]>=0)
       snprintf(label,sizeof(label),"CARD  %s",cards[g->shop_cards[i]].name);
     else snprintf(label,sizeof(label),"CARD  SOLD OUT");
   }else if(i==3)snprintf(label,sizeof(label),"RELIC %s",
        g->shop_relic>=0?relic_defs[g->shop_relic].name:"SOLD OUT");
   else if(i==4)snprintf(label,sizeof(label),"SERVICE REMOVE CARD");
   else if(i==5)snprintf(label,sizeof(label),"POTION RANDOM");
   else if(i==6)snprintf(label,sizeof(label),"LEAVE THE STORE");
   fittext(19,y+5,label,30,chosen?GOLD:WHITE);
 }
 const char *kind="SERVICE";caption[0]=0;title[0]=0;
 if(selected<3){
    kind="CARD";
    int id=g->shop_cards[selected];
    if(id<0){snprintf(title,sizeof(title),"SOLD OUT");snprintf(caption,sizeof(caption),"This card has already been purchased.");}
    else{
      snprintf(title,sizeof(title),"%s",cards[id].name);
      snprintf(caption,sizeof(caption),"%s",cards[id].desc);
      price=50+25*cards[id].rarity-discount;
    }
 }else if(selected==3){
    kind="RELIC";
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
    kind="POTION";
    snprintf(title,sizeof(title),"RANDOM POTION");
    snprintf(caption,sizeof(caption),"Buy one random potion if your belt has an empty slot.");
    price=50;
 }else if(selected==6){
    snprintf(title,sizeof(title),"LEAVE SHOP");
    snprintf(caption,sizeof(caption),"Keep your purchases and continue along the connected route.");
 }else{
    kind="POTION";
    snprintf(title,sizeof(title),"RANDOM POTION");
    snprintf(caption,sizeof(caption),"Receive one random potion if your three-slot belt has space.");
    price=50;
 }
 if(selected<3&&g->shop_cards[selected]>=0){
   card_render(291,60,132,143,g->shop_cards[selected],1,1);
 }else{
   text(257,62,kind,GOLD,1);
   fittext(257,81,title,29,WHITE);
   rect(255,106,203,1,GOLD);
   wrapped_text(257,117,caption,27,7,WHITE);
 }
 if(price){
   snprintf(cost,sizeof(cost),"PRICE %d GOLD",price);
   text(257,207,cost,g->gold>=price?GREEN:RED,1);
 }
 footer("X BUY  O LEAVE  SQ DECK  TRI POTIONS");
}
static void potion_screen(const Game*g){
 header(g,"POTION BELT");
 for(int i=0;i<g->potion_count;i++){
  int y=62+i*58;
  gilded_panel(30,y,420,50,i==g->selection);
  text(43,y+5,game_potion_name(g->potions[i]),GOLD,1);
  wrapped_text(43,y+22,game_potion_description(g->potions[i]),55,2,WHITE);
 }
 if(!g->potion_count)center(240,110,"NO POTIONS",MUTED,1);
 footer("UP/DOWN BROWSE  O RETURN");
}
static void relic_screen(const Game*g){
 header(g,"RELICS / ITEMS DISCOVERED");
 char b[112];
 snprintf(b,sizeof(b),"%d RELICS",g->relic_count);
 text(351,49,b,GOLD,1);
 int first=CLAMP(g->selection-6,0,(g->relic_count>10?g->relic_count-10:0));
 for(int i=first;i<g->relic_count&&i<first+10;i++){
   int id=g->relics[i],y=66+(i-first)*18;
   if(i==g->selection)rect(16,y-2,448,17,RGB(75,63,44));
   snprintf(b,sizeof(b),"%s - %s",relic_defs[id].name,relic_defs[id].desc);
   fittext(24,y,b,62,i==g->selection?GOLD:WHITE);
 }
 footer("UP/DOWN BROWSE   X/CIRCLE RETURN");
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
   rect(x,43,109,21,g->scroll==i?RGB(93,69,47):RGB(35,46,60));
   border(x,43,109,21,g->scroll==i?GOLD:RGB(78,88,100));
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
 gilded_panel(342,70,128,161,0);
 if(count){
   int raw=pile[g->selection],id=CARD_ID(raw);
   fittext(349,83,cards[id].name,16,GOLD);
   wrapped_text(349,111,card_description(raw),16,8,WHITE);
 }else center(402,137,"EMPTY PILE",MUTED,1);
 if(page+6<count)text(348,216,"DOWN: MORE",GOLD,1);
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
 gilded_panel(346,67,126,172,0);
 if(count){
  int c=pending==25?g->discard[g->selection]:g->hand[g->selection],id=CARD_ID(c);
  fittext(350,78,cards[id].name,16,GOLD);
  wrapped_text(350,101,card_description(c),16,6,WHITE);
  if(pending==23){
   text(350,174,"UPGRADE PREVIEW",GOLD,1);
   if(!game_card_is_upgradeable(c))text(350,191,"MAXED / UNABLE",RED,1);
   else wrapped_text(350,190,card_description(c|UPBIT),16,4,GREEN);
  }else text(350,186,pending==25?"TO TOP OF DRAW":"SEND TO EXHAUST",GOLD,1);
 }
 if(page+6<count)text(348,220,"MORE CARDS BELOW",MUTED,1);
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
 gilded_panel(344,55,126,188,0);
 if(g->deck_count>0){
   int raw=g->deck[g->selection],id=CARD_ID(raw);
   fittext(351,66,cards[id].name,16,GOLD);
   text(351,84,"CURRENT",WHITE,1);
   wrapped_text(351,99,card_description(raw),16,
                (g->prior_screen==REST||g->prior_screen==EVENT)?4:8,WHITE);
   if(g->prior_screen==REST||g->prior_screen==EVENT){
     text(351,149,"AFTER UPGRADE",GOLD,1);
     if(!game_card_is_upgradeable(raw))text(351,168,"MAX UPGRADED",RED,1);
     else wrapped_text(351,166,card_description(raw|UPBIT),16,5,GREEN);
   }else{
     snprintf(b,sizeof(b),"CARD %d / %d",g->selection+1,g->deck_count);
     text(351,167,b,GOLD,1);
   }
 }
 if(first+6<g->deck_count)text(351,224,"DOWN: NEXT PAGE",GOLD,1);
 footer(g->prior_screen==REST||g->prior_screen==EVENT?
        "D-PAD CHOOSE  X UPGRADE  O RETURN":"D-PAD BROWSE  O RETURN");
}
static void title_screen(const Game*g){
 (void)g;
 /* High-resolution original title art is converted to a 480x272 RGB565
    texture; the real executable menu is intentionally separate. */
 /* The completely new knight scene leaves quiet sky at right.
    Keep all UI off the player and his oversized sword. */
 text(304,73,"RIFT ASCENT",RGB(34,20,24),2);
 text(302,71,"RIFT ASCENT",GOLD,2);
 button(300,149,164,32,"BEGIN ASCENT",g->selection==0);
 button(300,190,164,30,"HOW TO PLAY",g->selection==1);
 if(profile_data&&profile_data->runs){
  char b[74];snprintf(b,sizeof(b),"RUNS %lu   WINS %lu   BEST FL %lu",
   (unsigned long)profile_data->runs,(unsigned long)profile_data->wins,
   (unsigned long)profile_data->furthest_floor);
  center(378,229,b,GOLD,1);
 }else center(380,229,"ONE WARRIOR / THREE ACTS",GOLD,1);
 footer("UP/DOWN SELECT     X CONFIRM");
}
static void controller_icon(int x,int y,int kind){
 circle(x,y,9,RGB(38,50,68));
 circle(x,y,8,RGB(90,109,128));
 if(kind==0){line(x-4,y-4,x+4,y+4,WHITE);line(x+4,y-4,x-4,y+4,WHITE);}
 if(kind==1)circle(x,y,5,WHITE);
 if(kind==2)triangle(x,y-5,x-5,y+4,x+5,y+4,WHITE);
 if(kind==3){rect(x-5,y-5,10,10,WHITE);rect(x-3,y-3,6,6,RGB(90,109,128));}
}
static void help_screen(const Game*g){
 header(g,"WALK THE SPIRE / CONTROLS");
 text(29,67,"D-PAD  Navigate cards, paths and menus",WHITE,1);
 text(29,87,"CROSS  Select or play a card",WHITE,1);
 text(29,107,"RIGHT  Move to END, CROSS ends turn",WHITE,1);
 text(29,127,"START  Inspect card, enemy, or status",WHITE,1);
 text(29,147,"TRIANGLE  Open potion belt / use any",WHITE,1);
 controller_icon(15,91,0);controller_icon(15,111,1);
 controller_icon(15,131,3);controller_icon(15,151,2);
 text(29,167,"Win fights, grow your deck, defeat 3 bosses.",GOLD,1);
 text(29,187,"Weak -25% dealt; Vulnerable +50% taken.",MUTED,1);
 text(29,215,"CROSS / CIRCLE   RETURN",GOLD,1);
 footer("RIFT ASCENT  /  ONE WARRIOR  /  THREE ACTS");
}
static void end_screen(const Game*g){
 header(g,g->screen==VICTORY?"VICTORY / THE HOLLOW KING FALLS":"DEFEAT / THE DARKNESS CLAIMS YOU");
 center(240,77,g->screen==VICTORY?"ASCENT COMPLETE":"THE END",g->screen==VICTORY?GOLD:RED,2);
 char b[80];snprintf(b,sizeof(b),"FOES VANQUISHED: %d",g->kill_count);center(240,130,b,WHITE,1);
 snprintf(b,sizeof(b),"FINAL DECK: %d CARDS",g->deck_count);center(240,150,b,WHITE,1);
 button(139,190,202,35,"X - NEW RUN",1);
 footer("EVERY RUN IS A NEW STORY");
}
static void inspector(const Game*g){
 if(!overlay_kind)return;
 dark_rect(0,29,W,226,214);
 gilded_panel(20,40,440,203,1);
 const char *tabs[]={"CARD","FOE","STATUS","POTIONS","RELICS"};
 for(int i=0;i<5;i++){
    int px=28+i*87;
    rect(px,48,83,19,overlay_page==i?RGB(115,75,42):RGB(34,46,59));
    center(px+41,52,tabs[i],overlay_page==i?GOLD:WHITE,1);
 }
 char b[112];
 if(overlay_page==0){
   if(g->hand_count>0&&g->selection<g->hand_count){
      int raw=g->hand[g->selection],id=CARD_ID(raw);
      center(240,79,cards[id].name,GOLD,2);
      wrapped_text(43,115,card_description(raw),55,3,WHITE);
      int n=rift132_preview_damage(g,raw);
      if(n>0){
         snprintf(b,sizeof(b),"PREDICTED DAMAGE: %d",n);
         text(43,164,b,RED,1);
      }
      text(43,186,"Strength: +1 damage per hit.",MUTED,1);
      text(43,201,"Weak: -25%, Vulnerable: +50%.",MUTED,1);
   }else text(43,99,"END TURN selected. Press X to confirm.",WHITE,1);
 }else if(overlay_page==1){
   if(g->enemy_count){
      const Enemy *e=&g->enemies[g->target];
      center(240,79,e->name,GOLD,2);
      int hits=game_enemy_intent_hits(g,g->target);
      int damage=game_enemy_intent_damage(g,g->target);
      if(hits){
        if(hits>1)snprintf(b,sizeof(b),"%d HITS x %d DAMAGE (BEFORE BLOCK)",hits,damage/hits);
        else snprintf(b,sizeof(b),"%d INCOMING DAMAGE (BEFORE BLOCK)",damage);
        center(240,109,b,RED,1);
      }
      wrapped_text(43,hits?129:109,game_intent_description(e->intent),55,6,WHITE);
      snprintf(b,sizeof(b),"FOE: STR %d | WEAK %d | VUL %d | PSN %d",
               e->strength,e->weak,e->vulnerable,e->poison);
      text(43,207,b,GOLD,1);
   }
 }else if(overlay_page==2){
   snprintf(b,sizeof(b),"HP %d/%d   BLOCK %d   ENERGY %d",g->hp,g->maxhp,g->block,g->energy);
   text(39,78,b,GOLD,1);
   int count=game_status_count(g),first=CLAMP(overlay_status_scroll,0,(count>7?count-7:0));
   for(int i=0;i<7&&first+i<count;i++){
    game_status_line(g,first+i,b,sizeof(b));
    rift117_small_text(39,99+i*17,b,WHITE,6,9);
   }
   if(!count)text(39,111,"No active powers or status effects.",MUTED,1);
   if(count>7){snprintf(b,sizeof(b),"%d-%d / %d",first+1,MIN(first+7,count),count);text(343,224,b,GOLD,1);}
 }else if(overlay_page==3) {
    for(int i=0;i<g->potion_count;i++){
       int y=82+i*44;int selected=(i==overlay_potion);
       if(selected)rect(33,y-3,410,39,RGB(66,66,50));
       snprintf(b,sizeof(b),"%d. %s",i+1,game_potion_name(g->potions[i]));
       text(42,y,b,selected?GOLD:WHITE,1);
       wrapped_text(52,y+14,game_potion_description(g->potions[i]),50,2,MUTED);
    }
    if(g->potion_count==0)center(240,113,"NO POTIONS IN YOUR BELT",MUTED,1);
 }else {
    for(int i=0;i<g->relic_count&&i<7;i++){
       int id=g->relics[i],y=80+i*20;
       snprintf(b,sizeof(b),"%s: %s",relic_defs[id].name,relic_defs[id].desc);
       fittext(38,y,b,56,i==g->selection?GOLD:WHITE);
    }
    if(g->relic_count>7)text(38,224,"FULL LIST: MAP -> START",MUTED,1);
 }
 footer(overlay_page==2?"LEFT/RIGHT TABS  UP/DOWN SCROLL  START CLOSE":overlay_page==3?"LEFT/RIGHT TABS  UP/DOWN POTION  X USE   START CLOSE":
                          "LEFT/RIGHT TABS  UP/DOWN TARGET  START/CIRCLE CLOSE");
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
 if(g->screen==REWARD&&victory_replay&&tick>=victory_tick &&
    tick-victory_tick<30){
   static Game shot;
   shot=victory_scene;
   shot.screen=COMBAT;
   for(int i=0;i<shot.enemy_count;i++)if(shot.enemies[i].hp<=0)shot.enemies[i].hp=1;
   unsigned age=tick-victory_tick;
   game_render_frame(&shot,pixels,stride,victory_tick+age/3);
   if(age>17){
      pix=pixels;sw=stride;dark_rect(0,28,W,227,(int)(age-17)*16);
   }
   return;
 }
 pix=pixels;sw=stride;background(g);
 ambient_particles(g);
 switch(g->screen){
 case TITLE:title_screen(g);break;
 case MAP:map_screen(g);break;
 case COMBAT:combat(g);break;
 case EVENT_RESULT:event_result_screen(g);break;
 case RELIC_RESULT:relic_result_screen(g);break;
 case REWARD:victory_replay=0;reward_screen(g);break;
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
   if(tick>=rift120_play_tick && tick-rift120_play_tick<24){
     unsigned age=tick-rift120_play_tick;
     int shown=age>=12&&rift120_auto_card>=0?rift120_auto_card:
       rift120_played_card;
     if(shown<0)shown=rift120_auto_card;
     if(shown>=0&&shown<CARD_COUNT*2){
       int x=rift133_play_x,y=108;
       card_combat_context=0;
       card_combat_context=g;
       card_render(x,y,112,144,shown,1,1);
       card_combat_context=0;
       if(age>=17)dark_rect(x,y,112,144,(int)(age-16)*29);
     }
   }else if(tick>=rift120_play_tick&&tick-rift120_play_tick>=24){
      rift120_played_card=-1;rift120_auto_card=-1;
   }
   inspector(g);
 }
 if(g->screen==MAP && fx_transition && animation_tick>=fx_transition_tick &&
    animation_tick-fx_transition_tick<23 && g->floor==0 && g->act>1){
    dark_rect(83,87,317,72,177);
    char act_label[32];snprintf(act_label,sizeof(act_label),"ACT %d BEGINS",g->act);
    center(240,108,act_label,GOLD,2);
 }
}

/* Existing callers get a deterministic still frame. PSP uses animated frames. */
void game_render(const Game*g,uint32_t*pixels,int stride){
 game_render_frame(g,pixels,stride,0);
}
