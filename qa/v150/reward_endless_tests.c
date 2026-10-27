#include "source/game.c"
#include "expected_rarities.h"
#include <assert.h>
#include <stdio.h>
int main(void){Game g,b,t;int counts[3]={0};
 for(int id=0;id<CARD_COUNT;id++)if(expected_rarity[id]>=0)assert(cards[id].rarity==expected_rarity[id]);
 for(int endless=0;endless<2;endless++)for(int act=1;act<=(endless?12:3);act++)for(int seed=1;seed<=500;seed++){
  game_new(&g,seed);g.act=act;g.endless=endless;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=0;int deck=g.deck_count,gold=g.gold;check_win(&g);
  if(!endless&&act==3){assert(g.screen==VICTORY&&g.deck_count==deck&&g.gold==gold);continue;}
  assert(g.screen==RELIC_RESULT);game_action(&g,(seed&1)?BTN_OK:BTN_BACK);assert(g.screen==REWARD);
  for(int i=0;i<3;i++){
   int id=CARD_ID(g.reward[i]);assert(solo_reward_eligible(id)&&expected_rarity[id]==2);counts[2]++;
   for(int j=0;j<i;j++)assert(id!=CARD_ID(g.reward[j]));
  }
  if(seed==1){assert(game_save(&g,"qa/v150/evidence/boss-reward-save.dat"));assert(game_load(&t,"qa/v150/evidence/boss-reward-save.dat"));assert(!memcmp(&t,&g,sizeof(g)));}
 }
 for(int screen=0;screen<3;screen++){
  game_new(&g,140);g.screen=screen==0?REWARD:screen==1?REWARD_PREVIEW:DECK_VIEW;g.prior_screen=REWARD;g.battle_type=NODE_BOSS;g.win_screen=1;
  g.reward[0]=21;g.reward[1]=8;g.reward[2]=46;
  assert(game_save(&g,"qa/v150/evidence/legacy-boss-offer.dat"));assert(game_load(&t,"qa/v150/evidence/legacy-boss-offer.dat"));
  for(int i=0;i<3;i++)assert(expected_rarity[CARD_ID(t.reward[i])]==2);
 }
 /* Full normal boss ending to endless and to menu, with exact possessions. */
 for(int choice=0;choice<3;choice++){
  game_new(&g,140);g.act=3;g.hp=25;g.gold=777;g.relics[g.relic_count++]=7;g.deck[g.deck_count++]=48|UPBIT;
  game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=0;check_win(&g);assert(g.screen==VICTORY);
  RiftProfile profile;profile_init(&profile);assert(profile_record(&profile,&g));b=g;
  if(choice==1)game_action(&g,BTN_RIGHT);game_action(&g,choice==2?BTN_BACK:BTN_OK);
  if(choice){assert(g.screen==TITLE&&!g.endless);continue;}
  assert(g.endless&&g.act==4&&g.screen==ACT_INTRO&&g.floor==0&&g.hp==g.maxhp&&g.gold==b.gold&&g.seed==b.seed);
  assert(g.deck_count==b.deck_count&&!memcmp(g.deck,b.deck,sizeof(g.deck)));assert(g.relic_count==b.relic_count&&!memcmp(g.relics,b.relics,sizeof(g.relics)));assert(g.potion_count==b.potion_count&&!memcmp(g.potions,b.potions,sizeof(g.potions)));
  assert(game_save(&g,"qa/v150/evidence/continued-endless.dat"));assert(game_load(&t,"qa/v150/evidence/continued-endless.dat"));assert(!memcmp(&g,&t,sizeof(g)));
  for(int i=0;i<120;i++)game_update(&g);assert(g.screen==BLESSING);game_action(&g,BTN_OK);assert(g.screen==MAP);
  game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=0;check_win(&g);assert(g.screen==RELIC_RESULT);game_action(&g,BTN_BACK);assert(g.screen==REWARD);g.selection=3;game_action(&g,BTN_OK);assert(g.screen==ACT_INTRO&&g.act==5);
  g.screen=DEFEAT;g.hp=0;assert(!profile_record(&profile,&g));assert(profile.runs==1&&profile.wins==1&&profile.losses==0);
 }
 printf("PASS 88 card rarity definitions (84 inherited plus 4 original); %d guaranteed Rare boss offers across Acts I-XII, unique choices, claim/skip and saved-offer repair; normal victory -> Act IV -> Act V, saved possessions, full heal, blessing, menu routes and one lifetime win\n",counts[2]);
}
