#!/usr/bin/env python3
"""1.0.2 item 07: custom on-screen face-button glyphs in footer instructions."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='''static void footer(const char*help){
 rect(0,255,W,17,RGB(9,16,27));rect(0,255,W,1,RGB(115,85,60));
 text(8,258,help,MUTED,1);
}'''
b='''static int footer_button(const char *p,int n){
 if(n==1&&p[0]=='X')return 1;
 if(n==1&&p[0]=='O')return 2;
 if(n==2&&p[0]=='S'&&p[1]=='Q')return 3;
 if(n==6&&!memcmp(p,"SQUARE",6))return 3;
 if(n==3&&!memcmp(p,"TRI",3))return 4;
 if(n==8&&!memcmp(p,"TRIANGLE",8))return 4;
 if(n==5&&!memcmp(p,"START",5))return 5;
 return 0;
}
static void footer_glyph(int x,int y,int kind){
 if(kind==5){rect(x-5,y-4,12,9,RGB(76,88,104));border(x-5,y-4,12,9,WHITE);return;}
 circle(x,y,7,RGB(47,57,76));circle(x,y,6,RGB(95,115,137));
 if(kind==1){line(x-3,y-3,x+3,y+3,WHITE);line(x+3,y-3,x-3,y+3,WHITE);}
 else if(kind==2){circle(x,y,4,WHITE);circle(x,y,2,RGB(95,115,137));}
 else if(kind==3){border(x-4,y-4,9,9,WHITE);}
 else if(kind==4){triangle(x,y-4,x-4,y+4,x+4,y+4,WHITE);}
}
static void footer(const char*help){
 rect(0,255,W,17,RGB(9,16,27));rect(0,255,W,1,RGB(115,85,60));
 int x=8;
 for(const char *p=help;*p&&x<470;){
  if(*p==' '){x+=3;p++;continue;}
  const char*q=p;while(*q&&*q!=' ')q++;
  int n=(int)(q-p),kind=footer_button(p,n);
  if(kind){footer_glyph(x+7,264,kind);x+=18;}
  else if(n<44){char word[46];memcpy(word,p,(size_t)n);word[n]=0;
    text(x,258,word,MUTED,1);x+=n*7+3;}
  p=q;
 }
}'''
assert a in s
p.write_text(s.replace(a,b,1))
print("PASS 07: footer button names become custom vector PSP button glyphs.")
