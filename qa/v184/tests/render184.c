#include <assert.h>
#include <stdio.h>
#include "../source/game.c"
#include "../source/render.c"
static uint32_t pixels[W*H];
static void shot(Game*g,const char*name){game_render_reset_fx();game_render_frame(g,pixels,W,300);assert(!rift_asset_errors);char path[300];snprintf(path,sizeof(path),"qa/v184/evidence/native184-%s.ppm",name);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);for(int i=0;i<W*H;i++){uint32_t v=pixels[i];unsigned char b[3]={v,v>>8,v>>16};fwrite(b,3,1,f);}fclose(f);}
static void fixture(Game*g,const char*n){char p[300];snprintf(p,sizeof(p),"qa/v184/evidence/fixture-%s.bin",n);FILE*f=fopen(p,"rb");assert(f&&fread(g,sizeof(*g),1,f)==1);fclose(f);}
int main(void){rift_assets_set_file("qa/v184/evidence/EBOOT.PBP");assert(rift_assets_open());Game g;char path[]="/tmp/rift184-render-XXXXXX";assert(mkdtemp(path)&&ascent_open(path));char name[80];
 game_new(&g,184);g.selection=2;shot(&g,"title");g.screen=ASCENT_OPTIONS;shot(&g,"options");
 for(int i=0;i<20;i++)ascent_account.cards[0]|=1u<<i;
 g.screen=ASCENT_COLLECTION;g.scroll=0;for(int i=0;i<6;i++){g.selection=i;snprintf(name,sizeof(name),"unlocks-%d",i);shot(&g,name);}
 for(int group=0;group<2;group++)for(int tab=0;tab<4;tab++){fixture(&g,group?"custom-items":"custom-cards");g.revision184.tab=tab;g.selection=0;snprintf(name,sizeof(name),"custom-%d-tab-%d",group,tab);shot(&g,name);game_action(&g,BTN_OK);snprintf(name,sizeof(name),"custom-%d-quantity-%d",group,tab);shot(&g,name);}
 fixture(&g,"custom-ready");shot(&g,"custom-confirm");
 for(int id=39;id<42;id++){game_new(&g,id);game_begin_event(&g,id);snprintf(name,sizeof(name),"bad-event-%d",id);shot(&g,name);game_action(&g,BTN_OK);snprintf(name,sizeof(name),"bad-result-%d",id);shot(&g,name);}
 fixture(&g,"target");shot(&g,"target-left");g.target=1;shot(&g,"target-right");
 game_new(&g,184);game_start_battle(&g,NODE_FIGHT);for(int i=0;i<g.enemy_count;i++){g.enemies[i].hp=1;g.enemies[i].block=0;}g.potion_count=1;g.potions[0]=3;Game before=g;game_use_potion(&g,0);game_render_feedback(&before,&g,BTN_POTION,500);assert(g.screen==REWARD&&!g.message[0]&&!r1310_potion_notice[0]);for(int i=0;i<3;i++)assert(!strstr(rift120_log[i],"Victory"));
 puts("PASS native 1.8.4 layout renders without streamed-art errors");return 0;}
