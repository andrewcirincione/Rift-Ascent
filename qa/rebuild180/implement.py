from pathlib import Path
import re,json
S=Path(__file__).parent/'source'
def edit(n,fn):
 p=S/n;p.write_text(fn(p.read_text()))
def sub(s,a,b):
 assert a in s,a[:100];return s.replace(a,b)

def header(s):
 s=sub(s,'#define ENEMY_COUNT 44','#define ENEMY_COUNT 45')
 s=sub(s,'#define GAME_VERSION 23','#define GAME_VERSION 180')
 s=sub(s,'STORY_ACT_BEFORE_BLESSING};','STORY_ACT_BEFORE_BLESSING,ASCENT_SLOTS_VIEW,ASCENT_OVERWRITE,ASCENT_COLLECTION,ASCENT_OPTIONS,ASCENT_ENDING,ASCENT_CARDS,ASCENT_POTIONS,ASCENT_RELIC,ASCENT_REVELATION,ASCENT_BOSS,ASCENT_TRUE_END,ASCENT_SAVE_ERROR};')
 s=sub(s,'BTN_TAB_RIGHT=8192','BTN_TAB_RIGHT=8192,BTN_DEBUG_KILL=16384,BTN_DEBUG_ACT=32768')
 s=sub(s,'typedef struct {char name[24];','#include "ascent_progress.h"\n\ntypedef struct {char name[24];')
 s=sub(s,' RegentState regent;\n} Game;',' RegentState regent;\n AscentRun ascent;\n} Game;')
 s=s.replace('\n#endif','''
const AscentAccount*ascent_profile(void);
const AscentSlot*ascent_slot(int slot);
int ascent_popcount(unsigned mask);
int ascent_secret(int character);
int ascent_open(const char*root);
int ascent_begin(Game*g,int slot,int character,unsigned seed);
int ascent_resume(Game*g,int slot);
int ascent_checkpoint(Game*g);
int ascent_card_available(const Game*g,int id);
int ascent_debug(void);
int ascent_set_debug(int enabled);
int ascent_story_screen(int screen);
#endif''')
 return s
edit('game.h',header)

def game(s):
 s=sub(s,'#include "regent_cards.inc"','#include "regent_cards.inc"\n#include "new_cards.inc"')
 s=sub(s,'#include "regent_upgrades.inc"','#include "regent_upgrades.inc"\n#include "new_upgrades.inc"')
 s=sub(s,'#include "regent_meta.inc"','#include "regent_meta.inc"\n#include "new_card_flags.h"\n#include "new_potions.h"')
 s=sub(s,'if(id>=REGENT_FIRST)return CARD_UP','if(id>=ASCENT_CARD_FIRST)return cards[id].cost;\n if(id>=REGENT_FIRST)return CARD_UP')
 s=sub(s,'static int preview_depth','static int ascent_complete(Game*,int);\nstatic void ascent_record_act(Game*);\nstatic int ascent_random_potion(Game*);\nstatic int preview_depth')
 at=s.index('const int RELIC_COUNT=');pos=s.rfind('};',0,at);s=s[:pos]+'#include "new_relics.inc"\n'+s[pos:]
 s=sub(s,'cards[id].rarity<=2','cards[id].rarity<=3')
 s=sub(s,'int roll=rnd(g,100);return reward_of_rarity(g,roll<9?2:roll<46?1:0,a,b);','int roll=rnd(g,100);return reward_of_rarity(g,roll<2?3:roll<11?2:roll<46?1:0,a,b);')
 s=sub(s,'g->battle_type==NODE_BOSS?2:g->battle_type==NODE_ELITE?(roll<10?2:roll<50?1:0):(roll<37?1:0)','g->battle_type==NODE_BOSS?(roll<20?3:2):g->battle_type==NODE_ELITE?(roll<5?3:roll<15?2:roll<50?1:0):(roll<2?3:roll<5?2:roll<37?1:0)')
 s=sub(s,'if(!n)return g->character==CHAR_NIGHTSHADE?N_BACKFLIP:5;','if(!n&&rarity==3)return reward_of_rarity(g,2,excluded0,excluded1);\n if(!n)return g->character==CHAR_NIGHTSHADE?N_BACKFLIP:5;')
 s=sub(s,'if(id==25)g->max_energy++;}','if(id==25)g->max_energy++;if(id==72){g->maxhp+=10;g->hp+=10;}}')
 s=sub(s,'int pool[64],n=0;','int pool[80],n=0;')
 s=sub(s,'(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51))','(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51)||(id>=64&&(g->ascent.relics&(1u<<(id-64)))))')
 s=sub(s,'return CLAMP(g->scroll,0,MIN(20,MIN(g->gold/2,g->maxhp-g->hp)));','return MIN(20,g->maxhp-g->hp);')
 s=sub(s,'if(g->battle_type==NODE_BOSS&&g->act>=3&&!g->endless){\n  g->screen=VICTORY;g->win_screen=0;g->relic_pending=0;g->selection=0;\n  msg(g,"THE SPIRE HAS FALLEN");return;\n }','''if(g->battle_type==NODE_BOSS)ascent_record_act(g);
 if(g->battle_type==NODE_BOSS&&g->act==3&&!g->ascent.won){
  if(!ascent_complete(g,0)){g->screen=ASCENT_SAVE_ERROR;g->selection=0;return;}
  g->screen=ASCENT_ENDING;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }
 if(g->battle_type==NODE_BOSS&&g->act==4&&g->ascent.secret){
  if(!ascent_complete(g,1)){g->screen=ASCENT_SAVE_ERROR;g->selection=1;return;}
  g->screen=ASCENT_TRUE_END;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }''')
 s=sub(s,'g->v151.potion_offer=rnd(g,6)','g->v151.potion_offer=ascent_random_potion(g)')
 s=sub(s,'g->gold+=g->reward_gold+(relic(g,16)?20:0);','g->gold+=g->reward_gold+(relic(g,16)?20:0)+(relic(g,67)?8:0);\n if(relic(g,68))heal(g,3);')
 s=sub(s,' {"Cathedral Horror",148,17,1,{17,15,16,4}},',' {"Cathedral Horror",148,17,1,{17,15,16,4}},\n {"THE UNMAKER",340,22,2,{1,11,10,4}},')
 s=sub(s,' e->intent=d->moves[slot];',''' e->intent=d->moves[slot];
 /* Most heavy openings are deferred; a deterministic one-in-eight exception
    keeps some danger while always showing the actual intended move. */
 if(g->turn<=1&&(e->intent==4||e->intent==7||e->intent==10||e->intent==11)&&
    (route_hash(g->seed^(unsigned)e->art*101u^(unsigned)g->floor*37u)&7u))e->intent=0;''')
 start=s.index(' g->combat_music=(route_hash');end=s.index('  init_enemy(g,&g->enemies[i],id);',start)
 s=s[:start]+''' unsigned music_hash=route_hash(g->seed^(unsigned)g->act*17041u^(unsigned)g->floor*6781u);
 g->combat_music=mode==NODE_ELITE?(music_hash&1?3:5):(music_hash%3==0?2:music_hash%3==1?4:6);
 g->turn=0;
 g->enemy_count=mode==NODE_FIGHT&&route_hash(g->seed^(unsigned)g->act*7919u^(unsigned)g->floor*101u)%4u==0?2:1;
 for(int i=0;i<g->enemy_count;i++){
  int act=MIN(2,g->act-1);
  int id=mode==NODE_BOSS?(g->act==4&&g->ascent.secret?44:enemy_boss[act]):
    mode==NODE_ELITE?enemy_elite[act][rnd(g,4)]:
    g->act>=4?enemy_normal[rnd(g,3)][rnd(g,10)]:enemy_normal[act][rnd(g,10)];
  if(i>0&&id==g->enemies[0].art){
   int pool[30],n=0;for(int a=0;a<(g->act>=4?3:1);a++)for(int k=0;k<10;k++){
    int candidate=enemy_normal[g->act>=4?a:act][k];if(candidate!=id)pool[n++]=candidate;
   }id=pool[rnd(g,n)];
  }
'''+s[end:]
 s=sub(s,'if(relic(g,19))for','if(relic(g,70))g->strength++;if(relic(g,71))g->dexterity++;\n if(relic(g,65))for(int i=0;i<g->enemy_count;i++)g->enemies[i].poison+=2;\n if(relic(g,19))for')
 s=sub(s,'if(relic(g,10))g->block+=12;','if(relic(g,64))g->block+=6;if(relic(g,66))regent_gain_stars(g,3);\n if(relic(g,10))g->block+=12;')
 s=sub(s,'if(relic(g,20)&&g->turn%3==0)heal(g,4);','if(relic(g,73)&&g->turn%3==0)gain_energy(g,1);\n if(relic(g,20)&&g->turn%3==0)heal(g,4);')
 s=sub(s,'int n=5+(relic(g,18)?1:0)','int n=5+(g->turn==1&&relic(g,69)?1:0)+(relic(g,18)?1:0)')
 s=sub(s,'if(g&&id>=REGENT_FIRST)return regent_cost_now','if(g&&id>=REGENT_FIRST&&id<ASCENT_CARD_FIRST)return regent_cost_now')
 s=sub(s,' int n=up?3:0;\n switch(id){',' int n=up?3:0,u=up,e=enemy;\n switch(id){\n#include "new_effects.inc"')
 s=sub(s,'(raw&CARD_FORCE_EXHAUST) ||','(raw&CARD_FORCE_EXHAUST) || (id>=ASCENT_CARD_FIRST&&id<ASCENT_CARD_LAST&&new_exhaust[id-ASCENT_CARD_FIRST]) ||')
 s=sub(s,' if(index<0)return;if(index<g->hand_count',' if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=ASCENT_CARD_FIRST){resolve_card(g,index,enemy,-1);return;}if(index<g->hand_count')
 s=sub(s,'if(id>=REGENT_FIRST)return g->regent.played','if(id>=ASCENT_CARD_FIRST)return 0;\n if(id>=REGENT_FIRST)return g->regent.played')
 s=sub(s,'if(id>=REGENT_FIRST){int n=g->regent.powers','if(id>=ASCENT_CARD_FIRST){if(cap>0)out[0]=0;return;}\n if(id>=REGENT_FIRST){int n=g->regent.powers')
 # New screens are handled separately to keep the stable gameplay switch intact.
 s=sub(s,'void game_action(Game*g,int b){','void game_action(Game*g,int b){\n if(ascent_action(g,b))return;')
 s=sub(s,'static int resume_available;','static int ascent_action(Game*,int);\nstatic int resume_available;')
 start=s.index(' case TITLE:\n',s.index('void game_action'));end=s.index(' case REGENT_CHOICE:',start)
 s=s[:start]+''' case TITLE:
  if(b==BTN_UP)g->selection=(CLAMP(g->selection,0,3)+3)%4;
  else if(b==BTN_DOWN)g->selection=(CLAMP(g->selection,0,3)+1)%4;
  else if(b==BTN_OK){
   if(g->selection<2){g->screen=ASCENT_SLOTS_VIEW;g->scroll=g->selection;g->selection=0;}
   else {g->screen=g->selection==2?ASCENT_COLLECTION:ASCENT_OPTIONS;g->selection=g->scroll=0;}
  }break;
 case CHARACTER_SELECT:
  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=(g->selection+((b==BTN_LEFT||b==BTN_UP)?2:1))%3;
  else if(b==BTN_OK){int c=g->selection,slot=g->master_index;unsigned seed=game_random(g);Game old=*g;
   if(!ascent_begin(g,slot,c,seed)){*g=old;msg(g,"Cannot write save slot. Please try again.");}}
  else if(b==BTN_BACK){g->screen=ASCENT_SLOTS_VIEW;g->scroll=1;g->selection=g->master_index;}break;
'''+s[end:]
 s=sub(s,'if(boss&&g->act>3){game_start_battle(g,NODE_BOSS);return;}','if(boss&&g->act==4&&g->ascent.secret){g->screen=ASCENT_BOSS;g->selection=0;return;}\n if(boss&&g->act>3){game_start_battle(g,NODE_BOSS);return;}')
 s=sub(s,'return (screen==STORY_ACT','if(ascent_story_screen(screen))return screen==ASCENT_BOSS?2:1;\n return (screen==STORY_ACT')
 s=sub(s,'return screen==ACT_INTRO||screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING;','return screen==ACT_INTRO||screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING||(ascent_story_screen(screen)&&screen!=ASCENT_BOSS);')
 s=sub(s,'if(screen==STORY_BOSS)return 0;','if(screen==STORY_BOSS||screen==ASCENT_BOSS)return 0;\n if(screen>=ASCENT_SLOTS_VIEW&&screen<=ASCENT_OPTIONS)return 0;')
 s=sub(s,'if(g->battle_type==NODE_ELITE)return 3;','if(g->battle_type==NODE_ELITE)return g->combat_music==5?5:3;')
 s=sub(s,'return g->combat_music==4?4:2;','return g->combat_music==4?4:g->combat_music==6?6:2;')
 s=sub(s,'\n return 1;\n}\nstatic void shop_buy','\n return g->act==1?1:g->act==2?7:g->act==3?8:9;\n}\nstatic void shop_buy')
 s=sub(s,'int potion=rnd(g,6)','int potion=ascent_random_potion(g)')
 s=sub(s,'return id>=0&&id<6?potion_titles[id]:"None";','return id>=6&&id<26?new_potion_names[id-6]:id>=0&&id<6?potion_titles[id]:"None";')
 s=sub(s,'return id>=0&&id<6?potion_details[id]:"Empty slot";','return id>=6&&id<26?new_potion_rules[id-6]:id>=0&&id<6?potion_details[id]:"Empty slot";')
 s=sub(s,'if(!combat&&id!=0)','if(!combat&&id!=0&&id!=11&&id!=19&&id!=21)')
 s=sub(s,'if(!combat&&g->hp==g->maxhp)','if(!combat&&(id==0||id==11)&&g->hp==g->maxhp)')
 s=sub(s,' if(combat&&relic(g,21))',' if(id>=6&&id<26){switch(id){\n#include "new_potion_effects.inc"\n }snprintf(g->message,sizeof(g->message),"Used %s.",game_potion_name(id));}\n if(combat&&relic(g,21))')
 s=sub(s,'if(g->effect_turn==1&&b==BTN_UP)g->scroll=MIN(g->scroll+1,MIN(20,MIN(g->gold/2,g->maxhp-g->hp)));\n  else if(g->effect_turn==1&&b==BTN_DOWN)g->scroll=MAX(0,g->scroll-1);\n  else if(b==BTN_LEFT||b==BTN_UP)','if(b==BTN_LEFT||b==BTN_UP)')
 s=s.replace('g->screen>STORY_ACT_BEFORE_BLESSING','g->screen>ASCENT_SAVE_ERROR').replace('g->potions[i]>5','g->potions[i]>25').replace('g->v151.potion_offer>5','g->v151.potion_offer>25')
 s=sub(s,'else if(version==22){','else if(version==23){if(n!=offsetof(Game,ascent))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}\n else if(version==22){')
 s+='\n#include "ascent_storage.inc"\n#include "ascent_flow.inc"\n'
 return s
edit('game.c',game)
edit('ninja_engine.inc',lambda s:sub(s,'static int ninja_owned_card(const Game*g,int id){return','static int ninja_owned_card(const Game*g,int id){if(id>=ASCENT_CARD_FIRST)return ascent_card_available(g,id);return'))
edit('card_mods.inc',lambda s:sub(s,'case MOD_WARD:if(id>=REGENT_FIRST)','case MOD_WARD:if(id>=ASCENT_CARD_FIRST)return new_block[id-ASCENT_CARD_FIRST];if(id>=REGENT_FIRST)'))
edit('card_preview.inc',lambda s:sub(s,'if(id>=REGENT_FIRST){regent_description_now','if(id>=ASCENT_CARD_FIRST){append_mod_rules(raw,out,cap,0);return;}\n if(id>=REGENT_FIRST){regent_description_now'))
# Moon Spring is a fixed free heal, or a paid permanent upgrade to max HP.
def events(s):
 s=sub(s,'Moonwater can mend your wounds, or wait in a bottle.','Moonwater mends wounds. Its silver root can strengthen your mortal heart.')
 s=sub(s,'{"DRINK","Heal up to 20 HP. Pay 2 Gold per HP. UP/DOWN adjusts.",0,0,0,0},{"BOTTLE MOONWATER","Pay 40 Gold. Gain a Healing potion (18 HP). Requires an empty belt slot.",0,40,0,0}','{"DRINK","Heal 20 HP for free, up to your maximum.",0,0,0,0},{"TAKE THE SILVER ROOT","Pay 35 Gold. Gain 6 maximum HP and heal 6.",0,35,0,0}')
 s=re.sub(r' if\(id==1&&!option\)\{.*?\}\n','',s)
 s=sub(s,'if(id==1){if(!option&&game_moon_heal_now(g)<1)return 0;if(option&&g->potion_count>=MAX_POTIONS)return 0;}','if(id==1&&!option&&game_moon_heal_now(g)<1)return 0;')
 start=s.index(' case 1:if(option)');end=s.index('\n case 2:',start)
 s=s[:start]+''' case 1:if(option){g->maxhp+=6;heal(g,6);snprintf(text,sizeof(text),"Gained 6 maximum HP and healed 6. Paid 35 Gold.");}else{int n=game_moon_heal_now(g);heal(g,n);snprintf(text,sizeof(text),"Healed %d HP.",n);}break;'''+s[end:]
 return s
edit('events150.inc',events)
print('Applied new overhaul hooks to the 1.71-r2 source')
