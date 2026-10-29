from pathlib import Path
p=Path(__file__).parent/'source'
f=p/'render.c';s=f.read_text().replace('#include "ascent_art.h"','#include "ascent_art.h"\n#include "expedition_art.h"').replace('#include "ascent_ui.inc"','#include "ascent_ui.inc"\n#include "expedition_ui.inc"')
s=s.replace('static const uint16_t *rift130_fight(const Game *g){','static const uint16_t *rift130_fight(const Game *g){\n if(g->act==7&&g->battle_type==NODE_BOSS)return expedition_god_arena;')
s=s.replace('||g->screen==ASCENT_SAVE_ERROR)&&victory_replay','||g->screen==ASCENT_SAVE_ERROR||g->screen==ABOMINATION_AFTER)&&victory_replay')
s=s.replace(' if(ascent_story_screen(g->screen)){',' if(expedition_scene(g->screen)){expedition_scene_view(g);return;}\n if(ascent_story_screen(g->screen)){')
s=s.replace(' case ASCENT_COLLECTION:ascent_collection_view(g);break;',' case ASCENT_COLLECTION:ascent_collection_view(g);break;\n case EXPEDITION_GIFT:expedition_gift_view(g);break;\n case MYSTERY_CARDS:expedition_mystery_view(g);break;')
s=s.replace('72,71,70,69,53,64,66,73,68,55,47,63,66,43,61,70,67,69,49,73,64,59,56,64,72,66,45,67,70,68,73,57,60,40,53,71,68,61,72,67','72,76,70,69,53,64,66,74,68,55,47,85,66,43,61,70,67,69,80,73,64,59,56,64,72,66,45,81,70,68,73,57,60,40,53,71,68,61,79,87')
s=s.replace(' if(id>=64&&id<74)r134_blit',' static const unsigned char expedition_icons[20]={0,1,2,3,4,5,6,7,2,1,7,4,0,5,3,6,1,2,7,5};\n if(id>=74&&id<94)r134_blit(x,y,size,size,expedition_relic_art[expedition_icons[id-74]],80,80,255);\n else if(id>=64&&id<74)r134_blit')
f.write_text(s)
f=p/'combat_fx.inc';s=f.read_text().replace('||after->screen==ASCENT_SAVE_ERROR','||after->screen==ASCENT_SAVE_ERROR||after->screen==ABOMINATION_AFTER')
s=s.replace(' else if(art==44){',' else if(art==45){iw=192;ih=224;rgba=expedition_unbegotten;}\n else if(art==44){')
s=s.replace('h=boss>=0||foe->art>=42?98:92;','h=art==45?98:boss>=0||foe->art>=42?98:92;') # scene clipped to 98px battle band
s=s.replace('144*h/160:art==44?','144*h/160:art==45?192*h/224:art==44?')
s=s.replace('  else if(art==44)r170_rgba_actor','  else if(art==45)r170_rgba_actor(ex-w/2,y,w,h,expedition_unbegotten,192,224,tilt,opacity,flash);\n  else if(art==44)r170_rgba_actor')
f.write_text(s)
f=p/'ascent_ui.inc';s=f.read_text().replace('tab==3?a->potions:a->relics;','tab==3?a->potions:tab==4?a->relics:0xfffffu;').replace('"POTIONS","RELICS"','"POTIONS","RELICS","BASE RELICS"').replace('CLAMP(g->scroll,0,4)','CLAMP(g->scroll,0,5)').replace('i<5;i++){int x=21+i*89','i<6;i++){int x=21+i*75').replace('46,n+64)','46,n+(tab==5?74:64))').replace('index+(tab==3?6:64)','index+(tab==3?6:tab==5?74:64)');f.write_text(s)
f=p/'ascent_flow.inc';s=f.read_text().replace('(g->scroll+4)%5','(g->scroll+5)%6').replace('(g->scroll+1)%5','(g->scroll+1)%6');f.write_text(s)
f=p/'game.c';s=f.read_text();line=' g->expedition.combat[E181_POWER_ENERGY]=g->expedition.combat[E181_WEAK_ATTACK]=g->expedition.combat[E181_HURT_BLOCK]=g->expedition.combat[E181_EXHAUST_HEAL]=0;\n';assert s.count(line)==1;s=s.replace(line,'');mark='void game_start_turn(Game*g){';assert mark in s;s=s.replace(mark,mark+'\n'+line.rstrip());s=s.replace('g->hp*4<=g->maxhp','(long long)g->hp*4<=g->maxhp');f.write_text(s)
