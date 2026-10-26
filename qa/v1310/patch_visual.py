from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'render.c';s=p.read_text()
def change(old,new):
 global s
 assert old in s,old[:100]
 s=s.replace(old,new)
change('#include "rift139_art.h"','#include "rift139_art.h"\n#include "rift1310_art.h"')
change('static int overlay_kind,overlay_page,overlay_potion,overlay_status_scroll;', 'static char r1310_potion_notice[72];static unsigned r1310_notice_tick;\nstatic int overlay_kind,overlay_page,overlay_potion,overlay_status_scroll;')
change('r137_feedback(before,after,action,tick);','''r137_feedback(before,after,action,tick);
 if(action==BTN_POTION&&before->screen==COMBAT&&after->potion_count<before->potion_count){
  snprintf(r1310_potion_notice,sizeof(r1310_potion_notice),"%s",after->message);r1310_notice_tick=tick;
 }else if(before->screen!=COMBAT)r1310_potion_notice[0]=0;''')
change(''' /* Bilinear reconstruction and a central crop; never paint over the frame. */
 const uint16_t*src=rift120_cards[id];''',''' /* Setup Strike has its own high-resolution illustration. */
 if(id==80){
  const int iw=240,ih=135;int crop=CLAMP(iw*h/w,35,ih),top=(ih-crop)/2;
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
   int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;
   int sx=xx*iw/w,sy=top+yy*crop/h;pix[dy*sw+dx]=r1310_setup[sy*iw+sx]|0xff000000u;
  }return;
 }
 /* Bilinear reconstruction and a central crop; never paint over the frame. */
 const uint16_t*src=rift120_cards[id];''')
change('static void rift131_shield(int x,int y){', '''static void r1310_block_number(int cx,int y,const char*value){
 int x=cx-r134_width(value,&r134_f9b)/2;
 /* Opaque dark stroke separates every digit from the cyan shield. */
 for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(dx||dy)r134_text(x+dx,y+dy,value,RGB(3,8,14),&r134_f9b);
 r134_text(x,y,value,RGB(255,255,244),&r134_f9b);
}
static void rift131_shield(int x,int y){''')
change('r134_center(x+w+13,y+7,value,WHITE,&r134_f9b);','r1310_block_number(x+w+13,y+7,value);')
change('const int x=32,y=28;','const int x=32,y=36;')
change('r134_center(x+153,y+10,value,WHITE,&r134_f9b);','r1310_block_number(x+153,y+10,value);')
change('r134_blit(x,57,17,17,glyph[j],dim,dim,255);','r134_blit(x,61,17,17,glyph[j],dim,dim,255);')
change('r135_shadow_text(x+18,60,value[j],WHITE,&r134_f9b);','r135_shadow_text(x+18,64,value[j],WHITE,&r134_f9b);')
start=s.index(' int lx=9;for(int i=2;i>=0;i--)if(rift120_log[i][0]&&lx<450){',s.index('static void combat'))
end=s.index(' r138_player_meter(g);',start)
s=s[:start]+''' /* Both living enemies get a complete, fixed row; no scrolling or turn log. */
 rect(0,17,480,19,RGB(11,12,16));int row=0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){
  char action[112],line[160];game_enemy_intent_text(g,i,action,sizeof(action));
  snprintf(line,sizeof(line),"%s: %s",g->enemies[i].name,action);
  r134_text(8,18+row*9,line,i==g->target?WHITE:MUTED,&r134_f8);row++;
 }
''' +s[end:]
change('rift120_hp_meter(ex-w/2,32,w,','rift120_hp_meter(ex-w/2,36,w,')
change('if(start>0)r134_text(40,184,"<",GOLD,&r134_f10);','''if(start>0){
  for(int yy=0;yy<22;yy++)for(int xx=0;xx<28;xx++){uint32_t c=r1310_arrow[yy*28+27-xx];int a=c>>24;if(a)pix[(179+yy)*sw+23+xx]=r134_mix(pix[(179+yy)*sw+23+xx],c,a);}
 }''')
change('if(start+5<g->hand_count)r134_text(418,184,">",GOLD,&r134_f10);','if(start+5<g->hand_count)r134_blit(419,178,28,22,r1310_arrow,28,22,255);')
change('if(g->selection>=g->hand_count){r134_blit(411,213,66,31,r139_end,66,31,255);r134_center(444,224,"END",RGB(40,13,12),&r134_f9b);}else button(416,215,62,29,"END",0);','''int end_selected=g->selection>=g->hand_count;
 r134_blit(410,end_selected?213:215,68,29,r1310_end,68,29,end_selected?255:145);
 r135_shadow_text(444-r134_width("END",&r134_f9b)/2,end_selected?224:226,"END",end_selected?RGB(255,245,222):MUTED,&r134_f9b);
 if(end_selected)r134_blit(431,200,28,13,r138_choice,42,28,255);
 if(r1310_potion_notice[0]&&animation_tick>=r1310_notice_tick&&animation_tick-r1310_notice_tick<360){
  int width=r134_width(r1310_potion_notice,&r134_f9)+14;dark_rect(25,65,width,15,225);r135_shadow_text(32,68,r1310_potion_notice,WHITE,&r134_f9);
 }''')
# Show one acquired upgraded card in its event; retain the existing two-card event.
change('''header(g,"THE ARMORY'S SPOILS");
 
 for(int i=0;i<2;i++){''','''header(g,g->effect_turn==2?"THE ANCIENT LESSON / CARD RECEIVED":"THE ARMORY'S SPOILS");
 if(g->effect_turn==2){
  card_render(159,47,162,178,g->reward[0],1,1);
  r134_center(240,235,"Added to your deck",GOLD,&r134_f9);
  footer("X CONTINUE  O CONTINUE");return;
 }
 for(int i=0;i<2;i++){''')
change('"Receive one random card already upgraded; identity is not known yet."','"Receive one random upgraded card."')
change('''button(198,229,84,23,"SKIP",g->selection==3);''','''if(g->ic[37]>0&&g->ic[37]<=6){char notice[90];snprintf(notice,sizeof(notice),"POTION GAINED: %s",game_potion_name(g->ic[37]-1));r134_center(240,219,notice,GREEN,&r134_f9b);}
 button(198,234,84,20,"SKIP",g->selection==3);''')
change('''footer("X BUY  O LEAVE  SQ DECK  TRI POTIONS");''','''if(!strncmp(g->message,"Potion gained:",14))r134_text(34,241,g->message,GREEN,&r134_f9b);
 footer("X BUY  O LEAVE  SQ DECK  TRI POTIONS");''')
p.write_text(s)
p=q/'combat_fx.inc';s=p.read_text()
# Preserve the character's stance while reducing all generated/idle/death frames ~6%.
change('pw=160,ph=91','pw=150,ph=85')
change('r137_blit(36,51,160,120,r138_death','r137_blit(41,58,150,113,r138_death')
change('r137_blit(47+dx,51+dy,173,120,r139_attack','r137_blit(51+dx,58+dy,163,113,r139_attack')
change('r137_blit(49+dx,51+dy,160,120,r137_hero_frames','r137_blit(53+dx,58+dy,150,113,r137_hero_frames')
change('r137_actor(36+dx,81+dy+wave','r137_actor(41+dx,87+dy+wave')
# Crop empty margins during sampling so tall and broad models use their allotted region.
change(''' if(w<1||h<1)return;
 for(int yy=0;yy<h;yy++){''',''' if(w<1||h<1)return;
 int left=0,top=0,cw=iw,ch=ih;
 for(int i=0;i<36;i++)if(src==rift_enemy_sprites[i]){left=r1310_enemy_bounds[i][0];top=r1310_enemy_bounds[i][1];cw=r1310_enemy_bounds[i][2];ch=r1310_enemy_bounds[i][3];break;}
 for(int yy=0;yy<h;yy++){''')
change('''int sx=xx*iw/w;if(flip)sx=iw-1-sx;
   uint16_t c=src[(yy*ih/h)*iw+sx];''','''int sx=xx*cw/w;if(flip)sx=cw-1-sx;
   uint16_t c=src[(top+yy*ch/h)*iw+left+sx];''')
change('''int ex=r134_enemy_x(g,e),w=g->enemy_count==1?112:94,h=88,y=81+wave(animation_tick/4+e*24u,96,1),tilt=0,flash=0,opacity=255;''','''int art=foe->art,maxw=g->enemy_count==1?144:112,h=92;
  int w=art==9?144:r1310_enemy_bounds[art][2]*h/r1310_enemy_bounds[art][3];
  if(w>maxw){h=h*maxw/w;w=maxw;}
  int ex=r134_enemy_x(g,e),y=174-h+wave(animation_tick/4+e*24u,96,1),tilt=0,flash=0,opacity=255;''')
change('int art=foe->art,flip=(art==1','int flip=(art==5||art==24||art==1')
change('||art==23','')
change('r139_hound,132,104','r1310_hound,144,96')
p.write_text(s)
