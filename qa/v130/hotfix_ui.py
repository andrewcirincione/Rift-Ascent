#!/usr/bin/env python3
"""Hotfix visual regressions introduced by 1.3.0 texture overlays.
Keep environments and card paintings, but remove UI textures that cover live text.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def one(a,b,tag):
 global s
 n=s.count(a)
 if n!=1:raise RuntimeError(f"{tag}: expected 1, saw {n}")
 s=s.replace(a,b,1)
# Stop stretching 70x91 decorative card sprites across 104x132 and
# overlaying descriptions, names, and artwork.
overlay='''sprite_scaled(x,y,w,h,d->type==0?rift130_ui_attack:
   d->type==1?rift130_ui_skill:rift130_ui_power,70,91);
 '''
assert s.count(overlay)==2,s.count(overlay)
s=s.replace(overlay,'')
# Stop covering HP after values have been drawn.
one(' sprite_scaled(x-5,y-5,w+24,33,rift130_ui_hp,110,36);','', 'HP overlay')
# Hand texture must fill the complete reserved band, no partial art behind cards.
one('art_blit(0,175,W,80,rift130_ui_hand,480,80);',
'''rect(0,175,W,80,RGB(11,14,24));
 rect(0,175,W,2,RGB(161,117,75));
 rect(0,252,W,2,RGB(161,117,75));''','hand plate')
# Replace nonfunctional three-orb energy texture with one centered gauge.
one(''' sprite_scaled(0,193,52,52,rift130_ui_energy,52,52);
 snprintf(b,sizeof(b),"%d",g->energy);
 center(26,207,b,WHITE,2);
 text(5,241,"ENERGY",GOLD,1);''',
''' circle(26,215,21,RGB(150,109,54));
 circle(26,215,18,RGB(14,31,57));
 circle(26,215,16,g->energy>0?RGB(29,104,161):RGB(55,62,75));
 snprintf(b,sizeof(b),"%d",g->energy);
 center(26,204,b,WHITE,2);
 text(5,241,"ENERGY",GOLD,1);''','single centered energy')
# Blue/gold shield symbol rather than a triangular placeholder.
one('''   triangle(bx+3,y+8,bx+10,y+8,bx+6,y+16,BLUE);''',
'''   line(bx+3,y+7,bx+9,y+7,WHITE);
   line(bx+3,y+7,bx+2,y+13,WHITE);
   line(bx+9,y+7,bx+10,y+13,WHITE);
   line(bx+2,y+13,bx+6,y+17,BLUE);
   line(bx+10,y+13,bx+6,y+17,BLUE);''','shield emblem')
# Card animation currently rises once as preplay, then again after resolution.
# Keep only the preplay interval while still showing short automatic cards.
one('''     int rise=age<9?(int)age*2:18;''',
'''     int rise=age<9?(int)age*2:18;''','animation anchor')
one('''     if(shown>=0&&shown<CARD_COUNT*2){''',
'''     if(shown>=0&&shown<CARD_COUNT*2 && (age<12 || rift120_auto_card>=0)){''','suppress duplicate manual animation')
# Eliminate debug-looking abbreviation while retaining potion access hint.
s=s.replace('"POTS %d"','"POTIONS %d"')
p.write_text(s)
print("PASS: 1.3 UI hotfix avoids art covering card description/HP, restores clean hand and centered energy")
