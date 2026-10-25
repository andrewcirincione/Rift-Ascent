#!/usr/bin/env python3
"""Rift Ascent 1.2.0 renderer/UI overhaul, applied after all v1.1.x patches."""
from pathlib import Path
import re,sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def replace(old,new,name):
 global s
 count=s.count(old)
 if count!=1:raise RuntimeError(f"v120 {name}: expected one anchor, got {count}")
 s=s.replace(old,new,1)
replace('#include "rift113_cards.h"',
 '#include "rift113_cards.h"\n#include "rift120_cards.h"\n#include "rift120_scenes.h"',
 'new art headers')
replace('static unsigned animation_tick;',
 '''static unsigned animation_tick;
static char rift120_log[3][64];static int rift120_log_count=0;
static int rift120_played_card=-1;static unsigned rift120_play_tick=0;
static void rift120_log_add(const char *s){
 if(!s||!*s)return;
 for(int i=0;i<2;i++)memcpy(rift120_log[i],rift120_log[i+1],64);
 snprintf(rift120_log[2],sizeof(rift120_log[2]),"%s",s);
 if(rift120_log_count<3)rift120_log_count++;
}''',
 'battle running event log')
replace('void game_render_feedback(const Game *before,const Game *after,int action,unsigned tick){',
 '''void game_render_feedback(const Game *before,const Game *after,int action,unsigned tick){
 if(before->screen==COMBAT && action==BTN_OK &&
    before->selection>=0&&before->selection<before->hand_count){
   int raw=before->hand[before->selection];
   rift120_played_card=raw;rift120_play_tick=tick;
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"PLAY %s",cards[CARD_ID(raw)].name);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT&&after->hp<before->hp){
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"FOE HIT: %d HP LOST",before->hp-after->hp);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT&&after->block>before->block){
   char linebuf[64];
   snprintf(linebuf,sizeof(linebuf),"SHIELD: +%d BLOCK",after->block-before->block);
   rift120_log_add(linebuf);
 }
 if(before->screen==COMBAT&&strcmp(before->message,after->message)!=0)
   rift120_log_add(after->message);
 if(before->screen!=COMBAT&&after->screen==COMBAT){
   rift120_log_count=0;memset(rift120_log,0,sizeof(rift120_log));
   rift120_log_add("A NEW FIGHT BEGINS");
 }''',
 'log and play animation hook')
# Replace obsolete scene / baked early-design art; overlays still dynamic.
start=s.index('static void background(const Game*g){')
end=s.index('static unsigned particle_hash',start)
s=s[:start]+'''static void background(const Game*g){
 const uint16_t *src=ra_map_bg;
 if(g->screen==TITLE||g->screen==HELP||g->screen==VICTORY||g->screen==DEFEAT)src=ra_title_bg;
 else if(g->screen==COMBAT)src=g->battle_type==NODE_BOSS?rift120_boss_bg:
     g->battle_type==NODE_ELITE?rift120_elite_bg:rift120_fight_bg;
 else if(g->screen==REST||(g->screen==DECK_VIEW&&g->prior_screen==REST))
   src=rift120_camp_bg;
 else if(g->screen==REWARD||g->screen==BOSS_REWARD||g->screen==RELIC_RESULT)
   src=g->battle_type==NODE_BOSS?rift120_boss_bg:rift120_fight_bg;
 else if((g->screen==DECK_VIEW||g->screen==CARD_CHOICE||g->screen==PAUSE_MENU)
         &&(g->prior_screen==COMBAT||g->screen==CARD_CHOICE))
   src=g->battle_type==NODE_BOSS?rift120_boss_bg:
     g->battle_type==NODE_ELITE?rift120_elite_bg:rift120_fight_bg;
 art_blit(0,0,W,H,src,W,H);
}
'''+s[end:]
s=s.replace('rift113_cards[art_id]','rift120_art(art_id,x,y)')
s=s.replace('rift113_cards[id]','rift120_art(id,x,y)')
replace('rect(x+3,y+3,w-6,15,hue);',
 '''rect(x+3,y+3,w-6,15,hue);
    rect(x+5,y+4,w-10,1,d->type==0?GOLD:d->type==1?BLUE:PURPLE);
    rect(x+5,y+17,w-10,1,d->type==0?RED:d->type==1?BLUE:PURPLE);''',
 'card trim small')
replace('rect(x+3,y+3,w-6,24,hue);',
 '''rect(x+3,y+3,w-6,24,hue);
 rect(x+5,y+4,w-10,1,d->type==0?GOLD:d->type==1?BLUE:PURPLE);
 rect(x+5,y+26,w-10,1,d->type==0?RED:d->type==1?BLUE:PURPLE);
 put(x+4,y+4,GOLD);put(x+w-5,y+4,GOLD);''',
 'card trim large')
# Completely consistent HP + shield metric for player and opponents.
anchor='static void background(const Game*g){'
loc=s.index(anchor)
s=s[:loc]+'''static void rift120_hp_meter(int x,int y,int w,int hp,int maxhp,int shield){
 char b[32];
 dark_rect(x-6,y-4,w+45,35,154);
 snprintf(b,sizeof(b),"%d/%d HP",hp,maxhp);fittext(x,y,b,15,WHITE);
 bar(x,y+13,w,8,hp,maxhp,RED);
 if(shield>0){
   border(x-1,y+12,w+2,10,BLUE);
   rect(x+w+5,y+12,13,11,RGB(25,65,102));
   border(x+w+5,y+12,13,11,BLUE);
   triangle(x+w+7,y+14,x+w+16,y+14,x+w+12,y+21,BLUE);
   snprintf(b,sizeof(b),"%d",shield);
   fittext(x+w+20,y+12,b,3,WHITE);
 }
}
'''+s[loc:]
replace('header(g,"BATTLE");',
 '''header(g,"");
 {
   char logline[160]="";
   for(int i=0;i<3;i++)if(rift120_log[i][0]){
     if(logline[0])strncat(logline,"  /  ",sizeof(logline)-strlen(logline)-1);
     strncat(logline,rift120_log[i],sizeof(logline)-strlen(logline)-1);
   }
   int chars=(int)strlen(logline);
   fittext(14,31,chars>61?logline+chars-61:logline,61,WHITE);
 }''',
 'battle title replaced by rolling action history')
old=''' dark_rect(37,143,122,25,107);
 snprintf(b,sizeof(b),"%d/%d HP",g->hp,g->maxhp);
 text(54,144,b,WHITE,1);
 bar(53,157,99,7,g->hp,g->maxhp,RED);
 if(g->block){snprintf(b,sizeof(b),"BLK %d",g->block);text(160,150,b,BLUE,1);}
 if(g->strength){snprintf(b,sizeof(b),"STR %d",g->strength);text(162,160,b,GOLD,1);}'''
replace(old,''' rift120_hp_meter(45,142,86,g->hp,g->maxhp,g->block);''',
        'unify player HP and move block beside health')
old='''  dark_rect(ex-42,142,84,28,144);
  snprintf(b,sizeof(b),"%d/%d",e->hp,e->maxhp);
  center(ex,142,b,WHITE,1);
  bar(ex-39,155,78,7,e->hp,e->maxhp,RED);
  if(e->block){
    /* Shield marker, separated from numeric HP and the health bar. */
    uint32_t shield=RGB(61,145,220);
    rect(ex-40,164,13,10,RGB(19,44,73));
    border(ex-40,164,13,10,shield);
    triangle(ex-38,165,ex-29,165,ex-33,172,shield);
    snprintf(b,sizeof(b),"%d BLOCK",e->block);
    fittext(ex-22,164,b,9,WHITE);
  }'''
replace(old,'''  rift120_hp_meter(ex-43,142,86,e->hp,e->maxhp,e->block);''',
        'unify enemy HP and move block beside health')
# Remove selected reward duplicate header/details and the dated reward background.
start=s.index('static void reward_screen(const Game*g){')
end=s.index('static void boss_reward_screen(',start)
s=s[:start]+'''static void reward_screen(const Game*g){
 header(g,"SPOILS OF BATTLE / CHOOSE ONE CARD");
 for(int i=0;i<3;i++)card_render(57+i*132,68,102,152,g->reward[i],g->selection==i,1);
 button(198,225,84,23,"SKIP",g->selection==3);
 footer("LEFT/RIGHT SELECT   X TAKE CARD OR SKIP");
}
'''+s[end:]
# Show shop card full, including its energy symbol, in lieu of detached wall of text.
replace(''' text(257,62,kind,GOLD,1);
 fittext(257,81,title,29,WHITE);
 rect(255,106,203,1,GOLD);
 wrapped_text(257,117,caption,27,7,WHITE);''',
 ''' if(selected<3&&g->shop_cards[selected]>=0){
   card_render(291,60,132,143,g->shop_cards[selected],1,1);
 }else{
   text(257,62,kind,GOLD,1);
   fittext(257,81,title,29,WHITE);
   rect(255,106,203,1,GOLD);
   wrapped_text(257,117,caption,27,7,WHITE);
 }''',
 'shop full-card preview')
replace('if(first+6<g->deck_count)text(351,224,"MORE BELOW v",GOLD,1);',
        'if(first+6<g->deck_count)text(351,224,"DOWN: NEXT PAGE",GOLD,1);',
        'upgrade pagination only when additional pages exist')
# No fake battlefield replay with Spoils title appearing beneath a second screen.
start=s.index(' case REWARD:\n   if(victory_replay')
end=s.index(' case BOSS_REWARD:',start)
s=s[:start]+''' case REWARD:victory_replay=0;reward_screen(g);break;
'''+s[end:]
# Convergent boss map row, fully visible as floor is approached, not floating to side.
replace(''' int first=CLAMP(g->floor+g->scroll-2,0,MAP_ROWS-5);
 if(g->floor==MAP_ROWS)first=MAP_ROWS-5;''',
 ''' int first=CLAMP(g->floor+g->scroll-2,0,MAP_ROWS-5);
 if(g->floor>=MAP_ROWS-4)first=MAP_ROWS-4;''',
 'map show boss as final row')
start=s.index(' if(g->floor>=MAP_ROWS-5){\n    int last_y=')
end=s.index(' text(324,63,"THE ASCENT"',start)
s=s[:start]+''' if(g->floor>=MAP_ROWS-4){
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
'''+s[end:]
# Campfire image is now the generated scene; don't draw old triangle pseudo-art on top.
replace(''' circle(230,122,37,RGB(71,49,39));triangle(199,155,232,63,264,155,RGB(197,91,51));
 triangle(215,154,235,90,253,154,GOLD);
 circle(229,138,12,RGB(251,187,85));''',''' dark_rect(34,44,413,43,110);''','campfire scene')
# Card play animation continues for a few frames. Automatic cards also appear
# in the rolling battle text via the game-generated messages.
replace(' if(g->screen==COMBAT)inspector(g);',
 ''' if(g->screen==COMBAT){
   if(rift120_played_card>=0 && tick>=rift120_play_tick &&
      tick-rift120_play_tick<14){
     card_render(192,49,96,116,rift120_played_card,1,1);
     border(190,47,100,120,GOLD);
   }else if(tick>=rift120_play_tick&&tick-rift120_play_tick>=14)
      rift120_played_card=-1;
   inspector(g);
 }''',
 'show played card before visual effects settle')
p.write_text(s)
print("PASS: v1.2.0 combat HUD, histories, play animation, map, shop, rewards, backgrounds and cards")
