#!/usr/bin/env python3
"""1.0.2 item 18: crisp routes and inspection of the next row without entering it."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);g=root/"game.c";r=root/"render.c"
s=g.read_text();rs=r.read_text()
def edit(a,b):
 global s
 assert a in s,a[:100];s=s.replace(a,b,1)
def draw(a,b):
 global rs
 assert a in rs,a[:100];rs=rs.replace(a,b,1)
edit(''' case MAP:
  if(b==BTN_LEFT)''',
''' case MAP:
  if(b==BTN_UP&&g->scroll==0&&g->floor<MAP_ROWS-1)g->scroll=1;
  else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll=0;
    if(!game_route_available(g,g->selection))
      for(int k=0;k<MAP_LANES;k++)
        if(game_route_available(g,k)){g->selection=k;break;}
  }
  else if(g->scroll>0&&b==BTN_LEFT&&g->selection>0)g->selection--;
  else if(g->scroll>0&&b==BTN_RIGHT&&g->selection<2)g->selection++;
  else if(g->scroll>0&&b==BTN_OK)
    msg(g,"PREVIEW ONLY. Press DOWN to select current floor.");
  else if(b==BTN_LEFT)''')
# Any new map entry cancels cursor preview.
edit(''' g->screen=MAP;g->selection=CLAMP(g->lane,0,2);''',
     ''' g->screen=MAP;g->scroll=0;g->selection=CLAMP(g->lane,0,2);''')
draw(''' case NODE_FIGHT: /* crossed swords */
    line(x-6,y-6,x+6,y+6,WHITE);
    line(x+6,y-6,x-6,y+6,WHITE);
    line(x-7,y+5,x-3,y+9,GOLD);line(x+7,y+5,x+3,y+9,GOLD);break;''',
''' case NODE_FIGHT: /* thicker anti-shimmer crossed swords */
    line(x-8,y-8,x+6,y+6,WHITE);line(x-7,y-8,x+7,y+6,WHITE);
    line(x+8,y-8,x-6,y+6,BLUE);line(x+7,y-8,x-7,y+6,BLUE);
    line(x-9,y+5,x-3,y+10,GOLD);line(x+9,y+5,x+3,y+10,GOLD);
    rect(x-2,y+2,5,5,RGB(67,55,50));break;''')
draw(''' case NODE_ELITE:
    circle(x,y-2,6,WHITE);circle(x-3,y-3,2,BG);circle(x+3,y-3,2,BG);
    rect(x-4,y+4,8,4,WHITE);break;''',
''' case NODE_ELITE:
    circle(x,y-3,9,WHITE);rect(x-7,y+2,14,8,WHITE);
    circle(x-4,y-4,3,BG);circle(x+4,y-4,3,BG);
    triangle(x-2,y+1,x,y-3,x+2,y+1,BG);
    for(int t=-4;t<=4;t+=4)line(x+t,y+5,x+t,y+10,BG);
    break;''')
draw('''circle(x,y,13,done?RGB(45,50,57):available?RGB(180,125,61):RGB(35,47,62));
   circle(x,y,9,done?RGB(73,76,77):(r==g->floor&&!available)?RGB(71,66,64):col);''',
'''circle(x,y,16,done?RGB(45,50,57):available?RGB(180,125,61):RGB(35,47,62));
   circle(x,y,12,done?RGB(73,76,77):(r==g->floor&&!available)?RGB(71,66,64):col);''')
draw('''if(r==g->floor&&c==g->selection){
    border(x-15,y-15,30,30,''',
'''if(r==g->floor+g->scroll&&c==g->selection){
    border(x-18,y-18,36,36,''')
draw(''' if(g->floor<MAP_ROWS)text(325,147,node_name(g->map[g->floor][g->selection]),node_color(g->map[g->floor][g->selection]),1);''',
''' if(g->floor+g->scroll<MAP_ROWS){
    int node=g->map[g->floor+g->scroll][g->selection];
    text(325,147,node_name(node),node_color(node),1);
 }''')
draw('''    int node=g->map[g->floor][g->selection];''',
'''    int node=g->map[g->floor+g->scroll][g->selection];''')
draw(''' text(324,193,"X TAKE PATH",GOLD,1);''',
''' if(g->scroll>0)text(324,193,"UPCOMING / VIEW ONLY",GREEN,1);
 else text(324,193,"X TAKE PATH",GOLD,1);''')
draw('''footer("FOLLOW ROADS  X ENTER  SQUARE DECK  START RELICS");''',
'''footer("UP PREVIEW   DOWN RETURN   X ENTER   SQUARE DECK");''')
g.write_text(s);r.write_text(rs)
print("PASS 18: larger fight/elite glyphs, upcoming row can be viewed but not entered.")
