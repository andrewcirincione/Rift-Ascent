#include "game.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"1.0.2 pause FAIL: %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
int main(void){
 Game a,b;game_new(&a,12345);
 game_action(&a,BTN_OK);CHECK(a.screen==MAP);
 game_action(&a,BTN_PAUSE);CHECK(a.screen==PAUSE_MENU);
 CHECK(game_pause_source()==MAP);
 game_action(&a,BTN_BACK);CHECK(a.screen==MAP);
 game_action(&a,BTN_PAUSE);
 b=a;b.screen=game_pause_source();
 CHECK(game_save(&b,"/tmp/rift_102_pause.dat")==1);
 CHECK(game_load(&b,"/tmp/rift_102_pause.dat")==1);
 CHECK(b.screen==MAP && b.seed==a.seed);
 a.prior_screen=MAP;a.screen=DECK_VIEW;a.selection=0;
 game_action(&a,BTN_PAUSE);
 CHECK(game_pause_source()==DECK_VIEW&&a.prior_screen==MAP);
 game_action(&a,BTN_PAUSE);
 CHECK(a.screen==DECK_VIEW&&a.prior_screen==MAP);
 remove("/tmp/rift_102_pause.dat");
 puts("PASS: SELECT pause, nested-menu resume and loadable snapshot");
 return 0;
}
