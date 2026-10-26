from pathlib import Path
p=Path(__file__).parent/'source/render.c';s=p.read_text().replace('#include "rift137_art.h"','#include "rift137_art.h"\n#include "rift138_art.h"')
s=s.replace('l.title_rows=r134_lines(title,inner,&r134_f9b,lines);','l.title_rows=r134_lines(title,inner,&r138_title9,lines);')
s=s.replace('const RiftFont *titlefont=small?&r134_f8:&r134_f9b;','const RiftFont *titlefont=small?&r138_title8:&r138_title9;')
s=s.replace(' r134_card_art(x+inset,arty,inner,arth,id);\n if(!active)dark_rect(x+inset,arty,inner,arth,96);', ''' int art_inset=small?6:11,art_width=w-2*art_inset;
 /* The requested fine picture outline sits beneath the existing sculpture. */
 rect(x+art_inset-1,arty-1,art_width+2,arth+2,RGB(132,104,67));
 r134_card_art(x+art_inset,arty,art_width,arth,id);
 if(!active)dark_rect(x+art_inset,arty,art_width,arth,96);''')
s=s.replace('if(g->screen==TITLE||g->screen==HELP||g->screen==VICTORY||g->screen==DEFEAT)\n   src=ra_title_bg;', 'if(g->screen==TITLE)src=r138_menu;\n else if(g->screen==HELP||g->screen==VICTORY||g->screen==DEFEAT)src=ra_title_bg;')
a=s.index('static void title_screen(');b=s.index('static void help_screen',a)
s=s[:a]+'''static void title_screen(const Game*g){
 r134_blit(269,37,197,91,r134_logo,222,102,255);
 for(int i=0;i<2;i++){
  int y=146+i*42,selected=g->selection==i;
  r134_blit(279,y,176,32,selected?r138_button_on:r138_button_off,176,selected?32:30,255);
  r134_center(367,y+12,i?"HOW TO PLAY":"BEGIN ASCENT",selected?RGB(255,234,184):WHITE,&r138_title9);
 }
 if(profile_data&&profile_data->runs){
  char b[60];snprintf(b,sizeof(b),"RUNS %lu    WINS %lu",(unsigned long)profile_data->runs,(unsigned long)profile_data->wins);r134_center(367,230,b,GOLD,&r134_f8);
 }else r134_center(367,230,"ONE WARRIOR / THREE ACTS",GOLD,&r134_f8);
 footer("UP/DOWN SELECT     X CONFIRM");
}

''' +s[b:]
a=s.index('static void reward_screen(');b=s.index('static void boss_reward_screen',a)
s=s[:a]+'''static void reward_screen(const Game*g){
 header(g,"SPOILS OF BATTLE / CHOOSE ONE CARD");
 for(int i=0;i<3;i++){
  int chosen=g->selection==i,x=33+i*144,y=chosen?49:56;
  card_render(x,y,126,166,g->reward[i],chosen,1);
  if(chosen)r134_blit(x+42,32,42,28,r138_choice,42,28,255);
  else dark_rect(x+6,y+8,114,150,38);
 }
 button(198,229,84,23,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT   X TAKE CARD OR SKIP");
}
''' +s[b:]
a=s.index('static const uint16_t *rift130_fight');s=s[:a]+'''static void r138_player_meter(const Game*g){
 const int x=32,y=28;char value[24];
 rect(x+36,y+13,89,9,RGB(40,17,23));
 if(g->maxhp>0)rect(x+36,y+13,89*CLAMP(g->hp,0,g->maxhp)/g->maxhp,9,RGB(160,30,43));
 r134_blit(x,y,140,30,r138_player_hp,140,30,255);
 snprintf(value,sizeof(value),"%d/%d",g->hp,g->maxhp);r135_shadow_text(x+82-r134_width(value,&r134_f9b)/2,y+13,value,WHITE,&r134_f9b);
 if(g->block>0){rift131_shield(x+143,y+5);snprintf(value,sizeof(value),"%d",g->block);r134_center(x+153,y+10,value,WHITE,&r134_f9b);}
}
''' +s[a:]
s=s.replace('rift120_hp_meter(52,32,112,g->hp,g->maxhp,g->block);','r138_player_meter(g);')
s=s.replace('"CURRENT EFFECTS / ACTIVE POWERS"','"STATUS EFFECTS"')
s=s.replace('r134_text(39,111+i*15,b,WHITE,&r134_f9);','r134_text(39,111+i*15,b,!strcmp(b,"POWERS")?GOLD:WHITE,!strcmp(b,"POWERS")?&r134_f9b:&r134_f9);')
p.write_text(s)
