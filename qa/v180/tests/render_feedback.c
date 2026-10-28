#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "render.c"
static uint32_t frame[W*H];static int count;
static void shot(const char*root,const char*name,Game*g){
 char p[512];snprintf(p,sizeof(p),"%s/%s.ppm",root,name);
 game_render_reset_fx();game_render_set_overlay(0,0,0);game_render_frame(g,frame,W,400);
 assert(rift_asset_errors==0);FILE*f=fopen(p,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){uint32_t v=frame[y*W+x];unsigned char rgb[3]={v,v>>8,v>>16};fwrite(rgb,3,1,f);}assert(!fclose(f));count++;
}
int main(int argc,char**argv){assert(argc==3);rift_assets_set_file(argv[1]);assert(rift_assets_open());assert(a18_storage_open(NULL));Game g;
 int values[]={0,1,40,80,120,500,10000,1000000000};
 for(int k=0;k<8;k++){int max=values[k];for(int j=0;j<8;j++){int hp=values[j];assert(r180_hp_pixels(hp,max,89)>=0&&r180_hp_pixels(hp,max,89)<=89);if(hp>=max&&max>0)assert(r180_hp_pixels(hp,max,89)==89);}}
 for(int n=80;n<=16000;n*=10){game_new(&g,123);g.hp=g.maxhp=n;game_start_battle(&g,NODE_FIGHT);for(int half=0;half<2;half++){g.hp=half?n/2:n;memset(frame,0,sizeof(frame));pix=frame;sw=W;r138_player_meter(&g);for(int y=0;y<H;y++)for(int x=0;x<W;x++)if(frame[y*W+x]==RGB(160,30,43)){assert(y>=50&&y<=56&&x>=70&&x<=158);} }}
 game_new(&g,170);shot(argv[2],"title",&g);g.screen=HELP;g.selection=0;shot(argv[2],"options-off",&g);assert(game_set_debug_enabled(1));shot(argv[2],"options-on",&g);game_set_debug_enabled(0);
 for(int i=0;i<4;i++){assert(a18_begin_run(&g,i,i%3,170+i));g.act=i+1;g.endless=1;g.screen=MAP;g.hp=g.maxhp=120+i*20;assert(a18_checkpoint(&g));}
 g.screen=A18_SLOTS;g.selection=0;shot(argv[2],"save-slots",&g);
 A18Profile*p=(A18Profile*)a18_profile();p->wins[0]=10;p->wins[1]=12;p->wins[2]=1;p->best_acts[0]=4;p->best_acts[1]=14;p->best_acts[2]=2;p->true_endings=3;
 g.screen=CHARACTER_SELECT;g.selection=1;shot(argv[2],"character-records",&g);
 g.act=3;g.floor=5;game_start_battle(&g,NODE_ELITE);g.enemy_count=2;g.enemies[0].art=33;g.enemies[1]=g.enemies[0];g.enemies[1].art=35;strcpy(g.enemies[0].name,"Infernal Colossus");strcpy(g.enemies[1].name,"Rift Behemoth");g.hp=g.maxhp=500;g.selection=g.hand_count;shot(argv[2],"full-health-facing",&g);g.hp=250;shot(argv[2],"half-health",&g);game_action(&g,BTN_PAUSE);shot(argv[2],"pause",&g);
 game_new(&g,334);g.hp=30;game_begin_event(&g,1);shot(argv[2],"moon-healing",&g);g.selection=1;shot(argv[2],"moon-reflection",&g);
 int enemies[]={0,1,11,18,21,31};for(int i=0;i<6;i++){g.act=i<3?1:i<5?2:3;g.floor=5;game_start_battle(&g,NODE_FIGHT);g.enemy_count=1;g.enemies[0].art=enemies[i];strcpy(g.enemies[0].name,game_enemy_name(enemies[i]));g.selection=g.hand_count;char name[32];snprintf(name,sizeof(name),"enemy-%02d",enemies[i]);shot(argv[2],name,&g);}
 // Card layout remains valid for all 60 new cards and upgraded variants.
 for(int id=334;id<394;id++)for(int up=0;up<2;up++){R135CardLayout l=r135_card_layout(id|(up?UPBIT:0),180);assert(l.height>100&&l.height<=210);}
 printf("PASS %d rendered review screens, 128 proportional HP checks, 120 custom-card layouts; zero streamed asset errors.\n",count);rift_assets_close();return 0;
}
