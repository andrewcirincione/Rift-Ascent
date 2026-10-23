#!/usr/bin/env python3
"""1.0.2 item 19: battle deck browser with draw, hand, discard and exhaust piles."""
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
edit('''  else if(b==BTN_ALT){g->prior_screen=COMBAT;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;}''',
'''  else if(b==BTN_ALT){
    g->master_index=g->selection;
    g->prior_screen=COMBAT;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;
  }''')
edit(''' case DECK_VIEW:
  if(b==BTN_UP''',
''' case DECK_VIEW:
  if(g->prior_screen==COMBAT){
    if(b==BTN_LEFT){g->scroll=(g->scroll+3)%4;g->selection=0;}
    else if(b==BTN_RIGHT){g->scroll=(g->scroll+1)%4;g->selection=0;}
    else if(b==BTN_BACK){
      g->screen=COMBAT;
      g->selection=MIN(g->master_index,g->hand_count);
    }else{
      int count=g->scroll==0?g->draw_count:
         g->scroll==1?g->hand_count:
         g->scroll==2?g->discard_count:g->exhaust_count;
      if(b==BTN_UP&&g->selection>=3)g->selection-=3;
      if(b==BTN_DOWN&&g->selection+3<count)g->selection+=3;
    }
    break;
  }
  if(b==BTN_UP''')
edit('''if(g->screen==DECK_VIEW&&(g->selection<0||g->selection>=g->deck_count))return 0;''',
'''if(g->screen==DECK_VIEW){
   int count=g->prior_screen==COMBAT?
      (g->scroll==0?g->draw_count:
       g->scroll==1?g->hand_count:
       g->scroll==2?g->discard_count:g->exhaust_count):g->deck_count;
   if(g->selection<0||g->selection>=MAX(1,count))return 0;
 }''')
needle="static void deck_screen(const Game*g){"
assert needle in rs
rs=rs.replace(needle,'''static void battle_piles_screen(const Game*g){
 header(g,"COMBAT DECK / DRAW - HAND - DISCARD - EXHAUST");
 const char *names[]={"DRAW","HAND","DISCARD","EXHAUST"};
 const int *pile=g->scroll==0?g->draw:g->scroll==1?g->hand:
                  g->scroll==2?g->discard:g->exhaust;
 int count=g->scroll==0?g->draw_count:g->scroll==1?g->hand_count:
           g->scroll==2?g->discard_count:g->exhaust_count;
 char b[96];
 for(int i=0;i<4;i++){
   int x=12+i*115;
   rect(x,43,109,21,g->scroll==i?RGB(93,69,47):RGB(35,46,60));
   border(x,43,109,21,g->scroll==i?GOLD:RGB(78,88,100));
   int n=i==0?g->draw_count:i==1?g->hand_count:
         i==2?g->discard_count:g->exhaust_count;
   snprintf(b,sizeof(b),"%s %d",names[i],n);
   fittext(x+8,49,b,14,g->scroll==i?GOLD:WHITE);
 }
 int page=(g->selection/6)*6;
 for(int i=page;i<count&&i<page+6;i++){
   int off=i-page,x=18+(off%3)*108,y=72+(off/3)*82;
   card_render(x,y,98,77,pile[i],i==g->selection,1);
 }
 gilded_panel(342,70,128,161,0);
 if(count){
   int raw=pile[g->selection],id=CARD_ID(raw);
   fittext(349,83,cards[id].name,16,GOLD);
   wrapped_text(349,111,card_description(raw),16,8,WHITE);
 }else center(402,137,"EMPTY PILE",MUTED,1);
 if(page+6<count)text(348,216,"DOWN: MORE",GOLD,1);
 footer("LEFT/RIGHT PILES  UP/DOWN CARDS  O CLOSE");
}
static void deck_screen(const Game*g){
 if(g->prior_screen==COMBAT){battle_piles_screen(g);return;}''',1)
g.write_text(s);r.write_text(rs)
print("PASS 19: view actual shuffled draw, hand, discard, exhausted cards in combat.")
