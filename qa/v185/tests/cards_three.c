#include <assert.h>
#include <stdio.h>
#include "../source/game.c"
int main(){int cases=0;for(int id=0;id<CARD_COUNT;id++)if(game_card_is_playable(id))for(int up=0;up<2;up++)for(int c=0;c<3;c++){
 Game g,h;game_new(&g,id*91+up+17);game_choose_character(&g,c);game_start_battle(&g,NODE_FIGHT);g.enemy_count=3;g.revision184.custom=1;g.ascent.cards=ASCENT_ALL_CARDS;g.hp=g.maxhp=5000;g.energy=50;g.regent.stars=15;g.relic_count=0;for(int e=0;e<3;e++){init_enemy(&g,&g.enemies[e],e);g.enemies[e].hp=g.enemies[e].maxhp=5000;}
 g.hand_count=3;g.hand[0]=id|(up?UPBIT:0);g.hand[1]=0;g.hand[2]=1;g.target=2;g.selection=0;h=g;CardPreview p;game_card_preview(&g,g.hand[0],2,&p);assert(!memcmp(&g,&h,sizeof(g)));game_play_card(&g,0,2);for(int step=0;step<30;step++){game_update(&g);if(g.screen==COMBAT)break;if(g.screen==NINJA_CHOICE||g.screen==REGENT_CHOICE||g.screen==CARD_CHOICE){game_action(&g,BTN_BACK);game_action(&g,BTN_OK);}else break;}cases++;
 }printf("PASS %d card/upgrade/character cases with three foes, target 3 and immutable previews.\n",cases);}
