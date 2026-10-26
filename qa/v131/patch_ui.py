#!/usr/bin/env python3
"""Correct v1.3.0 card geometry, opaque chrome, HP, energy, and preplay motion."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def one(a,b,label):
 global s
 n=s.count(a)
 if n!=1:raise RuntimeError(f"v131 {label}: {n} anchors")
 s=s.replace(a,b,1)

# A texture drawn AFTER text hid card artwork, costs and descriptions.
opaque='''sprite_scaled(x,y,w,h,d->type==0?rift130_ui_attack:
   d->type==1?rift130_ui_skill:rift130_ui_power,70,91);
 '''
assert s.count(opaque)==2
s=s.replace(opaque,"/* Gothic frame uses existing unobstructed inset. */\n ")

one('art_blit(x+5,art_top+1,w-10,art_height-2,rift120_cards[art_id],60,40);',
'''{
  int aw=MIN(w-10,(art_height-2)*3/2);
  art_blit(x+(w-aw)/2,art_top+1,aw,art_height-2,rift120_cards[art_id],60,40);
}''',"narrow card painting aspect")
one(' art_blit(x+4,y+28,w-8,arth,rift120_cards[id],60,40);',
''' {
 int aw=MIN(w-8,arth*3/2);
 rect(x+4,y+28,w-8,arth,RGB(12,16,27));
 art_blit(x+(w-aw)/2,y+28,aw,arth,rift120_cards[id],60,40);
 }''',"selected art aspect")
one('int x=CLAMP(56+(idx-start)*73-18,54,292);',
    'int x=CLAMP(56+(idx-start)*73-21,52,292);',"hand x")
one('card_render(x,164,120,91,g->hand[idx],1,active);',
    'card_render(x,118,110,130,g->hand[idx],1,active);',"selected hand proportions")

a=s.index('static void rift120_hp_meter(')
b=s.index('static const uint16_t *rift130_fight(',a)
s=s[:a]+'''/* One HP capsule shared by player and enemies, not two stacked textures. */
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
'''+s[b:]

one(''' art_blit(0,175,W,80,rift130_ui_hand,480,80);
 rect(0,175,W,2,RGB(168,124,74));
 rect(0,252,W,2,RGB(168,124,74));
 for(int i=0;i<4;i++){
  line(56+i*97,176,74+i*97,184,RGB(75,61,57));
  line(74+i*97,184,56+i*97,191,RGB(75,61,57));
 }
 rect(54,184,357,67,RGB(16,20,32));
 border(54,184,357,67,RGB(107,79,64));
 sprite_scaled(0,193,52,52,rift130_ui_energy,52,52);
 snprintf(b,sizeof(b),"%d",g->energy);
 center(26,207,b,WHITE,2);
 text(5,241,"ENERGY",GOLD,1);''',
''' rect(0,175,W,80,RGB(13,17,27));
 rect(0,175,W,2,RGB(154,112,66));
 rect(0,250,W,2,RGB(154,112,66));
 rect(53,185,356,63,RGB(21,26,37));
 border(53,185,356,63,RGB(104,80,60));
 rect(55,187,352,1,RGB(62,62,66));
 circle(26,218,23,RGB(107,76,56));
 circle(26,218,20,RGB(18,41,74));
 circle(26,218,16,g->energy?RGB(31,97,159):RGB(46,54,70));
 snprintf(b,sizeof(b),"%d",g->energy);
 center(26,207,b,WHITE,2);
 text(5,242,"ENERGY",GOLD,1);''',"hand and meaningful centered energy")
one(''' if(g->potion_count>0){
    snprintf(b,sizeof(b),"POTS %d",g->potion_count);
    text(414,241,b,GREEN,1);
 }''',
''' for(int k=0;k<MAX_POTIONS;k++){
   int xx=416+k*19,yy=241;
   uint32_t flask=k<g->potion_count?RGB(118,195,174):RGB(61,68,77);
   rect(xx+5,yy-5,5,3,RGB(155,143,118));
   rect(xx+3,yy-2,9,9,RGB(29,45,57));
   rect(xx+5,yy,5,5,flask);
   border(xx+3,yy-2,9,9,k<g->potion_count?GOLD:RGB(77,83,90));
 }''',"replace POTS debugging text with actual belt slots")
one('e->art==2||e->art==6||e->art==7||e->art==11||',
    'e->art==2||e->art==7||e->art==11||',"Stone Giant orientation")
one('rift120_played_card=raw;rift120_play_tick=tick;',
'''if(rift120_played_card!=raw || tick<rift120_play_tick ||
  tick-rift120_play_tick>26){
   rift120_played_card=raw;rift120_play_tick=tick;
 }''',"stop manually played card rising twice")
one('''rift120_auto_card=after->ic[46]-1;
    rift120_play_tick=tick;''',
'''rift120_auto_card=after->ic[46]-1;
    if(rift120_played_card<0)rift120_play_tick=tick;''',
"autoplay keeps original animation clock")
one('''       int x=188,y=59-rise;
       card_combat_context=0;
       card_render(x,y,104,132,shown,1,1);
       border(x-2,y-2,108,136,age<12?GOLD:BLUE);
       if(age>=17)dark_rect(x,y,104,132,(int)(age-16)*29);''',
'''       int x=185,y=54-rise;
       card_combat_context=0;
       card_render(x,y,110,130,shown,1,1);
       border(x-2,y-2,114,134,age<12?GOLD:BLUE);
       if(age>=17)dark_rect(x,y,110,130,(int)(age-16)*29);''',
 "single properly proportioned played card")
helper='''static const char *rift131_clean_rules(const char *s){
 if(!s)return "";
 if(s[0]=='C'&&s[1]=='o'&&s[2]=='s'&&s[3]=='t'&&s[4]==' '&&
    s[5]>='0'&&s[5]<='9'&&s[6]=='.'&&s[7]==' ')
   return s+8;
 return s;
}
'''
s=s.replace('static void card_render(',helper+'static void card_render(',1)
one('const char *description=card_description(raw);',
    'const char *description=rift131_clean_rules(card_description(raw));',
    "remove redundant cost from compact card")
one(' const char *rules=card_description(raw);',
    ' const char *rules=rift131_clean_rules(card_description(raw));',
    "remove redundant cost from enlarged card")
p.write_text(s)
print("PASS v1.3.1: card layout, actual shield, clean HP, energy, hand and fixed play motion")
