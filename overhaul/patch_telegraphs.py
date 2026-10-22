#!/usr/bin/env python3
"""Make enemy action telegraphs and route risk readable without opening inspector."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
a='  snprintf(b,sizeof(b),"%s %d",intent_name(e->intent),e->intent_value);'
b='''  switch(e->intent){
    case 1:snprintf(b,sizeof(b),"BLOCK +%d",e->intent_value);break;
    case 4:snprintf(b,sizeof(b),"+2 STR +8 BLK");break;
    case 5:snprintf(b,sizeof(b),"HEX %d+BURN",e->intent_value/2);break;
    case 6:snprintf(b,sizeof(b),"VENOM %d+PSN",e->intent_value/2);break;
    case 7:snprintf(b,sizeof(b),"2 x %d HIT",e->intent_value/2+e->strength);break;
    case 8:snprintf(b,sizeof(b),"DRAIN %d-EN",e->intent_value/2);break;
    case 9:snprintf(b,sizeof(b),"HEAL + BLK");break;
    case 10:snprintf(b,sizeof(b),"CRUSH %d",e->intent_value+e->intent_value/4);break;
    case 11:snprintf(b,sizeof(b),"DREAD +CURSE");break;
    case 12:snprintf(b,sizeof(b),"LEECH %d",e->intent_value/2);break;
    case 13:snprintf(b,sizeof(b),"FORTIFY +STR");break;
    default:snprintf(b,sizeof(b),"%s %d",intent_name(e->intent),e->intent_value);break;
  }'''
if a not in s:raise RuntimeError("intent text anchor not found")
s=s.replace(a,b,1)
a=''' text(324,177,"X  ENTER",GOLD,1);text(324,196,"SQUARE DECK",MUTED,1);'''
b=''' if(g->floor<MAP_ROWS) {
    int node=g->map[g->floor][g->selection];
    const char *risk=node==NODE_FIGHT?"STANDARD ENCOUNTER":
      node==NODE_ELITE?"DANGEROUS / RELIC":
      node==NODE_EVENT?"UNKNOWN OUTCOME":
      node==NODE_SHOP?"BUY / UPGRADE":
      node==NODE_REST?"HEAL / UPGRADE":
      node==NODE_TREASURE?"RARE TREASURE":"BOSS APPROACH";
    fittext(324,169,risk,21,node_color(node));
 }
 text(324,193,"X TAKE PATH",GOLD,1);
 text(324,213,"SQ DECK",MUTED,1);'''
if a not in s:raise RuntimeError("map sidebar anchor not found")
s=s.replace(a,b,1)
p.write_text(s)
print("Added semantic intent telegraphs and path risk summaries.")
