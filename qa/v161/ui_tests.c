#include "game.h"
#include "render.c"
#include <assert.h>
#include <stddef.h>
static void snap(Game*g,const char*name){static uint32_t f[W*H];char path[256];game_render_reset_fx();game_render_frame(g,f,W,300);snprintf(path,sizeof(path),"qa/v161/evidence/native-%s.ppm",name);FILE*out=fopen(path,"wb");assert(out);fprintf(out,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(f[i]&255,out);fputc(f[i]>>8&255,out);fputc(f[i]>>16&255,out);}fclose(out);if(g->screen!=PAUSE_MENU){snprintf(path,sizeof(path),"qa/v161/evidence/state-%s.dat",name);assert(game_save(g,path));}}
static void arena(Game*g,int c){game_new(g,16001);game_choose_character(g,c);game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=180;g->enemies[0].art=29;snprintf(g->enemies[0].name,24,"%s",game_enemy_name(29));g->enemies[0].intent=0;g->enemies[0].intent_value=9;g->enemies[0].poison=0;g->energy=4;g->block=7;g->hand_count=5;int cards_[5]={R_VENERATE,R_WROUGHT_IN_WAR,R_FALLING_STAR,R_BULWARK,R_SOVEREIGN_BLADE};for(int i=0;i<5;i++)g->hand[i]=c==CHAR_REGENT?cards_[i]:c==CHAR_NIGHTSHADE?(i%2?N_DEFEND_SILENT:N_STRIKE_SILENT):(i%2?1:0);g->selection=5;}
int main(void){Game g;char name[64];
 game_new(&g,16101);game_action(&g,BTN_ALT);assert(g.screen==HELP);game_action(&g,BTN_PAUSE);assert(g.screen==HELP);game_action(&g,BTN_BACK);assert(g.screen==TITLE);
 game_new(&g,16101);game_set_resume_available(0);g.selection=1;game_action(&g,BTN_OK);assert(g.screen==CHARACTER_SELECT);
 game_new(&g,16101);game_set_resume_available(1);g.selection=1;game_action(&g,BTN_OK);assert(g.screen==NEW_RUN_CONFIRM&&g.selection==0);snap(&g,"new-run-cancel");
 game_action(&g,BTN_START);assert(g.screen==NEW_RUN_CONFIRM);game_action(&g,BTN_PAUSE);assert(g.screen==NEW_RUN_CONFIRM);game_action(&g,BTN_OK);assert(g.screen==TITLE&&game_resume_available());
 game_action(&g,BTN_OK);game_action(&g,BTN_DOWN);assert(g.selection==1);snap(&g,"new-run-replace");game_action(&g,BTN_OK);assert(g.screen==CHARACTER_SELECT);game_action(&g,BTN_PAUSE);assert(g.screen==CHARACTER_SELECT);game_action(&g,BTN_BACK);assert(game_resume_available());
 for(int c=0;c<3;c++){arena(&g,c);snprintf(name,sizeof(name),"idle-%d",c);snap(&g,name);g.selection=0;snprintf(name,sizeof(name),"combat-%d",c);snap(&g,name);
  g.hand_count=1;g.hand[0]=c==2?R_VENERATE:c==1?N_STRIKE_SILENT:0;g.selection=0;snprintf(name,sizeof(name),"play-%d",c);snap(&g,name);
  g.enemies[0].hp=1;g.hand[0]=c==2?R_FALLING_STAR:c==1?N_STRIKE_SILENT:0;snprintf(name,sizeof(name),"finisher-%d",c);snap(&g,name);
 }
 arena(&g,2);g.selection=2;snap(&g,"costs");game_action(&g,BTN_PAUSE);snap(&g,"pause");game_action(&g,BTN_BACK);
 for(int id=0;id<6;id++){arena(&g,2);g.screen=REWARD;g.reward[0]=R_COMET;g.reward[1]=R_BULWARK;g.reward[2]=R_DYING_STAR;g.v151.potion_offer=id;g.selection=4;
  snprintf(name,sizeof(name),"potion-%d-take",id);snap(&g,name);game_action(&g,BTN_DOWN);assert(g.selection==5);snprintf(name,sizeof(name),"potion-%d-leave",id);snap(&g,name);game_action(&g,BTN_UP);assert(g.selection==4);game_action(&g,BTN_RIGHT);assert(g.selection==4);
  int count=g.potion_count;game_action(&g,BTN_OK);assert(g.potion_count==count+1&&g.v151.potion_offer<0&&!g.v151.card_done);
 }
 arena(&g,2);g.screen=REWARD;g.reward[0]=R_COMET;g.reward[1]=R_BULWARK;g.reward[2]=R_DYING_STAR;g.v151.card_done=1;g.v151.potion_offer=0;g.selection=4;game_action(&g,BTN_UP);assert(g.selection==4);game_action(&g,BTN_DOWN);assert(g.selection==5);game_action(&g,BTN_OK);assert(g.screen==MAP&&g.potion_count==0);
 arena(&g,2);g.screen=REWARD;g.reward[0]=R_COMET;g.reward[1]=R_BULWARK;g.reward[2]=R_DYING_STAR;g.v151.potion_offer=1;g.selection=4;g.potion_count=3;game_action(&g,BTN_OK);assert(g.screen==REWARD&&g.v151.potion_offer==1);snap(&g,"belt-full");
 arena(&g,2);g.hand[0]=R_VENERATE;g.selection=0;Game before=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();int count;const GameCombatFx*events=game_combat_fx_events(&count);int gain=0;for(int i=0;i<count;i++)if(events[i].kind==GFX_STARS)gain+=events[i].amount;assert(gain==g.regent.stars-before.regent.stars&&gain>0);
 game_render_feedback(&before,&g,BTN_OK,100);static uint32_t f[W*H];for(int a=0;a<=40;a+=4){game_render_frame(&g,f,W,100+a);snprintf(name,sizeof(name),"qa/v161/evidence/stars-gain-%02d.ppm",a);FILE*out=fopen(name,"wb");fprintf(out,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(f[i]&255,out);fputc(f[i]>>8&255,out);fputc(f[i]>>16&255,out);}fclose(out);}
 arena(&g,0);g.enemies[0].hp=1;g.selection=0;before=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();assert(g.screen==REWARD);game_render_feedback(&before,&g,BTN_OK,100);
 int end=r1311_finish_end();for(int a=r1311_kill_at;a<end+12;a++){assert(game_render_music_paused(100+a));assert(game_render_fx_busy(100+a));}assert(!game_render_music_paused(100+end+12));assert(!game_render_fx_busy(100+end+12));
 game_render_reset_fx();game_render_begin_transition(&before,200);assert(game_render_transition_busy(200));game_render_frame(&g,f,W,224);for(int i=0;i<W*H;i++)assert((f[i]&0xffffff)==0);assert(!game_render_transition_busy(248));game_render_frame(&g,f,W,248);
 arena(&g,2);g.screen=REST;g.selection=0;snap(&g,"rest-heal");g.selection=1;snap(&g,"rest-upgrade");
 arena(&g,2);g.screen=VICTORY;g.selection=0;snap(&g,"endless-choice");
 arena(&g,2);g.screen=DECK_VIEW;g.prior_screen=COMBAT;g.scroll=2;g.selection=0;snap(&g,"pile-tabs");
 arena(&g,2);g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;for(int tab=0;tab<6;tab++){game_render_set_overlay(1,tab,0);snprintf(name,sizeof(name),"inventory-%d",tab);snap(&g,name);}game_render_set_overlay(0,0,0);
 FILE*out=fopen("qa/v161/evidence/state-layout.json","w");fprintf(out,"{\"size\":%zu",sizeof(Game));
#define OFF(x) fprintf(out,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(character);OFF(hand);OFF(hand_count);OFF(selection);OFF(target);OFF(energy);OFF(block);OFF(hp);OFF(turn);OFF(deck_count);OFF(deck);OFF(relic_count);OFF(potion_count);OFF(discard_count);OFF(exhaust_count);OFF(ninja);OFF(enemies);OFF(v151);OFF(potions);OFF(relics);OFF(regent);
 fprintf(out,"}\n");fclose(out);puts("PASS 1.6.1 UI navigation, save warning, independent rewards, Stars trace, fade midpoint and victory audio timeline");return 0;}
