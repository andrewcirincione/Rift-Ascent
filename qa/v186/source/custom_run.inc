/* Editable custom loadout. The draft is itself a resumable slot. All effects
   are applied once, only after the player confirms the complete loadout. */
int custom_setup_screen(int s){return s>=CUSTOM_CARDS&&s<=CUSTOM_CONFIRM;}
static int custom_card_tab(int id){
 if(id==63||id==85||id==N_FLANKING||id==N_SNEAKY)return -1;
 if(id>=ASCENT_CARD_FIRST)return (id-ASCENT_CARD_FIRST)/20;
 if(id<NINJA_FIRST)return 0;if(id<NINJA_LAST)return 1;return 2;
}
static int custom_unique(const Game*g,int type,int index){
 int count=type==0?g->deck_count:type==1?g->relic_count:g->potion_count;
 for(int i=0;i<count;i++){
  int raw=type==0?g->deck[i]:type==1?game_relic_id(g,i):g->potions[i],seen=0;
  for(int j=0;j<i;j++)if(raw==(type==0?g->deck[j]:type==1?game_relic_id(g,j):g->potions[j])){seen=1;break;}
  if(!seen&&index--==0)return raw;
 }return -1;
}
int custom_id(const Game*g,int tab,int index){
 if(index<0)return -1;
 if(g->screen==CUSTOM_CARDS||(g->screen==CUSTOM_QUANTITY&&g->prior_screen==CUSTOM_CARDS)){
  if(tab==3)return custom_unique(g,0,index);
  for(int type=0;type<4;type++)for(int id=0;id<CARD_COUNT;id++)if(cards[id].type==type&&custom_card_tab(id)==tab&&ascent_card_available(g,id)&&index--==0)return id;
 }else{
  if(tab==2||tab==3)return custom_unique(g,tab==2?1:2,index);
  if(tab==1)return index<26?index:-1;
  for(int id=0;id<RELIC_COUNT;id++)if(id!=30&&(id<37||id>39)&&index--==0)return id;
 }return -1;
}
int custom_count(const Game*g,int tab){int n=0;while(custom_id(g,tab,n)>=0)n++;return n;}
int custom_copies(const Game*g,int tab,int id){
 int cards=g->screen==CUSTOM_CARDS||(g->screen==CUSTOM_QUANTITY&&g->prior_screen==CUSTOM_CARDS),n=0;
 if(cards){for(int i=0;i<g->deck_count;i++)n+=g->deck[i]==id;}
 else if(tab==0||tab==2){for(int i=0;i<g->relic_count;i++)n+=game_relic_id(g,i)==id;}
 else for(int i=0;i<g->potion_count;i++)n+=g->potions[i]==id;
 return n;
}
static int custom_limit(const Game*g){
 int cards=g->prior_screen==CUSTOM_CARDS,tab=g->revision184.tab;
 if(cards&&tab==3||!cards&&tab>=2)return custom_copies(g,tab,g->revision184.item);
 return cards?MAX_DECK-g->deck_count:tab==0?MAX_RELICS-g->relic_count:MAX_POTIONS-g->potion_count;
}
static void custom_apply_quantity(Game*g){
 int tab=g->revision184.tab,id=g->revision184.item,n=g->revision184.quantity,cards=g->prior_screen==CUSTOM_CARDS;
 int removing=cards?tab==3:tab>=2;
 if(n<1||n>custom_limit(g)){msg(g,"No room for that quantity.");return;}
 if(!removing&&cards&&!ascent_card_available(g,CARD_ID(id))){msg(g,"This card has not been unlocked.");return;}
 if(!removing){for(int i=0;i<n;i++){if(cards)g->deck[g->deck_count++]=id;else if(tab==0)*relic_slot(g,g->relic_count++)=id;else g->potions[g->potion_count++]=id;}}
 else{int count=cards?g->deck_count:tab==2?g->relic_count:g->potion_count,out=0;
  for(int i=0;i<count;i++){int value=cards?g->deck[i]:tab==2?game_relic_id(g,i):g->potions[i];if(value==id&&n){n--;continue;}
   if(cards)g->deck[out++]=value;else if(tab==2)*relic_slot(g,out++)=value;else g->potions[out++]=value;}
  if(cards)g->deck_count=out;else if(tab==2)g->relic_count=out;else g->potion_count=out;
 }
 g->screen=g->prior_screen;g->selection=MIN(g->revision184.return_selection,MAX(0,custom_count(g,tab)-1));msg(g,"");
}
static void custom_launch(Game*g){
 if(!g->deck_count){msg(g,"Choose at least one card before starting.");return;}
 for(int i=0;i<g->deck_count;i++)if(!ascent_card_available(g,CARD_ID(g->deck[i]))){g->screen=CUSTOM_CARDS;g->revision184.tab=3;g->selection=0;msg(g,"Remove locked cards before starting.");return;}
 int relics[MAX_RELICS],n=g->relic_count;for(int i=0;i<n;i++)relics[i]=game_relic_id(g,i);g->relic_count=0;
 for(int i=0;i<n;i++)add_relic(g,relics[i]);
 g->revision184.building=0;g->revision184.tab=0;g->selection=g->scroll=0;msg(g,"");begin_act(g);
}
static int custom_action(Game*g,int b){
 if(!custom_setup_screen(g->screen)||b==BTN_PAUSE)return 0;
 if(g->screen==CUSTOM_QUANTITY){
  int limit=custom_limit(g);
  if(b==BTN_LEFT||b==BTN_DOWN)g->revision184.quantity=MAX(1,g->revision184.quantity-(b==BTN_DOWN?5:1));
  if(b==BTN_RIGHT||b==BTN_UP)g->revision184.quantity=MIN(MAX(1,limit),g->revision184.quantity+(b==BTN_UP?5:1));
  if(b==BTN_ALT&&g->prior_screen==CUSTOM_CARDS&&g->revision184.tab<3&&game_card_is_upgradeable(g->revision184.item&~UPBIT))g->revision184.item^=UPBIT;
  if(b==BTN_OK)custom_apply_quantity(g);
  if(b==BTN_BACK){g->screen=g->prior_screen;g->selection=g->revision184.return_selection;msg(g,"");}return 1;
 }
 if(g->screen==CUSTOM_CONFIRM){
  if(b==BTN_UP)g->selection=(g->selection+2)%3;if(b==BTN_DOWN)g->selection=(g->selection+1)%3;
  if(b==BTN_BACK||(b==BTN_OK&&g->selection==1)){g->screen=CUSTOM_CARDS;g->selection=0;g->revision184.tab=3;msg(g,"");}
  else if(b==BTN_OK&&g->selection==0)custom_launch(g);
  else if(b==BTN_OK&&g->selection==2){unsigned seed=g->rng;if(ascent_forget(g))game_new(g,seed);else msg(g,"Could not discard the slot. Try again.");}return 1;
 }
 int tab=g->revision184.tab,n=custom_count(g,tab);
 if(b==BTN_TAB_LEFT||b==BTN_TAB_RIGHT){g->revision184.tab=(tab+(b==BTN_TAB_LEFT?3:1))%4;g->selection=0;msg(g,"");return 1;}
 if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);if(b==BTN_RIGHT)g->selection=MIN(MAX(0,n-1),g->selection+1);
 if(b==BTN_UP)g->selection=MAX(0,g->selection-3);if(b==BTN_DOWN)g->selection=MIN(MAX(0,n-1),g->selection+3);
 if(b==BTN_OK&&n){g->revision184.item=custom_id(g,tab,g->selection);g->revision184.quantity=1;g->revision184.return_selection=g->selection;g->prior_screen=g->screen;g->screen=CUSTOM_QUANTITY;msg(g,"");}
 else if(b==BTN_START){
  if(g->screen==CUSTOM_CARDS){if(!g->deck_count)msg(g,"Choose at least one card first.");else {g->screen=CUSTOM_ITEMS;g->revision184.tab=g->selection=0;msg(g,"");}}
  else{g->screen=CUSTOM_CONFIRM;g->selection=0;msg(g,"");}
 }else if(b==BTN_BACK){
  if(g->screen==CUSTOM_ITEMS){g->screen=CUSTOM_CARDS;g->revision184.tab=3;g->selection=0;}
  else {g->screen=CUSTOM_CONFIRM;g->selection=1;}
 }return 1;
}
