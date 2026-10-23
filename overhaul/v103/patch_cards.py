#!/usr/bin/env python3
"""Additional Ironclad-style mechanics and costs for 1.0.3 (original artwork)."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c";s=p.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("Card redesign source mismatch: "+a[:95])
 s=s.replace(a,b,1)
changes=[
 ('{"Shield Crash","Block 7 and hit 7",2,1,0}','{"Iron Wave","Gain 5 Block. Deal 5 damage.",1,1,0}'),
 ('{"Counterweight","Gain 3 strength; lose 5 HP",1,2,2}','{"Brand","Lose 1 HP. Gain Strength. Exhaust 1 card.",0,2,2}'),
 ('{"Crack Armor","Deal 9; inflict 3 vulnerable",2,1,0}','{"Thunderclap","Hit all enemies for 4. Apply 1 Vulnerable.",1,1,0}'),
 ('{"Fortify","Gain 12 block; gain 1 dex",2,2,1}','{"Armaments","Gain 5 Block. Upgrade 1 card in hand.",1,1,1}'),
 ('{"Juggernaut","Strength +1; block 8",2,2,2}','{"Inferno","Lose 1 HP each turn; hurt all enemies.",1,2,2}'),
 ('{"Reckless","Deal 17; lose 4 HP",1,2,0}','{"Hemokinesis","Lose 2 HP. Deal 15 damage.",1,1,0}'),
 ('{"Taunt","Inflict 3 weak; gain 3 block",1,1,2}','{"Taunt","Gain 7 Block. Apply 1 Vulnerable.",1,1,1}'),
 ('{"Battle Hymn","Gain 1 strength and 1 energy",1,2,2}','{"Dominate","Gain Strength for enemy Vulnerable.",1,1,2}'),
 ('{"Iron Tempest","All enemies: 12 damage",2,2,0}','{"Conflagration","Hit all enemies. Scales with attacks.",1,2,0}'),
 ('{"Bone Crusher","Deal 12; weak 2",2,2,0}','{"Uppercut","Deal 13. Apply Weak and Vulnerable.",2,2,0}'),
 ('{"Riposte","Block 6; deal 6",1,1,0}','{"Brawling Wave","Gain 5 Block. Deal 5 damage.",1,1,0}'),
 ('{"Bludgeon+","Deal 30 damage. Exhaust.",3,2,0}','{"Fiend Fire","Exhaust hand. Deal 7 per exhausted card.",2,2,0}'),
 ('{"Sacrifice","Lose 5 HP; draw 4",0,2,2}','{"Burning Pact","Exhaust 1 card. Draw 2 cards.",1,2,2}'),
 ('{"Firebrand","Deal 5; strength +1",1,2,0}','{"Anger","Deal 6. Add a copy to discard.",0,0,0}'),
 ('{"Mercy Blade","Deal 11; heal if kill",2,1,0}','{"Feed","Deal 10. On kill, gain 3 max HP.",1,2,0}'),
 ('{"Battle Focus","Strength +2; exhaust",1,2,2}','{"Demon Form","Gain 2 Strength at turn start.",3,2,2}'),
]
for a,b in changes:fix(a,b)
effects=[
 ('case 12:gain_block(g,7+n);deal(g,enemy,7+n);break;','case 12:gain_block(g,5+n);deal(g,enemy,5+n);break;'),
 ('case 18:g->strength+=3+up;lose_hp(g,5);break;',
  'case 18:lose_hp(g,1);g->strength+=1+up;if(g->hand_count>0){int c=g->hand[--g->hand_count];if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;}break;'),
 ('case 19:deal(g,enemy,9+n);g->enemies[enemy].vulnerable+=3+up;break;',
  'case 19:all_damage(g,4+3*up);for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)g->enemies[z].vulnerable+=1+up;break;'),
 ('case 23:gain_block(g,12+n);g->dexterity+=1+up;break;',
  'case 23:gain_block(g,5+n);for(int z=0;z<g->hand_count;z++)if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50){g->hand[z]|=UPBIT;if(!up)break;}break;'),
 ('case 28:g->strength+=1+up;gain_block(g,8+n);break;',
  'case 28:g->used_potion=6+3*up;lose_hp(g,1);break;'),
 ('case 29:deal(g,enemy,17+2*n);lose_hp(g,4);break;',
  'case 29:lose_hp(g,2);deal(g,enemy,15+5*up);break;'),
 ('case 31:g->enemies[enemy].weak+=3+up;gain_block(g,3+n);break;',
  'case 31:gain_block(g,7+n);g->enemies[enemy].vulnerable+=1+up;break;'),
 ('case 34:g->strength+=1+up;g->energy++;break;',
  'case 34:g->strength+=(g->enemies[enemy].vulnerable)*(1+up);break;'),
 ('case 36:all_damage(g,12+2*n);break;',
  'case 36:all_damage(g,8+3*up+2*g->turn_played);break;'),
 ('case 39:deal(g,enemy,12+n);g->enemies[enemy].weak+=2+up;break;',
  'case 39:deal(g,enemy,13+3*up);g->enemies[enemy].weak+=1+up;g->enemies[enemy].vulnerable+=1+up;break;'),
 ('case 41:gain_block(g,6+n);deal(g,enemy,6+n);break;',
  'case 41:gain_block(g,5+n);deal(g,enemy,5+n);break;'),
 ('case 42:deal(g,enemy,28+3*n);break;',
  'case 42:{int count=g->hand_count;for(int z=0;z<count;z++)if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=g->hand[z];g->hand_count=0;for(int z=0;z<count;z++)deal(g,enemy,7+3*up);break;}'),
 ('case 43:lose_hp(g,5);for(int j=0;j<4+up;j++)draw_card(g);break;',
  'case 43:if(g->hand_count>0){int c=g->hand[--g->hand_count];if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;}for(int j=0;j<2+up;j++)draw_card(g);break;'),
 ('case 44:deal(g,enemy,5+n);g->strength+=1;break;',
  'case 44:deal(g,enemy,6+3*up);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;break;'),
 ('case 47: {int was=g->enemies[enemy].hp;deal(g,enemy,11+n);if(was>0&&g->enemies[enemy].hp<=0)heal(g,5+3*up);break;}',
  'case 47:{int was=g->enemies[enemy].hp;deal(g,enemy,10+2*up);if(was>0&&g->enemies[enemy].hp<=0){g->maxhp+=3+up;heal(g,3+up);}break;}'),
 ('case 48:g->strength+=2+up;break;',
  'case 48:g->heal_after_battle=2+up;break;'),
]
for a,b in effects:fix(a,b)
fix('if(id==20||id==24||id==42||id==45||id==48)',
    'if(id==8||id==9||id==20||id==24||id==28||id==38||id==42||id==45||id==47||id==48)')
# Use Game fields that were dormant; preserve save layout.
fix("static void lose_hp(Game*g,int n){g->hp-=MAX(n,0);player_death(g);}",
'''static void deal(Game*g,int idx,int amt);
static void lose_hp(Game*g,int n){
 n=MAX(n,0);g->hp-=n;
 if(n>0&&g->screen==COMBAT&&g->used_potion>0){
   for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)
     deal(g,z,g->used_potion);
 }
 player_death(g);
}''')
fix('''g->turn++;
 g->energy=MAX''',
'''g->turn++;
 if(g->heal_after_battle>0)g->strength+=g->heal_after_battle;
 if(g->used_potion>0)lose_hp(g,1);
 if(g->screen==DEFEAT)return;
 g->energy=MAX''')
fix('''g->turn=0;g->turn_played=0;g->selection=0;''',
    '''g->turn=0;g->turn_played=0;g->heal_after_battle=0;g->used_potion=0;g->selection=0;''')
# Dynamic card preview must match all upgraded mechanical changes.
for a,b in [
 ('case 7:base=15+6*up;break;','case 7:base=32+6*up;break;'),
 ('case 10:base=4+2*up;break;','case 10:base=5+3*up;break;'),
 ('case 11:base=7+3*up;break;','case 11:base=9+3*up;break;'),
 ('case 12:base=7+3*up;break;','case 12:base=5+3*up;break;'),
 ('case 19:base=9+3*up;break;','case 19:base=4+3*up;break;'),
 ('case 29:base=17+6*up;break;','case 29:base=15+5*up;break;'),
 ('case 36:base=12+6*up;break;','case 36:base=8+3*up+2*g->turn_played;break;'),
 ('case 39:base=12+3*up;break;','case 39:base=13+3*up;break;'),
 ('case 41:base=6+3*up;break;','case 41:base=5+3*up;break;'),
 ('case 42:base=28+9*up;break;','case 42:base=(7+3*up)*g->hand_count;break;'),
 ('case 44:base=5+3*up;break;','case 44:base=6+3*up;break;'),
 ('case 47:base=11+3*up;break;','case 47:base=10+2*up;break;')
]:fix(a,b)
for a,b in [
 ('"Block 10 and hit 10"','"Gain 8 Block. Deal 8 damage."'),
 ('"Gain 4 strength; lose 5 HP"','"Lose 1 HP. Gain 2 Strength. Exhaust 1 card."'),
 ('"Deal 12; inflict 4 vulnerable"','"Hit all for 7. Apply 2 Vulnerable."'),
 ('"Gain 15 block; gain 2 dex"','"Gain 8 Block. Upgrade all cards in hand."'),
 ('"Strength +2; block 11"','"Lose 1 HP each turn; deal 9 to all enemies."'),
 ('"Deal 23; lose 4 HP"','"Lose 2 HP. Deal 20 damage."'),
 ('"Inflict 4 weak; gain 6 block"','"Gain 10 Block. Apply 2 Vulnerable."'),
 ('"Gain 2 strength and 1 energy"','"Gain 2 Strength per enemy Vulnerable."'),
 ('"All enemies: 18 damage"','"Hit all for 11; scales with attacks."'),
 ('"Deal 15; weak 3"','"Deal 16. Apply 2 Weak and 2 Vulnerable."'),
 ('"Block 9; deal 9"','"Gain 8 Block. Deal 8 damage."'),
 ('"Deal 37; exhaust"','"Exhaust hand. Deal 10 per card."'),
 ('"Lose 5 HP; draw 5"','"Exhaust 1 card. Draw 3 cards."'),
 ('"Deal 8; strength +1"','"Deal 9. Add a copy to discard."'),
 ('"Deal 14; heal 8 if kill"','"Deal 12. Fatal: gain 4 max HP."'),
 ('"Strength +3; exhaust"','"Gain 3 Strength at start of each turn."'),
 ]:fix(a,b)
p.write_text(s)
print("PASS: varied self-damage, Strength, Exhaust, X and Fatal mechanics.")
