/* Choices are quoted from one table, then validated again at commitment.
   Opening/browsing/canceling a card picker never spends a resource. */
typedef struct {const char*title;const char*detail;int action,gold,hp,maxhp;} EventOffer;
static const char*event_titles[EVENT_COUNT]={
 "CURSED PURSE","MOON SPRING","FORGOTTEN TOME","ANCIENT SHRINE",
 "TRAPPED CHEST","LOST TRAVELER","FORGOTTEN ARMORY","LIVING ALTAR",
 "BLACK WATER","CINDER GAMBIT","RUSTED FORGE","LAST SENTINEL",
 "ECHO CRYPT","BLACK ANVIL","CINDER SCRIPTORIUM","RELIQUARY AUCTION","WARDEN'S BARGAIN"};
static const char*event_lore[EVENT_COUNT]={
 "A scorched purse is caught in a thorned iron trap.",
 "Moonwater mends wounds. Its silver root can strengthen your mortal heart.",
 "The grimoire offers a difficult lesson or a familiar one.",
 "Two candles burn: one for endurance, one for relief.",
 "You can reach the offering, or collect the loose coins.",
 "A weary pilgrim can spare either a meal or their coin.",
 "Choose one weapon from the cursed armory.",
 "The altar trades today's blood for tomorrow's life.",
 "A deep draught leaves a scar. A sip carries no curse.",
 "The fissure takes its price in blood or a burden.",
 "The smith can hone your deck or lighten it.",
 "The sentinel demands blood or a lingering curse.",
 "A second blade answers every swing from the tomb.",
 "The smith can sharpen an edge or reinforce a guard.",
 "Burn a card's future to strengthen its next play.",
 "Two relics wait. One costs coin; one costs vitality.",
 "The archivist can cut a page, or bind its twin."};
static const EventOffer event_offers[EVENT_COUNT][2]={
 {{"SEIZE THE PURSE","Gain 75 Gold. Lose 8 HP.",0,0,8,0},{"GATHER LOOSE COINS","Gain 25 Gold. No HP cost.",0,0,0,0}},
 {{"DRINK","Heal 20 HP for free, up to your maximum.",0,0,0,0},{"TAKE THE SILVER ROOT","Pay 35 Gold. Gain 6 maximum HP and heal 6.",0,35,0,0}},
 {{"STUDY THE DEEPER LESSON","Pay 35 Gold. Take the shown upgraded Uncommon card.",0,35,0,0},{"READ THE MARGIN","Take the shown Common card for free.",0,0,0,0}},
 {{"LIGHT THE ENDURING FLAME","Gain 5 maximum HP and heal 5 HP.",0,0,0,0},{"LIGHT THE MERCIFUL FLAME","Heal 18 HP. Maximum HP is unchanged.",0,0,0,0}},
 {{"REACH THROUGH THE TRAP","Lose 7 HP. Take the shown relic.",0,0,7,0},{"TAKE THE LOOSE COINS","Gain 40 Gold. Leave the relic.",0,0,0,0}},
 {{"SHARE A MEAL","Heal 16 HP.",0,0,0,0},{"TAKE THE COIN","Gain 45 Gold.",0,0,0,0}},
 {{"CLAIM THE WEAPON","Lose 6 HP. Take the shown Uncommon Attack.",0,0,6,0},{"CLAIM THE TECHNIQUE","Lose 6 HP. Take the shown Uncommon Skill.",0,0,6,0}},
 {{"OFFER YOUR BLOOD","Gain 10 maximum HP. Lose 17 current HP.",0,0,17,0},{"BUY THE ALTAR'S MERCY","Pay 45 Gold. Heal 30 HP.",0,45,0,0}},
 {{"DRINK DEEPLY","Heal 28 HP. Add Injury: an unplayable card.",0,0,0,0},{"TAKE A SMALL SIP","Heal 9 HP. No card is added.",0,0,0,0}},
 {{"PAY IN BLOOD","Gain 110 Gold. Lose 16 HP.",0,0,16,0},{"TAKE THE BURDEN","Gain 70 Gold. Add Injury: an unplayable card.",0,0,0,0}},
 {{"HONE ONE CARD","Pay 25 Gold to upgrade one card.",EVENT_UPGRADE,25,0,0},{"LIGHTEN YOUR DECK","Pay 50 Gold to remove one card.",EVENT_REMOVE,50,0,0}},
 {{"ACCEPT THE DREAD","Take the shown relic. Add Dread: unplayable; if held, costs 1 Energy next turn.",0,0,0,0},{"PAY THE BLOOD PRICE","Lose 12 HP. Take the shown relic without a curse.",0,0,12,0}},
 {{"ECHO / 70 GOLD","Modify one fixed-cost Attack: play it twice; costs 1 more Energy.",MOD_ECHO,70,0,0},{"BOUND / 40 GOLD","Modify one card with Retain: keep it in hand at turn end.",MOD_BOUND,40,0,0}},
 {{"KEEN / 8 HP","Modify one Attack: +2 base damage per hit, before Strength, Weak and Vulnerable.",MOD_KEEN,0,8,0},{"WARD / 8 HP","Modify one card that grants Block: +4 base Block per card effect, before Dexterity.",MOD_WARD,0,8,0}},
 {{"FLEETING / 55 GOLD","Modify an Attack or Skill: costs 1 less Energy, but Exhausts when played.",MOD_FLEETING,55,0,0},{"KINDLED / 35 GOLD","Modify an Attack or Skill: draw 1 extra card when played, but Exhaust it.",MOD_KINDLED,35,0,0}},
 {{"BID WITH GOLD / 95 GOLD","Pay 95 Gold for this relic.",0,95,0,0},{"BID WITH VITALITY","Lose 10 maximum HP for this other relic. Current HP is capped at the new maximum; at least 25 maximum HP must remain.",0,0,0,10}},
 {{"CUT A PAGE / 35 GOLD","Remove one card from your deck. You must keep at least one card.",EVENT_REMOVE,35,0,0},{"BIND A TWIN / 60 GOLD","Copy one playable card, including its upgrade and modification. Add the copy to your deck.",EVENT_COPY,60,0,0}}
};
const char*game_event_title(int id){return id>=0&&id<EVENT_COUNT?event_titles[id]:"MYSTERY";}
const char*game_event_lore(int id){return id>=0&&id<EVENT_COUNT?event_lore[id]:"";}
static int event_card_ok(int raw,int action){
 if(action<MOD_COUNT)return game_can_modify(raw,action);
 if(action==EVENT_UPGRADE)return game_card_is_upgradeable(raw);
 if(action==EVENT_REMOVE)return CARD_ID(raw)<CARD_COUNT;
 return game_card_is_playable(raw)&&CARD_ID(raw)!=63&&CARD_ID(raw)!=85&&CARD_ID(raw)!=CARD_GIANT_ROCK;
}
int game_event_card_eligible(const Game*g,int raw){return event_card_ok(raw,g->next_screen);}
static int event_relic_id(const Game*g,int option){return g->reward[g->effect_turn==15?option:0];}
void game_event_option(const Game*g,int option,char*title,int tc,char*detail,int dc){
 if(option==2){snprintf(title,tc,"LEAVE");snprintf(detail,dc,"Leave without spending anything.");return;}
 int id=CLAMP(g->effect_turn,0,EVENT_COUNT-1);option=CLAMP(option,0,1);
 const EventOffer*o=&event_offers[id][option];snprintf(title,tc,"%s",o->title);snprintf(detail,dc,"%s",o->detail);
 if(id==2||id==6){int raw=g->reward[option];if(CARD_ID(raw)<CARD_COUNT)snprintf(title,tc,"%s%s",cards[CARD_ID(raw)].name,CARD_UP(raw)?"+":"");}
 if((id==4&&!option)||id==11||id==15){int r=event_relic_id(g,option);if(r>=0&&r<RELIC_COUNT)snprintf(detail,dc,"%s\n%s: %s",o->detail,relic_defs[r].name,relic_defs[r].desc);else snprintf(detail,dc,"No unowned relic remains for this offer.");}
}
int game_event_choice_available(const Game*g,int option){
 if(option==2)return 1;
 if(option<0||option>1||g->effect_turn<0||g->effect_turn>=EVENT_COUNT)return 0;
 const EventOffer*o=&event_offers[g->effect_turn][option];int id=g->effect_turn;
 if(g->gold<o->gold||g->hp<=o->hp||(o->maxhp>0&&g->maxhp-o->maxhp<25))return 0;
 if(o->action){int n=0;for(int i=0;i<g->deck_count;i++)n+=event_card_ok(g->deck[i],o->action);
  if(!n||(o->action==EVENT_REMOVE&&g->deck_count<=1)||(o->action==EVENT_COPY&&g->deck_count>=MAX_DECK))return 0;}
 if((id==2||id==6||(id==8&&!option)||(id==9&&option)||(id==11&&!option))&&g->deck_count>=MAX_DECK)return 0;
 if(id==1&&!option&&game_moon_heal_now(g)<1)return 0;
 if((id==3&&option)||(id==5&&!option)||(id==7&&option)||id==8)if(g->hp==g->maxhp)return 0;
 if((id==4&&!option)||id==11||id==15){int r=event_relic_id(g,option);if(r<0||r>=RELIC_COUNT||relic(g,r)||g->relic_count>=MAX_RELICS)return 0;if(o->maxhp&&g->maxhp-o->maxhp-(r==11?12:0)<25)return 0;}
 return 1;
}
static int event_typed_card(Game*g,int type){
 int pool[128],weights[128],n=0,total=0;
 for(int id=3;id<CARD_COUNT;id++)if(solo_reward_eligible(id)&&ninja_owned_card(g,id)&&cards[id].rarity==1&&cards[id].type==type){
  int owned=0;for(int j=0;j<g->deck_count;j++)owned|=CARD_ID(g->deck[j])==id;
  pool[n]=id;weights[n]=owned?1:3;total+=weights[n++];}
 int roll=rnd(g,total);for(int i=0;i<n;i++){if(roll<weights[i])return pool[i];roll-=weights[i];}return type?5:11;
}
int game_next_event(Game*g){
 if(rnd(g,25)==0)return EVENT_COUNT; /* Rare optional cross-character offer. */
 unsigned full=(1u<<EVENT_COUNT)-1u,seen=(unsigned)g->v151.relic[R162_EVENT_SEEN]&full;
 int last=g->v151.relic[R162_EVENT_LAST]-1,pool[EVENT_COUNT],n=0;
 if(seen==full)seen=0;
 for(int id=0;id<EVENT_COUNT;id++)if(!(seen&(1u<<id))&&!(last>=12&&last<=14&&id>=12&&id<=14))pool[n++]=id;
 /* If the only unseen rooms are modifications, interpose one non-modification
    room without discarding the bag. The remaining rooms are still owed. */
 if(!n)for(int id=0;id<EVENT_COUNT;id++)if((id<12||id>14)&&id!=last)pool[n++]=id;
 int id=pool[rnd(g,n)];g->v151.relic[R162_EVENT_SEEN]=(int)(seen|(1u<<id));g->v151.relic[R162_EVENT_LAST]=id+1;return id;
}
void game_begin_event(Game*g,int id){
 if(id==EVENT_COUNT){expedition_mystery(g);return;}
 g->screen=EVENT;g->effect_turn=CLAMP(id,0,EVENT_COUNT-1);g->selection=0;
 g->scroll=MIN(20,MIN(g->gold/2,g->maxhp-g->hp));g->next_screen=0;g->reward[0]=g->reward[1]=g->reward[2]=-1;msg(g,"");
 if(id==2){g->reward[0]=reward_of_rarity(g,1,-1,-1)|UPBIT;g->reward[1]=reward_of_rarity(g,0,-1,-1);}
 if(id==6){g->reward[0]=event_typed_card(g,0);g->reward[1]=event_typed_card(g,1);}
 if(id==4||id==11||id==15){g->reward[0]=random_relic(g);if(id==15)g->reward[1]=random_relic_except(g,g->reward[0]);}
}
static void event_pay(Game*g,const EventOffer*o){
 g->gold-=o->gold;g->hp-=o->hp;g->maxhp-=o->maxhp;g->hp=MIN(g->hp,g->maxhp);
}
static void event_result(Game*g,int card,const char*text){
 g->screen=EVENT_RESULT;g->selection=0;g->reward[0]=card;g->reward[1]=-1;msg(g,text);
}
static void event_card_commit(Game*g){
 if(g->selection<0||g->selection>=g->deck_count||!game_event_card_eligible(g,g->deck[g->selection])){msg(g,"This card is not eligible. Choose another.");return;}
 if(!game_event_choice_available(g,g->shop_selection)){msg(g,"Cannot afford this choice. No resources spent.");return;}
 int action=g->next_screen,raw=g->deck[g->selection];
 const EventOffer*o=&event_offers[g->effect_turn][g->shop_selection];
 if(action!=o->action){msg(g,"Invalid offer. No resources spent.");return;}
 event_pay(g,o);char text[72];
 if(action<MOD_COUNT){raw|=action<<CARD_MOD_SHIFT;g->deck[g->selection]=raw;snprintf(text,sizeof(text),"%s gained %s.",cards[CARD_ID(raw)].name,game_mod_name(action));}
 else if(action==EVENT_UPGRADE){raw|=UPBIT;g->deck[g->selection]=raw;snprintf(text,sizeof(text),"Upgraded %s.",cards[CARD_ID(raw)].name);}
 else if(action==EVENT_COPY){add_card(g,raw);snprintf(text,sizeof(text),"Added a second %s to your deck.",cards[CARD_ID(raw)].name);}
 else{for(int i=g->selection;i+1<g->deck_count;i++)g->deck[i]=g->deck[i+1];g->deck_count--;snprintf(text,sizeof(text),"Removed %s from your deck.",cards[CARD_ID(raw)].name);}
 event_result(g,raw,text);
}
static void do_event(Game*g,int option){
 if(option==2){enter_map(g);msg(g,"You leave the chamber.");return;}
 if(!game_event_choice_available(g,option)){msg(g,"Unavailable: check costs, deck or potion space.");return;}
 int id=g->effect_turn;const EventOffer*o=&event_offers[id][option];
 if(o->action){g->next_screen=o->action;g->shop_selection=option;g->screen=EVENT_CARD;g->selection=0;
  while(g->selection+1<g->deck_count&&!game_event_card_eligible(g,g->deck[g->selection]))g->selection++;
  msg(g,"");return;}
 int card=-1,relic_id=-1;char text[72]="";
 event_pay(g,o);
 switch(id){
 case 0:g->gold+=option?25:75;snprintf(text,sizeof(text),option?"Gained 25 Gold.":"Gained 75 Gold. Lost 8 HP.");break;
 case 1:if(option){g->maxhp+=6;heal(g,6);snprintf(text,sizeof(text),"Gained 6 maximum HP and healed 6. Paid 35 Gold.");}else{int n=game_moon_heal_now(g);heal(g,n);snprintf(text,sizeof(text),"Healed %d HP.",n);}break;
 case 2:case 6:card=g->reward[option];add_card(g,card);snprintf(text,sizeof(text),"Received %s%s.",cards[CARD_ID(card)].name,CARD_UP(card)?"+":"");break;
 case 3:if(!option){g->maxhp+=5;heal(g,5);snprintf(text,sizeof(text),"Gained 5 maximum HP and healed 5 HP.");}else{heal(g,18);snprintf(text,sizeof(text),"Healed up to 18 HP.");}break;
 case 4:if(option){g->gold+=40;snprintf(text,sizeof(text),"Gained 40 Gold.");}else relic_id=event_relic_id(g,option);break;
 case 5:if(option){g->gold+=45;snprintf(text,sizeof(text),"Gained 45 Gold.");}else{heal(g,16);snprintf(text,sizeof(text),"Healed up to 16 HP.");}break;
 case 7:if(option){heal(g,30);snprintf(text,sizeof(text),"Healed up to 30 HP. Paid 45 Gold.");}else{g->maxhp+=10;snprintf(text,sizeof(text),"Gained 10 maximum HP. Lost 17 current HP.");}break;
 case 8:heal(g,option?9:28);if(!option)add_card(g,52);snprintf(text,sizeof(text),option?"Healed up to 9 HP.":"Healed up to 28 HP. Added Injury.");break;
 case 9:g->gold+=option?70:110;if(option)add_card(g,52);snprintf(text,sizeof(text),option?"Gained 70 Gold. Added Injury.":"Gained 110 Gold. Lost 16 HP.");break;
 case 11:if(!option)add_card(g,53);relic_id=event_relic_id(g,option);break;
 case 15:relic_id=event_relic_id(g,option);break;
 }
 if(relic_id>=0){add_relic(g,relic_id);g->screen=RELIC_RESULT;g->relic_pending=0;g->relic_resume=MAP;g->win_screen=0;g->reward[0]=relic_id;g->selection=0;snprintf(g->message,sizeof(g->message),"Acquired %s.",relic_defs[relic_id].name);return;}
 event_result(g,card,text);
}
