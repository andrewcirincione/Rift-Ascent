from pathlib import Path
p=Path(__file__).parent/'source'
f=p/'events150.inc';s=f.read_text()
s=s.replace('"WARDEN\'S BARGAIN"};','"WARDEN\'S BARGAIN",\n "BELL WITHOUT A TONGUE","MIRROR OF THE UNBORN","BLIND APOTHECARY","CLOCKMAKER\'S DEBT","EMPTY PROCESSION","FERRYMAN\'S TOLL","THE CROOKED COIN"};')
s=s.replace('"The archivist can cut a page, or bind its twin."};','''"The archivist can cut a page, or bind its twin.",
 "The silent bell asks you to give it a voice.",
 "Your reflection reaches for a card you have not played.",
 "The blind brewer knows every poison by its weight.",
 "The clocksmith can borrow from the years you have left.",
 "An empty coffin travels beneath a hundred patient hands.",
 "The boatman accepts coin, blood, or a memory.",
 "A crooked coin spins above a crack in the floor."};''')
s=s.replace('event_offers[EVENT_COUNT][2]','event_offers[EVENT_COUNT][3]')
s=s.replace('\n};\nconst char*game_event_title', ''',
 {{"RING THE BELL","Lose 7 HP. Receive the shown Ancient card, or a Rare if no Ancient is unlocked.",0,0,7,0}},
 {{"BREAK THE REFLECTION","Lose 8 HP to remove one card.",EVENT_REMOVE,0,8,0},{"STEAL ITS HAND","Pay 30 Gold to copy one playable card.",EVENT_COPY,30,0,0},{"REMEMBER ITS FACE","Pay 25 Gold. Give one card Retain.",MOD_BOUND,25,0,0}},
 {{"BUY TWO VIALS","Pay 35 Gold. Gain two random unlocked potions. Requires two empty belt slots.",0,35,0,0},{"EMPTY YOUR BELT","Trade every carried potion for the shown relic. Requires at least one potion.",0,0,0,0},{"DRINK THE ANTIDOTE","Trade your first carried potion to heal 20 HP.",0,0,0,0}},
 {{"BORROW YOUR TOMORROWS","Lose 6 maximum HP. Gain 2 permanent Strength for future combats. At least 25 maximum HP must remain.",0,0,0,6},{"TURN BACK TWO HANDS","Pay 60 Gold. Upgrade two random eligible cards. Requires two upgradeable cards.",0,60,0,0}},
 {{"TAKE THE COFFIN'S GIFT","Receive the shown relic. Add Dread: unplayable; if held, costs 1 Energy next turn.",0,0,0,0}},
 {{"REST IN THE BOAT","Pay 15 Gold. Heal 12 HP.",0,15,0,0},{"PAY IN BLOOD","Lose 8 HP. Gain 60 Gold.",0,0,8,0},{"SURRENDER A MEMORY","Pay 50 Gold to remove one card.",EVENT_REMOVE,50,0,0}},
 {{"CALL THE TOSS","Pay 15 Gold. Equal chances: gain 80 Gold, gain 25 Gold, heal 25 HP, or gain 5 maximum HP and heal 5.",0,15,0,0}}
};
static const unsigned char event_offer_counts[EVENT_COUNT]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,3,3,2,1,3,1};
int game_event_choice_count(const Game*g){return event_offer_counts[CLAMP(g->effect_turn,0,EVENT_COUNT-1)]+1;}
const char*game_event_title''')
s=s.replace('if(option==2){snprintf','if(option==game_event_choice_count(g)-1){snprintf').replace('option=CLAMP(option,0,1);','option=CLAMP(option,0,event_offer_counts[id]-1);')
s=s.replace('if(id==2||id==6){int raw=g->reward[option];','if(id==2||id==6||id==17){int raw=g->reward[id==17?0:option];')
s=s.replace('(id==4&&!option)||id==11||id==15','(id==4&&!option)||id==11||id==15||(id==19&&option==1)||id==21')
s=s.replace('if(option==2)return 1;','if(option==game_event_choice_count(g)-1)return 1;')
s=s.replace('if(option<0||option>1||g->effect_turn<0||g->effect_turn>=EVENT_COUNT)return 0;','if(g->effect_turn<0||g->effect_turn>=EVENT_COUNT||option<0||option>=event_offer_counts[g->effect_turn])return 0;')
s=s.replace('if(id==1&&!option&&game_moon_heal_now(g)<1)return 0;','''if(id==1&&!option&&game_moon_heal_now(g)<1)return 0;
 if((id==17||id==21)&&g->deck_count>=MAX_DECK)return 0;
 if(id==19){if(!option&&g->potion_count>MAX_POTIONS-2)return 0;if(option&&g->potion_count<1)return 0;if(option==2&&g->hp==g->maxhp)return 0;}
 if(id==20&&option==1&&count_upgradeable(g)<2)return 0;
 if(id==22&&!option&&g->hp==g->maxhp)return 0;''')
s=s.replace('if(id==4||id==11||id==15){g->reward[0]=random_relic(g);','if(id==17)g->reward[0]=reward_of_rarity(g,3,-1,-1);\n if(id==4||id==11||id==15||id==19||id==21){g->reward[0]=random_relic(g);')
s=s.replace('if(option==2){enter_map(g);','if(option==game_event_choice_count(g)-1){enter_map(g);')
s=s.replace('case 2:case 6:card=', 'case 17:card=g->reward[0];add_card(g,card);snprintf(text,sizeof(text),"The bell remembers your voice. Received %s.",cards[CARD_ID(card)].name);break;\n case 2:case 6:card=')
s=s.replace('case 15:relic_id=event_relic_id(g,option);break;','''case 15:relic_id=event_relic_id(g,option);break;
 case 19:if(!option){for(int i=0;i<2;i++)g->potions[g->potion_count++]=ascent_random_potion(g);snprintf(text,sizeof(text),"Received two potions. Paid 35 Gold.");}else if(option==1){g->potion_count=0;relic_id=event_relic_id(g,option);}else{for(int i=0;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;heal(g,20);snprintf(text,sizeof(text),"Traded the first potion. Healed up to 20 HP.");}break;
 case 20:if(!option){g->permanent_strength+=2;snprintf(text,sizeof(text),"Gained 2 permanent Strength. Lost 6 maximum HP.");}else{for(int k=0;k<2;k++){int pool[MAX_DECK],n=0;for(int i=0;i<g->deck_count;i++)if(game_card_is_upgradeable(g->deck[i]))pool[n++]=i;if(n)g->deck[pool[rnd(g,n)]]|=UPBIT;}snprintf(text,sizeof(text),"Upgraded two cards. Paid 60 Gold.");}break;
 case 21:add_card(g,53);relic_id=event_relic_id(g,option);break;
 case 22:if(!option){heal(g,12);snprintf(text,sizeof(text),"Healed up to 12 HP. Paid 15 Gold.");}else{g->gold+=60;snprintf(text,sizeof(text),"Gained 60 Gold. Lost 8 HP.");}break;
 case 23:switch(rnd(g,4)){case 0:g->gold+=80;snprintf(text,sizeof(text),"Paid 15 Gold. Won 80 Gold.");break;case 1:g->gold+=25;snprintf(text,sizeof(text),"Paid 15 Gold. Won 25 Gold.");break;case 2:heal(g,25);snprintf(text,sizeof(text),"Paid 15 Gold. Healed up to 25 HP.");break;default:g->maxhp+=5;heal(g,5);snprintf(text,sizeof(text),"Paid 15 Gold. Gained 5 maximum HP and healed 5.");break;}break;''')
f.write_text(s)
f=p/'game.c';s=f.read_text().replace('g->selection=(g->selection+2)%3;\n  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%3;\n  else if(b==BTN_BACK)do_event(g,2);','g->selection=(g->selection+game_event_choice_count(g)-1)%game_event_choice_count(g);\n  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%game_event_choice_count(g);\n  else if(b==BTN_BACK)do_event(g,game_event_choice_count(g)-1);')
s=s.replace('g->selection>2||g->effect_turn<0','g->selection>=game_event_choice_count(g)||g->effect_turn<0')
s=s.replace('g->shop_selection>1))return 0;','g->shop_selection>=game_event_choice_count(g)-1))return 0;')
f.write_text(s)
f=p/'render.c';s=f.read_text().replace('int card_offer=(id==2||id==6)&&g->selection<2;','int card_offer=((id==2||id==6)&&g->selection<2)||(id==17&&!g->selection);')
s=s.replace('const uint32_t*art=id>=12?r150_events[id-12]:r134_event_icons[id];','static const int art_map[7]={3,12,8,16,11,5,0};int artid=id<17?id:art_map[id-17];\n const uint32_t*art=artid>=12?r150_events[artid-12]:r134_event_icons[artid];')
s=s.replace('for(int i=0;i<3;i++){\n  int y=i<2?123+i*46:217,h=i<2?43:25;','int choices=game_event_choice_count(g);\n for(int i=0;i<choices;i++){\n  int leave=i==choices-1,y=leave?217:choices==4?122+i*31:123+i*46,h=leave?25:choices==4?29:43;')
s=s.replace('MIN(i<2?2:1,r134_lines','MIN(leave?1:2,r134_lines')
s=s.replace('(g->selection==0?"35 GOLD":"FREE"):"6 HP")','(g->selection==0?"35 GOLD":"FREE"):id==17?"7 HP":"6 HP")')
f.write_text(s)
