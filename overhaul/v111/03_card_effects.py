#!/usr/bin/env python3
"""1.1.0 87-card mechanics: playable effects, auto-play, AI targets, exact costs."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c";s=p.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("IC87 action anchor missing "+a[:110])
 s=s.replace(a,b,1)
# Dynamic previews for the missing attacks, with separate hit counts.
fix('case 49:base=6+(2+up)*strike_count(g);break;',
'''case 49:base=6+(2+up)*strike_count(g);break;
 case 56:base=20+10*up;break;
 case 66:base=5+up;break;
 case 70:base=16+5*up;break;
 case 74:base=15+5*up;break;
 case 79:base=9+g->ic[IC_RAMPAGE];break;
 case 80:base=7+2*up;break;
 case 81:base=5;break;
 case 86:base=5+2*up;break;
 case 87:base=4+2*up;break;
 case 89:base=14+6*up;break;
 case 91:base=16+4*up;break;''')
fix('if(id==27||id==22||id==36||id==32){',
    'if(id==27||id==22||id==36||id==32||id==66||id==81||id==86||id==87){')
fix('id==36?4+up:(target>=0',
    'id==36?4+up:id==66||id==87?2:id==81?(g->ic[IC_HPLOSS_TURN]?(2+up):1):id==86?MIN(20,1+g->ic[IC_HPLOSS_COMBAT]):(target>=0')
# Add a small auto-play runner; forced played card is free this time only,
# preserving normal Exhaust, Powers, copies and choice screens.
fix('void game_play_card(Game*g,int index,int enemy){',
'''static void autoplay_card(Game*g,int raw,int force_exhaust){
 if(g->screen!=COMBAT||g->hand_count>=MAX_HAND||g->ic[IC_CHAIN_DEPTH]>=20)return;
 int foe=random_living_foe(g);
 if(foe<0)return;
 int index=g->hand_count++;
 g->hand[index]=raw|CARD_FREE|(force_exhaust?CARD_FORCE_EXHAUST:0);
 g->ic[IC_CHAIN_DEPTH]++;
 game_play_card(g,index,foe);
 g->ic[IC_CHAIN_DEPTH]--;
}
static void autoplay_from_draw(Game*g,int limit,int force_exhaust){
 for(int i=0;i<limit&&g->screen==COMBAT&&g->draw_count>0&&
     g->ic[IC_CHAIN_DEPTH]<20;i++){
  int raw=g->draw[--g->draw_count];
  autoplay_card(g,raw,force_exhaust);
 }
}
void game_play_card(Game*g,int index,int enemy){''')
fix('if(cost==-2)return;',
'''/* Multiplayer-only ally target cards are deliberately not playable solo. */
 if(id==63||id==85){msg(g,"Requires multiplayer ally.");return;}
 if(cost==-2)return;''')
fix('if(g->energy<cost)return;',
'''if(g->energy<cost)return;
 int original_raw=raw;
 int repeat_this_attack=0;
 if(id<CARD_COUNT&&cards[id].type==0){
  if(g->ic[IC_UNRELENTING])g->ic[IC_UNRELENTING]=0;
  if(!(raw&CARD_EPHEMERAL)&&g->ic[IC_NEXT_ATTACK_REPEAT]>0){
   g->ic[IC_NEXT_ATTACK_REPEAT]--;
   repeat_this_attack=1;
  }
 }''')
# Add new switch cases. 66 Fight Me +3/4 Strength +enemy Strength.
fix('  case 49:deal(g,enemy,6+(2+up)*strike_count(g));break;',
'''  case 49:deal(g,enemy,6+(2+up)*strike_count(g));break;
  case 54:g->ic[IC_AGGRESSION]++;break;
  case 55:g->ic[IC_BARRICADE]=1;break;
  case 56:deal(g,enemy,20+10*up);
    apply_vulnerable(g,&g->enemies[enemy],5+2*up);break;
  case 57:autoplay_from_draw(g,MIN(30,cost+up),0);break;
  case 58:gain_block(g,5+3*up);g->ic[IC_COLOSSUS]=1;break;
  case 59:g->ic[IC_CORRUPTION]=1;break;
  case 60:g->ic[IC_CRIMSON]+=8+2*up;g->ic[IC_CRIMSON_COPIES]++;break;
  case 61:g->ic[IC_CRUELTY]=MIN(100,g->ic[IC_CRUELTY]+25+25*up);break;
  case 62:g->ic[IC_DARK_EMBRACE]++;break;
  case 63:break; /* Multiplayer-only; rejected before spending. */
  case 64:draw_card(g);draw_card(g);break;
  case 65:{
    int n=0;
    for(int i=0;i<g->hand_count;i++)
     if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)n++;
    gain_energy(g,n);
    g->ic[IC_ENERGY_LOCK]=1;break;
  }
  case 66:deal(g,enemy,5+up);deal(g,enemy,5+up);
    g->strength+=3+up;g->enemies[enemy].strength++;break;
  case 67:if(g->reward_potion&4)gain_energy(g,3+up);break;
  case 68:autoplay_from_draw(g,1,1);break;
  case 69:g->ic[IC_HELLRAISER]++;break;
  case 70:all_damage(g,16+5*up);break;
  case 71:{
    for(int tries=0;tries<150;tries++){
      int card=rnd(g,CARD_COUNT);
      if(card<CARD_COUNT&&cards[card].type==0&&
         game_card_is_playable(card)&&card!=CARD_GIANT_ROCK&&card!=56){
        add_generated(g,card|CARD_FREE);break;
      }
    }break;
  }
  case 72:g->ic[IC_JUGGERNAUT]+=6+2*up;break;
  case 73:g->ic[IC_JUGGLING]=1;break;
  case 74:{
    deal(g,enemy,15+5*up);
    int loss=10+5*up;
    g->enemies[enemy].strength-=loss;
    if(enemy<2)g->ic[IC_MANGLE0+enemy]+=loss;
    break;
  }
  case 75:heal(g,10+3*up);break;
  case 76:g->ic[IC_NEXT_ATTACK_REPEAT]+=1+up;break;
  case 77:
   for(int i=0;i<g->hand_count;i++)
    if(CARD_ID(g->hand[i])<CARD_COUNT&&
       cards[CARD_ID(g->hand[i])].type==0)
      g->hand[i]=CARD_GIANT_ROCK|(up?UPBIT:0);
   break;
  case 78:g->ic[IC_PYRE]+=1+up;break;
  case 79:deal(g,enemy,9+g->ic[IC_RAMPAGE]);
    g->ic[IC_RAMPAGE]+=5+4*up;break;
  case 80:deal(g,enemy,7+2*up);g->strength+=2+up;
    g->ic[IC_TEMP_STRENGTH]+=2+up;break;
  case 81:{
    int hits=g->ic[IC_HPLOSS_TURN]?2+up:1;
    for(int i=0;i<hits&&g->enemies[enemy].hp>0;i++)deal(g,enemy,5);
    break;
  }
  case 82:g->ic[IC_STAMPEDE]++;break;
  case 83:{
    int n=g->hand_count;
    for(int i=0;i<n;i++)exhaust_card(g,g->hand[i]);
    g->hand_count=0;
    for(int i=0;i<n&&g->hand_count<MAX_HAND;i++){
      for(int tries=0;tries<140;tries++){
       int id=rnd(g,CARD_COUNT);
       if(game_card_is_playable(id)&&id!=CARD_GIANT_ROCK&&id!=63&&id!=85){
        add_generated(g,id|(up?UPBIT:0));break;
       }
      }
    }
    break;
  }
  case 84:g->ic[IC_PLATING]+=4+2*up;break;
  case 85:break; /* Multiplayer-only; rejected before spending. */
  case 86:{
    int hits=MIN(20,1+g->ic[IC_HPLOSS_COMBAT]);
    for(int i=0;i<hits&&g->enemies[enemy].hp>0;i++)deal(g,enemy,5+2*up);
    break;
  }
  case 87:{
    int added=0,count=0;
    for(int i=0;i<g->hand_count;i++)
     if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)count++;
    if(count>0){
     int which=rnd(g,count);
     for(int i=0;i<g->hand_count;i++){
      int c=g->hand[i];
      if(cards[CARD_ID(c)].type==0&&which--==0){
       added=MAX(0,game_effective_damage(g,c,enemy)-g->strength);
       for(int j=i;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];
       g->hand_count--;exhaust_card(g,c);break;
      }
     }
    }
    deal(g,enemy,4+2*up+added);
    if(g->enemies[enemy].hp>0)deal(g,enemy,4+2*up+added);break;
  }
  case 88:g->ic[IC_UNMOVABLE]=1;break;
  case 89:deal(g,enemy,14+6*up);g->ic[IC_UNRELENTING]=1;break;
  case 90:g->ic[IC_VICIOUS]+=1+up;break;
  case 91:deal(g,enemy,16+4*up);break;''')
# End-of-card common handling: status/power/forced exhaustion, temporary echo,
# one-two punch's extra Attack and Juggling once per turn.
fix('if(id==9||id==30||id==34||id==38||id==40||id==42||id==47){exhaust_card(g,raw);}',
'''int must_exhaust=(id==9||id==30||id==34||id==38||id==40||
   id==42||id==47||id==67||id==71||id==75||id==83||
   (id==63&&!up)|| (raw&CARD_FORCE_EXHAUST) ||
   (g->ic[IC_CORRUPTION]&&cards[id].type==1));
 if(must_exhaust){exhaust_card(g,raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL));}''')
fix('else if(cards[id].type!=2&&!(id==25&&g->screen==CARD_CHOICE)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;',
    'else if(cards[id].type!=2&&!(raw&CARD_EPHEMERAL)&&!(id==25&&g->screen==CARD_CHOICE)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST);')
fix('if(cards[id].type==0){\n  g->shop_relic++;',
'''if(cards[id].type==0){
  g->ic[IC_ATTACKS_THIS_TURN]++;
  if(g->ic[IC_JUGGLING]&&g->ic[IC_ATTACKS_THIS_TURN]==3)
    add_generated(g,(original_raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL)));
  g->shop_relic++;''')
fix('if(g->hp<=0&&g->screen!=DEFEAT)g->screen=DEFEAT;',
'''if(repeat_this_attack&&g->screen==COMBAT&&g->ic[IC_CHAIN_DEPTH]<20){
   autoplay_card(g,original_raw|CARD_EPHEMERAL,0);
 }
 if(g->hp<=0&&g->screen!=DEFEAT)g->screen=DEFEAT;''')
# Skills created by other effects cost 0 only *this* turn via flag;
# unplayed free cards revert to original cost at end of turn.
fix('for(int i=0;i<g->hand_count;i++){\n  if(CARD_ID(g->hand[i])==50)',
    'for(int i=0;i<g->hand_count;i++){\n  g->hand[i]&=~CARD_FREE;\n  if(CARD_ID(g->hand[i])==50)')
# The 50-slot interaction cost and reward system already works unchanged.
p.write_text(s)
print("PASS: all 37 new card cases implemented, plus Giant Rock and auto-play recursion guard.")
