#!/usr/bin/env python3
"""1.0.2 item 08: show exact event card rewards visually before continuing."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);h=root/"game.h";g=root/"game.c";r=root/"render.c"
hs=h.read_text();s=g.read_text();rs=r.read_text()
def edit(a,b):
 global s
 assert a in s,a[:90];s=s.replace(a,b,1)
assert "RELIC_VIEW,PAUSE_MENU};" in hs
hs=hs.replace("RELIC_VIEW,PAUSE_MENU};","RELIC_VIEW,PAUSE_MENU,EVENT_RESULT};",1)
edit(''' case 6:add_card(g,card_reward(g));add_card(g,card_reward(g));lose_hp(g,10);msg(g,"Two weapons recovered, but the armory cuts deep.");break;''',
''' case 6:{
   int one=card_reward(g),two=card_reward(g);
   if(g->deck_count<MAX_DECK)add_card(g,one);
   if(g->deck_count<MAX_DECK)add_card(g,two);
   g->reward[0]=one;g->reward[1]=two;
   lose_hp(g,10);
   if(g->screen!=DEFEAT){
     g->screen=EVENT_RESULT;
     snprintf(g->message,sizeof(g->message),"Obtained %s and %s (-10 HP)",
              cards[one].name,cards[two].name);
   }
   break;
 }''')
edit("if(g->screen!=DEFEAT)enter_map(g);","if(g->screen!=DEFEAT&&g->screen!=EVENT_RESULT)enter_map(g);")
edit(" case RELIC_VIEW:",''' case EVENT_RESULT:
   if(b==BTN_OK||b==BTN_BACK)enter_map(g);
   break;
 case RELIC_VIEW:''')
edit("g->screen>PAUSE_MENU","g->screen>EVENT_RESULT")
needle="static void reward_screen(const Game*g){"
assert needle in rs
rs=rs.replace(needle,'''static void event_result_screen(const Game*g){
 header(g,"EVENT RESULT / SEE EXACT REWARDS");
 center(240,49,"THE ARMORY'S SPOILS",GOLD,2);
 for(int i=0;i<2;i++){
   int x=103+i*148;
   card_render(x,75,128,142,g->reward[i],0,1);
   fittext(x,224,cards[CARD_ID(g->reward[i])].name,17,GOLD);
 }
 center(240,239,"-10 HP FROM THE ARMORY",RED,1);
 footer("X CONTINUE  O CONTINUE");
}
'''+needle,1)
needle=" case REWARD:"
assert needle in rs
rs=rs.replace(needle," case EVENT_RESULT:event_result_screen(g);break;\n"+needle,1)
h.write_text(hs);g.write_text(s);r.write_text(rs)
print("PASS 08: exact cards from armory shown as event result before route continues.")
