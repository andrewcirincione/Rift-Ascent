#define main prior_ui_fixture_main
#include "../v160/ui_fixtures.c"
#undef main
static uint32_t frame[W*H];static unsigned cases;
static uint64_t combined=1469598103934665603ULL;
static void hash_frame(const char*name){
 uint64_t hash=1469598103934665603ULL;
 for(unsigned i=0;i<sizeof(frame);i++){hash^=((unsigned char*)frame)[i];hash*=1099511628211ULL;}
 assert(!rift_asset_errors);(void)name;combined^=hash;combined*=1099511628211ULL;cases++;
}
int main(void){
 Game g;char name[96];assert(rift_assets_open());
 /* Every card image, both costs, upgrades, frames and long descriptions. */
 for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++){
  arena(&g,id>=REGENT_FIRST?CHAR_REGENT:id>=NINJA_FIRST?CHAR_NIGHTSHADE:CHAR_WARDEN);g.regent.stars=15;g.energy=5;
  g.hand[2]=id|(up?UPBIT:0);g.selection=2;game_render_reset_fx();game_render_frame(&g,frame,W,300);
  snprintf(name,sizeof(name),"card-%d-%d",id,up);hash_frame(name);
 }
 /* Published UI fixtures include all 44 enemies and menu types. */
 const char*fixtures[]={"title-disabled","title-resume","characters","combat","no-stars","blade-preview","stars-15","stars-3","nightshade","warden","upgrade-cost","upgrade-mod","choice","status","shop"};
 for(unsigned i=0;i<sizeof(fixtures)/sizeof(fixtures[0]);i++){
  char path[128];snprintf(path,sizeof(path),"qa/v160/evidence/state-%s.dat",fixtures[i]);assert(game_load(&g,path));game_render_reset_fx();game_render_frame(&g,frame,W,300);hash_frame(fixtures[i]);
 }
 for(int e=0;e<ENEMY_COUNT;e++)for(int c=0;c<3;c++){
  arena(&g,c);g.enemies[0].art=e;g.strength=20;g.regent.stars=15;game_render_reset_fx();game_render_frame(&g,frame,W,300);snprintf(name,sizeof(name),"enemy-%d-character-%d",e,c);hash_frame(name);
 }
 for(int node=NODE_EVENT;node<=NODE_TREASURE;node++)for(int seed=0;seed<8;seed++){
  game_new(&g,16002+seed);game_choose_character(&g,seed%3);g.screen=MAP;g.floor=0;g.map[0][0]=node;g.selection=0;game_choose_node(&g,0);
  game_render_reset_fx();game_render_frame(&g,frame,W,300);snprintf(name,sizeof(name),"room-%d-%d",node,seed);hash_frame(name);
 }
 arena(&g,CHAR_REGENT);g.relic_count=3;g.relics[0]=50;g.relics[1]=60;g.relics[2]=63;g.potion_count=2;g.potions[0]=0;g.potions[1]=1;
 for(int tab=0;tab<4;tab++){game_render_set_overlay(1,tab,0);game_render_reset_fx();game_render_frame(&g,frame,W,300);snprintf(name,sizeof(name),"overlay-%d",tab);hash_frame(name);}game_render_set_overlay(0,0,0);
 g.screen=MAP;g.selection=0;game_render_reset_fx();game_render_frame(&g,frame,W,300);hash_frame("map");
 g.screen=ACT_INTRO;game_render_frame(&g,frame,W,300);hash_frame("act-intro");
 /* Player attack/block/power/death and enemy strikes traverse streamed FX. */
 for(int c=0;c<3;c++)for(int mode=0;mode<5;mode++){
  arena(&g,c);g.strength=20;g.regent.stars=15;g.hand_count=1;g.selection=0;
  g.hand[0]=mode==0?(c==2?R_WROUGHT_IN_WAR:c==1?N_STRIKE_SILENT:0):mode==1?(c==2?R_BULWARK:c==1?N_DEFEND_SILENT:1):(c==2?R_FURNACE:c==1?N_ACCURACY:8);
  if(mode==4){g.hp=1;g.block=0;g.hand_count=g.selection=0;}
  Game before=g;game_render_reset_fx();game_combat_fx_begin();int action=mode>=3?BTN_BACK:BTN_OK;game_action(&g,action);game_combat_fx_end();game_render_feedback(&before,&g,action,300);
  for(unsigned tick=300;tick<=540;tick+=4){game_render_frame(&g,frame,W,tick);snprintf(name,sizeof(name),"motion-%d-%d-%u",c,mode,tick);hash_frame(name);}
 }
 printf("cases=%u combined-frame-hash=%016llx\n",cases,(unsigned long long)combined);
 assert(fflush(stdout)==0);assert(!ferror(stdout));
 fprintf(stderr,"PASS %u native visual equivalence cases; asset errors=%u\n",cases,rift_asset_errors);rift_assets_close();return 0;
}
