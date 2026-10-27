#include "source/game.c"
#include <assert.h>
#include <stdio.h>
int main(void){Game g;char desc[320],expect[50];int n=0;int strengths[]={-5,0,5,20},vulns[]={0,1,2,10,99};
 for(int up=0;up<2;up++)for(int s=0;s<4;s++)for(int weak=0;weak<2;weak++)for(int v=0;v<5;v++)for(int cruelty=0;cruelty<=50;cruelty+=25)for(int glass=0;glass<2;glass++){
 game_new(&g,140);g.relic_count=0;if(glass)g.relics[g.relic_count++]=3;game_start_battle(&g,NODE_FIGHT);
 g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=30000;g.enemies[0].block=7;g.enemies[0].vulnerable=vulns[v];g.strength=strengths[s];g.weak=weak;g.ic[IC_CRUELTY]=cruelty;g.hand_count=1;g.hand[0]=35|(up?UPBIT:0);g.selection=0;
 int expected=MAX(0,4+(2+up)*vulns[v]+strengths[s]);if(weak)expected=expected*3/4;if(vulns[v]){expected=expected*3/2;expected=expected*(100+cruelty)/100;}expected+=5*glass;
 game_card_description_now(&g,g.hand[0],0,desc,sizeof(desc));snprintf(expect,sizeof(expect),"Deal %d damage after modifiers.",expected);assert(strstr(desc,expect));snprintf(expect,sizeof(expect),"Base: 4 + %d",2+up);assert(strstr(desc,expect));snprintf(expect,sizeof(expect),"(%d now)",vulns[v]);assert(strstr(desc,expect));
 game_play_card(&g,0,0);assert(g.enemies[0].hp==30000-MAX(0,expected-7));n++;
 }
 assert(strstr(card_description(35),"2 base damage for each turn of Vulnerable"));assert(strstr(card_description(35|UPBIT),"3 base damage for each turn of Vulnerable"));
 printf("PASS %d Bully/Bully+ cases: Vulnerable duration bonus, negative/positive Strength, Weak, Vulnerable multiplier, Cruelty, Glass Emblem, Block and exact live description\n",n);
}
