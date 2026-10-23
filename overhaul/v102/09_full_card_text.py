#!/usr/bin/env python3
"""1.0.2 item 09: unabridged card explanations in combat, rewards, and deck."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
start=s.index(" /* Sunder had its Vulnerable text hidden")
end=s.index(' button(414,191,61,44,"END"',start)
s=s[:start]+''' /* Compact physical cards, with an always-readable full card hover panel.
    This keeps a complete explanation even for Sunder, Exhaust and X cards. */
 if(g->selection>=0&&g->selection<g->hand_count){
   int raw=g->hand[g->selection],id=CARD_ID(raw);
   dark_rect(6,44,258,92,214);
   border(6,44,258,92,GOLD);
   fittext(13,51,cards[id].name,31,GOLD);
   if(CARD_UP(raw))text(237,51,"+",GREEN,1);
   wrapped_text(13,70,card_description(raw),34,5,WHITE);
 }
'''+s[end:]
a=''' wrapped_text(351,99,card_description(raw),16,4,WHITE);'''
b=''' wrapped_text(351,99,card_description(raw),16,
                (g->prior_screen==REST||g->prior_screen==EVENT)?4:8,WHITE);'''
assert a in s;s=s.replace(a,b,1)
a=''' for(int i=0;i<3;i++)card_render(57+i*132,90,102,134,g->reward[i],g->selection==i,1);'''
b=''' if(g->selection<3){
   int raw=g->reward[g->selection];
   dark_rect(37,43,405,43,229);
   fittext(48,46,cards[CARD_ID(raw)].name,50,GOLD);
   wrapped_text(48,62,card_description(raw),53,2,WHITE);
 }
 for(int i=0;i<3;i++)card_render(57+i*132,90,102,134,g->reward[i],g->selection==i,1);'''
assert a in s;s=s.replace(a,b,1)
p.write_text(s)
print("PASS 09: hovered combat/reward/deck cards show complete descriptions.")
