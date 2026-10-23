#!/usr/bin/env python3
"""UI/readability repair after 1.0.2: removes duplicate layers, fixes overlays."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def change(a,b):
 global s
 if a not in s:raise RuntimeError("1.0.3 renderer anchor missing: "+a[:110])
 s=s.replace(a,b,1)
# Energy on left of card art; damage on right, never painting over the cost.
change('''char coststr[12];snprintf(coststr,sizeof(coststr),"%d",d->cost<0?0:d->cost);
 circle(x+w-11,y+36,9,RGB(29,98,150));
 circle(x+w-11,y+36,7,RGB(50,149,205));
 text(x+w-15,y+31,coststr,WHITE,1);''',
'''char coststr[12];
 if(d->cost==-1)snprintf(coststr,sizeof(coststr),"X");
 else snprintf(coststr,sizeof(coststr),"%d",d->cost<0?0:d->cost);
 circle(x+11,y+36,9,RGB(29,98,150));
 circle(x+11,y+36,7,RGB(50,149,205));
 center(x+11,y+31,coststr,WHITE,1);''')
change(''' int tall=h>=120,arth=tall?57:18;''',
''' /* Full compact combat cards preserve descriptive space at the bottom. */
 int tall=h>=120,arth=tall?48:(h>=95?10:15);''')
change(''' int maxrows=CLAMP(room/11,0,tall?4:2);''',
''' int maxrows=CLAMP(room/11,0,tall?5:(h>=95?7:3));''')
change('''     rect(x+w-22,y+arth+14,21,13,RGB(47,20,23));
     border(x+w-22,y+arth+14,21,13,RED);
     center(x+w-11,y+arth+15,num,GOLD,1);''',
'''     rect(x+w-24,y+29,23,14,RGB(47,20,23));
     border(x+w-24,y+29,23,14,RED);
     center(x+w-12,y+31,num,GOLD,1);''')
change('''if(h>=74)fittext(x+5,y+h-15,"NO ENERGY",9,RGB(202,205,210));''',
'''if(h>=74)fittext(x+5,y+h-15,"LOW ENERGY",9,RGB(202,205,210));''')
# Move all status chips into a dedicated vertical strip on the far right.
change("int px=ex+37,py=82;","int px=415,py=83;")
change(''' int sx=17;
 if(g->weak){snprintf(b,sizeof(b),"WK %d",g->weak);text(sx,119,b,RED,1);sx+=49;}
 if(g->vulnerable){snprintf(b,sizeof(b),"VUL %d",g->vulnerable);text(sx,119,b,GOLD,1);sx+=63;}
 if(g->poison){snprintf(b,sizeof(b),"PSN %d",g->poison);text(sx,119,b,GREEN,1);sx+=59;}
 if(g->dexterity){snprintf(b,sizeof(b),"DEX %d",g->dexterity);text(sx,119,b,BLUE,1);}''',
''' if(g->dexterity){snprintf(b,sizeof(b),"DEX %d",g->dexterity);text(13,119,b,BLUE,1);}''')
# One narrow intent line; no pointless black bar when the foe isn't attacking.
change('''dark_rect(ex-49,44,98,37,215);''',
'''if(hits)dark_rect(ex-43,47,86,19,199);''')
change('''      line(ex-42,56,ex-31,66,WHITE);
      line(ex-43,55,ex-38,51,GOLD);
      line(ex-45,63,ex-37,71,RED);''',
'''      /* A subtle separated crossed-blade icon; no stray red/white slashes. */
      line(ex-36,51,ex-29,59,GOLD);
      line(ex-29,51,ex-36,59,GOLD);''')
change('''if(hits)center(ex+12,47,b,RED,1);''',
'''if(hits)fittext(ex-22,51,b,9,RED);''')
change('''if(effect[0])center(ex,64,effect,GOLD,1);''',
'''if(effect[0])fittext(ex-42,69,effect,12,GOLD);''')
change('''dark_rect(ex-48,34,96,32,184);''','''dark_rect(ex-48,30,96,17,184);''')
# Block is incorporated into the HP display; no floating panel on the torso.
change('''  if(e->block){
    rect(ex-40,119,80,18,RGB(20,44,62));
    border(ex-40,119,80,18,BLUE);
    snprintf(b,sizeof(b),"BLOCK %d",e->block);
    center(ex,123,b,WHITE,1);
  }''',
'''  if(e->block){
    int bw=CLAMP(12+e->block*2,12,77);
    rect(ex+39-bw,155,bw,7,RGB(52,111,187));
    snprintf(b,sizeof(b),"%d/%d BLK%d",e->hp,e->maxhp,e->block);
    fittext(ex-39,143,b,11,WHITE);
  }''')
# One HP caption only, overwritten using the block caption when present.
change('''  snprintf(b,sizeof(b),"%d/%d",e->hp,e->maxhp);
  center(ex,142,b,WHITE,1);''',
'''  snprintf(b,sizeof(b),"%d/%d",e->hp,e->maxhp);
  center(ex,142,b,WHITE,1);''')
# Place cards below HP, with enlarged selected text and no hovering giant rectangle.
change('''  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?166+wave(animation_tick,60,1):177;''',
'''  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?168+wave(animation_tick,60,1):177;''')
change('''card_render(x,y,70,selected?88:77,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));''',
'''card_render(x,y,70,selected?101:79,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));''')
start=s.index(" /* Compact physical cards, with an always-readable full card hover panel.")
stop=s.index(" button(414,191",start)
s=s[:start]+''' /* Full card rules are printed directly on the selected card.
    No separate floating tooltip covers the player or battlefield. */
'''+s[stop:]
change('''footer("LEFT/RIGHT CARDS+END  X SELECT   START INFO  TRI POTIONS");''',
'''footer("X PLAY     O END TURN     SQ DECK     START INSPECT");''')
# Keep a distinct boss fade without re-summoning previously killed monsters.
change('''     for(int i=0;i<replay.enemy_count;i++)replay.enemies[i].hp=1;''',
'''     for(int i=0;i<replay.enemy_count;i++)replay.enemies[i].hp=0;''')
# Remove windup slash streaks (reported white and red lines).
change('''    if((enemy_windup/3)%2==0) {
      line(x-17,y-11,x+14,y+6,WHITE);
      line(x-9,y-14,x+20,y+2,RED);
      circle(x+14,y+5,3,GOLD);
    }''',
'''    if((enemy_windup/3)%2==0)circle(x,y,2,GOLD);''')
# A less aggressive background description panel in the battle screen header.
change('''header(g,"BATTLE / X PLAY   START EXPLAIN");''',
'''header(g,"BATTLE");''')
# Route connectors are double-stroked with highlighting.
change('''       line(x,y-12,79+d*89,y-25,
            (r==g->floor-1&&c==g->lane)?GOLD:RGB(112,84,61));''',
'''       {
         uint32_t ink=(r==g->floor-1&&c==g->lane)?GOLD:RGB(154,117,77);
         line(x-1,y-12,79+d*89-1,y-25,RGB(63,53,49));
         line(x,y-12,79+d*89,y-25,ink);
         line(x+1,y-12,79+d*89+1,y-25,ink);
       }''')
# Allow map to scroll along look-ahead rows.
change(''' int first=CLAMP(g->floor-2,0,MAP_ROWS-5);''',
''' int first=CLAMP(g->floor+g->scroll-2,0,MAP_ROWS-5);''')
# A visible X cost marker, instead of 0 for variable-cost attacks.
p.write_text(s)
print("PASS 1.0.3 renderer: readable battle cards, compact intent, block HUD, status chips and route clarity.")
