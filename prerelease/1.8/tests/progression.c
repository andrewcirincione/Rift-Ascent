#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
static void setup(Game*g,int c,unsigned seed){game_new(g,seed);game_choose_character(g,c);g->act=1;g->floor=3;game_start_battle(g,NODE_FIGHT);g->energy=100;g->hp=g->maxhp;g->a18.available_cards=0xfffff;g->a18.available_potions=0xfffff;g->a18.available_relics=1023;g->enemy_count=2;for(int i=0;i<2;i++){g->enemies[i].art=0;g->enemies[i].hp=g->enemies[i].maxhp=100000;g->enemies[i].block=0;}g->target=0;}
static void win(Game*g,int act){g->act=act;g->floor=18;game_start_battle(g,NODE_BOSS);g->energy=100;g->hand_count=1;g->hand[0]=0;g->enemies[0].block=0;g->enemies[0].hp=1;g->target=0;game_play_card(g,0,0);CHECK(g->screen==(act==3?A18_ENDING:A18_TRUE_END));CHECK(game_validate(g));}
static void cards_test(void){
 int total=0;Game g,old;CardPreview p;unsigned counts[3][4]={{0}};
 CHECK(CARD_COUNT==394);CHECK(RELIC_COUNT==74);
 for(int id=334;id<394;id++){
  int c=(id-334)/20;counts[c][cards[id].rarity]++;
  for(int up=0;up<2;up++)for(int mod=0;mod<MOD_COUNT;mod++){
   int raw=id|(up?UPBIT:0);if(mod&&!game_can_modify(raw,mod))continue;raw|=mod<<CARD_MOD_SHIFT;
   setup(&g,c,170000+id);g.regent.stars=12;g.regent.forge=15;g.strength=3;g.dexterity=2;g.weak=1;g.enemies[0].vulnerable=1;g.enemies[0].poison=8;
   g.exhaust_count=4;for(int e=0;e<4;e++)g.exhaust[e]=1;g.hand_count=3;g.hand[0]=raw;g.hand[1]=1;g.hand[2]=0;g.selection=0;
   old=g;A18Profile before=*a18_profile();game_card_preview(&g,raw,0,&p);CHECK(!memcmp(&g,&old,sizeof(g)));CHECK(!memcmp(&before,a18_profile(),sizeof(before)));
   game_play_card(&g,0,0);CHECK(game_validate(&g));CHECK(g.energy>=0);CHECK(g.regent.stars>=0);total++;
  }
 }
 for(int c=0;c<3;c++)for(int r=0;r<4;r++)CHECK(counts[c][r]==(r==3?2:6));
 setup(&g,2,10);g.regent.powers[R_VOID_FORM-REGENT_FIRST]=1;g.hand_count=3;g.hand[0]=374;g.hand[1]=374;g.hand[2]=374;g.energy=3;game_play_card(&g,0,0);game_play_card(&g,0,0);CHECK(g.energy==3);game_play_card(&g,0,0);CHECK(g.energy==2);
 setup(&g,1,10);g.ninja.v[NS_AFTERIMAGE]=2;g.ninja.v[NS_POUNCE]=1;g.ninja.v[NS_PLANNER]=1;g.hand_count=1;g.hand[0]=355;g.energy=3;game_play_card(&g,0,0);CHECK(g.energy==3);CHECK(g.block>=9);CHECK(g.ninja.v[NS_SKILLS_TURN]==1);CHECK(g.discard[g.discard_count-1]&CARD_SLY);
 printf("PASS all 60 cards: %d base/upgrade/mod cases, preview purity, character hooks\n",total);
}
static void unlocks_test(void){
 Game g;
 for(unsigned seed=1;seed<=300;seed++){
  a18_storage_open(0);
  for(int c=0;c<3;c++)for(int w=1;w<=11;w++){
   CHECK(a18_begin_run(&g,c,c,seed*179u+w*11+c));unsigned mask=a18_profile()->cards[c];win(&g,3);
   CHECK(a18_profile()->wins[c]==(unsigned)w);CHECK(g.a18.award_win_number==w);
   if(w<=10){int a=g.a18.card_awards[0],b=g.a18.card_awards[1];CHECK(a18_card_character(a)==c&&a18_card_character(b)==c);CHECK(cards[a].rarity!=cards[b].rarity);CHECK(!(mask&(1u<<((a-334)%20))));CHECK(!(mask&(1u<<((b-334)%20))));}
   else CHECK(g.a18.card_awards[0]==-1&&g.a18.card_awards[1]==-1);
   CHECK(a18_popcount(a18_profile()->cards[c])==2*(w>10?10:w));CHECK(a18_secret_unlocked(c)==(w>=10));
   unsigned old=a18_profile()->wins[c];win(&g,3);CHECK(a18_profile()->wins[c]==old);
   for(int i=0;i<3;i++)game_action(&g,BTN_OK);CHECK(g.screen==A18_UNLOCK_CARDS);
   game_action(&g,BTN_OK);CHECK(g.screen==A18_UNLOCK_POTIONS);game_action(&g,BTN_OK);CHECK(g.screen==A18_UNLOCK_RELIC);game_action(&g,BTN_OK);
   CHECK(g.screen==(w>=10?A18_BEYOND:VICTORY));
   if(w>=10){for(int i=0;i<3;i++)game_action(&g,BTN_OK);CHECK(g.act==4);win(&g,4);CHECK(a18_profile()->true_endings&(1u<<c));CHECK(a18_profile()->act4_clears[c]==(unsigned)(w-9));CHECK(a18_profile()->best_acts[c]==4);unsigned clears=a18_profile()->act4_clears[c];win(&g,4);CHECK(a18_profile()->act4_clears[c]==clears);for(int i=0;i<3;i++)game_action(&g,BTN_OK);CHECK(g.screen==VICTORY);}
  }
  CHECK(a18_profile()->potions==0xfffff);CHECK(a18_profile()->relics==1023);CHECK(a18_profile()->true_endings==7);
 }
 puts("PASS 900 character progression sequences: distinct-rarity unlocks, tenth-win eligibility, Act IV clear counters, true endings, replay protection");
}
static void potions_test(void){Game g;for(int id=6;id<26;id++)for(int bonus=0;bonus<2;bonus++){
 setup(&g,id%3,id);g.hp=30;g.block=0;g.energy=2;g.weak=2;g.poison=3;g.vulnerable=2;g.hand_count=0;g.exhaust_count=3;for(int i=0;i<3;i++)g.exhaust[i]=1;
 if(bonus)g.relics[g.relic_count++]=17;g.potion_count=1;g.potions[0]=id;game_use_potion(&g,0);CHECK(g.potion_count==0);CHECK(game_validate(&g));CHECK(strcmp(game_potion_name(id),"None"));CHECK(strlen(game_potion_description(id))>12);
 }
 setup(&g,0,1);g.screen=MAP;g.hp=30;g.potion_count=1;g.potions[0]=6;game_use_potion(&g,0);CHECK(g.potion_count==1);g.potions[0]=24;game_use_potion(&g,0);CHECK(g.potion_count==0&&g.hp>30);
 puts("PASS 20 new potion effects, amplification, noncombat eligibility");}
static void saves_test(void){char path[]="/tmp/a18-saves-XXXXXX";CHECK(mkdtemp(path));CHECK(a18_storage_open(path));Game g,loaded;
 for(int s=0;s<4;s++){CHECK(a18_begin_run(&g,s,s%3,100+s));g.gold=300+s;CHECK(a18_checkpoint(&g));}
 CHECK(a18_storage_open(path));for(int s=0;s<4;s++){CHECK(a18_resume_slot(&loaded,s));CHECK(loaded.gold==300+s&&loaded.character==s%3);}
 CHECK(a18_resume_slot(&g,0));win(&g,3);CHECK(a18_checkpoint(&g));CHECK(a18_storage_open(path));CHECK(a18_profile()->wins[0]==1);CHECK(a18_resume_slot(&g,0));win(&g,3);CHECK(a18_profile()->wins[0]==1);CHECK(a18_checkpoint(&g));
 CHECK(a18_begin_run(&g,1,0,899));CHECK(a18_popcount(g.a18.available_cards)==2);CHECK(a18_resume_slot(&g,2));CHECK(g.a18.available_cards==0);
 game_new(&g,1);g.selection=1;game_action(&g,BTN_OK);CHECK(g.screen==A18_SLOTS);g.selection=1;game_action(&g,BTN_OK);CHECK(g.screen==A18_SLOT_CONFIRM);game_action(&g,BTN_BACK);CHECK(a18_resume_slot(&g,1)&&g.seed==899);
 g.gold=90;CHECK(a18_checkpoint(&g));g.gold=91;CHECK(a18_checkpoint(&g));char file[300];unsigned gens[2]={0};for(int i=0;i<2;i++){snprintf(file,sizeof(file),"%s/A18_SLOT_2.%d",path,i);FILE*f=fopen(file,"rb");CHECK(f);fseek(f,8,SEEK_SET);CHECK(fread(&gens[i],4,1,f)==1);fclose(f);}int newest=gens[1]>gens[0];snprintf(file,sizeof(file),"%s/A18_SLOT_2.%d",path,newest);FILE*f=fopen(file,"wb");fputs("broken",f);fclose(f);CHECK(a18_storage_open(path));CHECK(a18_resume_slot(&g,1)&&g.gold==90);
 puts("PASS four independent dual-generation saves, shared profile, replay safety, overwrite cancellation and corruption recovery");
}
static void encounter_test(void){Game g;int counts[3]={0};for(int c=0;c<3;c++)for(unsigned seed=1;seed<=1000;seed++){game_new(&g,seed);g.act=c+1;g.floor=5;game_start_battle(&g,NODE_FIGHT);CHECK(game_validate(&g));counts[c]+=g.enemy_count==2;for(int e=0;e<g.enemy_count;e++)CHECK(g.enemies[e].intent!=7&&g.enemies[e].intent!=10&&g.enemies[e].intent!=16);if(g.enemy_count==2)CHECK(g.enemies[0].art!=g.enemies[1].art);}for(int c=0;c<3;c++)CHECK(counts[c]>250&&counts[c]<650);printf("PASS multi-enemy fights per 1000: %d/%d/%d; no heavy openers or clone pairs\n",counts[0],counts[1],counts[2]);}
int main(void){CHECK(offsetof(Game,a18)==28172);a18_storage_open(0);cards_test();potions_test();encounter_test();unlocks_test();saves_test();puts("ALL 1.8 PRERELEASE NATIVE TESTS PASSED");return 0;}
