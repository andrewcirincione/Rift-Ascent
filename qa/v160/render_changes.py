from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'render.c';s=p.read_text().replace('#include "rift151_art.h"','#include "rift151_art.h"\n#include "rift160_art.h"')
s=s.replace('static void r134_card_art(','''static void r160_sprite(int x,int y,int w,int h,const Rift160Sprite*src,int combat){
 for(int yy=0;yy<h;yy++){int dy=y+yy;if(dy<(combat?54:0)||dy>=(combat?174:H))continue;
  for(int xx=0;xx<w;xx++){int dx=x+xx;if((unsigned)dx>=W)continue;uint32_t c=src->palette[src->pixels[(yy*96/h)*144+xx*144/w]];int a=c>>24;if(a)pix[dy*sw+dx]=r134_mix(pix[dy*sw+dx],c,a);}}
}
static void r134_card_art(''')
s=s.replace('if(id>=NINJA_FIRST&&id<NINJA_LAST){\n  int iw=', '''if(id>=REGENT_FIRST&&id<EXTRA_LAST){
  int iw=160,ih=112,crop=CLAMP(iw*h/w,35,ih),top=(ih-crop)/6;const uint8_t*src=r160_cards[id-REGENT_FIRST];const uint16_t*pal=r160_card_palettes[id-REGENT_FIRST];
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){int dx=x+xx,dy=y+yy;if((unsigned)dx>=W||(unsigned)dy>=H)continue;pix[dy*sw+dx]=r134_565(pal[src[(top+yy*crop/h)*iw+xx*iw/w]]);}return;
 }
 if(id>=NINJA_FIRST&&id<NINJA_LAST){
  int iw=''',1)
s=s.replace('"TRIANGLE","TRI"','"TRIANGLE","TRI","SELECT"').replace('ids[]={2,1,3,2,0,1,4,4}','ids[]={2,1,3,2,0,1,4,4,5}').replace('for(int i=0;i<8;i++){int n=(int)strlen(names[i]);','for(int i=0;i<9;i++){int n=(int)strlen(names[i]);')
s=s.replace('r135_start,r1311_triangle};','r135_start,r1311_triangle,r160_select};').replace('(id==3?22:14)','(id==5?40:id==3?22:14)').replace('int w=id==3?22:14;','int w=id==5?40:id==3?22:14;')
s=s.replace('if(id>=53&&id<63)r134_blit','if(id==63)r134_blit(x,y,size,size,r160_crown,80,80,255);\n else if(id>=53&&id<63)r134_blit')
needle=' if(id>=NINJA_FIRST&&(game_card_is_sly'
pos=s.index(needle)
s=s[:pos]+''' int star=game_card_star_cost(card_combat_context,raw);
 if(star!=0){char sb[12];if(star==-1)snprintf(sb,sizeof(sb),"X");else snprintf(sb,sizeof(sb),"%d",star);int sx=x+inset+orb+2;
  r134_blit(sx,arty,orb,orb,r160_star,40,40,255);r135_shadow_text(sx+(orb-r134_width(sb,&r134_f9b))/2,arty+(small?1:4),sb,card_combat_context&&star>card_combat_context->regent.stars?RGB(255,145,105):RGB(255,241,172),&r134_f9b);
 }
'''+s[pos:]
s=s.replace('if(selected&&!active&&card_combat_context&&!small&&energy>card_combat_context->energy){','if(selected&&!active&&card_combat_context&&!small&&(energy>card_combat_context->energy||star>card_combat_context->regent.stars)){')
s=s.replace('snprintf(missing,sizeof(missing),"NEED %d MORE ENERGY",energy-card_combat_context->energy);','if(energy>card_combat_context->energy)snprintf(missing,sizeof(missing),"NEED %d MORE ENERGY",energy-card_combat_context->energy);else snprintf(missing,sizeof(missing),"NEED %d MORE STARS",star-card_combat_context->regent.stars);')
s=s.replace('r134_blit(4,199,46,46,g->energy==0?r140_energy_empty:r134_energy,46,46,255);\n char b[16];snprintf(b,sizeof(b),"%d",g->energy);r134_center(27,212,b,WHITE,&r134_f18b);','''char b[16];
 if(g->character==CHAR_REGENT){r134_blit(8,180,38,38,g->energy==0?r140_energy_empty:r134_energy,46,46,255);snprintf(b,sizeof(b),"%d",g->energy);r134_center(27,190,b,WHITE,&r134_f18b);
  r134_blit(9,219,36,36,r160_star,40,40,g->regent.stars?255:135);snprintf(b,sizeof(b),"%d",g->regent.stars);const RiftFont*f=g->regent.stars>99?&r134_f9b:&r134_f18b;r135_shadow_text(27-r134_width(b,f)/2,231,b,RGB(255,246,190),f);
 }else{r134_blit(4,199,46,46,g->energy==0?r140_energy_empty:r134_energy,46,46,255);snprintf(b,sizeof(b),"%d",g->energy);r134_center(27,212,b,WHITE,&r134_f18b);}''')
s=s.replace('cost!=-2&&(cost==-1||cost<=g->energy));','cost!=-2&&(cost==-1||cost<=g->energy)&&game_card_star_cost(g,g->hand[i])<=g->regent.stars);',1)
s=s.replace('cost!=-2&&(cost==-1||cost<=g->energy));','cost!=-2&&(cost==-1||cost<=g->energy)&&game_card_star_cost(g,g->hand[g->selection])<=g->regent.stars);',1)
s=s.replace('card_combat_context=0;footer(need_energy?', 'int need_stars=g->selection<g->hand_count&&game_card_star_cost(g,g->hand[g->selection])>g->regent.stars;\n card_combat_context=0;footer(need_stars?"NOT ENOUGH STARS   O END   SQ DECK   START STATUS / INVENTORY":need_energy?')
a=s.index('static void character_screen(');b=s.index('static void ninja_choice_screen(',a)
s=s[:a]+'''static void character_screen(const Game*g){
 Game display=*g;display.hp=display.maxhp=g->selection==1?70:g->selection==2?75:80;display.gold=g->selection?99:95;
 header(&display,g->endless?"ENDLESS / CHOOSE YOUR CHARACTER":"ASCENT / CHOOSE YOUR CHARACTER");
 const char*names[]={"WARDEN","NIGHTSHADE","REGENT"};const char*hp[]={"80 HP / 3 ENERGY","70 HP / 3 ENERGY","75 HP / 3 ENERGY"};
 const char*build[]={"Strength / Block","Shivs / Poison","Stars / Forge"};const char*gift[]={"Heal 6 after victory","Draw 2 at combat start","Start combat: 3 Stars"};
 for(int i=0;i<3;i++){int x=12+i*156,chosen=g->selection==i;gilded_panel(x,47,144,194,chosen);
  r134_center(x+72,57,names[i],chosen?GOLD:WHITE,&r138_title10);
  if(i)r160_sprite(x,78,144,96,i==1?&r160_nightshade[0]:&r160_regent[0],0);
  else{int w=140,h=80;for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){uint16_t c=rift_hero[(yy*RIFT_HERO_H/h)*RIFT_HERO_W+xx*RIFT_HERO_W/w];if(c!=RIFT_TRANSPARENT_565)pix[(94+yy)*sw+x+2+xx]=r134_565(c);}}
  r134_center(x+72,188,hp[i],WHITE,&r134_f9b);r134_center(x+72,204,build[i],MUTED,&r134_f9);r134_center(x+72,220,gift[i],GOLD,&r134_f8);
  if(chosen)r134_blit(x+51,36,42,28,r138_choice,42,28,255);else dark_rect(x+7,74,130,164,65);
 }
 footer("LEFT/RIGHT CHOOSE   X BEGIN   O BACK");
}
static void regent_choice_screen(const Game*g){
 header(g,game_regent_choice_title(g));int count=game_regent_choice_count(g),first=g->selection/6*6;
 char b[80];snprintf(b,sizeof(b),"%d more to choose",g->regent.choice_remaining);r134_center(240,48,b,GOLD,&r134_f9);
 for(int i=first;i<count&&i<first+6;i++){int n=i-first,x=17+n%3*103,y=69+n/3*86,eligible=game_regent_choice_eligible(g,i);card_render(x,y,97,80,game_regent_choice_raw(g,i),i==g->selection,eligible);if(!eligible)dark_rect(x+7,y+9,83,62,115);if(i==g->selection)r134_blit(x+31,y-7,34,22,r138_choice,42,28,255);}
 if(count){int raw=game_regent_choice_raw(g,g->selection);R135CardLayout l=r150_hand_preview_layout(raw);card_render(473-l.width,70,l.width,l.height,raw,1,1);}
 footer("D-PAD CHOOSE   X CONFIRM   O FINISH   SELECT PAUSE");
}
'''+s[b:]
a=s.index(' for(int i=0;i<3;i++){',s.index('static void title_screen('));b=s.index(' if(profile_data',a)
s=s[:a]+''' const char*buttons[]={"RESUME RUN","BEGIN ASCENT","ENDLESS MODE","HOW TO PLAY"};
 for(int i=0;i<4;i++){int y=128+i*28,selected=g->selection==i,enabled=i!=0||game_resume_available();
  r134_blit(279,y,176,27,r138_button_on,176,32,enabled?(selected?255:145):80);
  r134_center(367,y+9,buttons[i],!enabled?RGB(95,98,102):selected?RGB(255,234,184):WHITE,&r138_title9);
 }
'''+s[b:]
s=s.replace('g->selection==1?"UNLIMITED ACTS / RISING DIFFICULTY":"ONE WARRIOR / THREE ACTS"','g->selection==2?"UNLIMITED ACTS / RISING DIFFICULTY":g->selection==0&&!game_resume_available()?"NO SAVED RUN":"THREE WARRIORS / THREE ACTS"').replace('TWO CHARACTERS','THREE CHARACTERS')
s=s.replace('case NINJA_CHOICE:ninja_choice_screen(g);break;','case NINJA_CHOICE:ninja_choice_screen(g);break;\n case REGENT_CHOICE:regent_choice_screen(g);break;')
s=s.replace('center(240,112,"X SAVE & QUIT",WHITE,1);\n   center(240,137,"CIRCLE / SELECT RESUME",WHITE,1);','''r134_blit(164,110,14,14,r135_cross,14,14,255);r134_text(186,113,"SAVE & QUIT",WHITE,&r134_f9b);
   r134_blit(139,137,14,14,r135_circle,14,14,255);r134_text(161,140,"/",WHITE,&r134_f9b);r134_blit(173,137,40,14,r160_select,40,14,255);r134_text(226,140,"RESUME",WHITE,&r134_f9b);''')
s=s.replace('g->screen==NINJA_CHOICE||g->screen==RELIC_VIEW','g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE||g->screen==RELIC_VIEW')
s+='''\nvoid game_render_reset_fx(void){r137_count=r137_duration=r1311_finishing=r138_death_active=r137_held_valid=0;rift120_played_card=rift120_auto_card=-1;r136_play_state_valid=0;hand_exchange=fx_transition=victory_replay=effect_kind=0;r140_potion_gained=-1;r1310_potion_notice[0]=0;}\n'''
p.write_text(s)
p=q/'combat_fx.inc';s=p.read_text()
a=s.index(' if(g->character==CHAR_NIGHTSHADE){',s.index('static void r137_scene'));b=s.index(' }else if(g->hp<=0&&r138_death_active){',a)
s=s[:a]+''' if(g->character==CHAR_NIGHTSHADE||g->character==CHAR_REGENT){
  int frame=0;
  if(g->hp<=0&&r138_death_active)frame=16+CLAMP((age-r138_death_start)/12,0,3);
  else if(pose==1){int local=pa>=0?r137_local_time(age-r137_at[pa],game_impact_tier(r137_events[pa].amount)):pre;frame=pa<0?5:local<3?6:local<13?7:9;}
  else if(pose==2)frame=11;
  else if(pose==3){int local=pp>=0?age-r137_at[pp]:pre;frame=local<10?13:14;}
  else if(player_flash)frame=15;
  else frame=(animation_tick/32u)%5;
  if(g->character==CHAR_REGENT)r160_particles(g,animation_tick/4,0);
  r160_sprite(48+dx,78,144,96,g->character==CHAR_NIGHTSHADE?&r160_nightshade[frame]:&r160_regent[frame],1);
  if(g->character==CHAR_REGENT)r160_particles(g,animation_tick/4,1);
'''+s[b:]
pos=s.index('static void r137_scene(')
s=s[:pos]+'''static int r160_star_count(int stars){return CLAMP(stars,0,15)*2;}
static void r160_particles(const Game*g,unsigned tick,int front){
 if(g->hp<=0)return;int level=CLAMP(g->regent.stars,0,15),count=r160_star_count(level);
 for(int i=0;i<count;i++){if((i%3==0)!=front)continue;unsigned seed=particle_hash(160u+i*8191u);int life=24+seed%24,age=(tick+(seed>>8))%life;
  int x=80+(seed>>13)%78+wave(tick+i*5,32,4),y=169-age*(50+level)/life,size=3+(seed>>20)%3+level/8,opacity=(life-age)*230/life;
  if(age<4)opacity=opacity*age/4;r137_blit(x,y,size,size,r160_spark,24,24,opacity,0);
 }
}
'''+s[pos:]
s=s.replace('return c==3||c==10||c==14||c==19||c==36||c==37||c==70;','return c==3||c==10||c==14||c==19||c==36||c==37||c==70||c==R_ASTRAL_PULSE||c==R_CRASH_LANDING||c==R_CRUSH_UNDER||c==R_DYING_STAR||c==R_METEOR_SHOWER||c==R_RADIATE||c==R_SEVEN_STARS||c==C_DRAMATIC_ENTRANCE;')
s=s.replace('if(g->character!=CHAR_NIGHTSHADE)r137_blit','if(g->character==CHAR_WARDEN)r137_blit')
p.write_text(s)
