#include "source/game.c"
#include <assert.h>
#include <stdio.h>
int main(void){Game g,b;int cases=0;int origins[]={REST,EVENT,SHOP,MAP,REWARD,COMBAT};
 for(int context=0;context<8;context++)for(int count=1;count<=(context==6?10:128);count++)for(int col=0;col<count;col++){
  game_new(&g,140);g.deck_count=count;for(int i=0;i<count;i++)g.deck[i]=0;g.selection=col;
  if(context<6){g.screen=DECK_VIEW;g.prior_screen=origins[context];g.draw_count=count;}
  else {g.screen=CARD_CHOICE;g.next_screen=context==6?17:25;g.hand_count=context==6?count:0;g.discard_count=context==7?count:0;}
  b=g;game_action(&g,BTN_DOWN);int expected=col/3<(count-1)/3?MIN((col/3+1)*3+col%3,count-1):col;assert(g.selection==expected);assert(g.gold==b.gold&&g.deck_count==b.deck_count&&g.screen==b.screen);
  if(col/3==(count-1)/3)assert(g.selection==col);
  game_action(&g,BTN_UP);assert(g.selection==(expected>=3?expected-3:expected));cases++;
 }
 game_new(&g,140);g.screen=DECK_VIEW;g.prior_screen=REST;g.deck_count=10;g.deck[9]=35;g.selection=8;game_action(&g,BTN_DOWN);assert(g.selection==9);game_action(&g,BTN_OK);assert(g.deck[9]==(35|UPBIT)&&g.screen==MAP);
 game_new(&g,140);g.screen=DECK_VIEW;g.prior_screen=SHOP;g.gold=100;g.selection=8;g.deck_count=10;game_action(&g,BTN_DOWN);assert(g.selection==9);game_action(&g,BTN_OK);assert(g.deck_count==9&&g.gold==40&&g.screen==SHOP);
 printf("PASS %d grid positions: short final rows from all columns, page boundaries, one/two/full final rows, no sideways bottom-row jump; upgrade, removal, browse, combat piles, exhaust and discarded-card choice; correct card confirmed\n",cases);
}
