#include "source/game.c"
#include <assert.h>
#include <stdio.h>
int main(void){Game g,b;char desc[320],line[80];int strengths[]={-5,0,5,20},counts[]={0,1,5,20,128};int tested=0;
 for(int up=0;up<2;up++)for(int s=0;s<4;s++)for(int weak=0;weak<2;weak++)for(int v=0;v<2;v++)for(int e=0;e<5;e++){
  game_new(&g,140);g.relic_count=0;game_start_battle(&g,NODE_FIGHT);g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=30000;g.enemies[0].block=7;g.enemies[0].vulnerable=v;g.strength=strengths[s];g.weak=weak;g.exhaust_count=counts[e];for(int i=0;i<g.exhaust_count;i++)g.exhaust[i]=1;
  g.hand_count=1;g.hand[0]=13|(up?UPBIT:0);g.selection=0;int expected=MAX(0,6+(3+up)*counts[e]+strengths[s]);if(weak)expected=expected*3/4;if(v)expected=expected*3/2;
  game_card_description_now(&g,g.hand[0],0,desc,sizeof(desc));snprintf(line,sizeof(line),"Deal %d damage after modifiers.",expected);assert(strstr(desc,line));snprintf(line,sizeof(line),"Base: 6 + %d",3+up);assert(strstr(desc,line));snprintf(line,sizeof(line),"(%d now)",counts[e]);assert(strstr(desc,line));game_play_card(&g,0,0);assert(g.enemies[0].hp==30000-MAX(0,expected-7));tested++;
 }
 int blocked=0;
 for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++)for(int energy=0;energy<4;energy++){
  game_new(&g,140);game_start_battle(&g,NODE_FIGHT);g.hand_count=1;g.hand[0]=id|(up?UPBIT:0);g.energy=energy;int cost=game_card_cost_now(&g,g.hand[0]);if(cost<=energy||!game_card_is_playable(g.hand[0])||id==63||id==85)continue;
  b=g;game_play_card(&g,0,0);assert(strstr(g.message,"more Energy"));memcpy(g.message,b.message,sizeof(g.message));assert(!memcmp(&g,&b,sizeof(g)));blocked++;
 }
 game_new(&g,140);game_start_battle(&g,NODE_FIGHT);g.energy=0;assert(game_card_cost_now(&g,35)==0);assert(game_card_cost_now(&g,10)==-1);g.ic[IC_CORRUPTION]=1;assert(game_card_cost_now(&g,9)==0);g.ic[IC_UNRELENTING]=1;assert(game_card_cost_now(&g,7)==0);assert(game_card_cost_now(&g,7|CARD_FREE)==0);
 printf("PASS %d Ashen Strike cases with current Exhaust pile, upgrades, Strength, Weak, Vulnerable and Block; %d unaffordable plays preserve all state; free, X-cost, Corruption and Unrelenting costs remain valid\n",tested,blocked);
}
