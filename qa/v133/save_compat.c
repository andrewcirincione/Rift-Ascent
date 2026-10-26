#include "game.h"
#include <assert.h>
#include <stdio.h>
int main(){Game g;assert(game_load(&g,"../evidence/save-v17.dat"));assert(g.version==18&&g.floor==11&&g.gold==321&&g.hp==47);assert(g.ic[IC_BARRICADE]==2&&g.ic[IC_PYRE]==3);assert(game_validate(&g));assert(game_load(&g,"../evidence/save-v17-combat.dat"));assert(g.screen==COMBAT&&g.hp==47&&g.floor==11&&g.gold==321);assert(game_validate(&g));assert(game_load(&g,"../evidence/save-v16-combat.dat"));assert(g.version==GAME_VERSION&&g.hp==47&&g.floor==11);for(int i=0;i<48;i++)assert(g.ic[i]==0);puts("PASS: v16 and actual v17 map and combat saves migrate with stats, piles, enemies, powers intact");}
