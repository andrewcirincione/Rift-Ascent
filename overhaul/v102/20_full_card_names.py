#!/usr/bin/env python3
"""1.0.2 item 20: two-line complete card titles with art and cost repositioned."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("card title anchor "+a[:100])
 s=s.replace(a,b,1)
sub(''' rect(x+3,y+3,w-6,15,hue);
 char title[26];snprintf(title,sizeof(title),"%.12s%s",d->name,up?"+":"");
 fittext(x+20,y+5,title,(w-24)/7,WHITE);
 art_blit(x+4,y+19,w-8,arth,art,100,60);
 border(x+4,y+19,w-8,arth,RGB(100,80,67));
 char coststr[12];snprintf(coststr,sizeof(coststr),"%d",d->cost<0?0:d->cost);
 circle(x+9,y+10,9,RGB(29,98,150));circle(x+9,y+10,7,RGB(50,149,205));text(x+5,y+5,coststr,WHITE,1);
 int room=(y+h-4)-(y+21+arth);''',
''' rect(x+3,y+3,w-6,24,hue);
 char title[40];snprintf(title,sizeof(title),"%s%s",d->name,up?"+":"");
 int cols=(w-10)/7;
 if(cols<2)cols=2;
 fittext(x+5,y+3,title,cols,WHITE);
 if((int)strlen(title)>cols)fittext(x+5,y+14,title+cols,cols,WHITE);
 art_blit(x+4,y+28,w-8,arth,art,100,60);
 border(x+4,y+28,w-8,arth,RGB(100,80,67));
 char coststr[12];snprintf(coststr,sizeof(coststr),"%d",d->cost<0?0:d->cost);
 circle(x+w-11,y+36,9,RGB(29,98,150));
 circle(x+w-11,y+36,7,RGB(50,149,205));
 text(x+w-15,y+31,coststr,WHITE,1);
 int room=(y+h-4)-(y+30+arth);''')
sub('''wrapped_text(x+5,y+21+arth,card_description(raw)''',
    '''wrapped_text(x+5,y+30+arth,card_description(raw)''')
p.write_text(s)
print("PASS 20: card names wrap across up to two lines; title is no longer truncated.")
