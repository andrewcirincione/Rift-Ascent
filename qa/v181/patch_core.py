from pathlib import Path
r=Path(__file__).parent/'source'
def change(file,old,new,count=1):
 p=r/file;s=p.read_text();assert s.count(old)>=count,(file,old[:120],s.count(old));p.write_text(s.replace(old,new,count))
change('game.h','#define ENEMY_COUNT 45','#define ENEMY_COUNT 46')
change('game.h','#define MAX_RELICS 64','#define MAX_RELICS 96\n#define LEGACY_RELIC_CAP 64')
change('game.h','#define GAME_VERSION 180','#define GAME_VERSION 181')
change('game.h','ASCENT_TRUE_END,ASCENT_SAVE_ERROR};','ASCENT_TRUE_END,ASCENT_SAVE_ERROR,EXPEDITION_STORY,EXPEDITION_GIFT,ABOMINATION_BEFORE,ABOMINATION_AFTER,MYSTERY_STORY,MYSTERY_CARDS};')
change('game.h','#include "ascent_progress.h"','#include "ascent_progress.h"\n#include "expedition.h"')
change('game.h','relic_count,relics[MAX_RELICS]','relic_count,relics[LEGACY_RELIC_CAP]')
change('game.h',' AscentRun ascent;\n',' AscentRun ascent;\n Expedition181 expedition;\n')
change('game.h','extern const int RELIC_COUNT;','extern const int RELIC_COUNT;\nint game_relic_id(const Game*,int);\nint expedition_scene(int screen);\nconst char*expedition_title(const Game*);\nconst char*expedition_line(const Game*);\nint expedition_art(const Game*);')
change('game.c','#include "story170_data.h"','#include "story170_data.h"\n#include "expedition_stories.h"')
change('game.c','static int ascent_complete(Game*,int);','static int ascent_complete(Game*,int);\nstatic int expedition_action(Game*,int);\nstatic void expedition_roll(Game*);\nstatic void expedition_prepare_gift(Game*);\nstatic void expedition_mystery(Game*);\nstatic void expedition_boss_spoils(Game*);\nstatic int expedition_relic(Game*);')
change('game.c','#include "new_relics.inc"','#include "new_relics.inc"\n#include "expedition_relics.inc"')
change('game.c','static int relic(const Game *g,int id){for(int i=0;i<g->relic_count;i++)if(g->relics[i]==id)return 1;return 0;}',
'''int game_relic_id(const Game*g,int i){if(i<0||i>=MAX_RELICS)return -1;return i<LEGACY_RELIC_CAP?g->relics[i]:g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int*relic_slot(Game*g,int i){return i<LEGACY_RELIC_CAP?&g->relics[i]:&g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int relic(const Game *g,int id){for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==id)return 1;return 0;}''')
change('game.c','static void enter_map(Game*g){','static void enter_map(Game*g){\n if(g->expedition.story_pending){g->expedition.story_pending=0;g->screen=EXPEDITION_STORY;g->selection=0;expedition_prepare_gift(g);return;}')
change('game.c','static void add_relic(Game*g,int id){','static void add_relic(Game*g,int id){if(id<0||id>=RELIC_COUNT)return;')
change('game.c','if(g->relics[i]==NR_SNAKE){g->relics[i]=NR_DRAKE;','if(game_relic_id(g,i)==NR_SNAKE){*relic_slot(g,i)=NR_DRAKE;')
change('game.c','g->relics[g->relic_count++]=id;','*relic_slot(g,g->relic_count++)=id;')
change('game.c','if(id==72){g->maxhp+=10;g->hp+=10;}','if(id==72){g->maxhp+=10;g->hp+=10;}if(id==85){g->maxhp+=8;g->hp+=8;}')
change('game.c','int pool[80],n=0;','int pool[MAX_RELICS],n=0;if(g->relic_count>=MAX_RELICS)return -1;')
change('game.c','(id>=64&&(g->ascent.relics&(1u<<(id-64))))','(id>=64&&id<74&&(g->ascent.relics&(1u<<(id-64))))||(id>=74&&id<94)')
change('game.c','g->energy+=2+up;','gain_energy(g,2+up);')
change('game.c','g->energy+=2;','gain_energy(g,2);')
change('game.c','if(id==4)g->energy+=2+(bonus>0);','if(id==4)gain_energy(g,2+(bonus>0));')
change('game.c','if(n>0&&g->screen==COMBAT&&!g->ic[IC_ENERGY_LOCK])','if(n>0&&(g->screen==COMBAT||g->screen==CARD_CHOICE||g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE)&&!g->ic[IC_ENERGY_LOCK])')
change('game.c','if(g->battle_type==NODE_BOSS){\n  int gift=random_relic(g);','if(g->battle_type==NODE_BOSS&&g->act==7){g->screen=ABOMINATION_AFTER;g->selection=0;g->win_screen=0;return;}\n if(g->battle_type==NODE_BOSS){\n  int gift=expedition_relic(g);')
change('game.c','fill_rewards(g);g->screen=REWARD;g->selection=0;\n if(g->battle_type==NODE_ELITE){\n  int gift=random_relic(g);','expedition_roll(g);\n fill_rewards(g);g->screen=REWARD;g->selection=0;\n if(g->battle_type==NODE_ELITE){\n  int gift=expedition_relic(g);')
change('game.c','g->gold+=g->reward_gold+(relic(g,16)?20:0)+(relic(g,67)?8:0);',
'''g->gold+=g->reward_gold+(relic(g,16)?20:0)+(relic(g,67)?8:0);
 if(relic(g,81)&&(g->battle_type==NODE_ELITE||g->battle_type==NODE_BOSS))g->gold+=35;
 if(relic(g,84)&&(g->reward_potion&4))heal(g,4);
 if(relic(g,85))heal(g,1);''')
change('game.c',' {"THE UNMAKER",340,22,2,{1,11,10,4}},',' {"THE UNMAKER",340,22,2,{1,11,10,4}},\n {"THE UNBEGOTTEN",610,31,2,{13,16,11,10}},')
change('game.c','if(d->tier==2 && e->phase==0 && e->hp<=e->maxhp/2){e->phase=1;e->strength+=3;}',
'''if(e->art==45){
  int phase=e->hp<=e->maxhp/3?2:e->hp<=e->maxhp*2/3?1:0;
  if(phase>e->phase){e->strength+=5*(phase-e->phase);e->phase=phase;e->weak=e->vulnerable=0;e->poison/=2;e->block+=18*phase;}
 }else if(d->tier==2 && e->phase==0 && e->hp<=e->maxhp/2){e->phase=1;e->strength+=3;}''')
change('game.c','e->intent=d->moves[slot];','e->intent=d->moves[slot];\n if(e->art==45){static const unsigned char moves[3][4]={{13,16,11,10},{7,13,10,16},{10,7,13,16}};e->intent=moves[CLAMP(e->phase,0,2)][slot];}')
change('game.c','124LL+24LL*(g->act-3);','124LL+24LL*(g->act-3)+2LL*(g->act-3)*(g->act-3);')
change('game.c','long long hp=(d->hp*percent+50)/100;','if(g->act>3)percent+=5LL*(g->act-3)*(g->act-3);\n long long hp=(d->hp*percent+50)/100;')
change('game.c','?(g->act==4&&g->ascent.secret?44:enemy_boss[','?(g->act==7?45:g->act==4&&g->ascent.secret?44:enemy_boss[')
change('game.c','memset(g->ic,0,sizeof(g->ic));','memset(g->ic,0,sizeof(g->ic));\n memset(g->expedition.combat,0,sizeof(g->expedition.combat));')
change('game.c','if(relic(g,65))for(int i=0;i<g->enemy_count;i++)g->enemies[i].poison+=2;',
'''if(relic(g,65))for(int i=0;i<g->enemy_count;i++)g->enemies[i].poison+=2;
 if(relic(g,75))for(int i=0;i<g->enemy_count;i++)g->enemies[i].weak=2;''')
change('game.c','ninja_start_turn_after(g);regent_start_turn(g);',
'''ninja_start_turn_after(g);regent_start_turn(g);
 g->expedition.combat[E181_POWER_ENERGY]=g->expedition.combat[E181_WEAK_ATTACK]=g->expedition.combat[E181_HURT_BLOCK]=g->expedition.combat[E181_EXHAUST_HEAL]=0;
 if(relic(g,76)&&g->turn==2)gain_energy(g,1);
 if(relic(g,79)&&g->turn%4==0)g->strength+=2;
 if(relic(g,86)&&g->draw_count<=3)relic_block(g,6);
 if(g->expedition.combat[E181_NEXT_BLOCK]){relic_block(g,8);g->expedition.combat[E181_NEXT_BLOCK]=0;}
 if(relic(g,90)&&g->turn==3)ninja_poison_all(g,4);
 if(relic(g,91)&&g->turn==1){int powers=0;for(int i=0;i<g->deck_count;i++)powers+=cards[CARD_ID(g->deck[i])].type==2;if(powers>=3)draw_card(g);else relic_block(g,4);}''')
change('game.c','if(relic(g,54)&&!g->ic[IC_ATTACKS_THIS_TURN])','if(relic(g,88)&&g->block==0)g->expedition.combat[E181_NEXT_BLOCK]=1;\n if(relic(g,54)&&!g->ic[IC_ATTACKS_THIS_TURN])')
change('game.c','if(r140_attack_context&&relic(g,26)&&g->hp*2<=g->maxhp)amt+=3;',
'''if(r140_attack_context&&relic(g,26)&&g->hp*2<=g->maxhp)amt+=3;
 if(r140_attack_context&&relic(g,87)&&e->hp*2<e->maxhp)amt+=4;''')
change('game.c','ninja_on_hit(g,idx,amt);regent_on_hit(g,idx,amt);',
'''ninja_on_hit(g,idx,amt);regent_on_hit(g,idx,amt);
 if(r140_attack_context&&relic(g,77)&&!g->expedition.combat[E181_WEAK_ATTACK]){g->expedition.combat[E181_WEAK_ATTACK]=1;e->weak=MIN(99,e->weak+1);}''')
change('game.c',' player_death(g);\n}\nstatic int attack_amount',
''' if(n>0&&g->hp>0&&relic(g,78)&&!g->expedition.combat[E181_HURT_BLOCK]){g->expedition.combat[E181_HURT_BLOCK]=1;relic_block(g,5);}
 if(g->hp>0&&g->hp*4<=g->maxhp&&relic(g,89)&&!g->expedition.combat[E181_LAST_MERCY]){g->expedition.combat[E181_LAST_MERCY]=1;heal(g,12);}
 player_death(g);
}
static int attack_amount''')
change('game.c','if(boss&&g->act==4&&g->ascent.secret)','if(boss&&g->act==7){g->screen=ABOMINATION_BEFORE;g->selection=0;return;}\n if(boss&&g->act==4&&g->ascent.secret)')
change('game.c','if(ascent_story_screen(screen))return','if(expedition_scene(screen))return screen==ABOMINATION_BEFORE?2:1;\n if(ascent_story_screen(screen))return')
change('game.c','return screen==ACT_INTRO||screen==STORY_ACT','return (expedition_scene(screen)&&screen!=ABOMINATION_BEFORE)||screen==ACT_INTRO||screen==STORY_ACT')
change('game.c','if(screen==STORY_BOSS||screen==ASCENT_BOSS)return 0;','if(screen==STORY_BOSS||screen==ASCENT_BOSS||screen==ABOMINATION_BEFORE)return 0;')
change('game.c','if(ascent_action(g,b))return;','if(expedition_action(g,b)||ascent_action(g,b))return;')
change('game.c','g->screen>ASCENT_SAVE_ERROR','g->screen>MYSTERY_CARDS')
change('game.c','for(int i=0;i<g->relic_count;i++)if(g->relics[i]<0||g->relics[i]>=RELIC_COUNT)return 0;',
'''for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)<0||game_relic_id(g,i)>=RELIC_COUNT)return 0;
 if(g->expedition.story_seen>0xfffffu||g->expedition.story_pending<0||g->expedition.story_pending>1||g->expedition.story_id<0||g->expedition.story_id>=20)return 0;
 if(expedition_scene(g->screen)&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==EXPEDITION_GIFT&&(g->selection<0||g->selection>1||g->expedition.gift_kind<0||g->expedition.gift_kind>1))return 0;
 if(g->screen==EXPEDITION_GIFT){int id=g->expedition.gift;if(g->expedition.gift_kind?(id<0||id>=RELIC_COUNT):!valid_card_raw(id))return 0;}
 if(g->screen==MYSTERY_CARDS){if(g->selection<0||g->selection>3)return 0;for(int i=0;i<3;i++)if(!valid_card_raw(g->reward[i])||cards[CARD_ID(g->reward[i])].rarity!=2)return 0;}''')
change('game.c','else if(version==23){','else if(version==180){if(n!=offsetof(Game,expedition))return 0;memcpy(&temp,raw,n);if(temp.relic_count>64)return 0;temp.version=GAME_VERSION;}\n else if(version==23){')
change('game.c','int id=temp.relics[i]==30?7:temp.relics[i];','int id=game_relic_id(&temp,i)==30?7:game_relic_id(&temp,i);')
change('game.c','temp.relics[count++]=id;','*relic_slot(&temp,count++)=id;')
change('game.c','#include "ascent_flow.inc"','#include "ascent_flow.inc"\n#include "expedition_flow.inc"')
change('relics151.inc','static void r151_exhaust(Game*g){',
'''static void r151_exhaust(Game*g){
 if((g->screen==COMBAT||g->screen==CARD_CHOICE)&&relic(g,80)&&!g->expedition.combat[E181_EXHAUST_HEAL]){g->expedition.combat[E181_EXHAUST_HEAL]=1;heal(g,2);}''')
change('relics151.inc','if(g->screen!=COMBAT)return;\n if(type==2)',
'''if(g->screen!=COMBAT)return;
 int played=++g->expedition.combat[E181_CARDS];
 if(relic(g,74)&&played%5==0)gain_energy(g,1);
 if(relic(g,82)&&type==2&&!g->expedition.combat[E181_POWER_ENERGY]){g->expedition.combat[E181_POWER_ENERGY]=1;gain_energy(g,1);}
 if(relic(g,83)&&played==10)for(int i=0;i<g->enemy_count;i++){g->enemies[i].weak+=3;g->enemies[i].vulnerable+=3;}
 if(type==0&&!g->expedition.combat[E181_FIRST_ATTACK]){g->expedition.combat[E181_FIRST_ATTACK]=1;if(relic(g,93))gain_energy(g,1);}
 if(type==2)''')
change('relics151.inc','static void r151_skip_reward(Game*g){','static void r151_skip_reward(Game*g){\n if(relic(g,92)){g->maxhp++;heal(g,1);}')
change('ascent_storage.inc','int ascent_resume(Game*g,int slot){',
'''static int ascent_read_run(const char*stem,unsigned side,Game*g,unsigned*generation){
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,sizeof(*g),generation))return game_validate(g);
 memset(g,0,sizeof(*g));
 if(!ascent_read(stem,side,g,offsetof(Game,expedition),generation)||g->version!=180||g->relic_count>64)return 0;
 g->version=GAME_VERSION;
 /* Old exhausted boss offers become an explicit, repeatable stat blessing. */
 if(g->screen==RELIC_RESULT&&g->relic_pending&&g->reward[0]<0)g->reward[0]=37;
 return game_validate(g);
}
int ascent_resume(Game*g,int slot){''')
change('ascent_storage.inc','int va=ascent_read(stem,0,&a,sizeof(a),&ga),vb=ascent_read(stem,1,&b,sizeof(b),&gb);','int va=ascent_read_run(stem,0,&a,&ga),vb=ascent_read_run(stem,1,&b,&gb);')
change('events150.inc','int game_next_event(Game*g){','int game_next_event(Game*g){\n if(rnd(g,25)==0)return EVENT_COUNT; /* Rare optional cross-character offer. */')
change('events150.inc','void game_begin_event(Game*g,int id){','void game_begin_event(Game*g,int id){\n if(id==EVENT_COUNT){expedition_mystery(g);return;}')
for file in ['render.c']:
 p=r/file;s=p.read_text()
 import re
 s=re.sub(r'g->relics\[([^\]]+)\]',r'game_relic_id(g,\1)',s);p.write_text(s)
change('Makefile','RIFT ASCENT 1.8','RIFT ASCENT 1.8.1')
with (r/'Makefile').open('a') as f:f.write('\ngame.o: expedition.h expedition_stories.h expedition_flow.inc expedition_relics.inc\nrender.o: expedition.h expedition_stories.h expedition_art.h expedition_ui.inc\n')
print('Patched 1.8.1 runtime; append-only save layout retained')
