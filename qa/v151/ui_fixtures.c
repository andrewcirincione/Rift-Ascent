#include "game.h"
#include "render.c"
#include <assert.h>
#include <stddef.h>
static void snap(Game*g,const char*name){static uint32_t frame[W*H];char path[256];r137_count=0;r138_death_active=0;rift120_played_card=rift120_auto_card=-1;hand_exchange=0;fx_transition=victory_replay=0;effect_kind=0;game_render_frame(g,frame,W,300);snprintf(path,sizeof(path),"qa/v151/evidence/native-%s.ppm",name);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");for(int i=0;i<W*H;i++){fputc(frame[i]&255,f);fputc(frame[i]>>8&255,f);fputc(frame[i]>>16&255,f);}fclose(f);snprintf(path,sizeof(path),"qa/v151/evidence/state-%s.dat",name);assert(game_save(g,path));}
static void arena(Game*g){game_new(g,15011);game_choose_character(g,CHAR_NIGHTSHADE);game_start_battle(g,NODE_FIGHT);g->enemy_count=2;for(int i=0;i<2;i++){g->enemies[i].hp=g->enemies[i].maxhp=140;g->enemies[i].art=i?32:29;snprintf(g->enemies[i].name,24,"%s",game_enemy_name(g->enemies[i].art));g->enemies[i].intent=0;g->enemies[i].intent_value=8+i*2;g->enemies[i].poison=0;}g->energy=6;g->hand_count=6;int hand[]={N_STRIKE_SILENT,N_DEADLY_POISON,N_MEMENTO_MORI|UPBIT|(MOD_ECHO<<CARD_MOD_SHIFT),N_BLADE_DANCE,N_BACKFLIP,N_HAZE};memcpy(g->hand,hand,sizeof(hand));g->selection=2;g->strength=3;g->dexterity=2;g->ninja.v[NS_DISCARDS_TURN]=2;g->enemies[0].vulnerable=2;g->enemies[1].weak=2;g->block=8;g->relics[g->relic_count++]=NR_SKULL;g->relics[g->relic_count++]=NR_BANDAGES;}
int main(void){Game g;game_new(&g,15011);snap(&g,"title");game_action(&g,BTN_OK);game_action(&g,BTN_RIGHT);snap(&g,"character");game_action(&g,BTN_OK);while(g.screen==ACT_INTRO)game_update(&g);snap(&g,"blessing");arena(&g);snap(&g,"combat");g.selection=1;snap(&g,"poison");g.selection=3;snap(&g,"shivs");g.hand_count=3;g.hand[0]=N_SURVIVOR;g.hand[1]=N_TACTICIAN;g.hand[2]=N_HAZE;g.selection=0;game_play_card(&g,0,0);snap(&g,"sly-choice");
 arena(&g);g.hand_count=1;g.hand[0]=N_ACCURACY;g.selection=0;snap(&g,"power");
 arena(&g);g.hand_count=1;g.hand[0]=N_DEFEND_SILENT;g.selection=0;snap(&g,"block");
 arena(&g);g.hand_count=1;g.hand[0]=N_STRIKE_SILENT;g.selection=0;snap(&g,"attack");
 arena(&g);g.hand_count=3;g.hand[0]=N_DEFEND_SILENT;g.hand[1]=N_REFLEX;g.hand[2]=N_SNAKEBITE;g.ninja.v[NS_PLANS]=1;g.ninja.powers[N_WELL_LAID_PLANS-NINJA_FIRST]=1;g.selection=3;snap(&g,"retain");
 arena(&g);for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;game_end_turn(&g);snap(&g,"rewards");game_action(&g,BTN_POTION);snap(&g,"reward-preview");
 arena(&g);g.screen=MAP;g.map[0][0]=NODE_SHOP;g.floor=0;game_choose_node(&g,0);g.gold=500;snap(&g,"shop");
 arena(&g);g.ninja.v[NS_THORNS]=4;g.ninja.v[NS_INTANGIBLE]=2;g.ninja.v[NS_BLUR]=1;g.ninja.v[NS_BURST]=2;g.ninja.powers[N_ABRASIVE-NINJA_FIRST]=1;g.ninja.powers[N_AFTERIMAGE-NINJA_FIRST]=2;g.ninja.v[NS_AFTERIMAGE]=2;g.ninja.powers[N_ACCURACY-NINJA_FIRST]=1;g.ninja.v[NS_ACCURACY]=6;snap(&g,"status");
 arena(&g);g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;g.screen=REWARD;g.reward[0]=N_DASH;g.reward[1]=N_BACKFLIP;g.reward[2]=N_ACCURACY;g.v151.potion_offer=5;g.selection=4;snap(&g,"reward-potion");
 g.v151.card_done=1;snap(&g,"reward-card-taken");
 arena(&g);g.screen=REST;g.selection=1;snap(&g,"rest");game_action(&g,BTN_OK);g.deck[0]=N_BULLET_TIME;g.selection=0;snap(&g,"upgrade-cost");g.deck[0]=21|(MOD_ECHO<<CARD_MOD_SHIFT);snap(&g,"upgrade-mod");g.deck[0]=21;snap(&g,"upgrade-zero");
 arena(&g);game_begin_event(&g,12);g.selection=0;snap(&g,"mystery");g.screen=MAP;snap(&g,"map");
 arena(&g);g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;snap(&g,"inventory");game_render_set_overlay(1,3,1);snap(&g,"inventory-potions");game_render_set_overlay(1,5,0);snap(&g,"inventory-deck");game_render_set_overlay(0,0,0);
 arena(&g);for(int i=53;i<63;i++)g.relics[g.relic_count++]=i;game_render_set_overlay(1,4,0);game_render_set_status_scroll(g.relic_count-1);snap(&g,"inventory-new-relics");game_render_set_overlay(0,0,0);snap(&g,"new-relics");
 arena(&g);g.character=CHAR_WARDEN;g.hand_count=1;g.hand[0]=0;g.selection=0;snap(&g,"warden-attack");
 arena(&g);game_begin_event(&g,12);game_action(&g,BTN_OK);snap(&g,"mystery-card");
 arena(&g);g.screen=RELIC_RESULT;g.relic_pending=1;g.reward[0]=62;g.selection=0;g.relic_resume=MAP;snap(&g,"relic-result");
 arena(&g);g.enemy_count=1;g.enemies[0].hp=1;g.hand_count=1;g.hand[0]=N_STRIKE_SILENT;g.selection=0;snap(&g,"finisher");
 arena(&g);g.hp=1;g.block=0;g.enemies[0].intent=0;g.enemies[0].intent_value=20;g.hand_count=0;g.selection=0;snap(&g,"death");
 FILE*f=fopen("qa/v151/evidence/state-layout.json","w");fprintf(f,"{\"size\":%zu",sizeof(Game));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(character);OFF(hand);OFF(hand_count);OFF(selection);OFF(target);OFF(energy);OFF(block);OFF(hp);OFF(turn);OFF(deck_count);OFF(deck);OFF(relic_count);OFF(potion_count);OFF(discard_count);OFF(exhaust_count);OFF(ninja);OFF(enemies);OFF(v151);OFF(potions);OFF(relics);
 fprintf(f,"}\n");fclose(f);puts("PASS native UI fixtures and save validation");return 0;}
