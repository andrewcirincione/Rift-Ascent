from pathlib import Path
p=Path('GAME/src/render.c');s=Path('qa/v133/source/render.c').read_text();q=Path('qa/v134')
s=s.replace('#include "rift133_art.h"','#include "rift133_art.h"\n#include "rift134_art.h"\n#include "rift134_fonts.h"')
# Animation records the real slot, then rises to the screen center.
s=s.replace('rift133_play_x=CLAMP(56+(g->selection-first)*73-21,35,300);','rift133_play_x=56+(g->selection-first)*71;')
s=s.replace('tick-rift120_play_tick>26','tick-rift120_play_tick>40')
a=s.index('static void gilded_panel(');b=s.index('static void put(int x,int y,uint32_t c){',a)
s=s[:a]+(q/'ui_helpers.inc').read_text()+'\nstatic void gilded_panel(int x,int y,int w,int h,int selected){r134_panel(x,y,w,h,selected);}\n'+s[b:]
a=s.index('static void text(int x');b=s.index('static void fittext(',a)
s=s[:a]+'''static void text(int x,int y,const char*s,uint32_t ink,int size){r134_text(x,y,s,ink,size==2?&r134_f18b:&r134_f10);}
static void center(int x,int y,const char*s,uint32_t ink,int size){r134_center(x,y,s,ink,size==2?&r134_f18b:&r134_f10);}
'''+s[b:]
a=s.index('static void rift117_small_text(');b=s.index('/* Returns the number',a)
s=s[:a]+'''static void rift117_small_text(int x,int y,const char*s,uint32_t ink,int cw,int ch){(void)ch;r134_text(x,y,s,ink,cw>=6?&r134_f9:&r134_f8);}
'''+s[b:]
a=s.index('static void rift131_shield(');b=s.index('static const uint16_t *rift130_fight',a)
s=s[:a]+'''static void rift131_shield(int x,int y){r134_blit(x,y,20,22,r134_map_icons[7],36,36,255);}
static void rift120_hp_meter(int x,int y,int w,int hp,int maxhp,int shield){
 char value[24];int inset=w*22/100,inner=w-2*inset;
 rect(x+inset,y+5,inner,14,RGB(35,17,22));
 if(maxhp>0)rect(x+inset,y+14,inner*CLAMP(hp,0,maxhp)/maxhp,4,RGB(195,36,46));
 r134_blit(x,y,w,24,r134_health_frame,112,24,255);
 snprintf(value,sizeof(value),"%d/%d",hp,maxhp);r134_center(x+w/2,y+5,value,WHITE,w<100?&r134_f8:&r134_f9b);
 if(shield>0){rift131_shield(x+w+3,y+2);snprintf(value,sizeof(value),"%d",shield);r134_center(x+w+13,y+7,value,WHITE,&r134_f9b);}
}
'''+s[b:]
a=s.index('static void header(');b=s.index('static int footer_button',a)
s=s[:a]+'''static void header(const Game*g,const char*caption){
 r134_blit(0,0,480,17,r134_header,480,17,255);
 char b[96];r134_text(8,3,"RIFT ASCENT",GOLD,&r134_f9b);
 snprintf(b,sizeof(b),"HP %d/%d",g->hp,g->maxhp);r134_text(120,3,b,WHITE,&r134_f9);
 snprintf(b,sizeof(b),"%d GOLD",g->gold);r134_text(237,3,b,GOLD,&r134_f9);
 snprintf(b,sizeof(b),"ACT %d   FLOOR %02d",g->act,g->floor+1);r134_text(353,3,b,WHITE,&r134_f9);
 dark_rect(0,17,480,13,180);r134_text(12,19,caption,MUTED,&r134_f9);
 if(g->screen==HELP||g->screen==DECK_VIEW||g->screen==CARD_CHOICE||g->screen==RELIC_VIEW||g->screen==POTION_VIEW||g->screen==REWARD||g->screen==EVENT_RESULT||g->screen==VICTORY||g->screen==DEFEAT)gilded_panel(10,39,460,213,1);
}
'''+s[b:]
a=s.index('static void footer(const char*help)');b=s.index('static void enemy_art',a)
s=s[:a]+'''static void footer(const char*help){r134_blit(0,258,480,14,r134_footer,480,14,255);const RiftFont*f=r134_width(help,&r134_f9)>462?&r134_f8:&r134_f9;r134_text(9,261,help,WHITE,f);}
'''+s[b:]
a=s.index('static void card_render(');b=s.index('static void rift_status_chip',a);s=s[:a]+(q/'card.inc').read_text()+'\n'+s[b:]
# Existing intent semantics, new generated icon art and compact layout.
s=s.replace('const uint16_t *glyph[3];','const uint32_t *glyph[3];')
s=s.replace('rift116_icon_swords','r134_map_icons[0]').replace('rift116_icon_shield','r134_map_icons[7]').replace('rift116_icon_up','r134_utility_icons[4]').replace('rift116_icon_down','r134_utility_icons[5]')
s=s.replace('widths[j]=24+(int)strlen(value[j])*7+3;','widths[j]=17+r134_width(value[j],&r134_f9)+3;')
a=s.index('   /* Transparent illustrated icons;');b=s.index('   x+=widths[j];',a)
s=s[:a]+'''   int dim=glyph[j]==r134_map_icons[0]||glyph[j]==r134_map_icons[7]?36:40;
   r134_blit(x,57,17,17,glyph[j],dim,dim,255);
   if(value[j][0])r134_text(x+18,61,value[j],j==0&&hits?GOLD:WHITE,&r134_f9);
'''+s[b:]
a=s.index('static void combat(const Game*g){');b=s.index('static uint32_t node_color',a);s=s[:a]+(q/'combat.inc').read_text()+'\n'+s[b:]
s=s.replace('sprite_scaled(x-15,y-15,30,30,rift110_map_icons[n],30,30);','r134_blit(x-17,y-17,34,34,r134_map_icons[n],36,36,255);')
s=s.replace('border(17,45,300,202,RGB(175,132,73));gilded_panel(319,47,156,200,0);','gilded_panel(15,41,299,209,0);gilded_panel(315,41,160,209,0);')
a=s.index('   uint32_t col=node_color(n);');b=s.index('   char num[16];',a)
s=s[:a]+'''   if(r==g->floor+g->scroll&&c==g->selection)r134_blit(x-22,y+1,44,13,r134_target,88,25,255);
   room_icon(n,x,y);
   if(done&&g->map_selected[r]!=c)dark_rect(x-17,y-17,34,34,125);
'''+s[b:]
s=s.replace('    circle(bossx,bossy,19,RGB(96,27,42));','').replace('if(g->floor==MAP_ROWS)border(bossx-21,bossy-21,42,42,GOLD);','if(g->floor==MAP_ROWS)r134_blit(bossx-24,bossy+6,48,14,r134_target,88,25,255);')
a=s.index(' circle(240,113,29,');b=s.index(' if(id>=0&&id<RELIC_COUNT)',a);s=s[:a]+' r134_blit(211,86,58,58,r134_utility_icons[1],40,40,255);\n'+s[b:]
s=s.replace('text(304,73,"RIFT ASCENT",RGB(34,20,24),2);\n text(302,71,"RIFT ASCENT",GOLD,2);','r134_blit(250,28,222,102,r134_logo,222,102,255);')
s=s.replace('button(300,149,164,32','button(280,145,184,35').replace('button(300,190,164,30','button(280,190,184,32')
# Restore travel, scale-up, hold and fade. Card center reaches (240,136).
a=s.index('   if(tick>=rift120_play_tick && tick-rift120_play_tick<24)');b=s.index('   inspector(g);',a)
s=s[:a]+'''   if(r134_playing()){
    int age=(int)(tick-rift120_play_tick),t=CLAMP(age,0,18),ease=t*(36-t);
    int shown=age>=18&&rift120_auto_card>=0?rift120_auto_card:rift120_played_card;
    if(shown<0)shown=rift120_auto_card;
    if(shown>=0){
     int x=rift133_play_x+(184-rift133_play_x)*ease/324;
     int y=174+(68-174)*ease/324,w=70+42*ease/324,h=84+52*ease/324;
     card_combat_context=g;card_render(x,y,w,h,shown,1,1);card_combat_context=0;
     if(age>=26)dark_rect(x,y,w,h,(age-25)*19);
    }
   }else if(tick>=rift120_play_tick&&tick-rift120_play_tick>=38){rift120_played_card=-1;rift120_auto_card=-1;}
'''+s[b:]
# Menus use generated panels/selection furniture, never new ornament primitives.
s=s.replace('rect(px,48,83,19,overlay_page==i?RGB(115,75,42):RGB(34,46,59));\n    center(px+41,52,tabs[i],overlay_page==i?GOLD:WHITE,1);','button(px,47,83,23,tabs[i],overlay_page==i);')
s=s.replace('if(selected)rect(33,y-3,410,39,RGB(66,66,50));','if(selected)gilded_panel(30,y-5,420,42,1);')
s=s.replace('if(i==g->selection)rect(16,y-2,448,17,RGB(75,63,44));','if(i==g->selection)button(17,y-5,446,22,"",1);')
s=s.replace('rect(12,y,224,21,chosen?RGB(93,69,47):RGB(27,36,49));\n   border(12,y,224,21,chosen?GOLD:RGB(53,67,79));','button(16,y,220,22,"",chosen);')
s=s.replace('fittext(19,y+5,label,30,chosen?GOLD:WHITE);','r134_text(31,y+6,label,chosen?GOLD:WHITE,&r134_f9);')
s=s.replace('rect(255,106,203,1,GOLD);','r134_blit(400,72,36,36,r134_utility_icons[selected==3?1:selected==5?0:2],40,40,255);')
s=s.replace('   rect(x,43,109,21,g->scroll==i?RGB(93,69,47):RGB(35,46,60));\n   border(x,43,109,21,g->scroll==i?GOLD:RGB(78,88,100));','   button(x,43,109,23,"",g->scroll==i);')
p.write_text(s)
print('Applied 1.3.4 renderer layout, typography, artwork and centered card flight')
