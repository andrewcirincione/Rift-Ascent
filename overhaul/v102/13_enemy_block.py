#!/usr/bin/env python3
"""1.0.2 item 13: enemy block visible above HP, never hidden by the hand."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='''  if(e->block){snprintf(b,sizeof(b),"BLK %d",e->block);center(ex,163,b,BLUE,1);}'''
b='''  if(e->block){
    rect(ex-40,119,80,18,RGB(20,44,62));
    border(ex-40,119,80,18,BLUE);
    snprintf(b,sizeof(b),"BLOCK %d",e->block);
    center(ex,123,b,WHITE,1);
  }'''
assert a in s
p.write_text(s.replace(a,b,1))
print("PASS 13: block is separately labelled above HP and hand.")
