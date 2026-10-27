#include "../source/game.c"
#include <assert.h>
int main(void){printf("%zu\n",offsetof(Game,deck));
 for(int ch=0;ch<2;ch++){Game g;game_new(&g,81);game_choose_character(&g,ch);g.gold=200;game_begin_event(&g,6);assert(game_save(&g,ch?"qa/v150b/evidence/state-armory-nightshade.dat":"qa/v150b/evidence/state-armory-warden.dat"));}
 Game g;game_new(&g,82);game_start_battle(&g,NODE_FIGHT);g.hand_count=5;g.hand[0]=83;for(int i=1;i<5;i++)g.hand[i]=0;g.energy=5;g.selection=0;assert(game_save(&g,"qa/v150b/evidence/state-warden-replacement.dat"));return 0;
}
