#!/usr/bin/env python3
"""v1.3.2 shared Gothic UI/buttons and true status-aware damage in card previews."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def one(a,b,label):
 global s
 n=s.count(a)
 if n!=1:raise RuntimeError(f"v132 {label}: {n} anchors")
 s=s.replace(a,b,1)
one('''static void button(int x,int y,int w,int h,const char*s,int selected){
 gilded_panel(x,y,w,h,selected);
 if(selected){rect(x+3,y+3,w-6,h-6,RGB(81,39,40));border(x+1,y+1,w-2,h-2,GOLD);}
 center(x+w/2,y+(h-11)/2,s,selected?RGB(253,216,156):WHITE,1);
}''',
'''/* Identical symmetrical retro-Gothic furniture across title, shop, rest,
 * relic and combat menus. Ornament stays outside the text-safe center. */
static void rift132_ornament(int cx,int cy,uint32_t ink){
 line(cx,cy-5,cx+4,cy,ink);line(cx+4,cy,cx,cy+5,ink);
 line(cx,cy+5,cx-4,cy,ink);line(cx-4,cy,cx,cy-5,ink);
 line(cx,cy-3,cx,cy+3,RGB(92,68,63));put(cx,cy,ink);
}
static void button(int x,int y,int w,int h,const char*s,int selected){
 uint32_t rim=selected?RGB(232,185,104):RGB(133,106,79);
 rect(x+2,y+3,w-3,h-3,RGB(8,12,22));
 rect(x,y,w,h,selected?RGB(65,29,42):RGB(18,24,38));
 border(x,y,w,h,rim);
 if(w>=76&&h>=23){
  line(x+8,y+3,x+w-9,y+3,selected?GOLD:RGB(89,80,78));
  line(x+8,y+h-4,x+w-9,y+h-4,RGB(106,72,62));
  rift132_ornament(x+11,y+h/2,rim);
  rift132_ornament(x+w-12,y+h/2,rim);
  line(x+19,y+h/2,x+24,y+h/2,rim);
  line(x+w-25,y+h/2,x+w-20,y+h/2,rim);
 }
 put(x+2,y+2,GOLD);put(x+w-3,y+2,GOLD);
 put(x+2,y+h-3,GOLD);put(x+w-3,y+h-3,GOLD);
 if(selected)line(x+26,y+6,x+w-27,y+6,RGB(171,87,77));
 center(x+w/2,y+(h-11)/2,s,selected?RGB(255,226,177):WHITE,1);
}''',"menu buttons")
one(''' rect(53,185,356,63,RGB(21,26,37));
 border(53,185,356,63,RGB(104,80,60));
 rect(55,187,352,1,RGB(62,62,66));''',
''' rect(53,185,356,63,RGB(18,23,36));
 border(53,185,356,63,RGB(135,96,69));
 line(62,189,400,189,RGB(86,65,58));
 line(62,244,400,244,RGB(86,65,58));
 for(int i=0;i<2;i++){
  int cx=i?394:68;
  rift132_ornament(cx,194,RGB(166,117,80));
  rift132_ornament(cx,240,RGB(166,117,80));
 }''',"hand bar")
one("static void card_render(int x,int y,int w,int h,int raw,int selected,int active){",
'''/* This engine routine already applies Strength, Weak, Vulnerable, relic
 * modifiers and repeat counts; sum over enemies only for ALL attacks. */
static int rift132_preview_damage(const Game *g,int raw){
 if(!g||g->screen!=COMBAT)return 0;
 int id=CARD_ID(raw),total=0;
 int all=(id==3||id==10||id==14||id==19||id==36||id==37||id==70);
 if(all){
  for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)
    total+=game_effective_damage(g,raw,i);
 }else{
  int target=g->target;
  if(target<0||target>=g->enemy_count||g->enemies[target].hp<=0){
   for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){target=i;break;}
  }
  total=game_effective_damage(g,raw,target);
 }
 return total;
}
static void card_render(int x,int y,int w,int h,int raw,int selected,int active){''',"total preview helper")
one('int val=game_effective_damage(card_combat_context,raw,card_combat_context->target);',
    'int val=rift132_preview_damage(card_combat_context);',"compact card")
one('''        rect(x+w-30,art_top+2,26,14,RGB(60,24,30));
        border(x+w-30,art_top+2,26,14,RED);
        center(x+w-17,art_top+3,amount,GOLD,1);''',
'''        rect(x+w-33,art_top+2,30,14,RGB(60,24,30));
        border(x+w-33,art_top+2,30,14,RED);
        center(x+w-18,art_top+3,amount,GOLD,1);''',"damage chip")
one(''' card_render(147,138,96,110,g->hand[idx],1,active);
 }''',
''' card_render(147,138,96,110,g->hand[idx],1,active);
 if(cards[id].type==0){
  int preview=rift132_preview_damage(g,g->hand[idx]);
  if(preview>0){
   char full[32];snprintf(full,sizeof(full),"TOTAL DMG %d",preview);
   rect(151,124,88,14,RGB(40,21,28));
   border(151,124,88,14,RGB(198,90,77));
   center(195,126,full,GOLD,1);
  }
 }
 }''',"selected damage chip")
one('int n=game_effective_damage(g,raw,g->target);',
    'int n=rift132_preview_damage(g,raw);',"inspector number")
p.write_text(s)
print("PASS 1.3.2 Gothic menu/hand and total modified damage preview")
