#include "game.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
int main(){Game old,g;assert(game_load(&old,"../evidence1310/state-release-combat.dat"));
 for(int version=16;version<=17;version++){char file[90];snprintf(file,sizeof(file),"../evidence1311/legacy-v%d.dat",version);assert(game_load(&g,file));assert(g.version==19&&!g.endless&&g.act==old.act&&g.hp==old.hp&&g.gold==old.gold&&g.deck_count==old.deck_count);assert(!memcmp(g.enemies,old.enemies,sizeof(g.enemies)));}
 g=old;g.endless=1;g.act=1001;g.screen=DEFEAT;g.hp=0;RiftProfile p,q;profile_init(&p);assert(profile_record(&p,&g));assert(p.furthest_floor==15000u+g.floor);assert(profile_save(&p,"../evidence1311/endless-profile.dat"));assert(profile_load(&q,"../evidence1311/endless-profile.dat"));assert(!memcmp(&p,&q,sizeof(p)));
 puts("PASS v16/v17/v18 migration, expanded enemy stats, endless lifetime floor record and reload");return 0;}
