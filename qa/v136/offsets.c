#include "game.h"
#include <stdio.h>
int main(void){printf("{\"enemies\":%zu,\"enemy_size\":%zu,\"enemy_hp\":%zu,\"enemy_block\":%zu,\"hand_count\":%zu,\"ic\":%zu}\n",offsetof(Game,enemies),sizeof(Enemy),offsetof(Enemy,hp),offsetof(Enemy,block),offsetof(Game,hand_count),offsetof(Game,ic));}
