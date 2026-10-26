#!/usr/bin/env python3
"""Rift Ascent 1.3.0: new image-backed locations, gothic HUD trim and card frames."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c"
s=p.read_text()
def one(a,b,why):
 global s
 n=s.count(a)
 if n!=1:raise RuntimeError(f"v130 {why}: expected 1 anchor, got {n}")
 s=s.replace(a,b,1)
one('#include "rift120_scenes.h"',
    '#include "rift120_scenes.h"\n#include "rift130_slots.h"',
    "art slots")
i=s.index('static void background(const Game*g){')
j=s.index('static unsigned particle_hash',i)
s=s[:i]+'''/* Every final scene is a full AI-generated painting. 11 fixed-size
   linker slots are filled in the signed build packaging step. */
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
'''+s[j:]
# A Gothic hand dais. Draw before cards, no text is hidden underneath.
one('rect(0,175,W,80,RGB(10,17,29));rect(0,175,W,1,GOLD);',
'''rect(0,175,W,80,RGB(11,14,24));
 rect(0,175,W,2,RGB(168,124,74));
 rect(0,252,W,2,RGB(168,124,74));
 for(int i=0;i<4;i++){
  line(56+i*97,176,74+i*97,184,RGB(75,61,57));
  line(74+i*97,184,56+i*97,191,RGB(75,61,57));
 }
 rect(54,184,357,67,RGB(16,20,32));
 border(54,184,357,67,RGB(107,79,64));''',
 'gothic hand plinth')
one('circle(26,218,24,RGB(19,38,67));circle(26,218,19,g->energy<=0?RGB(77,82,87):',
'''circle(26,218,25,RGB(126,95,57));
 circle(26,218,23,RGB(19,38,67));
 circle(26,218,19,g->energy<=0?RGB(77,82,87):''',
 'energy setting')
one('text(5,241,"ENERGY",MUTED,1);',
'''text(5,241,"ENERGY",GOLD,1);
 for(int i=0;i<4;i++){
   int angle=i*90;
   if(i==0||i==2)line(26,191+i*12,26,195+i*12,WHITE);
   else put(3+(i==1?45:0),218,GOLD);
 }''',
 'energy runes')
# The card frames have differentiated colored insets, corner gilding, and no
# arbitrary object-like shapes mistaken for actual card illustrations.
s=s.replace('rift118_card_panel(x,y,w,h,selected);',
'''rift118_card_panel(x,y,w,h,selected);
 {
   uint32_t etched=d->type==0?RGB(176,83,68):
                   d->type==1?RGB(81,151,196):
                   d->type==2?RGB(163,101,195):RGB(139,122,100);
   line(x+6,y+2,x+w-7,y+2,etched);
   line(x+5,y+h-4,x+w-6,y+h-4,etched);
   put(x+4,y+4,GOLD);put(x+w-5,y+4,GOLD);
   put(x+4,y+h-5,GOLD);put(x+w-5,y+h-5,GOLD);
 }''')
assert s.count('uint32_t etched')==2

# Use full AI-painted textures for combat chrome, not placeholder polygons.
one('#include "rift130_slots.h"',
    '#include "rift130_slots.h"\n#include "rift130_ui.h"','UI art')
one('rect(0,175,W,80,RGB(11,14,24));',
    'art_blit(0,175,W,80,rift130_ui_hand,480,80);','painted hand plinth')
i=s.index(' circle(26,218,25,RGB(126,95,57));')
j=s.index(' int visible=5,start=0;',i)
s=s[:i]+''' sprite_scaled(0,193,52,52,rift130_ui_energy,52,52);
 snprintf(b,sizeof(b),"%d",g->energy);
 center(26,207,b,WHITE,2);
 text(5,241,"ENERGY",GOLD,1);
'''+s[j:]
i=s.index('static void rift120_hp_meter(')
j=s.index('static const uint16_t *rift130_fight(',i)
t=s[i:j]
needle=' if(shield>0){'
assert needle in t
t=t.replace(needle,''' sprite_scaled(x-5,y-5,w+24,33,rift130_ui_hp,110,36);
 if(shield>0){''',1)
s=s[:i]+t+s[j:]
needle='if(selected){rift118_round_outline(x-1,y-1,w+2,h+2,GOLD,5);}'
assert s.count(needle)==2
s=s.replace(needle,'''sprite_scaled(x,y,w,h,d->type==0?rift130_ui_attack:
   d->type==1?rift130_ui_skill:rift130_ui_power,70,91);
 if(selected){rift118_round_outline(x-1,y-1,w+2,h+2,GOLD,5);}''')

p.write_text(s)
print("PASS: 11 act-specific AI-scene slots, gothic hand and energy UI, typed card filigree")
