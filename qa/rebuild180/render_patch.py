from pathlib import Path
r=Path(__file__).parent/'source'
p=r/'render.c';s=p.read_text().replace('#include "sound.h"','#include "sound.h"\n#include "ascent_art.h"\n#include "ascent_dialogue.h"')
s=s.replace('g->screen==STORY_ACT||g->screen==STORY_BOSS||g->screen==STORY_ACT_BEFORE_BLESSING;','g->screen==STORY_ACT||g->screen==STORY_BOSS||g->screen==STORY_ACT_BEFORE_BLESSING||ascent_story_screen(g->screen);')
s=s.replace('static void r134_card_art(int x,int y,int w,int h,int id){','static void r134_card_art(int x,int y,int w,int h,int id){\n if(id>=334&&id<394){art_blit(x,y,w,h,ascent_card_art[id-334],180,120);return;}')
s=s.replace('if(id==63)r134_blit','if(id>=64&&id<74)r134_blit(x,y,size,size,ascent_relic_art[id-64],80,80,255);\n else if(id==63)r134_blit')
s=s.replace('static void r140_potion_card(', '#include "ascent_ui.inc"\nstatic void r140_potion_card(')
s=s.replace('r134_blit(42,y+8,36,36,r134_utility_icons[0],40,40,255);','ascent_potion_icon(42,y+8,36,g->potions[i]);')
s=s.replace('static void title_screen(const Game*g){', 'static void title_screen(const Game*g){')
a=s.index(' const char*buttons[]={"RESUME RUN"',s.index('static void title_screen'));b=s.index('\n footer("UP/DOWN CHOOSE',a)
s=s[:a]+''' const char*buttons[]={"RESUME RUN","BEGIN ASCENT","UNLOCKS","OPTIONS"};
 for(int i=0;i<4;i++){int y=126+i*28,selected=CLAMP(g->selection,0,3)==i,enabled=i!=0||game_resume_available();
  r134_blit(279,y,176,26,r138_button_on,176,32,enabled?(selected?255:145):80);
  r161_centered_text(289,y+2,156,22,buttons[i],&r138_title9,!enabled?RGB(95,98,102):selected?RGB(255,234,184):WHITE);
 }
 const AscentAccount*a=ascent_profile();char b[96];
 dark_rect(15,181,235,68,175);
 snprintf(b,sizeof(b),"CARDS  W %d/20   V %d/20   P %d/20",ascent_popcount(a->cards[0]),ascent_popcount(a->cards[1]),ascent_popcount(a->cards[2]));r134_text(25,191,b,GOLD,&r134_f8);
 snprintf(b,sizeof(b),"POTIONS  %d/20     RELICS  %d/10",ascent_popcount(a->potions),ascent_popcount(a->relics));r134_text(25,208,b,WHITE,&r134_f8);
 r134_text(25,230,"RIFT ASCENT 1.8",MUTED,&r134_f8);
''' +s[b:]
s=s.replace('else dark_rect(x+7,74,130,164,65);','else dark_rect(x+7,74,130,164,65);\n  ascent_badges(x,74+dy,i);')
s=s.replace('"VICTORY / THE HOLLOW KING FALLS"','"ASCENT COMPLETE"').replace('if(g->screen==VICTORY)r134_blit(110,48,260,80,r140_victory,324,100,255);','if(g->screen==VICTORY){r134_center(240,74,g->ascent.ascended?"THE TRUE ASCENT":"THE SPIRE HAS FALLEN",GOLD,&r134_f18b);}')
s=s.replace('SQ DECK   TRI POTIONS','TRI POTIONS   SQ DECK').replace('SQ DECK  TRI POTIONS','TRI POTIONS  SQ DECK')
s=s.replace('r134_blit(126,88,228,70,r140_victory,324,100,255);','/* Preserve the finisher hold/fade without the victory graphic. */')
# New completion screens retain the full stable combat finisher timeline.
s=s.replace('g->screen==VICTORY||g->screen==DEFEAT)&&victory_replay','g->screen==VICTORY||g->screen==DEFEAT||g->screen==ASCENT_ENDING||g->screen==ASCENT_TRUE_END||g->screen==ASCENT_SAVE_ERROR)&&victory_replay')
s=s.replace('after->screen==VICTORY||after->screen==DEFEAT)', 'after->screen==VICTORY||after->screen==DEFEAT||after->screen==ASCENT_ENDING||after->screen==ASCENT_TRUE_END||after->screen==ASCENT_SAVE_ERROR)')
s=s.replace(' if(g->screen==STORY_ACT||g->screen==STORY_BOSS||g->screen==STORY_ACT_BEFORE_BLESSING){\n  rect', ' if(ascent_story_screen(g->screen)){ascent_scene_view(g);return;}\n if(g->screen==STORY_ACT||g->screen==STORY_BOSS||g->screen==STORY_ACT_BEFORE_BLESSING){\n  rect')
s=s.replace('r162_acts[g->act>3?3:CLAMP(g->act-1,0,2)]','g->act==4&&g->ascent.secret?ascent_act4_title:r162_acts[g->act>3?3:CLAMP(g->act-1,0,2)]')
s=s.replace('if(g->endless)r134_center(240,235,"ENDLESS ASCENT",GOLD,&r138_title9);','if(g->endless)r134_center(240,235,g->act==4&&g->ascent.secret?"BEFORE THE FIRST THRONE":"ENDLESS ASCENT",GOLD,&r138_title9);')
s=s.replace('case TITLE:title_screen(g);break;', '''case TITLE:title_screen(g);break;
 case ASCENT_SLOTS_VIEW:ascent_slots_view(g);break;
 case ASCENT_OVERWRITE:ascent_overwrite_view(g);break;
 case ASCENT_OPTIONS:ascent_options_view(g);break;
 case ASCENT_COLLECTION:ascent_collection_view(g);break;
 case ASCENT_CARDS:case ASCENT_POTIONS:case ASCENT_RELIC:ascent_unlock_view(g);break;
 case ASCENT_SAVE_ERROR:ascent_heading("UNLOCKS NOT YET SAVED");r134_wrap(70,95,340,4,15,"The completion is waiting to be saved. Check free space on the Memory Stick and press X to retry.",WHITE,&r134_f10,0);footer("X RETRY SAVE");break;''')
s=s.replace('g->screen==TITLE||g->screen==NEW_RUN_CONFIRM||g->screen==HELP)', 'g->screen==TITLE||g->screen==NEW_RUN_CONFIRM||g->screen==HELP||g->screen==ASCENT_SLOTS_VIEW||g->screen==ASCENT_OVERWRITE||g->screen==ASCENT_OPTIONS||g->screen==ASCENT_COLLECTION)')
s=s.replace('static const uint16_t *rift130_fight(const Game *g){','static const uint16_t *rift130_fight(const Game *g){\n if(g->act>=4)return g->act==4&&g->ascent.secret&&g->battle_type==NODE_BOSS?ascent_unmaker_arena:ascent_act4_fight;')
s=s.replace('const uint16_t *src=r162_world[act][1];','const uint16_t *src=g->act>=4?ascent_act4_map:r162_world[act][1];')
# A narrower inset stays inside the decorative frame at any maximum HP.
s=s.replace('rect(x+36,y+13,89,9','rect(x+39,y+14,82,7').replace('rect(x+36,y+13,(int)(89LL*CLAMP(g->hp,0,g->maxhp)/g->maxhp),9','rect(x+39,y+14,(int)(82LL*CLAMP(g->hp,0,g->maxhp)/g->maxhp),7')
p.write_text(s)
p=r/'combat_fx.inc';s=p.read_text().replace('rift_sound_effect(10);','/* No victory fanfare. */')
s=s.replace('after->screen==VICTORY||after->screen==REWARD||after->screen==RELIC_RESULT;', 'after->screen==VICTORY||after->screen==REWARD||after->screen==RELIC_RESULT||after->screen==ASCENT_ENDING||after->screen==ASCENT_TRUE_END||after->screen==ASCENT_SAVE_ERROR;')
s=s.replace('after->screen==VICTORY)&&r137_duration','after->screen==VICTORY||after->screen==ASCENT_ENDING||after->screen==ASCENT_TRUE_END||after->screen==ASCENT_SAVE_ERROR)&&r137_duration')
s=s.replace('if(b>=0){iw=r170_boss_size','if(art==44){iw=160;ih=192;rgba=ascent_unmaker;}\n else if(b>=0){iw=r170_boss_size')
s=s.replace('int w=boss>=0?', 'int w=art==44?160*h/192:boss>=0?').replace('||art==35||art==28','||art==33||art==28')
s=s.replace('if(boss>=0)r170_rgba_actor','if(art==44)r170_rgba_actor(ex-w/2,y,w,h,ascent_unmaker,160,192,tilt,opacity,flash);\n  else if(boss>=0)r170_rgba_actor')
p.write_text(s)
p=r/'ascent_flow.inc';s=p.read_text().replace('if(b==BTN_LEFT||b==BTN_TAB_LEFT)','if(b==BTN_TAB_LEFT)').replace('if(b==BTN_RIGHT||b==BTN_TAB_RIGHT)','if(b==BTN_TAB_RIGHT)')
s=s.replace('if(b==BTN_UP&&g->selection>=3)', 'if(b==BTN_LEFT&&g->selection>0)g->selection--;\n  if(b==BTN_RIGHT)g->selection=MIN(g->scroll==4?9:19,g->selection+1);\n  if(b==BTN_UP&&g->selection>=3)')
p.write_text(s)
