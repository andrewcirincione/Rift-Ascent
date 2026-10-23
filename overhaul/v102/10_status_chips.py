#!/usr/bin/env python3
"""1.0.2 item 10: readable full-word status chips outside the enemy sprite."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='''static void rift_status_chip(int x,int y,const char *glyph,int stacks,uint32_t ink){
 char label[20];snprintf(label,sizeof(label),"%s%d",glyph,stacks);
 circle(x+5,y+5,8,ink);
 fittext(x,y,label,5,WHITE);
}'''
b='''static void rift_status_chip(int x,int y,const char *glyph,int stacks,uint32_t ink){
 char label[24];snprintf(label,sizeof(label),"%s %d",glyph,stacks);
 rect(x,y,61,15,RGB(21,30,44));border(x,y,61,15,ink);
 circle(x+7,y+7,5,ink);
 fittext(x+15,y+2,label,6,WHITE);
}'''
assert a in s;s=s.replace(a,b,1)
# Move multiple foes left to reserve an unobstructed status lane.
s=s.replace("(i==0?333:425)","(i==0?284:380)")
s=s.replace("i==0?333:425","i==0?284:380")
s=s.replace("i==0?334:427","i==0?284:380")
s=s.replace("g->target==0?333:425","g->target==0?284:380")
s=s.replace('''if(g->strength){rift_status_chip(22,py,"S",g->strength,GOLD);py+=20;}''',
            '''if(g->strength){rift_status_chip(10,py,"STR",g->strength,GOLD);py+=20;}''')
for kind,tag in [("weak","WEAK"),("vulnerable","VUL"),("poison","PSN")]:
 a=f'if(g->{kind}){{rift_status_chip(22,py,"'+{"weak":"W","vulnerable":"V","poison":"P"}[kind]+f'",g->{kind},'
 if a not in s:raise RuntimeError("player status anchor "+a)
 s=s.replace(a,f'if(g->{kind}){{rift_status_chip(10,py,"{tag}",g->{kind},',1)
for kind,tag in [("strength","STR"),("weak","WEAK"),("vulnerable","VUL"),("poison","PSN")]:
 a='rift_status_chip(px,py,"'+{"strength":"S","weak":"W","vulnerable":"V","poison":"P"}[kind]+'",'
 assert a in s,a
 s=s.replace(a,'rift_status_chip(px,py,"'+tag+'",',1)
a='''  if(e->vulnerable||e->weak||e->poison){
   snprintf(b,sizeof(b),"%s%s%s",e->vulnerable?"VUL ":"",e->weak?"WEAK ":"",e->poison?"PSN":"");
   center(ex,130,b,GOLD,1);
  }'''
assert a in s;s=s.replace(a,"",1)
a="    int px=ex>395?443:ex+32,py=83;"
assert a in s;s=s.replace(a,"    int px=ex+37,py=82;",1)
p.write_text(s)
print("PASS 10: no duplicate VUL rows; selected foe effects in right-side full-name chips.")
