from pathlib import Path
s=Path(__file__).parent/'source';p=s/'render.c';t=p.read_text().replace('#include "rift150_art.h"','#include "rift150_art.h"\n#include "rift150b_art.h"')
t=t.replace('static void r134_card_art(int x,int y,int w,int h,int id){','''static void r134_card_art(int x,int y,int w,int h,int id){
 if(id>=NINJA_FIRST&&id<NINJA_LAST){
  int iw=180,ih=120,crop=CLAMP(iw*h/w,35,ih),top=(ih-crop)/5;
  const uint16_t*src=r150b_cards[id-NINJA_FIRST];
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;pix[dy*sw+dx]=r134_565(src[(top+yy*crop/h)*iw+xx*iw/w]);}return;
 }
''',1)
t=t.replace('g->screen==CARD_CHOICE||g->screen==RELIC_VIEW','g->screen==CARD_CHOICE||g->screen==NINJA_CHOICE||g->screen==RELIC_VIEW',1)
# Expose acquired Sly / Inky marks without displacing the independent mod seal.
t=t.replace(' int mod=CARD_MOD(raw);if(mod>0',''' if(id>=NINJA_FIRST&&(game_card_is_sly(card_combat_context,raw)||(raw&CARD_INKY))){
  const char*mark=(raw&CARD_INKY)?"INKY":"SLY";int mw=r134_width(mark,&r134_f8)+4,mx=x+w/2-mw/2,my=arty+arth-9;
  dark_rect(mx,my,mw,9,220);r134_center(x+w/2,my,mark,RGB(171,225,189),&r134_f8);
 }
 int mod=CARD_MOD(raw);if(mod>0''',1)
# Character chooser uses actual new model rather than placeholder artwork.
a=t.index('static void blessing_screen(')
new='''static void character_screen(const Game*g){
 header(g,g->endless?"ENDLESS / CHOOSE YOUR CHARACTER":"ASCENT / CHOOSE YOUR CHARACTER");
 for(int i=0;i<2;i++){
  int x=20+i*230,chosen=g->selection==i;gilded_panel(x,47,210,194,chosen);
  r134_center(x+105,57,i?"NIGHTSHADE":"WARDEN",chosen?GOLD:WHITE,&r138_title10);
  if(i)r134_blit(x+25,76,160,119,r150b_ninja[0],134,100,255);
  else{int w=174,h=99;for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){uint16_t c=rift_hero[(yy*RIFT_HERO_H/h)*RIFT_HERO_W+xx*RIFT_HERO_W/w];if(c!=RIFT_TRANSPARENT_565)pix[(88+yy)*sw+x+18+xx]=r134_565(c);}}
  r134_center(x+105,193,i?"70 HP   3 ENERGY":"80 HP   3 ENERGY",WHITE,&r134_f9b);
  r134_center(x+105,209,i?"Shivs / Poison / Discard":"Strength / Block / Exhaust",MUTED,&r134_f9);
  r134_center(x+105,224,i?"Draw 2 extra cards at combat start":"Heal 6 HP after each victory",GOLD,&r134_f8);
  if(chosen)r134_blit(x+84,36,42,28,r138_choice,42,28,255);else dark_rect(x+7,73,196,164,90);
 }
 footer("LEFT/RIGHT CHOOSE   X BEGIN   O BACK");
}
static void ninja_choice_screen(const Game*g){
 header(g,game_ninja_choice_title(g));char b[96];
 if(g->ninja.choice_kind==1)snprintf(b,sizeof(b),"Choose %d more. Sly cards play for free when discarded.",g->ninja.choice_remaining);
 else if(g->ninja.choice_kind==4)snprintf(b,sizeof(b),"Retain up to %d more cards. Circle finishes selection.",g->ninja.choice_remaining);
 else snprintf(b,sizeof(b),g->ninja.choice_kind==2?"Give one Skill Sly for this turn.":"Receive 3 independent copies next turn.");
 r134_center(240,47,b,GOLD,&r134_f9);
 int first=g->selection/6*6;
 for(int i=first;i<g->hand_count&&i<first+6;i++){int off=i-first,x=17+off%3*103,y=69+off/3*86,eligible=game_ninja_choice_eligible(g,i);card_render(x,y,97,80,g->hand[i],i==g->selection,eligible);if(!eligible)dark_rect(x+7,y+9,83,62,115);if(i==g->selection)r134_blit(x+31,y-7,34,22,r138_choice,42,28,255);}
 if(g->hand_count){int raw=g->hand[g->selection];R135CardLayout l=r150_hand_preview_layout(raw);card_render(473-l.width,70,l.width,l.height,raw,1,1);}
 footer(g->ninja.choice_kind==4?"D-PAD CHOOSE   X RETAIN   O FINISH":"D-PAD CHOOSE   X CONFIRM   SELECT SAVE & QUIT");
}
'''
t=t[:a]+new+t[a:]
a=t.index('static void blessing_screen(');b=t.index('static void reward_preview_screen',a)
t=t[:a]+'''static void blessing_screen(const Game*g){
 char title[60];snprintf(title,sizeof(title),"ACT %d / CHOOSE YOUR BLESSING",g->act);header(g,title);
 for(int i=0;i<3;i++){int x=22+i*149,id=g->reward[i],chosen=i==g->selection;
  gilded_panel(x,48,140,196,chosen);
  if(id>=1000){int raw=id-1000;r134_card_art(x+19,72,102,55,raw);r134_wrap(x+13,135,114,2,13,cards[raw].name,GOLD,&r138_title9,0);r134_wrap(x+13,166,114,6,11,raw==N_SUPPRESS?"Transform Neutralize into Suppress, or gain it if you have no Neutralize.":"Add Wraith Form to your deck. Gain Intangible; lose Dexterity each turn.",WHITE,&r134_f9,0);}
  else{r134_blit(x+49,69,42,42,r134_utility_icons[1],40,40,255);r134_wrap(x+13,119,114,2,13,relic_defs[id].name,GOLD,&r138_title9,0);r134_wrap(x+13,154,114,7,11,relic_defs[id].desc,WHITE,&r134_f9,0);}
  if(chosen)r134_blit(x+49,39,42,28,r138_choice,42,28,255);else dark_rect(x+7,55,126,182,70);
 }
 footer("LEFT/RIGHT CHOOSE   X ACCEPT BLESSING");
}
'''+t[b:]
t=t.replace(' case TITLE:title_screen(g);break;',' case TITLE:title_screen(g);break;\n case CHARACTER_SELECT:character_screen(g);break;\n case NINJA_CHOICE:ninja_choice_screen(g);break;')
p.write_text(t)
p=s/'combat_fx.inc';t=p.read_text();needle=' if(g->hp<=0&&r138_death_active){';a=t.index(needle)
t=t[:a]+''' if(g->character==CHAR_NIGHTSHADE){
  int frame=0;
  if(g->hp<=0&&r138_death_active)frame=16+CLAMP((age-r138_death_start)/12,0,3);
  else if(pose==1){int local=pa>=0?r137_local_time(age-r137_at[pa],game_impact_tier(r137_events[pa].amount)):pre;frame=4+(local<3?0:local<8?1:local<14?2:3);}
  else if(pose==2)frame=9;
  else if(pose==3){int local=pp>=0?age-r137_at[pp]:pre;frame=12+(local<4?0:local<10?1:local<13?2:3);}
  else frame=(animation_tick/32u)%4;
  r138_hero_clip=1;r137_blit(54+dx,74,134,100,r150b_ninja[frame],134,100,255,0);r138_hero_clip=0;
 }else'''+t[a:]
p.write_text(t)
# Checkpoints copy only canonical source files. Art headers come from prepare_art.
q=Path(__file__).parent
(q/'build_source.py').write_text('''from pathlib import Path
import shutil,re
q=Path(__file__).parent;root=q.parents[1]
for folder in ['GAME/src','portable_build']:
 for p in (q/'source').iterdir():
  if p.name=='psp_start.c' and folder=='GAME/src':continue
  shutil.copyfile(p,root/folder/p.name)
p=root/'portable_build/Makefile';t=p.read_text();t=re.sub(r'PSP_EBOOT_TITLE = .*','PSP_EBOOT_TITLE = RIFT ASCENT 1.5b',t);p.write_text(t)
(root/'portable_build/PARAM.SFO').unlink(missing_ok=True)
''')
