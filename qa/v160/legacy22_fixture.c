#include "../v151/source/game.c"
#include <assert.h>
int main(void){Game g;game_new(&g,16022);game_choose_character(&g,CHAR_NIGHTSHADE);game_start_battle(&g,NODE_FIGHT);g.energy=20;g.hand_count=2;g.hand[0]=N_STRIKE_SILENT|UPBIT|CARD_FREE|CARD_SLY|(MOD_ECHO<<CARD_MOD_SHIFT)|(900<<CARD_INSTANCE_SHIFT);g.hand[1]=N_SURVIVOR;game_play_card(&g,1,0);assert(g.screen==NINJA_CHOICE);assert(game_save(&g,"qa/v160/evidence/legacy-v22-choice.dat"));return 0;}
