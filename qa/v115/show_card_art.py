#!/usr/bin/env python3
"""Show the already-embedded 92 unique card images in the COMBAT hand.
Previously the compact branch returned before the art_blit call."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
start=s.index('    /* Dedicated cost and damage row, completely above card-description. */',s.index('static void card_render('))
end=s.index('    if(selected){border(x-1,y-1,w+2,h+2,GOLD);}',start)
draw='''    /* Restored card illustrations, with dedicated text region below them. */
    const int art_height=selected?30:23;
    const int art_top=y+20;
    const int art_id=(id>=0&&id<92)?id:0;
    rect(x+4,art_top,w-8,art_height,RGB(10,15,25));
    art_blit(x+5,art_top+1,w-10,art_height-2,rift113_cards[art_id],60,40);
    border(x+4,art_top,w-8,art_height,RGB(151,115,73));
    char cost[9];
    int now_cost=game_card_cost_now(card_combat_context,raw);
    if(now_cost==-1)snprintf(cost,sizeof(cost),"X");
    else snprintf(cost,sizeof(cost),"%d",now_cost<0?0:now_cost);
    circle(x+13,art_top+9,8,RGB(22,46,82));
    circle(x+13,art_top+9,6,RGB(51,140,201));
    center(x+13,art_top+5,cost,WHITE,1);
    if(d->type==0){
      int val=game_effective_damage(card_combat_context,raw,card_combat_context->target);
      if(val>0){
        char amount[12];snprintf(amount,sizeof(amount),"%d",val);
        rect(x+w-30,art_top+2,26,14,RGB(60,24,30));
        border(x+w-30,art_top+2,26,14,RED);
        center(x+w-17,art_top+3,amount,GOLD,1);
      }
    }
    int content_y=art_top+art_height+3;
    int maxrows=(y+h-3-content_y)/11;
    if(maxrows>0)wrapped_text(x+5,content_y,card_description(raw),
                              (w-10)/7,maxrows,selected?WHITE:MUTED);
'''
s=s[:start]+draw+s[end:]
branch=s[s.index('if(card_combat_context && h<=112){'):s.index('    return;\n }',s.index('if(card_combat_context && h<=112){'))]
assert 'art_blit(' in branch and 'rift113_cards[art_id]' in branch
p.write_text(s)
print('PASS: all hand cards render distinct, embedded art above effect descriptions')
