#!/usr/bin/env python3
"""Final screenshot-derived 1.0.2 polish: compact hover and no zero-damage forecast."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("1.0.2 screenshot polish anchor missing "+a[:100])
 s=s.replace(a,b,1)
sub('''   dark_rect(6,44,258,92,214);
   border(6,44,258,92,GOLD);''',
'''   int desc_len=(int)strlen(card_description(raw));
   int rows=(desc_len+33)/34;
   rows=CLAMP(rows,1,5);
   int hover_h=29+rows*11;
   dark_rect(6,44,258,hover_h,214);
   border(6,44,258,hover_h,GOLD);''')
sub('''      if(hits>1)snprintf(b,sizeof(b),"%d HITS x %d DAMAGE (BEFORE BLOCK)",hits,damage/hits);
      else snprintf(b,sizeof(b),"%d INCOMING DAMAGE (BEFORE BLOCK)",damage);
      center(240,109,b,RED,1);
      wrapped_text(43,129,game_intent_description(e->intent),55,5,WHITE);''',
'''      if(hits){
        if(hits>1)snprintf(b,sizeof(b),"%d HITS x %d DAMAGE (BEFORE BLOCK)",hits,damage/hits);
        else snprintf(b,sizeof(b),"%d INCOMING DAMAGE (BEFORE BLOCK)",damage);
        center(240,109,b,RED,1);
      }
      wrapped_text(43,hits?129:109,game_intent_description(e->intent),55,6,WHITE);''')
sub(''' if(n==6&&!memcmp(p,"SQUARE",6))return 3;''',
''' if(n==6&&!memcmp(p,"SQUARE",6))return 3;
 if(n==6&&!memcmp(p,"CIRCLE",6))return 2;''')
sub('''  if(*p==' '){x+=3;p++;continue;}
  const char*q=p;while(*q&&*q!=' ')q++;''',
'''  if(*p==' '){x+=3;p++;continue;}
  if(*p=='/'||*p=='+'){text(x,258,"/",MUTED,1);x+=9;p++;continue;}
  const char*q=p;while(*q&&*q!=' '&&*q!='/'&&*q!='+')q++;''')
p.write_text(s)
print("PASS: compact description hover, no zero-damage inspector, compound icon labels.")
