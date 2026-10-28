#include "game.h"
#include "render.c"
#include <assert.h>
static uint32_t frame[512*H+2];
static void save(Game*g,const char*name){char p[180];snprintf(p,sizeof(p),"qa/v171/evidence/state-%s.dat",name);assert(game_validate(g)&&game_save(g,p));}
static void paint(Game*g,unsigned tick){frame[0]=frame[512*H+1]=0x12345678;game_render_frame(g,frame+1,512,tick);assert(frame[0]==0x12345678&&frame[512*H+1]==0x12345678);assert(!rift_asset_errors);}
int main(void){Game g,b;char p[160],name[60];int frames=0;assert(rift_assets_open());
 for(int c=0;c<3;c++){
  snprintf(p,sizeof(p),"qa/v170/evidence/state-fight-%d-1.dat",c);assert(game_load(&g,p));assert(g.enemies[0].art==15);g.selection=g.hand_count;
  snprintf(name,sizeof(name),"idle-%d",c);save(&g,name);game_render_reset_fx();for(unsigned t=300;t<800;t+=4){paint(&g,t);frames++;}
  g.hand_count=1;g.selection=0;g.hand[0]=c==2?R_STRIKE_REGENT:c==1?N_STRIKE_SILENT:0;g.energy=3;
  snprintf(name,sizeof(name),"play-%d",c);save(&g,name);b=g;game_render_reset_fx();game_combat_fx_begin();game_action(&g,BTN_OK);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,300);assert(g.enemies[0].hp<b.enemies[0].hp);
  for(unsigned t=300;t<400;t+=4){paint(&g,t);frames++;}
  g=b;g.enemies[0].hp=1;snprintf(name,sizeof(name),"finisher-%d",c);save(&g,name);b=g;game_render_reset_fx();game_combat_fx_begin();game_action(&g,BTN_OK);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,300);assert(g.enemies[0].hp<=0&&g.screen!=COMBAT);
  for(unsigned t=300;t<700;t+=4){paint(&g,t);frames++;}
 }
 for(int guard=0;guard<2;guard++){
  assert(game_load(&g,"qa/v171/evidence/state-idle-0.dat"));g.enemies[0].intent=guard?1:0;g.enemies[0].intent_value=guard?8:7;save(&g,guard?"guard-0":"swing-0");b=g;game_render_reset_fx();game_combat_fx_begin();game_action(&g,BTN_BACK);game_combat_fx_end();game_render_feedback(&b,&g,BTN_BACK,300);
  if(guard)assert(g.enemies[0].block>0);else assert(g.hp<b.hp);
  for(unsigned t=300;t<450;t+=4){paint(&g,t);frames++;}
 }
 printf("PASS %d native Maw frames: all characters, idle, incoming hit, enemy attack, guard and killing-blow fade; framebuffer canaries intact, no asset errors\n",frames);rift_assets_close();return 0;
}
