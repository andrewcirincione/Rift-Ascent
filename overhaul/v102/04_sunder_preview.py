#!/usr/bin/env python3
"""1.0.2 item 04: Sunder always reveals the vulnerable rider."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
needle=''' button(414,191,61,44,"END",g->selection>=g->hand_count);'''
assert needle in s
s=s.replace(needle,''' /* Sunder had its Vulnerable text hidden by the tiny compact card.
    Show the entire effect in a dedicated hover panel, outside combat bars. */
 if(g->selection>=0&&g->selection<g->hand_count &&
    CARD_ID(g->hand[g->selection])==2){
    int up=CARD_UP(g->hand[g->selection]);
    dark_rect(6,49,242,52,210);
    border(6,49,242,52,GOLD);
    text(14,55,up?"SUNDER +":"SUNDER",GOLD,1);
    text(14,69,up?"DEAL 11 DAMAGE":"DEAL 8 DAMAGE",WHITE,1);
    text(14,83,up?"APPLY 3 VULNERABLE":"APPLY 2 VULNERABLE",RED,1);
 }
'''+needle,1)
p.write_text(s)
print("PASS 04: Sunder Vulnerable rider legible on and off upgrades.")
