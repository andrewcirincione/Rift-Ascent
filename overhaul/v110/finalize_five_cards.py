#!/usr/bin/env python3
"""Complete 50-slot Ironclad roster: five wiki cards, dynamic costs, discard choice.

Runs after expand_ironclad_roster.py. Keeps the original numeric IDs (0-49),
existing status IDs (50-53), GAME_VERSION=16 and Game struct layout.
"""
from pathlib import Path
import sys
root=Path(sys.argv[1]);p=root/"game.c";h=root/"game.h";r=root/"render.c"
s=p.read_text();rs=r.read_text();hs=h.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("Last-five game anchor missing "+a[:90])
 s=s.replace(a,b,1)
def draw(a,b):
 global rs
 if a not in rs:raise RuntimeError("Last-five render anchor missing "+a[:90])
 rs=rs.replace(a,b,1)
for a,b in [
 ('{"Cleave","Deal 8 damage to all enemies.",1,1,0}', '{"Stomp","12 to ALL. Cost -1 per Attack played this turn.",3,1,0}'),
 ('{"Vital Strike","Deal 7; heal 2",1,1,0}', '{"Headbutt","Deal 9. Choose a Discard card: top of Draw.",1,0,0}'),
 ('{"War Cry","Draw 3; discard 1",0,1,2}', '{"Pillage","Deal 6. Draw until a non-Attack.",1,1,0}'),
 ('{"Coagulate","Heal 5 and gain 5 block",2,2,1}', '{"Pact\'s End","3+ Exhausted: deal 17 to ALL.",0,1,0}'),
 ('{"Grit","Block 7; add 1 strength",2,1,1}', '{"Molten Fist","Deal 10. Double foe Vulnerable. Exhaust.",1,1,0}'),
 ('"Deal 11 to all foes"','"15 to ALL. Cost -1 per Attack played."'),
 ('"Deal 10; heal 3"','"Deal 12. Choose Discard card for Draw top."'),
 ('"Draw 4; discard 1"','"Deal 9. Draw until a non-Attack."'),
 ('"Heal 7 and gain 8 block"','"3+ Exhausted: deal 23 to ALL."'),
 ('"Block 10; add 1 strength"','"Deal 14. Double foe Vulnerable. Exhaust."')
]:fix(a,b)
fix('if(id==21&&CARD_UP(raw))return 0;', 'if(id==21&&CARD_UP(raw))return 0;\n if(id==37)return 0;')
fix('void game_play_card(Game*g,int index,int enemy){', '''int game_card_cost_now(const Game*g,int raw){
 if(CARD_ID(raw)==3&&g){int cost=3-g->shop_relic;return cost>0?cost:0;}
 return game_card_cost(raw);
}
void game_play_card(Game*g,int index,int enemy){''')
fix('int raw=g->hand[index],id=CARD_ID(raw),up=CARD_UP(raw),cost=game_card_cost(raw);',
    'int raw=g->hand[index],id=CARD_ID(raw),up=CARD_UP(raw),cost=game_card_cost_now(g,raw);')
fix('if(cost==-2)return;\n if(cost==-1)',
    'if(cost==-2||(id==37&&g->exhaust_count<3))return;\n if(cost==-1)')
fix('case 3:all_damage(g,8+n);break;', 'case 3:all_damage(g,12+3*up);break;')
fix('case 25:deal(g,enemy,7+n);heal(g,2+up);break;', '''case 25:{
 deal(g,enemy,9+3*up);
 int remaining=0;
 for(int q=0;q<g->enemy_count;q++)if(g->enemies[q].hp>0)remaining++;
 if(g->discard_count>0&&remaining>0){
   g->next_screen=25;g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;
 }break;
}''')
fix('''case 26:
 for(int j=0;j<3+up;j++)draw_card(g);
 if(g->hand_count>0){g->next_screen=26;g->scroll=up;g->selection=0;
  g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
 break;''','''case 26:{
 deal(g,enemy,6+3*up);
 while(g->hand_count<MAX_HAND&&(g->draw_count||g->discard_count)&&
       !(g->reward_potion&1)){
   int before=g->hand_count;draw_card(g);
   if(g->hand_count<=before)break;
   int newest=CARD_ID(g->hand[g->hand_count-1]);
   if(newest>=CARD_COUNT||cards[newest].type!=0)break;
 }break;
}''')
fix('case 37:heal(g,5+up*2);gain_block(g,5+n);break;',
    'case 37:if(g->exhaust_count>=3)all_damage(g,17+6*up);break;')
fix('case 40:gain_block(g,7+n);g->strength+=1;break;',
    'case 40:deal(g,enemy,10+4*up);g->enemies[enemy].vulnerable=MIN(99,g->enemies[enemy].vulnerable*2);break;')
fix('else if(cards[id].type!=2&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;',
    'else if(cards[id].type!=2&&!(id==25&&g->screen==CARD_CHOICE)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;')
fix('if(id==9||id==30||id==34||id==38||id==42||id==47){exhaust_card(g,raw);}',
    'if(id==9||id==30||id==34||id==38||id==40||id==42||id==47){exhaust_card(g,raw);}')
fix('case 3:base=8+3*up;break;', 'case 3:base=12+3*up;break;')
fix('case 25:base=7+3*up;break;',
    'case 25:base=9+3*up;break;case 26:base=6+3*up;break;case 37:base=g->exhaust_count>=3?17+6*up:0;break;case 40:base=10+4*up;break;')
fix('int pending=g->next_screen,up=g->scroll,c=g->hand[idx];',
    'int pending=g->next_screen,c=g->hand[idx];')
fix('if(g->screen!=CARD_CHOICE||idx<0||idx>=g->hand_count)return;', '''int up=g->scroll;
 if(g->screen!=CARD_CHOICE||idx<0||
 idx>=(g->next_screen==25?g->discard_count:g->hand_count))return;
 if(g->next_screen==25){
  int c=g->discard[idx];
  for(int i=idx;i+1<g->discard_count;i++)g->discard[i]=g->discard[i+1];
  g->discard_count--;
  if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=c;
  g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
  if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=25|(up?UPBIT:0);
  g->selection=0;check_win(g);return;
 }''')
fix('case CARD_CHOICE:\n  if(b==BTN_UP&&g->selection>=3)',
    'case CARD_CHOICE:{\n  int choices=g->next_screen==25?g->discard_count:g->hand_count;\n  if(b==BTN_UP&&g->selection>=3)')
fix('else if(b==BTN_DOWN&&g->selection+3<g->hand_count)g->selection+=3;',
    'else if(b==BTN_DOWN&&g->selection+3<choices)g->selection+=3;')
fix('else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->hand_count)g->selection++;',
    'else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<choices)g->selection++;')
fix('else if(b==BTN_BACK)msg(g,"Choose a card to finish the effect.");\n  break;',
    'else if(b==BTN_BACK)msg(g,"Choose a card to finish the effect.");\n  break;}')
fix('g->screen==CARD_CHOICE&&\n (g->hand_count<1||g->selection<0||g->selection>=g->hand_count||',
    'g->screen==CARD_CHOICE&&\n ((g->next_screen==25?g->discard_count:g->hand_count)<1||g->selection<0||\n g->selection>=(g->next_screen==25?g->discard_count:g->hand_count)||')
fix('g->next_screen!=17&&g->next_screen!=18&&g->next_screen!=23&&g->next_screen!=26&&g->next_screen!=43',
    'g->next_screen!=17&&g->next_screen!=18&&g->next_screen!=23&&g->next_screen!=25&&g->next_screen!=43')
draw('pending==17?"TRUE GRIT+ - EXHAUST": "PLACE ON TOP OF DRAW"',
     'pending==17?"TRUE GRIT+ - EXHAUST": "HEADBUTT - CHOOSE DISCARDED CARD"')
draw('int page=(g->selection/6)*6;\n for(int i=page;i<g->hand_count&&i<page+6;i++){',
     'int page=(g->selection/6)*6;\n int count=pending==25?g->discard_count:g->hand_count;\n for(int i=page;i<count&&i<page+6;i++){')
draw('card_render(x,y,99,84,g->hand[i],i==g->selection,1);',
     'card_render(x,y,99,84,pending==25?g->discard[i]:g->hand[i],i==g->selection,1);')
draw('if(g->hand_count){\n  int c=g->hand[g->selection],id=CARD_ID(c);',
     'if(count){\n  int c=pending==25?g->discard[g->selection]:g->hand[g->selection],id=CARD_ID(c);')
draw('pending==26?"TO DRAW PILE":"SEND TO EXHAUST"',
     'pending==25?"TO TOP OF DRAW":"SEND TO EXHAUST"')
draw('if(page+6<g->hand_count)text(348,220,"MORE CARDS BELOW",MUTED,1);',
     'if(page+6<count)text(348,220,"MORE CARDS BELOW",MUTED,1);')
draw('int active=game_card_cost(g->hand[idx])!=-2 &&',
     'int active=game_card_cost_now(g,g->hand[idx])!=-2 &&')
draw('(game_card_cost(g->hand[idx])<=g->energy||game_card_cost(g->hand[idx])==-1);',
     '(game_card_cost_now(g,g->hand[idx])<=g->energy||game_card_cost_now(g,g->hand[idx])==-1)&&\n         (id!=37||g->exhaust_count>=3);')
draw('game_card_cost(g->hand[i])!=-2&&(game_card_cost(g->hand[i])<=g->energy||game_card_cost(g->hand[i])==-1)',
     'game_card_cost_now(g,g->hand[i])!=-2&&(game_card_cost_now(g,g->hand[i])<=g->energy||game_card_cost_now(g,g->hand[i])==-1)&&\n (id!=37||g->exhaust_count>=3)')
draw('if(game_card_cost(raw)==-1)snprintf(cost,sizeof(cost),"X");',
     'if(game_card_cost_now(card_combat_context,raw)==-1)snprintf(cost,sizeof(cost),"X");')
draw('else snprintf(cost,sizeof(cost),"%d",game_card_cost(raw)<0?0:game_card_cost(raw));',
     'else snprintf(cost,sizeof(cost),"%d",game_card_cost_now(card_combat_context,raw)<0?0:game_card_cost_now(card_combat_context,raw));')
draw('if(h>70)fittext(x+5,y+h-14,"LOW ENERGY",(w-10)/7,WHITE);',
     'if(h>70)fittext(x+5,y+h-14,(id==37&&card_combat_context->exhaust_count<3)?"NEED 3 EXHAUST":"LOW ENERGY",(w-10)/7,WHITE);')
needle='int game_card_cost(int raw);'
if needle not in hs:raise RuntimeError("Missing header anchor")
hs=hs.replace(needle,needle+'\nint game_card_cost_now(const Game *g,int raw);',1)
p.write_text(s);r.write_text(rs);h.write_text(hs)
print("PASS five last replacements: Stomp, Headbutt, Pillage, Pact's End, Molten Fist.")
