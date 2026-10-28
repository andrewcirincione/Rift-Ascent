#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../source/game.c"
static void clear(Game*g,int act){g->act=act;g->endless=act>3;game_start_battle(g,NODE_BOSS);for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp=0;check_win(g);}
int main(void){
 char root[]="/tmp/rift-new180-XXXXXX";assert(mkdtemp(root));assert(ascent_open(root));
 assert(CARD_COUNT==394&&RELIC_COUNT==74&&!ascent_debug());
 for(int c=0;c<3;c++){
  for(int win=0;win<12;win++){
   Game g,again;assert(ascent_begin(&g,c,c,1000+100*c+win));assert(game_validate(&g));
   unsigned before=ascent_profile()->cards[c];clear(&g,3);assert(g.screen==ASCENT_ENDING);assert(game_validate(&g));
   assert(ascent_profile()->wins[c]==(unsigned)win+1);
   if(win<10){int a=g.ascent.unlocked_cards[0],b=g.ascent.unlocked_cards[1];assert(a>=334&&b>=334&&a!=b&&cards[a].rarity!=cards[b].rarity);assert(ascent_popcount(ascent_profile()->cards[c])==2*(win+1));assert((ascent_profile()->cards[c]&before)==before);}
   else assert(g.ascent.unlocked_cards[0]==-1&&g.ascent.unlocked_cards[1]==-1);
   assert(ascent_checkpoint(&g));assert(ascent_resume(&again,c));assert(again.screen==ASCENT_ENDING);assert(ascent_complete(&again,0));assert(ascent_profile()->wins[c]==(unsigned)win+1);
   assert(ascent_secret(c)==(win>=9));
   for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==ASCENT_CARDS);game_action(&g,BTN_OK);assert(g.screen==ASCENT_POTIONS);game_action(&g,BTN_OK);assert(g.screen==ASCENT_RELIC);game_action(&g,BTN_OK);
   if(win>=9){assert(g.screen==ASCENT_REVELATION);for(int i=0;i<3;i++)game_action(&g,BTN_OK);}
   else {assert(g.screen==VICTORY);game_action(&g,BTN_OK);}
   assert(g.act==4&&g.screen==ACT_INTRO);
   if(win>=9){clear(&g,4);assert(g.screen==ASCENT_TRUE_END);assert(ascent_profile()->ascended&(1u<<c));assert(ascent_profile()->wins[c]==(unsigned)win+1);
    for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==VICTORY);
    game_action(&g,BTN_OK);assert(g.act==5&&g.screen==ACT_INTRO);
   }
  }
 }
 assert(ascent_popcount(ascent_profile()->potions)==20&&ascent_popcount(ascent_profile()->relics)==10);
 assert(ascent_open(root));assert(ascent_profile()->wins[0]==12&&ascent_profile()->wins[1]==12&&ascent_profile()->wins[2]==12);
 int pairs=0,total=0;for(int act=1;act<=6;act++)for(int seed=1;seed<=1000;seed++){Game g;game_new(&g,seed);g.act=act;g.endless=act>3;g.floor=seed%18;game_start_battle(&g,NODE_FIGHT);pairs+=g.enemy_count==2;total++;if(g.enemy_count==2)assert(g.enemies[0].art!=g.enemies[1].art);assert(game_validate(&g));}
 assert(pairs>total*22/100&&pairs<total*28/100);printf("PASS pairs %d/%d; 36 wins; all unlocks; idempotence; new profile cold load\n",pairs,total);
 for(int id=334;id<394;id++)for(int up=0;up<2;up++){
  Game g;assert(ascent_begin(&g,3,(id-334)/20,10000+id));g.act=1;game_start_battle(&g,NODE_FIGHT);g.hp=g.maxhp=1000;g.energy=20;g.regent.stars=30;g.regent.forge=12;g.hand_count=2;g.hand[0]=id|(up?UPBIT:0);g.hand[1]=g.character==1?N_DEFEND_SILENT:g.character==2?R_DEFEND_REGENT:1;g.selection=0;
  for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=g.enemies[e].maxhp=10000;
  CardPreview preview;Game original=g;game_card_preview(&g,g.hand[0],0,&preview);assert(!memcmp(&g,&original,sizeof(g)));
  game_play_card(&g,0,0);assert(g.turn_played==1);assert(game_validate(&g));assert(g.screen==COMBAT);
 }
 puts("PASS 120 fresh card base/upgraded resolutions and preview isolation");
 return 0;
}
