#!/usr/bin/env python3
"""Replace 11 original/redundant cards with tested Ironclad mechanics. Preserve IDs."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);p=root/"game.c";r=root/"render.c"
s=p.read_text();rs=r.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("roster source anchor "+a[:100])
 s=s.replace(a,b,1)
for a,b in [
 ('{"Siphon","Heal 3; deal 5",1,1,0}', '{"Ashen Strike","Deal 6 +3 per Exhausted card.",1,1,0}'),
 ('{"Rust Cut","Deal 4; inflict 2 weak",1,1,0}', '{"Breakthrough","Lose 1 HP. Deal 9 to ALL.",1,0,0}'),
 ('{"Overpower","Deal 22; suffer 2 weak",3,2,0}', '{"Cinder","Deal 18. Exhaust a random card in hand.",2,0,0}'),
 ('{"Preparation","Gain 1 energy; draw 1",0,1,2}', '{"Evil Eye","Gain 8 Block twice if you Exhausted this turn.",1,1,1}'),
 ('{"Blood Pact","Heal 8; exhaust",1,2,2}', '{"Feel No Pain","Gain 3 Block whenever a card is Exhausted.",1,1,2}'),
 ('{"Blade Storm","Deal 3 four times",2,2,0}', '{"Sword Boomerang","3 damage to a random enemy 3 times.",1,0,0}'),
 ('{"Twin Guard","Gain 4 block twice",1,1,1}', '{"Tremble","Apply 3 Vulnerable. Exhaust.",1,0,1}'),
 ('{"Shatter","Deal 13; remove enemy block",2,1,0}', '{"Dismantle","Deal 8. Twice if enemy is Vulnerable.",1,1,0}'),
 ('{"Piercing Strike","Deal 9; ignore block",2,1,0}', '{"Bully","Deal 4, +2 per Vulnerable on target.",0,1,0}'),
 ('{"Adrenal Force","Energy +2; exhaust",0,2,2}', '{"Second Wind","Exhaust all non-Attacks. +5 Block each.",1,1,1}'),
 ('{"Fracture","Deal 6; vulnerable 2; draw 1",1,2,0}', '{"Perfected Strike","Deal 6, +2 per Strike you own.",2,0,0}'),
 ('"Heal 5; deal 8"','"Deal 6 +4 per Exhausted card."'),
 ('"Deal 7; inflict 3 weak"','"Lose 1 HP. Deal 13 to ALL."'),
 ('"Deal 28; suffer 1 weak"','"Deal 24. Exhaust a random hand card."'),
 ('"Gain 2 energy; draw 1"','"Gain 11 Block twice if Exhausted this turn."'),
 ('"Heal 14; exhaust"','"Gain 4 Block per card Exhausted."'),
 ('"Deal 3 five times"','"3 damage to a random enemy 4 times."'),
 ('"Gain 7 block twice"','"Apply 4 Vulnerable. Exhaust."'),
 ('"Deal 16; remove enemy block"','"Deal 10. Twice if target Vulnerable."'),
 ('"Deal 12; ignore block"','"Deal 4, +3 per Vulnerable on target."'),
 ('"Energy +3; exhaust"','"Exhaust all non-Attacks. +7 Block each."'),
 ('"Deal 9; vulnerable 3; draw 1"','"Deal 6, +3 per Strike you own."')
]:fix(a,b)
fix("int game_effective_damage(const Game *g,int raw,int target) {",
    "static int strike_count(const Game*g);\nint game_effective_damage(const Game *g,int raw,int target) {")
fix("static void all_damage(Game*g,int n){",'''static int has_strike_tag(const char *name){
 for(;*name;name++){
  const char *p=name,*key="Strike";
  while(*p&&*key&&*p==*key){p++;key++;}
  if(!*key)return 1;
 }
 return 0;
}
static void all_damage(Game*g,int n){''')
fix("static void all_damage(Game*g,int n){",'''static int strike_count(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++){
  int id=CARD_ID(g->deck[i]);
  if(id>=0&&id<CARD_COUNT&&has_strike_tag(cards[id].name))count++;
 }
 return count;
}
static void all_damage(Game*g,int n){''')
fix("static void select_live_enemy(Game*g,int direction);",'''static void exhaust_card(Game*g,int raw){
 if(g->exhaust_count<MAX_PILE){
  g->exhaust[g->exhaust_count++]=raw;
  g->reward_potion|=4;
  if(g->shop_cards[2]>0)g->block=MIN(999,g->block+g->shop_cards[2]);
 }
}
static void select_live_enemy(Game*g,int direction);''')
fix("g->shop_cards[0]=g->shop_cards[1]=0;g->turn=0;",
    "g->shop_cards[0]=g->shop_cards[1]=g->shop_cards[2]=0;g->turn=0;")
fix("g->reward_potion=1;break;","g->reward_potion|=1;break;")
fix("if(id==8||id==9||id==34||id==20||id==24||id==28||id==38||id==41||id==42||id==45||id==47||id==48)",
    "if(id==9||id==30||id==34||id==38||id==42||id==47)")
fix("if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=raw;}",
    "exhaust_card(g,raw);}")
fix("else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;",
    "else if(cards[id].type!=2&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;")
fix("if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;",
    "exhaust_card(g,c);")
fix("else if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=c;",
    "else exhaust_card(g,c);")
fix("for(int z=0;z<count;z++)if(g->exhaust_count<MAX_PILE)g->exhaust[g->exhaust_count++]=g->hand[z];",
    "for(int z=0;z<count;z++)exhaust_card(g,g->hand[z]);")
for a,b in [
 ("case 13:heal(g,3+up*2);deal(g,enemy,5+n);break;",
  "case 13:deal(g,enemy,6+(3+up)*g->exhaust_count);break;"),
 ("case 14:deal(g,enemy,4+n);g->enemies[enemy].weak+=2+up;break;",
  "case 14:lose_hp(g,1);if(g->screen==COMBAT)all_damage(g,9+4*up);break;"),
 ("case 15:deal(g,enemy,22+2*n);g->weak+=2-up;break;",
  "case 15:deal(g,enemy,18+6*up);if(g->hand_count>0){int k=rnd(g,g->hand_count),c=g->hand[k];for(int j=k;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];g->hand_count--;exhaust_card(g,c);}break;"),
 ("case 16:g->energy+=1+up;draw_card(g);break;",
  "case 16:gain_block(g,8+3*up);if(g->reward_potion&4)gain_block(g,8+3*up);break;"),
 ("case 20:heal(g,8+2*n);break;","case 20:g->shop_cards[2]+=3+up;break;"),
 ("case 22:for(int j=0;j<4+up;j++)deal(g,enemy,3);break;",
  "case 22:for(int j=0;j<3+up;j++){int count=0;for(int k=0;k<g->enemy_count;k++)if(g->enemies[k].hp>0)count++;if(!count)break;int k=rnd(g,count);for(int q=0;q<g->enemy_count;q++)if(g->enemies[q].hp>0&&k--==0){deal(g,q,3);break;}}break;"),
 ("case 30:gain_block(g,4+n);gain_block(g,4+n);break;",
  "case 30:g->enemies[enemy].vulnerable+=3+up;break;"),
 ("case 32:g->enemies[enemy].block=0;deal(g,enemy,13+n);break;",
  "case 32:{int hits=g->enemies[enemy].vulnerable?2:1;for(int z=0;z<hits;z++)deal(g,enemy,8+2*up);break;}"),
 ("case 35:damage_enemy(g,enemy,9+n,1);break;",
  "case 35:deal(g,enemy,4+(2+up)*g->enemies[enemy].vulnerable);break;"),
 ("case 45:g->energy+=2+up;break;",
  "case 45:{int ix=0;while(ix<g->hand_count){int c=g->hand[ix];if(cards[CARD_ID(c)].type!=0){for(int j=ix;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];g->hand_count--;exhaust_card(g,c);gain_block(g,5+2*up);}else ix++;}break;}"),
 ("case 49:deal(g,enemy,6+n);g->enemies[enemy].vulnerable+=2+up;draw_card(g);break;",
  "case 49:deal(g,enemy,6+(2+up)*strike_count(g));break;")
]:fix(a,b)
for a,b in [
 ("case 13:base=5+3*up;break;","case 13:base=6+(3+up)*g->exhaust_count;break;"),
 ("case 14:base=4+3*up;break;","case 14:base=9+4*up;break;"),
 ("case 15:base=22+6*up;break;","case 15:base=18+6*up;break;"),
 ("case 32:base=13+3*up;break;","case 32:base=8+2*up;break;"),
 ("case 35:base=9+3*up;break;","case 35:base=4+(2+up)*(target>=0&&target<g->enemy_count?g->enemies[target].vulnerable:0);break;"),
 ("case 49:base=6+3*up;break;","case 49:base=6+(2+up)*strike_count(g);break;")
]:fix(a,b)
fix("if(id==27||id==22||id==36)dmg*= (id==27?2:4+up);",
    "if(id==27||id==22||id==36||id==32){int hits=id==27?2:id==22?3+up:id==36?4+up:(target>=0&&target<g->enemy_count&&g->enemies[target].vulnerable?2:1);dmg*=hits;}")
p.write_text(s);r.write_text(rs)
print("PASS: 11 updated wiki card definitions, unified Exhaust triggers and Feel No Pain power.")
