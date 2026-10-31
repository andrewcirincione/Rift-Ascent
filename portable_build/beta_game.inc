/* Journey Forge and run recap, carried forward from Beta 1. */
int game_beta_modifier(const Game*g,int choice){
 if(choice==2)return MOD_ECHO;
 const int mods[]={MOD_KEEN,MOD_WARD,MOD_BOUND};
 return mods[route_hash(g->seed^(unsigned)g->act*913u^(unsigned)g->floor*37u)%3];
}
int game_beta_eligible(const Game*g,int choice,int raw){
 if(choice==0)return game_card_is_upgradeable(raw);
 if(choice<0||choice>2||(choice==2&&g->maxhp<=10))return 0;
 return game_can_modify(raw,game_beta_modifier(g,choice));
}
int game_beta_preview(const Game*g,int choice,int raw){
 if(!game_beta_eligible(g,choice,raw))return raw;
 return choice==0?raw|UPBIT:raw|(game_beta_modifier(g,choice)<<CARD_MOD_SHIFT);
}
int game_beta_offer(const Game*g){
 int screen=g->screen==PAUSE_MENU?g->prior_screen:g->screen;
 if(screen!=EXPEDITION_GIFT&&screen!=BETA_FORGE&&screen!=BETA_FORGE_CARD)return 0;
 for(int i=0;i<g->deck_count;i++)for(int c=0;c<3;c++)if(game_beta_eligible(g,c,g->deck[i]))return 1;
 return 0;
}
const char*game_enemy_plan(int art){
 switch(art){
 case 6:return "Cycle: Brace > Crush > Purge > Strike";
 case 15:return "Cycle: Brace > Sear > Crush > Recover";
 case 24:return "Cycle: Expose > Pierce > Purge > Recover";
 case 8:return "Cycle: Fortify > Flurry > Crush > Recover";
 case 44:return "Cycle: Brace > Dread > Rend > Recover";
 case 23:return "Cycle: Fortify > Static > Flurry > Recover";
 case 22:return "Cycle: Brace > Double > Crush > Recover";
 case 50:return "Cycle: Brace > Sear > Cinder > Recover";
 case 35:return "Cycle: Guard > Crush > Rend > Recover";
 default:return "";
 }
}
static int beta_action(Game*g,int b){
 if(g->screen==VICTORY||g->screen==DEFEAT){
  g->scroll=CLAMP(g->scroll,0,2);
  if(b==BTN_TAB_LEFT||b==BTN_TAB_RIGHT||b==BTN_ALT||b==BTN_POTION){
   g->scroll=b==BTN_ALT?1:b==BTN_POTION?2:(g->scroll+(b==BTN_TAB_LEFT?2:1))%3;g->master_index=0;return 1;
  }
  if(g->scroll){
   int count=g->scroll==1?g->deck_count:g->relic_count;
   if(b==BTN_BACK){g->scroll=g->master_index=0;return 1;}
   if(b==BTN_UP)g->master_index=MAX(0,g->master_index-(g->scroll==1?3:1));
   if(b==BTN_DOWN)g->master_index=MIN(MAX(0,count-1),g->master_index+(g->scroll==1?3:1));
   if(b==BTN_LEFT)g->master_index=MAX(0,g->master_index-1);
   if(b==BTN_RIGHT)g->master_index=MIN(MAX(0,count-1),g->master_index+1);
   return 1;
  }
 }
 if(g->screen==BETA_FORGE){
  if(b==BTN_UP||b==BTN_LEFT)g->selection=(g->selection+2)%3;
  if(b==BTN_DOWN||b==BTN_RIGHT)g->selection=(g->selection+1)%3;
  if(b==BTN_BACK){g->screen=EXPEDITION_GIFT;g->selection=2;}
  if(b==BTN_OK){for(int i=0;i<g->deck_count;i++)if(game_beta_eligible(g,g->selection,g->deck[i])){
   g->effect_turn=g->selection;g->selection=i;g->screen=BETA_FORGE_CARD;return 1;
  }msg(g,"No eligible cards for this bargain.");}
  return b!=BTN_PAUSE;
 }
 if(g->screen==BETA_FORGE_CARD){
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  if(b==BTN_DOWN)g->selection=grid_down(g->selection,g->deck_count);
  if(b==BTN_LEFT&&g->selection%3)g->selection--;
  if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)g->selection++;
  if(b==BTN_BACK){g->screen=BETA_FORGE;g->selection=g->effect_turn;}
  if(b==BTN_OK&&game_beta_eligible(g,g->effect_turn,g->deck[g->selection])){
   g->deck[g->selection]=game_beta_preview(g,g->effect_turn,g->deck[g->selection]);
   if(g->effect_turn==2){g->maxhp-=4;g->hp=MIN(g->hp,g->maxhp);}
   enter_map(g);
   msg(g,"The forge has reshaped your card.");
  }
  return b!=BTN_PAUSE;
 }
 return 0;
}
