#!/usr/bin/env python3
"""Wiki verified 1.1.0 card rules corrections. Preserve save data layout."""
from pathlib import Path
import sys
root=Path(sys.argv[1]); p=root/"game.c"; h=root/"game.h"; r=root/"render.c"
s=p.read_text(); hs=h.read_text(); rs=r.read_text()
def edit(a,b):
 global s
 if a not in s:raise RuntimeError("card rule anchor missing: "+a[:100])
 s=s.replace(a,b,1)
def red(a,b):
 global rs
 if a not in rs:raise RuntimeError("card UI anchor missing: "+a[:100])
 rs=rs.replace(a,b,1)
def head(a,b):
 global hs
 if a not in hs:raise RuntimeError("header anchor missing: "+a[:100])
 hs=hs.replace(a,b,1)
# Official STS2 type and baseline descriptions.
for a,b in [
 ('{"Battle Trance","Draw 3 cards.",0,1,2}',
  '{"Battle Trance","Draw 3. Cannot draw more this turn.",0,1,1}'),
 ('{"Bloodletting","Lose 3 HP. Gain 2 Energy.",0,1,2}',
  '{"Bloodletting","Lose 3 HP. Gain 2 Energy.",0,1,1}'),
 ('{"Brand","Lose 1 HP. Gain Strength. Exhaust 1 card.",0,2,2}',
  '{"Brand","Lose 1 HP. Exhaust 1 card. Gain 1 Strength.",0,2,1}'),
 ('{"Dominate","Gain Strength for enemy Vulnerable.",1,1,2}',
  '{"Dominate","Apply 1 Vulnerable. Gain 1 Strength per Vulnerable. Exhaust.",1,1,1}'),
 ('{"Offering","Lose 6 HP. Gain 2 Energy. Draw 3.",0,2,2}',
  '{"Offering","Lose 6 HP. Gain 2 Energy. Draw 3. Exhaust.",0,2,1}'),
 ('{"Burning Pact","Exhaust 1 card. Draw 2 cards.",1,2,2}',
  '{"Burning Pact","Exhaust 1 card. Draw 2 cards.",1,2,1}'),
 ("\"Draw 4 cards\"","\"Draw 4. Cannot draw more this turn.\""),
 ("\"Deal 12; draw 1\"","\"Deal 10 damage. Draw 2.\""),
 ("\"Deal block + 5 as damage\"","\"Deal damage equal to your Block. Cost 0.\""),
 ("\"Gain 8 Block. Upgrade all cards in hand.\"",
  "\"Gain 5 Block. Upgrade ALL cards in hand.\""),
 ("\"Gain 2 Strength per enemy Vulnerable.\"",
  "\"Apply 2 Vulnerable. Gain 1 Strength per Vulnerable. Exhaust.\"")
]: edit(a,b)

# Body Slam+ officially has zero cost. Render and UI must use the actual cost.
head("const char *card_description(int raw);",
     "const char *card_description(int raw);\nint game_card_cost(int raw);")
edit("const char *card_description(int raw){",
'''int game_card_cost(int raw){
 int id=CARD_ID(raw);
 if(id<0||id>=CARD_COUNT)return -2;
 if(id==21&&CARD_UP(raw))return 0;
 return cards[id].cost;
}
const char *card_description(int raw){''')
edit("int raw=g->hand[index],id=CARD_ID(raw),up=CARD_UP(raw),cost=cards[id].cost;",
     "int raw=g->hand[index],id=CARD_ID(raw),up=CARD_UP(raw),cost=game_card_cost(raw);")
red('if(d->cost==-1)snprintf(cost,sizeof(cost),"X");',
    'if(game_card_cost(raw)==-1)snprintf(cost,sizeof(cost),"X");')
red('else snprintf(cost,sizeof(cost),"%d",d->cost<0?0:d->cost);',
    'else snprintf(cost,sizeof(cost),"%d",game_card_cost(raw)<0?0:game_card_cost(raw));')
red('if(d->cost==-1)snprintf(coststr,sizeof(coststr),"X");',
    'if(game_card_cost(raw)==-1)snprintf(coststr,sizeof(coststr),"X");')
red('else snprintf(coststr,sizeof(coststr),"%d",d->cost<0?0:d->cost);',
    'else snprintf(coststr,sizeof(coststr),"%d",game_card_cost(raw)<0?0:game_card_cost(raw));')
red('cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1)',
    'game_card_cost(g->hand[i])!=-2&&(game_card_cost(g->hand[i])<=g->energy||game_card_cost(g->hand[i])==-1)')
red('int active=cards[id].cost!=-2 &&',
    'int active=game_card_cost(g->hand[idx])!=-2 &&')
red('(cards[id].cost<=g->energy||cards[id].cost==-1);',
    '(game_card_cost(g->hand[idx])<=g->energy||game_card_cost(g->hand[idx])==-1);')

# Battle Trance: use already-persisted reward_potion (unused by base engine) as a
# one-turn No Draw flag. Preserve Game struct/save ABI.
edit('static void draw_card(Game*g){',
     'static void draw_card(Game*g){\n if(g->screen==COMBAT&&g->reward_potion)return;')
edit(' g->turn=0;g->turn_played=0;g->heal_after_battle=0;',
     ' g->reward_potion=0;g->turn=0;g->turn_played=0;g->heal_after_battle=0;')
edit(' g->turn_played=0;\n if(relic(g,14)', ' g->turn_played=0;g->reward_potion=0;\n if(relic(g,14)')
edit('case 4:for(int j=0;j<3+up;j++)draw_card(g);break;',
     'case 4:for(int j=0;j<3+up;j++)draw_card(g);g->reward_potion=1;break;')

# Pommel Strike+ gains a 2nd draw and 1 damage, not +3 damage.
edit('case 11:deal(g,enemy,9+n);draw_card(g);break;',
     'case 11:deal(g,enemy,9+up);for(int j=0;j<1+up;j++)draw_card(g);break;')
edit('case 11:base=9+3*up;break;', 'case 11:base=9+up;break;')

# Body Slam upgraded damage is unchanged; cost is 0.
edit('case 21:deal(g,enemy,g->block+up*5);break;',
     'case 21:deal(g,enemy,g->block);break;')
edit('case 21:base=g->block+5*up;break;',
     'case 21:base=g->block;break;')

# Armaments+ retains base block; it upgrades all remaining hand cards.
edit('case 23:gain_block(g,5+n);for(int z=0;',
     'case 23:gain_block(g,5);for(int z=0;')

# Dominate applies 1/2 Vulnerable before evaluating Strength.
edit('case 34:g->strength+=(g->enemies[enemy].vulnerable)*(1+up);break;',
     'case 34:g->enemies[enemy].vulnerable+=1+up;g->strength+=g->enemies[enemy].vulnerable;break;')
edit('if(id==8||id==9||id==20', 'if(id==8||id==9||id==34||id==20')

h.write_text(hs);p.write_text(s);r.write_text(rs)
print("PASS: audit corrections to Battle Trance, Pommel Strike+, Body Slam+, Armaments+, Dominate and card types.")
