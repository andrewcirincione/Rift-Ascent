#!/usr/bin/env python3
"""Render/UI upgrade for original PSP EBOOT. No newlib or allocated textures."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def replace(old,new):
 global s
 if old not in s:raise RuntimeError("render anchor missing: "+old[:90])
 s=s.replace(old,new,1)
replace("static unsigned animation_tick;",
"""static unsigned animation_tick;
static int overlay_kind,overlay_page,overlay_potion;
static int fx_enemy_damage[2],fx_player_damage,fx_heal,fx_block,fx_target,fx_attack;
static unsigned fx_tick,fx_transition_tick;
static int fx_transition;
void game_render_set_overlay(int kind,int page,int potion){
 overlay_kind=kind;overlay_page=page;overlay_potion=potion;
}
void game_render_feedback(const Game *before,const Game *after,int action,unsigned tick){
 fx_tick=tick;fx_player_damage=before->hp>after->hp?before->hp-after->hp:0;
 fx_heal=after->hp>before->hp?after->hp-before->hp:0;
 fx_block=after->block>before->block?after->block-before->block:0;
 fx_target=before->target;fx_attack=(action==BTN_OK && before->screen==COMBAT);
 for(int i=0;i<2;i++)
  fx_enemy_damage[i]=(i<before->enemy_count&&i<after->enemy_count &&
                       before->enemies[i].hp>after->enemies[i].hp)
                   ?before->enemies[i].hp-after->enemies[i].hp:0;
 if(before->screen!=after->screen){fx_transition=1;fx_transition_tick=tick;}
}
""")
# Antialias outlines along existing 7x11 font boundaries to soften jagged type
start=s.index("static void text(int x,int y,const char*s,uint32_t c,int size){")
end=s.index("\nstatic void center(",start)
s=s[:start]+"""static uint32_t blend_text(uint32_t bg,uint32_t ink,int opacity) {
 int br=bg&255,bg_=bg>>8&255,bb=bg>>16&255;
 int ir=ink&255,ig=ink>>8&255,ib=ink>>16&255;
 return RGB((br*(255-opacity)+ir*opacity)/255,
            (bg_*(255-opacity)+ig*opacity)/255,
            (bb*(255-opacity)+ib*opacity)/255);
}
static int glyph_on(const unsigned char *glyph,int x,int y) {
 return x>=0&&x<7&&y>=0&&y<11&&(glyph[y]&(1u<<x))!=0;
}
static void text(int x,int y,const char*s,uint32_t c,int size){
 if(!s)return;
 for(int n=0;s[n]&&x<W;n++){
  int chr=(unsigned char)s[n];if(chr<32||chr>126)chr='?';
  const unsigned char*glyph=rift_font[chr-32];
  for(int yy=0;yy<11;yy++)for(int xx=0;xx<7;xx++){
    if(glyph_on(glyph,xx,yy))rect(x+xx*size,y+yy*size,size,size,c);
    else if(size==1){
      int edges=glyph_on(glyph,xx-1,yy)+glyph_on(glyph,xx+1,yy)
                +glyph_on(glyph,xx,yy-1)+glyph_on(glyph,xx,yy+1);
      int px=x+xx,py=y+yy;
      if(edges&&(unsigned)px<W&&(unsigned)py<H)
        pix[(size_t)py*sw+px]=blend_text(pix[(size_t)py*sw+px],c,edges>=2?74:43);
    }
  }
  x+=size*7;
 }
}"""+s[end:]
# consistent small cards, leave descriptive detail to inspect; don't go under footer
replace(""" wrapped_text(x+5,y+21+arth,card_description(raw),(w-10)/7,tall?4:2,selected?WHITE:MUTED);""",
""" int room=(y+h-4)-(y+21+arth);
 int maxrows=CLAMP(room/11,0,tall?4:2);
 if(maxrows>0)wrapped_text(x+5,y+21+arth,card_description(raw),(w-12)/7,maxrows,
                           selected?WHITE:MUTED);""")
# Draw enemy idle, recoil and attack lunge
replace(""" int hero_bob=wave(animation_tick,110,2);
 sprite(RIFT_HERO_X,RIFT_HERO_Y+hero_bob,RIFT_HERO_W,RIFT_HERO_H,rift_hero);""",
""" unsigned age=animation_tick>=fx_tick?animation_tick-fx_tick:999;
 int hero_bob=wave(animation_tick,110,2);
 int recoil=fx_player_damage>0&&age<14?(int)((14-age)*((age&1)?-1:1)/2):0;
 sprite(RIFT_HERO_X+recoil,RIFT_HERO_Y+hero_bob,RIFT_HERO_W,RIFT_HERO_H,rift_hero);""")
replace("""  int bob=wave(animation_tick+i*24u,96,2);""",
"""  int bob=wave(animation_tick+i*24u,96,3);
  if(fx_enemy_damage[i]>0&&age<12)ex+=((int)(12-age)*((age&1)?1:-1))/2;
  if(fx_player_damage>0&&age<12)ex-=(int)((12-age)*2/3);""")
replace(""" header(g,"BATTLE / CROSS PLAY  CIRCLE END TURN");""",
""" /* Fly-up damage and block text plus a small spray of combat sparks. */
 if(age<29) {
   char amount[28];
   if(fx_player_damage>0){
     snprintf(amount,sizeof(amount),"-%d",fx_player_damage);
     center(95,85-(int)(age/2),amount,RED,1);
   } else if(fx_heal>0){
     snprintf(amount,sizeof(amount),"+%d",fx_heal);
     center(95,85-(int)(age/2),amount,GREEN,1);
   } else if(fx_block>0){
     snprintf(amount,sizeof(amount),"+%d BLOCK",fx_block);
     center(95,78-(int)(age/2),amount,BLUE,1);
   }
   for(int i=0;i<g->enemy_count;i++)if(fx_enemy_damage[i]>0){
      int ex=g->enemy_count==1?337:(i==0?333:425);
      snprintf(amount,sizeof(amount),"-%d",fx_enemy_damage[i]);
      center(ex,90-(int)(age/2),amount,age<8?WHITE:RED,1);
      for(int k=0;k<12;k++){
        unsigned seed=particle_hash((unsigned)(i*97+k*2003));
        int dx=((int)(seed%47)-23)*(int)age/15;
        int dy=((int)((seed>>8)%39)-20)*(int)age/15;
        put(ex+dx,110+dy,((k+age)&1)?GOLD:RED);
      }
   }
 }
 header(g,"BATTLE / X PLAY   START EXPLAIN");""")
# status effects and damage preview under header in combat
replace(""" if(g->strength){snprintf(b,sizeof(b),"STR %d",g->strength);text(162,160,b,GOLD,1);}""",
""" if(g->strength){snprintf(b,sizeof(b),"STR %d",g->strength);text(162,160,b,GOLD,1);}
 int sx=17;
 if(g->weak){snprintf(b,sizeof(b),"WK %d",g->weak);text(sx,119,b,RED,1);sx+=49;}
 if(g->vulnerable){snprintf(b,sizeof(b),"VUL %d",g->vulnerable);text(sx,119,b,GOLD,1);sx+=63;}
 if(g->poison){snprintf(b,sizeof(b),"PSN %d",g->poison);text(sx,119,b,GREEN,1);sx+=59;}
 if(g->dexterity){snprintf(b,sizeof(b),"DEX %d",g->dexterity);text(sx,119,b,BLUE,1);}
""")
replace("""  if(e->block){snprintf(b,sizeof(b),"BLK %d",e->block);center(ex,163,b,BLUE,1);}""",
"""  if(e->block){snprintf(b,sizeof(b),"BLK %d",e->block);center(ex,163,b,BLUE,1);}
  if(e->vulnerable||e->weak||e->poison){
   snprintf(b,sizeof(b),"%s%s%s",e->vulnerable?"VUL ":"",e->weak?"WEAK ":"",e->poison?"PSN":"");
   center(ex,130,b,GOLD,1);
  }""")
# clear END selection and potions
replace(""" if(g->selection>=visible)start=g->selection-visible+1;""",
""" if(g->selection>=visible&&g->selection<g->hand_count)start=g->selection-visible+1;
 else if(g->selection>=g->hand_count&&g->hand_count>visible)start=g->hand_count-visible;""")
replace("""  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?180+wave(animation_tick,60,1):185;""",
"""  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?175+wave(animation_tick,60,1):181;""")
replace("""  card_render(x,y,70,selected?72:67,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));""",
"""  card_render(x,y,70,selected?78:71,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));""")
replace(""" button(426,198,48,40,"END",0);
 if(g->potion_count>0){snprintf(b,sizeof(b),"POT %d",g->potion_count);text(426,241,b,GREEN,1);}
 footer("D-PAD CARD/TARGET    X PLAY    O END    TRIANGLE POTION");""",
""" button(414,191,61,44,"END",g->selection>=g->hand_count);
 if(g->potion_count>0){
    snprintf(b,sizeof(b),"POTS %d",g->potion_count);
    text(414,241,b,GREEN,1);
 }
 footer("LEFT/RIGHT CARDS+END  X SELECT   START INFO  TRI POTIONS");""")
# title selection and help
replace(""" button(177,151,134,30,"X - NEW RUN",1);
 button(177,187,134,27,"SQUARE - HELP",0);""",
""" button(162,149,161,32,"BEGIN ASCENT",g->selection==0);
 button(162,185,161,30,"HOW TO PLAY",g->selection==1);""")
replace('footer("X BEGIN ASCENT     SQUARE CONTROLS");','footer("UP/DOWN SELECT     X CONFIRM");')
# Map edges from actual route masks not every possible route
replace("""if(r<MAP_ROWS-1){for(int d=0;d<3;d++)if(abs(d-c)<=1)line(x,y-12,79+d*89,y-25,RGB(101,77,53));}""",
"""if(r<MAP_ROWS-1){
     unsigned routes=game_route_mask(g,r,c);
     for(int d=0;d<3;d++)if(routes&(1u<<d))
       line(x,y-12,79+d*89,y-25,
            (r==g->floor-1&&c==g->lane)?GOLD:RGB(112,84,61));
    }""")
replace("""int x=79+c*89,n=g->map[r][c],done=r<g->floor,available=r==g->floor&&(r==0||abs(c-g->lane)<=1);""",
"""int x=79+c*89,n=g->map[r][c],done=r<g->floor,
       available=r==g->floor&&game_route_available(g,c);""")
replace("""   circle(x,y,9,done?RGB(73,76,77):col);""",
"""   circle(x,y,9,done?RGB(73,76,77):(r==g->floor&&!available)?RGB(71,66,64):col);""")
replace('footer("LEFT/RIGHT SELECT   X ENTER    SQUARE VIEW DECK");',
        'footer("ROADS BRANCH / CHOOSE LIT NODE    X ENTER   SQUARE DECK");')
# information overlays (keep screen readable, no layout overlap)
anchor="void game_render_frame(const Game*g,uint32_t*pixels,int stride,unsigned tick){"
inspector="""static void inspector(const Game*g){
 if(!overlay_kind)return;
 dark_rect(0,29,W,226,214);
 gilded_panel(20,40,440,203,1);
 const char *tabs[]={"CARD","FOE","STATUS","POTIONS"};
 for(int i=0;i<4;i++){
    int px=30+i*106;
    rect(px,48,96,19,overlay_page==i?RGB(115,75,42):RGB(34,46,59));
    center(px+48,52,tabs[i],overlay_page==i?GOLD:WHITE,1);
 }
 char b[112];
 if(overlay_page==0){
   if(g->hand_count>0&&g->selection<g->hand_count){
      int raw=g->hand[g->selection],id=CARD_ID(raw);
      center(240,79,cards[id].name,GOLD,2);
      wrapped_text(43,115,card_description(raw),55,3,WHITE);
      int n=game_effective_damage(g,raw,g->target);
      if(n>0){
         snprintf(b,sizeof(b),"PREDICTED DAMAGE: %d",n);
         text(43,164,b,RED,1);
      }
      text(43,186,"Strength: +1 damage per hit.",MUTED,1);
      text(43,201,"Weak: -25%, Vulnerable: +50%.",MUTED,1);
   }else text(43,99,"END TURN selected. Press X to confirm.",WHITE,1);
 }else if(overlay_page==1){
   if(g->enemy_count){
      const Enemy *e=&g->enemies[g->target];
      center(240,79,e->name,GOLD,2);
      snprintf(b,sizeof(b),"%s  /  %d",game_intent_title(e->intent),e->intent_value);
      center(240,109,b,RED,1);
      wrapped_text(43,135,game_intent_description(e->intent),55,5,WHITE);
   }
 }else if(overlay_page==2){
   snprintf(b,sizeof(b),"HP %d / %d     BLOCK %d",g->hp,g->maxhp,g->block);
   text(43,83,b,WHITE,1);
   snprintf(b,sizeof(b),"STRENGTH %+d  DEXTERITY %+d",g->strength,g->dexterity);text(43,104,b,GOLD,1);
   snprintf(b,sizeof(b),"WEAK %d  VULNERABLE %d  POISON %d",g->weak,g->vulnerable,g->poison);
   text(43,126,b,WHITE,1);
   wrapped_text(43,154,"Weak reduces attack by 25%. Vulnerable increases damage taken by 50%. Poison damages you at turn end.",53,5,MUTED);
 }else {
    for(int i=0;i<g->potion_count;i++){
       int y=82+i*44;int selected=(i==overlay_potion);
       if(selected)rect(33,y-3,410,39,RGB(66,66,50));
       snprintf(b,sizeof(b),"%d. %s",i+1,game_potion_name(g->potions[i]));
       text(42,y,b,selected?GOLD:WHITE,1);
       wrapped_text(52,y+14,game_potion_description(g->potions[i]),50,2,MUTED);
    }
    if(g->potion_count==0)center(240,113,"NO POTIONS IN YOUR BELT",MUTED,1);
 }
 footer(overlay_page==3?"LEFT/RIGHT TABS  UP/DOWN POTION  X USE   START CLOSE":
                          "LEFT/RIGHT TABS  UP/DOWN TARGET  START/CIRCLE CLOSE");
}
static void transition_overlay(void){
 if(!fx_transition||animation_tick<fx_transition_tick)return;
 unsigned age=animation_tick-fx_transition_tick;
 if(age>=24){fx_transition=0;return;}
 /* Fade a new event in rather than popping to it instantly. */
 int darkness=(int)((24-age)*205/24);
 dark_rect(0,28,W,227,darkness);
 for(int i=0;i<12;i++){
   unsigned seed=particle_hash(i*99991u+fx_transition_tick);
   int x=(int)(seed%480u);
   int y=49+(int)((seed>>12)%169u)-(int)age;
   put(x,y,GOLD);
 }
}
"""
replace(anchor,inspector+anchor)
replace(""" case HELP:help_screen(g);break;
 }
}""",
""" case HELP:help_screen(g);break;
 }
 transition_overlay();
 if(g->screen==COMBAT)inspector(g);
}""")
p.write_text(s)
print("Applied combat visuals, anti-aliasing, map paths, help overlay, end turn UI.")
