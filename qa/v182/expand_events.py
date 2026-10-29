from pathlib import Path
import json
p=Path(__file__).parent/'source'
# Fifteen additional rooms requested during the 1.8.2 review.
rooms=[
("THREADS UNDER SKIN","A seamstress pulls silver thread from beneath her skin.",[("STITCH A MEMORY","Lose 9 HP. Give one card Retain.","MOD_BOUND",0,9,0),("LIGHT THE THREAD","Pay 35 Gold. One Attack or Skill draws an extra card, but Exhausts.","MOD_KINDLED",35,0,0)]),
("FORGE OF WHISPERS","Three hammers argue over the shape of your next mistake.",[("SHARPEN","Pay 30 Gold. One Attack gains +2 base damage per hit.","MOD_KEEN",30,0,0),("REINFORCE","Lose 10 HP. One Block-granting card gains +4 base Block.","MOD_WARD",0,10,0),("LET IT ANSWER","Pay 65 Gold and lose 5 HP. A fixed-cost Attack plays twice, but costs 1 more Energy.","MOD_ECHO",65,5,0)]),
("PAPER SAINT","The saint offers to fold a piece of your life into a card.",[("FOLD YOUR YEARS","Lose 4 maximum HP. One Attack or Skill costs 1 less Energy, but Exhausts. Keep at least 25 maximum HP.","MOD_FLEETING",0,0,4)]),
("GRAVE ENGRAVER","The engraver has space for one more word on your weapon.",[("CARVE A WOUND","Lose 3 maximum HP. One Attack gains +2 base damage per hit. Keep at least 25 maximum HP.","MOD_KEEN",0,0,3),("CARVE A PROMISE","Pay 25 Gold. Give one card Retain.","MOD_BOUND",25,0,0),("CARVE A SPARK","Lose 8 HP. One Attack or Skill draws an extra card, but Exhausts.","MOD_KINDLED",0,8,0)]),
("RED COMMUNION","A cup of living iron waits beside a bowl of clean water.",[("DRINK THE IRON","Lose 8 HP. Next combat only: gain 3 Strength and begin with 2 Vulnerable.","0",0,8,0),("CHOOSE CLEAN WATER","Pay 30 Gold. Heal 18 HP.","0",30,0,0)]),
("SALT CHAPEL","White salt has preserved every kneeling pilgrim except you.",[("KNEEL IN THE SALT","Heal 28 HP. Begin your next combat with 2 Weak.","0",0,0,0),("TAKE A SALT WARD","Pay 15 Gold. Begin your next combat with 12 extra Block.","0",15,0,0)]),
("LEECH GARDEN","Pale leeches grow inside glass fruit. The gardener smiles.",[("LET THEM FEED","Lose 5 maximum HP, then heal to full. Keep at least 25 maximum HP.","0",0,0,5),("PRESS TWO FRUITS","Lose 9 HP. Gain two random unlocked potions. Requires two empty belt slots.","0",0,9,0)]),
("GLASS HEART","A second heartbeat sounds from an unbroken glass chest.",[("ACCEPT THE HEART","Gain 12 maximum HP and heal 12. Add an unplayable Injury to your deck.","0",0,0,0),("CUT A SHARD FREE","Pay 40 Gold to remove one card.","EVENT_REMOVE",40,0,0)]),
("RELIQUARY OF DEBTS","The dead kept careful accounts. Three debts remain unpaid.",[("PAY IN BLOOD","Lose 8 HP. Receive this relic.","0",0,8,0),("PAY IN COIN","Pay 65 Gold. Receive this other relic.","0",65,0,0),("SETTLE THE SMALL DEBT","Pay 20 Gold. Gain one random unlocked potion.","0",20,0,0)]),
("THE STILL ROOM","An alchemist can distill more than medicine from your belt.",[("DISTILL A LESSON","Pay 20 Gold and trade your first potion to upgrade one chosen card.","EVENT_UPGRADE",20,0,0),("DISTILL A HEARTBEAT","Trade your first two potions. Gain 8 maximum HP and heal 8.","0",0,0,0),("SELL THE WHOLE BELT","Lose every carried potion. Gain 18 Gold for each potion sold.","0",0,0,0)]),
("BONE ORCHARD","Two pale fruits bear the memories of warriors buried below.",[("BUY THE GOLDEN FRUIT","Pay 45 Gold. Receive the shown Rare card.","0",45,0,0),("PLUCK THE THORNED FRUIT","Lose 7 HP. Receive the shown upgraded Uncommon card.","0",0,7,0)]),
("WAR DRUM","A drum of stretched hide beats in time with your pulse.",[("LEARN ITS RHYTHM","Pay 25 Gold. Gain 3 Strength for your next combat only.","0",25,0,0),("STRIKE IT WITH BLOOD","Lose 6 HP. Draw 2 extra cards in your next opening hand, up to the hand limit.","0",0,6,0)]),
("TAX ON BREATH","The collector accepts either blood now or coin for shelter.",[("PAY THE BLOOD TAX","Lose 10 HP. Gain 100 Gold.","0",0,10,0),("BUY A NIGHT'S SHELTER","Pay 50 Gold. Heal 25 HP and begin your next combat with 12 extra Block.","0",50,0,0)]),
("THE FACELESS DRAFTER","The stranger erases a face from a card and offers you the pen.",[("REWRITE A CARD","Pay 20 Gold. Replace one card with a different random Uncommon from your character. Its old upgrade and modifier are lost.","EVENT_TRANSFORM",20,0,0),("KEEP YOUR OLD NAME","Gain 15 Gold. Your deck stays the same.","0",0,0,0)]),
("CANDLE FOR A STRANGER","A stranded soul needs a little warmth to find the road.",[("SHARE YOUR FLAME","Lose 6 HP. Next combat only: gain 2 Strength and begin with 10 extra Block.","0",0,6,0)])]
(Path(__file__).parent/'extra-events.json').write_text(json.dumps([dict(id=i+24,title=r[0],lore=r[1],offers=r[2]) for i,r in enumerate(rooms)],indent=2)+'\n')
f=p/'game.h';s=f.read_text().replace('#define EVENT_COUNT 24','#define EVENT_COUNT 39').replace('EVENT_UPGRADE=7,EVENT_REMOVE,EVENT_COPY','EVENT_UPGRADE=7,EVENT_REMOVE,EVENT_COPY,EVENT_TRANSFORM').replace('struct {int rare_offset,room_pending,room_type;} revision182;','struct {int rare_offset,room_pending,room_type;unsigned event_seen_hi;int next_combat[5];} revision182;');f.write_text(s)
f=p/'events150.inc';s=f.read_text();s=s.replace('"THE CROOKED COIN"};','"THE CROOKED COIN",\n '+',\n '.join(json.dumps(r[0]) for r in rooms)+'};');s=s.replace('"A crooked coin spins above a crack in the floor."};','"A crooked coin spins above a crack in the floor.",\n '+',\n '.join(json.dumps(r[1]) for r in rooms)+'};')
a=s.index('\n};\nstatic const unsigned char event_offer_counts');s=s[:a]+',\n '+',\n '.join('{'+','.join('{'+','.join([json.dumps(o[0]),json.dumps(o[1]),o[2],str(o[3]),str(o[4]),str(o[5])])+'}' for o in r[2])+'}' for r in rooms)+s[a:]
s=s.replace('2,1,3,3,2,1,3,1};','2,1,3,3,2,1,3,1,'+','.join(str(len(r[2])) for r in rooms)+'};')
s=s.replace('if(action==EVENT_REMOVE)return CARD_ID(raw)<CARD_COUNT;','if(action==EVENT_REMOVE||action==EVENT_TRANSFORM)return CARD_ID(raw)<CARD_COUNT;')
s=s.replace('g->effect_turn==15?option:0','(g->effect_turn==15||g->effect_turn==32)?option:0')
s=s.replace('id==2||id==6||id==17','id==2||id==6||id==17||id==34')
s=s.replace('||(id==19&&option==1)||id==21','||(id==19&&option==1)||id==21||(id==32&&option<2)')
# One shared capacity gate prevents paying for a capped, queued combat effect.
s=s.replace('int game_event_choice_available(','''static int event_next_fits(const Game*g,int strength,int block,int draw,int weak,int vulnerable){
 const int add[5]={strength,block,draw,weak,vulnerable},limit[5]={30,99,5,5,5};
 for(int i=0;i<5;i++)if(g->revision182.next_combat[i]+add[i]>limit[i])return 0;return 1;
}
int game_event_choice_available(''',1)
s=s.replace('if((id==17||id==21)&&g->deck_count>=MAX_DECK)return 0;','''if((id==17||id==21||id==34||(id==31&&!option))&&g->deck_count>=MAX_DECK)return 0;
 if(id==28&&!option&&!event_next_fits(g,3,0,0,0,2))return 0;
 if(id==28&&option&&g->hp==g->maxhp)return 0;
 if(id==29&&(!option?(g->hp==g->maxhp||!event_next_fits(g,0,0,0,2,0)):!event_next_fits(g,0,12,0,0,0)))return 0;
 if(id==30&&(option?g->potion_count>MAX_POTIONS-2:g->hp>=g->maxhp-5))return 0;
 if(id==32&&option==2&&g->potion_count>=MAX_POTIONS)return 0;
 if(id==33&&g->potion_count<(option==1?2:1))return 0;
 if(id==35&&!event_next_fits(g,option?0:3,0,option?2:0,0,0))return 0;
 if(id==36&&option&&!event_next_fits(g,0,12,0,0,0))return 0;
 if(id==38&&!event_next_fits(g,2,10,0,0,0))return 0;''')
s=s.replace('int game_next_event(Game*g){','static int event_is_modification(int id){return (id>=12&&id<=14)||(id>=24&&id<=27);}\nint game_next_event(Game*g){')
s=s.replace('unsigned full=(1u<<EVENT_COUNT)-1u,seen=(unsigned)g->v151.relic[R162_EVENT_SEEN]&full;','uint64_t full=(1ull<<EVENT_COUNT)-1ull,seen=((uint64_t)g->revision182.event_seen_hi<<32)|(uint32_t)g->v151.relic[R162_EVENT_SEEN];seen&=full;')
s=s.replace('seen&(1u<<id)','seen&(1ull<<id)').replace('!(last>=12&&last<=14&&id>=12&&id<=14)','!(event_is_modification(last)&&event_is_modification(id))').replace('(id<12||id>14)&&id!=last','!event_is_modification(id)&&id!=last')
s=s.replace('g->v151.relic[R162_EVENT_SEEN]=(int)(seen|(1u<<id));','seen|=1ull<<id;g->v151.relic[R162_EVENT_SEEN]=(int)(uint32_t)seen;g->revision182.event_seen_hi=(unsigned)(seen>>32);')
s=s.replace('if(id==17)g->reward[0]=reward_of_rarity(g,3,-1,-1);','if(id==17)g->reward[0]=reward_of_rarity(g,3,-1,-1);\n if(id==34){g->reward[0]=reward_of_rarity(g,2,-1,-1);g->reward[1]=reward_of_rarity(g,1,-1,-1)|UPBIT;}')
s=s.replace('id==15||id==19||id==21','id==15||id==19||id==21||id==32').replace('if(id==15)g->reward[1]','if(id==15||id==32)g->reward[1]')
s=s.replace('event_pay(g,o);char text[72];','''event_pay(g,o);char text[72];
 if(g->effect_turn==33){for(int i=0;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;}''')
s=s.replace('else if(action==EVENT_COPY){add_card','else if(action==EVENT_TRANSFORM){raw=reward_of_rarity(g,1,CARD_ID(raw),-1);g->deck[g->selection]=raw;snprintf(text,sizeof(text),"Rewritten as %s.",cards[CARD_ID(raw)].name);}\n else if(action==EVENT_COPY){add_card')
s=s.replace('Unavailable: check costs, deck or potion space.','Unavailable: check costs, deck, potions or queued boons.')
s=s.replace('case 2:case 6:card=','case 2:case 6:case 34:card=')
mark='\n }\n if(relic_id>=0)'
addition='''
 case 28:if(!option){g->revision182.next_combat[0]+=3;g->revision182.next_combat[4]+=2;snprintf(text,sizeof(text),"Next combat: +3 Strength, 2 Vulnerable. Lost 8 HP.");}else{heal(g,18);snprintf(text,sizeof(text),"Paid 30 Gold. Healed up to 18 HP.");}break;
 case 29:if(!option){heal(g,28);g->revision182.next_combat[3]+=2;snprintf(text,sizeof(text),"Healed up to 28 HP. Next combat starts with 2 Weak.");}else{g->revision182.next_combat[1]+=12;snprintf(text,sizeof(text),"Paid 15 Gold. Next combat starts with +12 Block.");}break;
 case 30:if(!option){g->hp=g->maxhp;snprintf(text,sizeof(text),"Lost 5 maximum HP. Healed to full.");}else{for(int i=0;i<2;i++)g->potions[g->potion_count++]=ascent_random_potion(g);snprintf(text,sizeof(text),"Lost 9 HP. Received two potions.");}break;
 case 31:g->maxhp+=12;heal(g,12);add_card(g,52);snprintf(text,sizeof(text),"Gained 12 maximum HP, healed 12, and added Injury.");break;
 case 32:if(option<2)relic_id=event_relic_id(g,option);else{int id=ascent_random_potion(g);g->potions[g->potion_count++]=id;snprintf(text,sizeof(text),"Paid 20 Gold. Received %s.",game_potion_name(id));}break;
 case 33:if(option==1){for(int i=0;i+2<g->potion_count;i++)g->potions[i]=g->potions[i+2];g->potion_count-=2;g->maxhp+=8;heal(g,8);snprintf(text,sizeof(text),"Traded two potions. Gained 8 maximum HP and healed 8.");}else{int gold=g->potion_count*18;g->potion_count=0;g->gold+=gold;snprintf(text,sizeof(text),"Sold the belt for %d Gold.",gold);}break;
 case 35:if(!option){g->revision182.next_combat[0]+=3;snprintf(text,sizeof(text),"Paid 25 Gold. Next combat: +3 Strength.");}else{g->revision182.next_combat[2]+=2;snprintf(text,sizeof(text),"Lost 6 HP. Next opening hand: draw 2 extra cards.");}break;
 case 36:if(!option){g->gold+=100;snprintf(text,sizeof(text),"Lost 10 HP. Gained 100 Gold.");}else{heal(g,25);g->revision182.next_combat[1]+=12;snprintf(text,sizeof(text),"Paid 50 Gold. Heal 25; next combat starts with +12 Block.");}break;
 case 37:g->gold+=15;snprintf(text,sizeof(text),"Kept your cards. Gained 15 Gold.");break;
 case 38:g->revision182.next_combat[0]+=2;g->revision182.next_combat[1]+=10;snprintf(text,sizeof(text),"Lost 6 HP. Next combat: +2 Strength and +10 Block.");break;
'''
assert mark in s;s=s.replace(mark,addition+mark);f.write_text(s)
f=p/'game.c';s=f.read_text();s=s.replace('if(relic(g,10))g->block+=12;if(relic(g,29))g->block+=10;','''if(relic(g,10))g->block+=12;if(relic(g,29))g->block+=10;
 int *boon=g->revision182.next_combat;int extra=boon[2],any=0;for(int i=0;i<5;i++)any|=boon[i];
 g->strength+=boon[0];g->block=MIN(999,g->block+boon[1]);g->weak+=boon[3];g->vulnerable+=boon[4];
 memset(boon,0,sizeof(g->revision182.next_combat));for(int i=0;i<extra;i++)draw_card(g);
 if(any)msg(g,"Your mystery bargains take effect. Inspect STATUS for details.");''')
s=s.replace('if(g->act<1||g->act>100000','if(g->revision182.event_seen_hi>127u)return 0;\n const int next_limits[5]={30,99,5,5,5};for(int i=0;i<5;i++)if(g->revision182.next_combat[i]<0||g->revision182.next_combat[i]>next_limits[i])return 0;\n if(g->act<1||g->act>100000',1)
s=s.replace('g->next_screen>EVENT_COPY','g->next_screen>EVENT_TRANSFORM')
s=s.replace(' if(g->character==CHAR_REGENT){STATUS(g->regent.stars', ''' STATUS(g->revision182.next_combat[0],"Next combat: +%d Strength for that fight");
 STATUS(g->revision182.next_combat[1],"Next combat: +%d starting Block");
 STATUS(g->revision182.next_combat[2],"Next combat: draw %d extra opening cards");
 STATUS(g->revision182.next_combat[3],"Next combat: start with %d Weak");
 STATUS(g->revision182.next_combat[4],"Next combat: start with %d Vulnerable");
 if(g->character==CHAR_REGENT){STATUS(g->regent.stars''')
f.write_text(s)
f=p/'render.c';s=f.read_text().replace('((id==2||id==6)&&g->selection<2)','((id==2||id==6||id==34)&&g->selection<2)');s=s.replace('art_map[7]={3,12,8,16,11,5,0}','art_map[22]={3,12,8,16,11,5,0,14,13,14,12,8,3,8,7,15,8,2,11,0,16,3}')
s=s.replace('id==17?"7 HP":"6 HP"','id==17?"7 HP":id==34?(g->selection?"7 HP":"45 GOLD"):"6 HP"')
s=s.replace('action==EVENT_REMOVE?"REMOVE / CHOOSE A CARD":"COPY / CHOOSE A CARD"','action==EVENT_REMOVE?"REMOVE / CHOOSE A CARD":action==EVENT_TRANSFORM?"TRANSFORM / CHOOSE A CARD":"COPY / CHOOSE A CARD"')
s=s.replace('action==EVENT_COPY?"COPY TO BE ADDED":"AFTER"','action==EVENT_COPY?"COPY TO BE ADDED":action==EVENT_TRANSFORM?"WILL BE REPLACED":"AFTER"');f.write_text(s)
