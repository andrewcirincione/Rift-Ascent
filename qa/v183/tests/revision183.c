#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
static void allowed(const Game*g,int raw){int id=CARD_ID(raw);assert(id<CARD_COUNT);if(id>=334)assert(ascent_card_available(g,id));}
int main(void){char root[]="/tmp/rift183-gates-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,b;unsigned checks=0;
 for(int c=0;c<3;c++)for(int mask=0;mask<22;mask++){
  game_new(&g,183+c*100+mask);game_choose_character(&g,c);g.ascent.cards=mask==0?0:mask==21?ASCENT_ALL_CARDS:1u<<(mask-1);
  for(int i=0;i<300;i++){
   for(int rarity=0;rarity<4;rarity++){allowed(&g,reward_of_rarity(&g,rarity,-1,-1));checks++;}
   for(int kind=0;kind<3;kind++){g.battle_type=kind==0?NODE_FIGHT:kind==1?NODE_ELITE:NODE_BOSS;fill_rewards(&g);for(int k=0;k<3;k++){allowed(&g,g.reward[k]);checks++;}}
   allowed(&g,card_reward_except(&g,-1,-1));checks++;
   for(int type=0;type<2;type++){allowed(&g,event_typed_card(&g,type));checks++;}
   for(int event=0;event<4;event++){int ids[]={2,6,17,34};game_begin_event(&g,ids[event]);for(int k=0;k<(event==2?1:2);k++){allowed(&g,g.reward[k]);checks++;}}
   g.expedition.story_id=i%20;expedition_prepare_gift(&g);if(!g.expedition.gift_kind){allowed(&g,g.expedition.gift);checks++;}
   for(int mode=0;mode<3;mode++){g.regent.task_count=0;regent_offer(&g,R_HEIRLOOM_HAMMER,0,0,mode,0);for(int k=0;k<g.regent.option_count;k++){allowed(&g,g.regent.options[k]);checks++;}}
   allowed(&g,regent_colorless(&g,i&1));checks++;
  }
  int n=g.deck_count;for(int id=334;id<394;id++){g.deck_count=n;add_card(&g,id|UPBIT);assert(g.deck_count==n+(ascent_card_available(&g,id)!=0));}
 }
 printf("PASS %u card source checks across all characters, zero/single/full unlock masks; locked acquisition guard\n",checks);
 for(int tab=0;tab<5;tab++){game_new(&g,183);g.screen=ASCENT_COLLECTION;g.scroll=tab;int n=game_collection_count(tab),custom=game_collection_unlock_count(tab);assert(n>custom);for(int i=0;i<n;i++){g.selection=i;assert(game_validate(&g));int id=game_collection_id(tab,i);if(i<custom)assert(id>=(tab<3?334:tab==3?6:64));else assert(tab<3?id<334:tab==3?id<6:id<64||id>=74);for(int j=0;j<i;j++)assert(game_collection_id(tab,j)!=id);}g.selection=n-1;game_action(&g,BTN_OK);assert(!g.selection);game_action(&g,BTN_ALT);assert(g.selection==custom);game_action(&g,BTN_ALT);assert(!g.selection);game_action(&g,BTN_TAB_RIGHT);assert(g.scroll==(tab+1)%5);printf("PASS tab %d: %d unlockable, %d base entries\n",tab,custom,n-custom);}
 assert(ascent_begin(&g,0,0,183));g.screen=MAP;g.gold=789;g.revision182.rare_offset=23;g.revision182.event_seen_hi=7;g.ascent.cards=5;g.version=182;assert(ascent_write("ASCENT18_RUN_1",&g,sizeof(g),50));assert(ascent_resume(&b,0)&&b.version==183&&b.gold==789&&b.revision182.rare_offset==23&&b.revision182.event_seen_hi==7&&b.ascent.cards==5);
 char path[300];snprintf(path,sizeof(path),"%s/SAVE.DAT",root);unsigned sum=hash_bytes((void*)&g,sizeof(g));FILE*f=fopen(path,"wb");assert(f);fwrite(&sum,4,1,f);fwrite(&g,sizeof(g),1,f);fclose(f);assert(game_load(&b,path)&&b.version==183&&b.gold==789);
 g.version=183;g.act=1;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=0;check_win(&g);assert(g.screen==RELIC_RESULT&&!g.message[0]);
 g.act=1;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=1;g.enemies[0].block=0;g.potion_count=1;g.potions[0]=3;game_use_potion(&g,0);assert(g.screen==RELIC_RESULT&&!g.message[0]&&!g.potion_count);
 puts("PASS killing Fire potion leaves no boss-reward text for the combat notification");
 puts("PASS 1.8.2 slot and legacy snapshot migration retains progress and rarity state; no boss reward message");return 0;}
