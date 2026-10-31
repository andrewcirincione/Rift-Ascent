/* Compare every affordable event option against leaving, then play the same
   next four rooms with the same policy and RNG. This measures consequences,
   rather than choosing from the inventory score alone. */
#define main campaign_main
#include "campaigns.c"
#undef main
static int rollout(Game*g,int act_number,int floor){
 for(int n=0;n<2500;n++){
  if(g->hp<=0||g->screen==DEFEAT)return 0;
  if(g->act>3||g->screen==VICTORY||(g->act-act_number)*MAP_ROWS+g->floor>=floor+4)return 1;
  if(!act(g))return -1;
 }return -1;
}
int main(void){char root[]="/tmp/rift-decisions-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));unsigned sampled[EVENT_COUNT]={0};
 puts("character,seed,event,choice,initial_hp,initial_gold,survived,end_hp,end_gold,end_act,end_floor");
 for(int c=0;c<3;c++)for(int seed=1;seed<=20;seed++){
 Game g;ascent_account.cards[c]=ASCENT_ALL_CARDS;ascent_account.potions=0xfffff;ascent_account.relics=1023;assert(ascent_begin(&g,0,c,seed*7919u+80000));
 for(int actions=0;actions<15000&&g.hp>0&&g.act<=3&&g.screen!=DEFEAT&&g.screen!=VICTORY;actions++){
  if(g.screen==EVENT&&sampled[g.effect_turn]<12){int e=g.effect_turn;sampled[e]++;
   for(int choice=0;choice<game_event_choice_count(&g);choice++)if(game_event_choice_available(&g,choice)){
    Game h=g;h.selection=choice;game_action(&h,BTN_OK);if(h.screen==EVENT_CARD)event_card(&h);
    int survived=rollout(&h,g.act,g.floor);
    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",c,seed,e,choice,g.hp,g.gold,survived,h.hp,h.gold,h.act,h.floor);fflush(stdout);
   }
  }
  if(!act(&g))break;
 }
 }
 return 0;
}
