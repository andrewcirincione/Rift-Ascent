#include "game.h"
#include "render.c"
#include <assert.h>
#include <stddef.h>
static void snap(Game*g,const char*name){static uint32_t f[W*H];char path[256];game_render_reset_fx();game_render_frame(g,f,W,300);snprintf(path,sizeof(path),"qa/v160/evidence/native-%s.ppm",name);FILE*out=fopen(path,"wb");assert(out);fprintf(out,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(f[i]&255,out);fputc(f[i]>>8&255,out);fputc(f[i]>>16&255,out);}fclose(out);if(g->screen!=PAUSE_MENU){snprintf(path,sizeof(path),"qa/v160/evidence/state-%s.dat",name);assert(game_save(g,path));}}
static void arena(Game*g,int c){game_new(g,16001);game_choose_character(g,c);game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=180;g->enemies[0].art=29;snprintf(g->enemies[0].name,24,"%s",game_enemy_name(29));g->enemies[0].intent=0;g->enemies[0].intent_value=9;g->enemies[0].poison=0;g->energy=4;g->block=7;g->hand_count=5;int cards_[5]={R_VENERATE,R_WROUGHT_IN_WAR,R_FALLING_STAR,R_BULWARK,R_SOVEREIGN_BLADE};for(int i=0;i<5;i++)g->hand[i]=c==CHAR_REGENT?cards_[i]:c==CHAR_NIGHTSHADE?(i%2?N_DEFEND_SILENT:N_STRIKE_SILENT):(i%2?1:0);g->selection=5;}
int main(void){Game g;game_new(&g,16001);g.selection=0;game_set_resume_available(0);snap(&g,"title-disabled");game_set_resume_available(1);snap(&g,"title-resume");g.screen=CHARACTER_SELECT;g.selection=2;snap(&g,"characters");
 arena(&g,CHAR_REGENT);g.selection=2;snap(&g,"combat");g.regent.stars=0;snap(&g,"no-stars");g.regent.stars=15;g.regent.forge=30;g.selection=4;snap(&g,"blade-preview");g.selection=5;snap(&g,"stars-15");g.regent.stars=3;snap(&g,"stars-3");
 game_action(&g,BTN_PAUSE);snap(&g,"pause");game_action(&g,BTN_BACK);
 arena(&g,CHAR_NIGHTSHADE);snap(&g,"nightshade");arena(&g,CHAR_WARDEN);snap(&g,"warden");
 arena(&g,CHAR_REGENT);g.screen=REST;g.selection=1;game_action(&g,BTN_OK);g.deck[0]=R_SEVEN_STARS;snap(&g,"upgrade-cost");g.deck[0]=R_SOVEREIGN_BLADE|(MOD_ECHO<<CARD_MOD_SHIFT);snap(&g,"upgrade-mod");
 arena(&g,CHAR_REGENT);g.hand[0]=R_DECISIONS_DECISIONS;g.regent.stars=8;g.selection=0;game_play_card(&g,0,0);snap(&g,"choice");
 arena(&g,CHAR_REGENT);g.regent.stars=15;g.regent.forge=30;g.regent.played[R_FURNACE-REGENT_FIRST]=1;g.regent.powers[R_FURNACE-REGENT_FIRST]=5;game_render_set_overlay(1,2,0);snap(&g,"status");game_render_set_overlay(0,0,0);
 arena(&g,CHAR_REGENT);g.screen=MAP;g.map[0][0]=NODE_SHOP;g.floor=0;game_choose_node(&g,0);g.gold=500;snap(&g,"shop");
 for(int c=0;c<3;c++){arena(&g,c);g.hand_count=1;g.hand[0]=c==2?R_WROUGHT_IN_WAR:c==1?N_STRIKE_SILENT:0;g.selection=0;char b[40];snprintf(b,sizeof(b),"attack-%d",c);snap(&g,b);g.hand[0]=c==2?R_BULWARK:c==1?N_DEFEND_SILENT:1;snprintf(b,sizeof(b),"block-%d",c);snap(&g,b);g.hand[0]=c==2?R_FURNACE:c==1?N_ACCURACY:8;snprintf(b,sizeof(b),"power-%d",c);snap(&g,b);g.hand[0]=c==2?R_WROUGHT_IN_WAR:c==1?N_STRIKE_SILENT:0;g.enemies[0].hp=1;snprintf(b,sizeof(b),"finisher-%d",c);snap(&g,b);g.enemies[0].hp=g.enemies[0].maxhp=200;g.hp=1;g.block=0;g.hand_count=g.selection=0;snprintf(b,sizeof(b),"death-%d",c);snap(&g,b);}
 for(int e=0;e<ENEMY_COUNT;e++){arena(&g,CHAR_REGENT);g.enemies[0].art=e;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(e));char b[40];snprintf(b,sizeof(b),"enemy-%02d",e);snap(&g,b);}
 FILE*f=fopen("qa/v160/evidence/state-layout.json","w");fprintf(f,"{\"size\":%zu",sizeof(Game));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(character);OFF(hand);OFF(hand_count);OFF(selection);OFF(target);OFF(energy);OFF(block);OFF(hp);OFF(turn);OFF(deck_count);OFF(deck);OFF(relic_count);OFF(potion_count);OFF(discard_count);OFF(exhaust_count);OFF(ninja);OFF(enemies);OFF(v151);OFF(potions);OFF(relics);OFF(regent);
 fprintf(f,"}\n");fclose(f);puts("PASS 1.6 native UI fixtures and saves");return 0;}
