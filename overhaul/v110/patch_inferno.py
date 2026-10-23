#!/usr/bin/env python3
"""Rift 1.1.0: align Inferno with STS2 current card text and combat behavior.

Reference: https://sts.wiki/cards/inferno (current: cost 1, 6 / 9 AoE).
Preserves GAME_VERSION=16, Game size and previous saves.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"game.c"
s=p.read_text()
def replace(a,b):
 global s
 if a not in s: raise RuntimeError("Inferno source anchor missing: "+a[:115])
 s=s.replace(a,b,1)

replace('{"Inferno","Lose 1 HP each turn; hurt all enemies.",1,2,2}',
        '{"Inferno","Start: lose 1 HP. On your turn, HP loss: 6 to ALL.",1,1,2}')
replace('"Lose 1 HP each turn; deal 9 to all enemies."',
        '"Start: lose 1 HP. On your turn, HP loss: 9 to ALL."')

# used_potion was already repurposed for the Inferno Power in v1.0.3.
# Keep same Game struct. High 16 bits = stacked copies, low 16 = AoE damage.
# Legacy v1.0.3 saves have only 6 or 9 in the lower bits, infer one copy.
replace("static void deal(Game*g,int idx,int amt);\nstatic void lose_hp(Game*g,int n){",
'''#define RIFT_INFERNO_POWER(g) ((g)->used_potion & 0xffff)
#define RIFT_INFERNO_COPIES(g) ((g)->used_potion >> 16 ? (g)->used_potion >> 16 : ((g)->used_potion > 0 ? 1 : 0))
static void lose_hp(Game*g,int n){''')
replace(''' if(n>0&&g->screen==COMBAT&&g->used_potion>0){
   for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)
     deal(g,z,g->used_potion);
 }''',
''' if(n>0&&g->screen==COMBAT&&RIFT_INFERNO_POWER(g)>0){
   /* Trigger once per HP-loss event, not once for each point of lost HP.
      Inferno's AoE is a Power effect: it ignores Strength, Weak, Vulnerable
      and on-attack relics but respects enemy Block. */
   const int power=RIFT_INFERNO_POWER(g);
   for(int z=0;z<g->enemy_count;z++){
     Enemy *foe=&g->enemies[z];
     if(foe->hp<=0)continue;
     int blocked=MIN(foe->block,power);
     foe->block-=blocked;
     foe->hp-=power-blocked;
   }
 }''')
replace("void game_start_turn(Game*g){",
        "static void check_win(Game*g);\nvoid game_start_turn(Game*g){")
replace(" if(g->used_potion>0)lose_hp(g,1);",
''' if(RIFT_INFERNO_POWER(g)>0){
    /* Each copy loses 1 HP at turn start and triggers every copy's AoE. */
    const int copies=RIFT_INFERNO_COPIES(g);
    for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
    if(g->screen==COMBAT)check_win(g);
 }''')
replace(" if(g->screen==DEFEAT)return;\n g->energy=MAX(0,g->max_energy",
        " if(g->screen!=COMBAT)return;\n g->energy=MAX(0,g->max_energy")
replace("  case 28:g->used_potion=6+3*up;lose_hp(g,1);break;",
'''  case 28:{
    /* Playing the Power does NOT immediately cost HP. Start next turn. */
    int copies=RIFT_INFERNO_COPIES(g);
    int dmg=RIFT_INFERNO_POWER(g);
    copies=MIN(copies+1,30);
    dmg=MIN(dmg+6+3*up,32000);
    g->used_potion=(copies<<16)|dmg;
    break;
  }''')
# Status damage on enemy turn must not fire Inferno.
replace(''' if(g->poison>0){lose_hp(g,g->poison);g->poison--;if(g->screen==DEFEAT)return;}''',
''' if(g->poison>0){
   int inferno=g->used_potion;g->used_potion=0;
   lose_hp(g,g->poison);g->used_potion=inferno;
   g->poison--;if(g->screen==DEFEAT)return;
 }''')
p.write_text(s)
print("PASS 1.1.0 Inferno: cost 1, 6/9 AoE on each self HP-loss event, own-turn only; stacks and preview text.")
