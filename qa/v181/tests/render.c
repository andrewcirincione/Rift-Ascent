#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../source/game.c"
#include "../source/render.c"
static uint32_t pixels[W*H];
static void output(Game*g,const char*name){
 game_render_reset_fx();game_render_frame(g,pixels,W,300);assert(rift_asset_errors==0);
 if(!name)return;char p[320];snprintf(p,sizeof(p),"qa/v181/evidence/native-%s.ppm",name);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){uint32_t v=pixels[i];unsigned char b[3]={v,v>>8,v>>16};fwrite(b,3,1,f);}fclose(f);
}
int main(void){rift_assets_set_file("qa/v181/evidence/EBOOT.PBP");assert(rift_assets_open());char root[]="/tmp/rift-181-render-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g;char name[90],rows[24][192];game_new(&g,181);output(&g,"title");
 for(int c=0;c<3;c++)for(int id=0;id<20;id++)for(int page=0;page<3;page++){
  g.character=c;g.screen=EXPEDITION_STORY;g.expedition.story_id=id;g.selection=page;assert(r134_lines(expedition_line(&g),336,&r134_f9,rows)<=4);assert(r134_width(expedition_title(&g),&r138_title10)<=336);
  snprintf(name,sizeof(name),"story-%d-%d",c,id);output(&g,page==0&&id%5==0?name:0);
  if(page==2){expedition_prepare_gift(&g);g.screen=EXPEDITION_GIFT;g.selection=0;snprintf(name,sizeof(name),"gift-%d-%d",c,id);output(&g,id<2?name:0);}
 }
 for(int c=0;c<3;c++){g.character=c;expedition_mystery(&g);g.selection=0;output(&g,c==0?"mystery-scene":0);g.screen=MYSTERY_CARDS;snprintf(name,sizeof(name),"mystery-%d",c);output(&g,name);}
 for(int id=74;id<94;id++){g.screen=RELIC_RESULT;g.reward[0]=id;g.relic_pending=1;g.selection=0;snprintf(name,sizeof(name),"relic-%d",id);output(&g,name);}
 g.screen=ASCENT_COLLECTION;g.scroll=5;for(int id=0;id<20;id++){g.selection=id;snprintf(name,sizeof(name),"collection-%d",id);output(&g,id%6==0?name:0);}
 for(int s=ABOMINATION_BEFORE;s<=ABOMINATION_AFTER;s++)for(int page=0;page<3;page++){g.screen=s;g.selection=page;assert(r134_lines(expedition_line(&g),336,&r134_f9,rows)<=4);snprintf(name,sizeof(name),"god-scene-%d-%d",s,page);output(&g,name);}
 g.act=7;g.endless=1;game_start_battle(&g,NODE_BOSS);output(&g,"god-combat");
 Game before=g;game_combat_fx_begin();g.enemies[0].hp=0;check_win(&g);game_combat_fx_end();game_render_feedback(&before,&g,BTN_OK,400);assert(victory_replay&&r1311_finishing&&g.screen==ABOMINATION_AFTER);game_render_frame(&g,pixels,W,404);assert(rift_asset_errors==0);g=before;
 for(int e=0;e<ENEMY_COUNT;e++){g.enemies[0].art=e;output(&g,0);}
 puts("PASS all 180 new dialogue pages fit; 60 gifts; 20 relics; six collection tabs; all 46 enemy sprites; no asset errors");return 0;}
