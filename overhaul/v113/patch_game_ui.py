#!/usr/bin/env python3
"""Rift Ascent 1.1.3: fix visible gameplay/UI regressions after v1.1.2."""
from pathlib import Path
import sys
p=Path(sys.argv[1]); g=p/"game.c"; h=p/"game.h"; r=p/"render.c"
s=g.read_text(); hs=h.read_text(); t=r.read_text()
def patch(source, old, new, name):
    if old not in source: raise RuntimeError("1.1.3 missing anchor: "+name+" "+old[:65])
    return source.replace(old,new,1)
# A visible node must be reachable from one of the three actual starting lanes.
# This is a visibility fix, not a save layout or route generation change.
s=patch(s,'int game_route_available(const Game *g,int lane) {',r"""unsigned game_route_row_mask(const Game *g,int row) {
 if(!g||row<0||row>=MAP_ROWS)return 0;
 unsigned live=(1u<<MAP_LANES)-1u;
 for(int r=0;r<row;r++){
  unsigned following=0;
  for(int c=0;c<MAP_LANES;c++)
   if(live&(1u<<c))following|=game_route_mask(g,r,c);
  live=following;
 }
 return live;
}
int game_route_available(const Game *g,int lane) {""",'map reachability')
hs=patch(hs,'unsigned game_route_mask(const Game *g,int row,int lane);',
'unsigned game_route_mask(const Game *g,int row,int lane);\nunsigned game_route_row_mask(const Game *g,int row);','map header')
# Preserve current 1.1.2 mechanic: random action, but tell player exactly what was lost.
s=patch(s,'g->hand_count--;exhaust_card(g,c);}break;','g->hand_count--;exhaust_card(g,c);\n    {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}}break;','cinder exhaust')
s=patch(s,'g->hand_count--;exhaust_card(g,c);}\n }break;','g->hand_count--;exhaust_card(g,c);\n   {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}}\n }break;','true grit exhaust')
s=patch(s,'g->hand_count--;exhaust_card(g,c);break;\n      }\n     }\n    }\n    deal(g,enemy,4+2*up+added);',
'''g->hand_count--;exhaust_card(g,c);
       {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}
       break;
      }
     }
    }
    deal(g,enemy,4+2*up+added);''','thrash exhaust')
# Correct descriptions for actions that select cards automatically.
s=s.replace('"Thrash","Deal 4 twice. Exhaust random Attack; gain its damage."', '"Thrash","Deal 4 twice. Random Attack exhausts; gain damage."')
s=s.replace('"Cinder","Deal 18. Exhaust a random card in hand."', '"Cinder","Deal 18. Exhaust a RANDOM card in hand."')
s=s.replace('"True Grit","Gain 7 Block. Exhaust a random card."', '"True Grit","Gain 7 Block. Exhaust RANDOM card."')
# Card effects that exhaust all cards already spell that out.
# Eliminate leftover implicit opaque status prose in the location info.
t=patch(t,'footer("X PLAY     O END TURN     SQ DECK     START INSPECT");',
        'footer("X PLAY   O END   SQ DECK   START INFO / INVENTORY");','combat footer')
# Make enemy intentions a single constrained 92-pixel label and never a
# yellow "VUL 2" pseudo-status (real stacks live in the target-side chips).
t=patch(t,'case 3:effect="VULN 2";break;case 4:effect="STR +2 BLK +8";break;',
        'case 3:effect="APPLIES VUL";break;case 4:effect="STR+2 BLK+8";break;','intent text')
t=patch(t,'case 10:effect="ADDS DAZE";break;case 11:effect="DREAD + BLOCK";break;',
        'case 10:effect="ADDS DAZE";break;case 11:effect="DREAD+BLK";break;','intent width')
t=patch(t,'if(effect[0])fittext(ex-42,69,effect,12,GOLD);',
        '''if(effect[0]){
      dark_rect(ex-46,67,92,17,231);
      fittext(ex-42,70,effect,12,GOLD);
    }''','intent black box')
# Enemy HP is exclusively at y=142, shield BLOCK is on its own line
# below the bar; prior "HP/HP BLK" rewrite obscured HP.
t=patch(t,'''if(e->block){
    int bw=CLAMP(12+e->block*2,12,77);
    rect(ex+39-bw,155,bw,7,RGB(52,111,187));
    snprintf(b,sizeof(b),"%d/%d BLK%d",e->hp,e->maxhp,e->block);
    fittext(ex-39,143,b,11,WHITE);
  }''','''if(e->block){
    /* Shield marker, separated from numeric HP and the health bar. */
    uint32_t shield=RGB(61,145,220);
    rect(ex-40,164,13,10,RGB(19,44,73));
    border(ex-40,164,13,10,shield);
    triangle(ex-38,165,ex-29,165,ex-33,172,shield);
    snprintf(b,sizeof(b),"%d BLOCK",e->block);
    fittext(ex-22,164,b,9,WHITE);
  }''','block display')
# The underlying battlefield artwork has static labels. Mask its HUD text
# before drawing game-state-only status/intent data.
t=patch(t,'dark_rect(283,29,129,46,237);','dark_rect(270,29,145,56,245);','legacy HUD cover')
# Display the last random effect/card in a compact notification.
t=patch(t,'header(g,"BATTLE");',
'''header(g,"BATTLE");
 if(!memcmp(g->message,"Random exhaust:",15)) {
   dark_rect(20,46,236,18,222);
   fittext(27,50,g->message,32,GOLD);
 }''','random effect toast')
t=patch(t,'header(g,"VICTORY / CHOOSE A CARD TO ADD");','header(g,"SPOILS OF BATTLE / CHOOSE A CARD");','reward header')
# The reward title was underneath a second dynamically drawn title panel.
t=patch(t,'center(240,49,"SPOILS OF BATTLE",GOLD,2);','/* Selected card preview occupies this region; only one reward heading. */','reward duplicate')
# Render only viable map nodes/edges, preventing stranded ghost nodes.
t=patch(t,'unsigned routes=game_route_mask(g,r,c);',
'''if(!(game_route_row_mask(g,r)&(1u<<c)))continue;
     unsigned routes=game_route_mask(g,r,c);''','map edges')
t=patch(t,'int x=79+c*89,n=g->map[r][c],done=r<g->floor,',
        'if(!(game_route_row_mask(g,r)&(1u<<c)))continue;\n   int x=79+c*89,n=g->map[r][c],done=r<g->floor,','map node visibility')
t=patch(t,'''line(168,last_y-13,168,52,GOLD);
    circle(168,58,12,RGB(68,23,31));
    room_icon(NODE_BOSS,168,58);
    text(190,53,"BOSS",RED,1);''',
'''/* Keep guardian in the upper right corner, off the final row. */
    line(257,last_y-16,285,55,GOLD);
    circle(288,53,10,RGB(68,23,31));
    sprite_scaled(277,42,22,22,rift110_map_icons[NODE_BOSS],30,30);''','boss position')
# The event screen has ample blank space for an explicit, readable summary.
start='static void event_screen(const Game*g){'
assert start in t
details=r"""static const char *rift113_event_details[12]={
 "Gain 75 gold now. Lose 8 HP immediately; may be fatal.",
 "Restore up to 20 HP. Pay up to 40 gold; partial payment allowed.",
 "Receive one random card already upgraded; identity is not known yet.",
 "Gain 5 maximum HP and heal 5 HP immediately.",
 "Receive one random relic, then lose 7 HP. Relic revealed afterward.",
 "Restore 8 HP and gain 30 gold. No lasting penalty.",
 "Receive two random cards. Lose 10 HP; both cards shown afterward.",
 "Gain 10 maximum HP. Lose 17 current HP; this can be fatal.",
 "Restore 28 HP. Add Injury: an unplayable card diluting future draws.",
 "Gain 110 gold, lose 16 HP. This can be fatal.",
 "Pay 12 gold. Choose TWO eligible deck cards to upgrade; effects previewed.",
 "Gain an unknown relic. Add Dread curse: unplayable; costs 1 energy next turn if held."
};
"""
t=t.replace(start,details+start,1)
a=''' int style=g->effect_turn%4;'''
b=''' int style=g->effect_turn%4;'''
# Keep scenic icon, but shrink it to make room for clear event explanation.
t=patch(t,'circle(240,136,32,RGB(36,42,61));','circle(240,135,27,RGB(36,42,61));','event decoration')
t=patch(t,''' fittext(42,88,rift_event_lore[g->effect_turn],56,WHITE);
 center(240,171,event_choices[g->effect_turn],WHITE,1);
 button(60,206,170,30,"ACCEPT",g->selection==0);button(251,206,170,30,"LEAVE",g->selection==1);''',
''' fittext(42,88,rift_event_lore[g->effect_turn],56,WHITE);
 dark_rect(23,165,434,43,226);
 border(23,165,434,43,RGB(110,90,73));
 wrapped_text(33,171,rift113_event_details[g->effect_turn],58,3,WHITE);
 center(240,210,event_choices[g->effect_turn],GOLD,1);
 button(60,224,170,27,"ACCEPT",g->selection==0);
 button(251,224,170,27,"LEAVE",g->selection==1);''','event information')
# Last event buttons end at 251 and footer begins at 255.
r.write_text(t);g.write_text(s);h.write_text(hs)
print("PASS 1.1.3 gameplay/UI event, routes, intents, block, spoil, choices, menu")
