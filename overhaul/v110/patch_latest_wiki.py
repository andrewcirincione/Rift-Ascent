#!/usr/bin/env python3
"""Pin upgraded numbers/conditional rules to STS2 July 2026 wiki revisions."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c";r=Path(sys.argv[1])/"render.c"
s=p.read_text();rs=r.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("Wiki rules missing game anchor "+a[:100])
 s=s.replace(a,b,1)
def draw(a,b):
 global rs
 if a not in rs:raise RuntimeError("Wiki rules missing render anchor "+a[:100])
 rs=rs.replace(a,b,1)
for a,b in [
 ('{"Pact\'s End","3+ Exhausted: deal 17 to ALL.",0,1,0}',
  '{"Pact\'s End","3+ Exhausted: deal 18 to ALL.",0,2,0}'),
 ('"3+ Exhausted: deal 23 to ALL."','"3+ Exhausted: deal 24 to ALL."'),
 ('"Lose 2 HP; gain 3 Energy"','"Lose 3 HP. Gain 3 Energy."'),
 ('{"Demon Form","Gain 2 Strength at turn start.",3,2,2}',
  '{"Demon Form","At turn start: +3 Strength.",3,2,2}'),
 ('"Gain 3 Strength at start of each turn."','"At turn start: +4 Strength."'),
 ('case 6:lose_hp(g,up?2:3);g->energy+=2+up;break;',
  'case 6:lose_hp(g,3);g->energy+=2+up;break;'),
 ('case 37:if(g->exhaust_count>=3)all_damage(g,17+6*up);break;',
  'case 37:if(g->exhaust_count>=3)all_damage(g,18+6*up);break;'),
 ('case 48:g->heal_after_battle=2+up;break;',
  'case 48:g->heal_after_battle+=3+up;break;'),
 ('case 37:base=g->exhaust_count>=3?17+6*up:0;break;',
  'case 37:base=g->exhaust_count>=3?18+6*up:0;break;'),
 ('if(cost==-2||(id==37&&g->exhaust_count<3))return;',
  'if(cost==-2)return;')
]:fix(a,b)
# STS2 Pact's End costs 0 and can be played below 3 Exhaust, doing zero damage.
draw('&&\n         (id!=37||g->exhaust_count>=3);',';')
draw('&&\n (id!=37||g->exhaust_count>=3)','')
draw('(id==37&&card_combat_context->exhaust_count<3)?"NEED 3 EXHAUST":"LOW ENERGY"',
     '"LOW ENERGY"')
p.write_text(s);r.write_text(rs)
print("PASS STS2 later patch: Bloodletting+ 3 HP; Demon Form 3/4; Pact End 18/24 & playable below threshold.")
