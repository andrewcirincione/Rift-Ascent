#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>
#include "../source/game.c"
static void save(const char*name,const void*data,size_t size){if(size==sizeof(Game))assert(game_validate(data));char path[240];snprintf(path,sizeof(path),"qa/v183/evidence/%s.bin",name);FILE*f=fopen(path,"wb");assert(f&&fwrite(data,size,1,f)==1);fclose(f);}
int main(void){char temp[]="/tmp/rift-183-fixtures-XXXXXX";assert(mkdtemp(temp)&&ascent_open(temp));ascent_account.debug=1;Game g,a;char n[80];
 for(int c=0;c<3;c++){
  assert(ascent_begin(&g,c,c,101+c));g.act=7;g.endless=1;g.floor=MAP_ROWS;g.hp=g.maxhp=800;game_story_start(&g,1);snprintf(n,sizeof(n),"fixture-god-before-%d",c);save(n,&g,sizeof(g));
  game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=1;g.hand_count=2;g.hand[0]=334+c*20;g.hand[1]=335+c*20;g.energy=10;g.regent.stars=20;g.selection=0;snprintf(n,sizeof(n),"fixture-god-kill-%d",c);save(n,&g,sizeof(g));
  assert(ascent_begin(&g,c,c,111+c));g.hp=g.maxhp=800;
  int found=0;for(int seed=1;seed<20000;seed++){g.seed=g.rng=seed;g.act=1;g.floor=4;game_start_battle(&g,c==1?NODE_ELITE:NODE_FIGHT);g.enemy_count=1;g.enemies[0].hp=1;g.hand_count=2;g.hand[0]=334+c*20;g.hand[1]=335+c*20;g.selection=0;g.energy=10;g.regent.stars=20;a=g;a.enemies[0].hp=0;check_win(&a);if(a.screen==RELIC_RESULT)game_action(&a,BTN_OK);game_action(&a,BTN_BACK);if(a.screen==EXPEDITION_STORY&&a.expedition.story_id==(c==1?1:0)){found=1;break;}}
  assert(found);snprintf(n,sizeof(n),"fixture-story-fight-%d",c);save(n,&g,sizeof(g));
  g.screen=MAP;g.revision182.room_pending=0;expedition_mystery(&g);snprintf(n,sizeof(n),"fixture-mystery-%d",c);save(n,&g,sizeof(g));
 }
 assert(ascent_begin(&g,3,0,144));g.hp=g.maxhp=800;game_start_battle(&g,NODE_FIGHT);g.relic_count=0;g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=9999;g.hand_count=4;g.hand[0]=65;g.hand[1]=6;g.hand[2]=38;g.hand[3]=0;g.energy=10;g.potion_count=1;g.potions[0]=4;g.selection=0;save("fixture-energy",&g,sizeof(g));
 g.screen=MAP;g.ascent.relics=1023;for(int i=0;i<RELIC_COUNT;i++)add_relic(&g,i);g.act=6;g.endless=1;game_start_battle(&g,NODE_BOSS);g.enemy_count=1;g.enemies[0].hp=1;g.hand_count=1;g.hand[0]=334;g.selection=0;g.energy=10;save("fixture-exhausted",&g,sizeof(g));
 for(int act=1;act<=7;act++){assert(ascent_begin(&g,3,0,100+act));g.act=act;g.endless=act>3;g.screen=MAP;g.selection=0;snprintf(n,sizeof(n),"fixture-map-%d",act);save(n,&g,sizeof(g));}
 assert(ascent_begin(&g,3,0,444));g.screen=ASCENT_COLLECTION;g.scroll=4;g.selection=10;save("fixture-collection",&g,sizeof(g));
 for(int c=0;c<3;c++)for(int act=1;act<=3;act++){
  assert(ascent_begin(&g,3,c,800+c*10+act));g.act=act;game_start_battle(&g,NODE_FIGHT);g.enemy_count=2;g.hp=g.maxhp=800;
  int normal=act==1?(c==0?0:c==1?10:11):act==2?(c==0?17:c==1?20:21):(c==0?28:c==1?31:32);
  int elite=enemy_elite[act-1][c];init_enemy(&g,&g.enemies[0],normal);init_enemy(&g,&g.enemies[1],elite);g.selection=g.hand_count;
  snprintf(n,sizeof(n),"fixture-visual-%d-%d",c,act);save(n,&g,sizeof(g));
 }
 for(int id=0;id<EVENT_COUNT;id++){assert(ascent_begin(&g,3,0,900+id));g.gold=200;g.hp=40;g.potion_count=2;g.potions[0]=0;g.potions[1]=1;g.relic_count=0;g.screen=MAP;g.floor=0;g.map[0][0]=NODE_FIGHT;game_begin_event(&g,id);snprintf(n,sizeof(n),"fixture-event-%02d",id);save(n,&g,sizeof(g));}
 for(int id=12;id<=16;id+=4){assert(ascent_begin(&g,3,0,1200+id));g.act=id==12?1:2;game_start_battle(&g,NODE_FIGHT);g.enemy_count=1;init_enemy(&g,&g.enemies[0],id);g.selection=g.hand_count;snprintf(n,sizeof(n),"fixture-visual-extra-%d",id);save(n,&g,sizeof(g));}
 for(int kind=0;kind<2;kind++){
  int found=0;for(int seed=1;seed<100;seed++){
   game_new(&g,seed);game_choose_character(&g,0);g.screen=MAP;g.floor=0;g.selection=0;g.map[0][0]=kind?NODE_ELITE:NODE_FIGHT;a=g;game_action(&a,BTN_OK);
   if(a.screen==COMBAT&&a.combat_music==(kind?6:5)){found=1;break;}}
  assert(found);snprintf(n,sizeof(n),"fixture-map-%s",kind?"elite":"normal");save(n,&g,sizeof(g));
 }

 for(int act=1;act<=3;act++)for(int k=0;k<4;k++){
  assert(ascent_begin(&g,3,0,1800+act*10+k));g.act=act;game_start_battle(&g,k==3?NODE_ELITE:NODE_FIGHT);g.enemy_count=1;g.hp=g.maxhp=80;
  init_enemy(&g,&g.enemies[0],47+(act-1)*4+k);g.selection=g.hand_count;snprintf(n,sizeof(n),"fixture-new-%d-%d",act,k);save(n,&g,sizeof(g));
 }
 assert(ascent_begin(&g,3,0,1830));game_start_battle(&g,NODE_ELITE);g.enemy_count=1;init_enemy(&g,&g.enemies[0],6);g.enemies[0].intent=21;g.enemies[0].intent_value=12;g.enemies[0].weak=2;g.enemies[0].vulnerable=2;g.enemies[0].poison=9;g.selection=g.hand_count;save("fixture-purge",&g,sizeof(g));
 game_start_battle(&g,NODE_FIGHT);g.enemy_count=2;init_enemy(&g,&g.enemies[0],2);init_enemy(&g,&g.enemies[1],47);g.selection=g.hand_count;g.hp=g.maxhp=80;save("fixture-target",&g,sizeof(g));
 for(int tab=0;tab<5;tab++){g.screen=ASCENT_COLLECTION;g.scroll=tab;g.selection=game_collection_unlock_count(tab);snprintf(n,sizeof(n),"fixture-base-%d",tab);save(n,&g,sizeof(g));}
 g.screen=ASCENT_COLLECTION;g.scroll=0;g.selection=0;save("fixture-unlocks",&g,sizeof(g));
 assert(ascent_begin(&g,3,0,1831));g.act=1;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=0;check_win(&g);save("fixture-boss-relic",&g,sizeof(g));
 assert(ascent_begin(&g,0,0,1832));g.screen=MAP;g.gold=789;g.revision182.rare_offset=23;g.version=182;
 unsigned head[5]={0x52413846u,1,100,sizeof(g),hash_bytes((void*)&g,sizeof(g))};FILE*old=fopen("qa/v183/evidence/legacy182-run.bin","wb");assert(old);fwrite(head,sizeof(head),1,old);fwrite(&g,sizeof(g),1,old);fclose(old);
 unsigned ah[5]={0x52413846u,1,100,sizeof(ascent_account),hash_bytes((void*)&ascent_account,sizeof(ascent_account))};old=fopen("qa/v183/evidence/legacy182-account.bin","wb");assert(old);fwrite(ah,sizeof(ah),1,old);fwrite(&ascent_account,sizeof(ascent_account),1,old);fclose(old);
 save("fixture-account",&ascent_account,sizeof(ascent_account));
 FILE*f=fopen("qa/v183/evidence/state-layout.json","w");assert(f);fprintf(f,"{\"size\":%zu,\"account_size\":%zu",sizeof(Game),sizeof(AscentAccount));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(selection);OFF(scroll);OFF(act);OFF(character);OFF(hp);OFF(maxhp);OFF(ascent);OFF(enemy_count);OFF(enemies);OFF(floor);OFF(battle_type);OFF(combat_music);OFF(potion_count);OFF(potions);OFF(hand_count);OFF(hand);OFF(reward);OFF(prior_screen);OFF(energy);OFF(ic);OFF(expedition);OFF(deck_count);OFF(relic_count);OFF(v151);OFF(revision182);OFF(gold);OFF(strength);OFF(block);OFF(weak);OFF(vulnerable);OFF(deck);OFF(message);
 fprintf(f,",\"enemy_size\":%zu,\"enemy_hp\":%zu,\"enemy_art\":%zu,\"account_cards\":%zu,\"account_wins\":%zu,\"account_ascended\":%zu,\"account_debug\":%zu,\"energy_lock\":%d,\"potion_offer\":%zu,\"screens\":{",sizeof(Enemy),offsetof(Enemy,hp),offsetof(Enemy,art),offsetof(AscentAccount,cards),offsetof(AscentAccount,wins),offsetof(AscentAccount,ascended),offsetof(AscentAccount,debug),IC_ENERGY_LOCK,offsetof(Game,v151.potion_offer));
#define SCREEN(x) fprintf(f,"\"" #x "\":%d,",x)
 SCREEN(DECK_VIEW);SCREEN(PAUSE_MENU);SCREEN(EVENT);SCREEN(EVENT_CARD);SCREEN(EVENT_RESULT);SCREEN(TITLE);SCREEN(MAP);SCREEN(COMBAT);SCREEN(VICTORY);SCREEN(ASCENT_SLOTS_VIEW);SCREEN(ASCENT_OPTIONS);SCREEN(ASCENT_COLLECTION);SCREEN(CHARACTER_SELECT);SCREEN(ACT_INTRO);SCREEN(BLESSING);SCREEN(STORY_ACT_BEFORE_BLESSING);SCREEN(EXPEDITION_STORY);SCREEN(EXPEDITION_GIFT);SCREEN(ABOMINATION_BEFORE);SCREEN(ABOMINATION_AFTER);SCREEN(MYSTERY_STORY);SCREEN(MYSTERY_CARDS);SCREEN(REWARD);SCREEN(RELIC_RESULT);fprintf(f,"\"DEFEAT\":%d}}\n",DEFEAT);fclose(f);return 0;
}
