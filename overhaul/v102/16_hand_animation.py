#!/usr/bin/env python3
"""1.0.2 item 16: animate hand discard and next-turn draw."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("hand animation anchor "+a[:100])
 s=s.replace(a,b,1)
sub("static unsigned fx_tick,fx_transition_tick;",'''static unsigned fx_tick,fx_transition_tick;
static int old_hand[10],old_hand_count=0;
static unsigned hand_exchange_tick=0;
static int hand_exchange=0;''')
sub(" fx_tick=tick;fx_player_damage=",''' if(before->screen==COMBAT&&after->screen==COMBAT&&
    before->turn!=after->turn){
   old_hand_count=before->hand_count<10?before->hand_count:10;
   for(int j=0;j<old_hand_count;j++)old_hand[j]=before->hand[j];
   hand_exchange_tick=tick;hand_exchange=1;
 }
 fx_tick=tick;fx_player_damage=''')
sub('''  card_render(x,y,70,selected?88:77,g->hand[i]''',
'''  if(hand_exchange&&animation_tick>=hand_exchange_tick){
    unsigned exchange_age=animation_tick-hand_exchange_tick;
    if(exchange_age<10)y=270; /* discard leaves before drawing */
    else if(exchange_age<23)y+= (int)((23-exchange_age)*6);
  }
  card_render(x,y,70,selected?88:77,g->hand[i]''')
sub(''' if(start+visible<g->hand_count){''',
''' if(hand_exchange&&animation_tick>=hand_exchange_tick){
   unsigned age=animation_tick-hand_exchange_tick;
   if(age<15){
      for(int i=0;i<old_hand_count&&i<5;i++){
        int x=56+i*73,y=177+(int)age*7;
        rect(x,y,63,70,RGB(34,38,48));
        border(x,y,63,70,RGB(103,107,110));
        fittext(x+4,y+13,cards[CARD_ID(old_hand[i])].name,8,MUTED);
      }
   }else if(age>=23)hand_exchange=0;
 }
 if(start+visible<g->hand_count){''')
p.write_text(s)
print("PASS 16: previous cards slide into discard before new cards rise into hand.")
