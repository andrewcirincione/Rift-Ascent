#include "game.h"
#include "render.c"
#include <assert.h>
int main(void){Game g;int count=0,maxh=0;for(int id=REGENT_FIRST;id<EXTRA_LAST;id++)for(int mod=0;mod<MOD_COUNT;mod++)for(int up=0;up<2;up++){
 if(mod&&!game_can_modify(id,mod))continue;game_new(&g,160);game_choose_character(&g,2);game_start_battle(&g,NODE_FIGHT);g.hp=g.maxhp=500;g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=100000;g.energy=4;g.regent.stars=15;g.regent.forge=100000;g.regent.created=10000;g.regent.stars_gained=100;g.regent.skills=20;g.regent.plays_combat=10000;g.strength=20;g.dexterity=10;g.weak=1;g.enemies[0].vulnerable=1;g.hand_count=1;g.hand[0]=id|(up?UPBIT:0)|(mod<<CARD_MOD_SHIFT);g.selection=0;card_combat_context=&g;R135CardLayout l=r150_hand_preview_layout(g.hand[0]);if(l.height>maxh)maxh=l.height;if(l.height>162)fprintf(stderr,"overflow %d mod%d up%d height%d width%d\n",id,mod,up,l.height,l.width);assert(l.height<=162);count++;
 }
 assert(r160_star_count(0)==0&&r160_star_count(3)==6&&r160_star_count(14)==28&&r160_star_count(15)==30&&r160_star_count(999)==30);printf("PASS %d live Proxy/Colorless modified/upgraded previews with large combat modifiers, max height%d; Star particle cap 15\n",count,maxh);return 0;}
