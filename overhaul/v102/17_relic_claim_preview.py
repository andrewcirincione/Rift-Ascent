#!/usr/bin/env python3
"""1.0.2 item 17: claim relic, see its power, then independently continue."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);h=root/"game.h";g=root/"game.c";r=root/"render.c"
hs=h.read_text();s=g.read_text();rs=r.read_text()
def edit(a,b):
 global s
 assert a in s,a[:100];s=s.replace(a,b,1)
assert "PAUSE_MENU,EVENT_RESULT};" in hs
hs=hs.replace("PAUSE_MENU,EVENT_RESULT};","PAUSE_MENU,EVENT_RESULT,RELIC_RESULT};",1)
edit(''' case BOSS_REWARD:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK){
    if(g->reward[g->selection]>=0)add_relic(g,g->reward[g->selection]);
    heal(g,g->maxhp/3);
    g->act++;g->floor=0;g->lane=1;map_generate(g);enter_map(g);
    msg(g,"A new ascent begins. Choose a path.");
  }
  break;''',
''' case BOSS_REWARD:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK){
    int relic_id=g->reward[g->selection];
    if(relic_id>=0)add_relic(g,relic_id);
    g->reward[0]=relic_id;g->win_screen=1;
    g->screen=RELIC_RESULT;
    if(relic_id>=0)snprintf(g->message,sizeof(g->message),
                             "RELIC OBTAINED: %s",relic_defs[relic_id].name);
  }
  break;
 case RELIC_RESULT:
  if(b==BTN_OK||b==BTN_BACK){
    if(g->win_screen){
      heal(g,g->maxhp/3);
      g->act++;g->floor=0;g->lane=1;map_generate(g);
      enter_map(g);msg(g,"A new ascent begins. Choose a path.");
    }else enter_map(g);
  }
  break;''')
edit("g->screen>EVENT_RESULT","g->screen>RELIC_RESULT")
edit(''' case NODE_TREASURE:{int r=random_relic(g);if(r>=0){add_relic(g,r);snprintf(g->message,sizeof(g->message),"Found relic: %s",relic_defs[r].name);}else msg(g,"The chest was empty.");enter_map(g);break;}''',
''' case NODE_TREASURE:{
   int r=random_relic(g);
   if(r>=0){
      add_relic(g,r);g->reward[0]=r;g->win_screen=0;g->screen=RELIC_RESULT;
      snprintf(g->message,sizeof(g->message),"FOUND RELIC: %s",relic_defs[r].name);
   }else{msg(g,"The chest was empty.");enter_map(g);}
   break;
 }''')
edit(''' case 4: {int r=random_relic(g);if(r>=0)add_relic(g,r);lose_hp(g,7);msg(g,"You take the relic, but the trap wounds you.");break;}''',
''' case 4:{
    int r=random_relic(g);if(r>=0)add_relic(g,r);
    lose_hp(g,7);
    if(g->screen!=DEFEAT&&r>=0){
       g->reward[0]=r;g->win_screen=0;g->screen=RELIC_RESULT;
       snprintf(g->message,sizeof(g->message),"TRAP -7 HP. FOUND: %s",relic_defs[r].name);
    }break;
 }''')
edit(''' case 11:{int r=random_relic(g);if(r>=0)add_relic(g,r);add_card(g,53);msg(g,"An heirloom taken; a dreadful curse follows.");break;}''',
''' case 11:{
   int r=random_relic(g);if(r>=0)add_relic(g,r);
   add_card(g,53);
   if(r>=0){
     g->reward[0]=r;g->win_screen=0;g->screen=RELIC_RESULT;
     snprintf(g->message,sizeof(g->message),"FOUND %s; ADDED DREAD",relic_defs[r].name);
   }break;
 }''')
edit("g->screen!=DEFEAT&&g->screen!=EVENT_RESULT",
     "g->screen!=DEFEAT&&g->screen!=EVENT_RESULT&&g->screen!=RELIC_RESULT")
a="static void event_result_screen(const Game*g){"
assert a in rs
rs=rs.replace(a,'''static void relic_result_screen(const Game*g){
 header(g,"NEW RELIC / INSPECT BEFORE CONTINUING");
 gilded_panel(39,47,401,193,1);
 int id=g->reward[0];
 center(240,58,"RELIC ACQUIRED",GOLD,2);
 circle(240,113,29,RGB(77,61,85));
 triangle(240,85,213,133,267,133,PURPLE);
 triangle(240,89,223,130,257,130,GOLD);
 if(id>=0&&id<RELIC_COUNT){
    center(240,150,relic_defs[id].name,GOLD,1);
    wrapped_text(84,173,relic_defs[id].desc,44,4,WHITE);
 }
 footer("X ACKNOWLEDGE RELIC AND CONTINUE");
}
'''+a,1)
a=" case EVENT_RESULT:event_result_screen(g);break;"
assert a in rs
rs=rs.replace(a,a+"\n case RELIC_RESULT:relic_result_screen(g);break;",1)
h.write_text(hs);g.write_text(s);r.write_text(rs)
print("PASS 17: boss, chest and event relics show their full power before route advances.")
