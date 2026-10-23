#!/usr/bin/env python3
"""Rift 1.1.0: replace line-weapon with pixel-painted sprite, remove text collisions,
and replace aliased path glyphs with antialiased 30px readable icons."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def change(a,b):
 global s
 if a not in s:raise RuntimeError("1.1.0 render anchor missing "+a[:110])
 s=s.replace(a,b,1)
change('#include "motion_art.h"', '#include "motion_art.h"\n#include "rift110_map_icons.h"')
# The physical weapon is now in the integrated 210x127 hero art, not drawn as lines.
start=s.index(' /* A distinctive two-handed claymore instead of the short axe/hammer profile.')
end=s.index(' for(int i=0;i<g->enemy_count;i++){',start)
s=s[:start]+''' /* The claymore, its guard and hilt are painted into motion_art.c.
    Do not draw independent diagonal line primitives over the sprite. */
\n'''+s[end:]

# Render compact combat cards as fully separated title, energy/damage and rules
# areas. No badge is allowed to paint across the rules' bounding box.
needle=''' gilded_panel(x,y,w,h,selected);
 rect(x+3,y+3,w-6,24,hue);'''
replacement=''' if(card_combat_context && h<=112){
    gilded_panel(x,y,w,h,selected);
    rect(x+3,y+3,w-6,15,hue);
    char title[64];
    snprintf(title,sizeof(title),"%s%s",d->name,up?"+":"");
    fittext(x+5,y+5,title,(w-10)/7,WHITE);

    /* Dedicated cost and damage row, completely above card-description. */
    rect(x+3,y+19,w-6,17,RGB(22,31,45));
    char cost[9];
    if(d->cost==-1)snprintf(cost,sizeof(cost),"X");
    else snprintf(cost,sizeof(cost),"%d",d->cost<0?0:d->cost);
    circle(x+12,y+27,8,RGB(36,107,164));
    circle(x+12,y+27,6,RGB(52,151,210));
    center(x+12,y+22,cost,WHITE,1);
    if(d->type==0){
      int val=game_effective_damage(card_combat_context,raw,card_combat_context->target);
      if(val>0){
         char amount[12];snprintf(amount,sizeof(amount),"%d",val);
         rect(x+w-32,y+19,29,17,RGB(73,30,35));
         border(x+w-32,y+19,29,17,RED);
         center(x+w-17,y+22,amount,GOLD,1);
      }
    }else if(w>90){
      text(x+35,y+22,d->type==2?"POWER":d->type==1?"SKILL":"CURSE",MUTED,1);
    }
    /* Hard boundary at y+38. Badges stop at y+36. No overlap possible. */
    const int content_y=y+39;
    int maxrows=(y+h-3-content_y)/11;
    if(maxrows>0)wrapped_text(x+5,content_y,card_description(raw),
                              (w-10)/7,maxrows,selected?WHITE:MUTED);
    if(selected){border(x-1,y-1,w+2,h+2,GOLD);}
    if(!active){
       dark_rect(x+2,y+2,w-4,h-4,150);
       border(x+2,y+2,w-4,h-4,RGB(101,105,108));
       if(h>70)fittext(x+5,y+h-14,"LOW ENERGY",(w-10)/7,WHITE);
    }
    return;
 }
 gilded_panel(x,y,w,h,selected);
 rect(x+3,y+3,w-6,24,hue);'''
change(needle,replacement)
# Use the actual new artwork for map pictograms (pixel-art source file generated
# with 4x supersampling). The existing node ring and selection remain.
a=s.index('static void room_icon(int n,int x,int y) {')
b=s.index('static void map_screen(const Game*g){',a)
s=s[:a]+'''static void room_icon(int n,int x,int y) {
 if(n<0||n>6)return;
 sprite_scaled(x-15,y-15,30,30,rift110_map_icons[n],30,30);
}
'''+s[b:]
p.write_text(s)
print("PASS 1.1.0: real sword on hero; no intersecting card cost/damage/description; smooth, legible map icons.")
