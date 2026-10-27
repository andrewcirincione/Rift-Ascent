#include "game.h"
#include "render.c"
#include <assert.h>
int main(void){int highest=0,inventory=0,cases=0;for(int id=0;id<CARD_COUNT;id++)for(int mod=0;mod<MOD_COUNT;mod++)for(int up=0;up<2;up++){if(mod&&!game_can_modify(id,mod))continue;int raw=id|(mod<<CARD_MOD_SHIFT)|(up?UPBIT:0);R135CardLayout a=r135_card_layout(raw,138),b=r135_card_layout(raw,140);if(a.height>highest)highest=a.height;if(b.height>inventory)inventory=b.height;if(a.height>180||b.height>166)fprintf(stderr,"overflow card=%d mod=%d up=%d upgrade=%d inventory=%d\n",id,mod,up,a.height,b.height);assert(a.height<=180&&b.height<=166);cases++;}printf("PASS %d upgrade/inventory layouts: max heights %d/%d; complete descriptions stay above the footer\n",cases,highest,inventory);return 0;}
