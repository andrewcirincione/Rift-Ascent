#!/usr/bin/env python3
"""1.0.2 item 01: safe SELECT pause / save-and-quit, resume with Circle."""
from pathlib import Path
import sys
base=Path(sys.argv[1]);g=base/"game.c";h=base/"game.h";r=base/"render.c"
s=g.read_text();hs=h.read_text();rs=r.read_text()
def edit(old,new):
 global s
 assert old in s,old[:80];s=s.replace(old,new,1)
def rend(old,new):
 global rs
 assert old in rs,old[:80];rs=rs.replace(old,new,1)
assert "RELIC_VIEW};" in hs
hs=hs.replace("RELIC_VIEW};","RELIC_VIEW,PAUSE_MENU};",1)
hs=hs.replace("BTN_START=256};","BTN_START=256,BTN_PAUSE=512};",1)
edit("void game_action(Game*g,int b){",'''void game_action(Game*g,int b){
 if(b==BTN_PAUSE){
   if(g->screen==PAUSE_MENU){g->screen=g->prior_screen;return;}
   if(g->screen!=TITLE&&g->screen!=VICTORY&&g->screen!=DEFEAT){
     g->prior_screen=g->screen;g->screen=PAUSE_MENU;return;
   }
 }''')
edit(" case RELIC_VIEW:",''' case PAUSE_MENU:
   if(b==BTN_BACK)g->screen=g->prior_screen;
   break;
 case RELIC_VIEW:''')
edit("g->screen>BOSS_REWARD","g->screen>PAUSE_MENU")
# Pause overlay does not enter saved snapshots. Preserve all remaining
# Game struct fields and version=16, so old save compatibility is unchanged.
rend(" case RELIC_VIEW:relic_screen(g);break;",''' case RELIC_VIEW:relic_screen(g);break;
 case PAUSE_MENU: {
   Game frame=*g;frame.screen=g->prior_screen;
   game_render_frame(&frame,pixels,stride,tick);
   dark_rect(0,24,W,231,209);
   gilded_panel(76,59,328,158,1);
   center(240,77,"GAME PAUSED",GOLD,2);
   center(240,112,"X SAVE & QUIT",WHITE,1);
   center(240,137,"CIRCLE / SELECT RESUME",WHITE,1);
   center(240,173,"PROGRESS SAVED TO MEMORY STICK",MUTED,1);
   break;
 }''')
h.write_text(hs);g.write_text(s);r.write_text(rs)
print("1.0.2 item01 pause/save UI and game state passed patch anchors.")
