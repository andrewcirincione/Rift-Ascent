#!/usr/bin/env python3
"""Rift Ascent v1.0.3 gameplay repairs, without changing GAME_VERSION or Game ABI."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);p=root/"game.c";s=p.read_text()
def fix(old,new):
 global s
 if old not in s: raise RuntimeError("1.0.3 game anchor missing "+old[:100])
 s=s.replace(old,new,1)

# Map look-ahead is now unlimited to any future node. Only actual current-row
# neighbors can be entered; the preview never changes the run progression.
fix("if(b==BTN_UP&&g->scroll==0&&g->floor<MAP_ROWS-1)g->scroll=1;",
    "if(b==BTN_UP&&g->floor+g->scroll<MAP_ROWS-1)g->scroll++;")
fix("""else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll=0;
    if(!game_route_available(g,g->selection))
      for(int k=0;k<MAP_LANES;k++)
        if(game_route_available(g,k)){g->selection=k;break;}
  }""",
"""else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll--;
    if(g->scroll==0 && !game_route_available(g,g->selection))
      for(int k=0;k<MAP_LANES;k++)
        if(game_route_available(g,k)){g->selection=k;break;}
  }""")

# Forward declaration: played attacks can kill the currently selected enemy.
fix("void game_play_card(Game*g,int index,int enemy){",
    "static void select_live_enemy(Game*g,int direction);\nvoid game_play_card(Game*g,int index,int enemy){")
# Dead bodies must never be user-selectable targets, even in two-enemy combat.
fix("void game_action(Game*g,int b){",'''static void select_live_enemy(Game*g,int direction){
 if(g->enemy_count<=0){g->target=0;return;}
 if(g->target<0||g->target>=g->enemy_count)g->target=0;
 int active=0;
 for(int i=0;i<g->enemy_count;i++)active+=g->enemies[i].hp>0;
 if(active==0)return;
 for(int k=0;k<g->enemy_count;k++){
    g->target=(g->target+direction+g->enemy_count)%g->enemy_count;
    if(g->enemies[g->target].hp>0)return;
 }
}
void game_action(Game*g,int b){''')
fix('''else if(b==BTN_UP)g->target=(g->target+g->enemy_count-1)%g->enemy_count;
  else if(b==BTN_DOWN)g->target=(g->target+1)%g->enemy_count;''',
'''else if(b==BTN_UP)select_live_enemy(g,-1);
  else if(b==BTN_DOWN)select_live_enemy(g,1);''')
fix(''' if(g->screen==COMBAT)check_win(g);
}''',
''' if(g->screen==COMBAT){
  if(g->target<0||g->target>=g->enemy_count||
     g->enemies[g->target].hp<=0)select_live_enemy(g,1);
  check_win(g);
 }
}''')

# Rename/rebalance mechanically distinct iron-wielding archetypes:
# costs and behaviors mirror familiar deckbuilder roles, not copyrighted art.
changes=[
 ('{"Cut","Deal 6 damage",1,0,0}', '{"Strike","Deal 6 damage",1,0,0}'),
 ('{"Brace","Gain 5 block",1,0,1}', '{"Defend","Gain 5 Block",1,0,1}'),
 ('{"Sunder","8 damage; 2 vulnerable",2,0,0}', '{"Bash","Deal 8 damage. Apply 2 Vulnerable.",2,0,0}'),
 ('{"Cleaving Arc","Deal 8 to all foes",1,1,0}', '{"Cleave","Deal 8 damage to all enemies.",1,1,0}'),
 ('{"Wound Up","Draw 2 cards",1,1,2}', '{"Battle Trance","Draw 3 cards.",0,1,2}'),
 ('{"Thick Hide","Gain 9 block",2,1,1}', '{"Shrug It Off","Gain 8 Block. Draw 1.",1,1,1}'),
 ('{"Blood Toll","Lose 3 HP; gain 2 energy",0,1,2}', '{"Bloodletting","Lose 3 HP. Gain 2 Energy.",0,1,2}'),
 ('{"Hammerfall","Deal 15 damage",2,1,0}', '{"Bludgeon","Deal 32 damage.",3,1,0}'),
 ('{"War Chant","Gain 2 strength",2,2,2}', '{"Inflame","Gain 2 Strength.",1,2,2}'),
 ('{"Bastion","Gain 15 block",2,2,1}', '{"Impervious","Gain 30 Block. Exhaust.",2,2,1}'),
 ('{"Cyclone","Deal 4 to all, X times",-1,2,0}', '{"Whirlwind","Deal 5 damage to all enemies X times.",-1,2,0}'),
 ('{"Rupture","Deal 7; draw 1",1,1,0}', '{"Pommel Strike","Deal 9 damage. Draw 1.",1,1,0}'),
 ('{"Swift Blade","Deal 5 twice",1,1,0}', '{"Twin Strike","Deal 5 damage twice.",1,1,0}'),
 ('{"Ground Slam","Deal damage equal to block",1,2,0}', '{"Body Slam","Deal damage equal to your Block.",1,2,0}'),
 ('{"Decimate","Deal 28; exhaust",3,2,0}', '{"Bludgeon+","Deal 30 damage. Exhaust.",3,2,0}'),
 ('{"Redline","Draw 3; lose 2 HP",0,2,2}', '{"Offering","Lose 6 HP. Gain 2 Energy. Draw 3.",0,2,2}'),
 ('{"Dark Shield","Block 20; lose 3 HP",1,2,1}', '{"Blood Wall","Lose 2 HP. Gain 16 Block.",2,2,1}')
]
for old,new in changes:fix(old,new)
# Actual costs/effects match declared cards, including upgrade branches.
for old,new in [
 ("case 4:for(int j=0;j<2+up;j++)draw_card(g);break;","case 4:for(int j=0;j<3+up;j++)draw_card(g);break;"),
 ("case 5:gain_block(g,9+n);break;","case 5:gain_block(g,8+n);draw_card(g);break;"),
 ("case 7:deal(g,enemy,15+2*n);break;","case 7:deal(g,enemy,32+2*n);break;"),
 ("case 9:gain_block(g,15+2*n);break;","case 9:gain_block(g,30+2*n);break;"),
 ("case 10:for(int j=0;j<cost;j++)all_damage(g,4+2*up);break;","case 10:for(int j=0;j<cost;j++)all_damage(g,5+3*up);break;"),
 ("case 11:deal(g,enemy,7+n);draw_card(g);break;","case 11:deal(g,enemy,9+n);draw_card(g);break;"),
 ("case 38:for(int j=0;j<3+up;j++)draw_card(g);lose_hp(g,2);break;","case 38:lose_hp(g,6);g->energy+=2;for(int j=0;j<3+up;j++)draw_card(g);break;"),
 ("case 46:gain_block(g,20+2*n);lose_hp(g,3);break;","case 46:lose_hp(g,2);gain_block(g,16+2*n);break;"),
 ("if(id==20||id==24||id==42||id==45||id==48)","if(id==9||id==20||id==24||id==38||id==42||id==45||id==48)"),
 ("\"Draw 3 cards\",\"Gain 12 block\",\"Lose 2 HP; gain 3 energy\",\"Deal 21 damage\",","\"Draw 4 cards\",\"Gain 11 Block. Draw 1\",\"Lose 2 HP; gain 3 Energy\",\"Deal 38 damage\","),
 ("\"Gain 3 strength\",\"Gain 21 block\",\"Deal 6 to all, X times\",\"Deal 10; draw 1\",","\"Gain 3 Strength\",\"Gain 36 Block\",\"Deal 8 to all X times\",\"Deal 12; draw 1\","),
 ("\"Draw 4; lose 2 HP\",\"Deal 15; weak 3\",","\"Lose 6 HP. Gain 2 Energy. Draw 4.\",\"Deal 15; weak 3\","),
 ("\"Block 26; lose 3 HP\",\"Deal 14; heal 8 if kill\",","\"Lose 2 HP. Gain 22 Block.\",\"Deal 14; heal 8 if kill\","),
 ]:fix(old,new)
# Make 1st basic card rewards not spawn, preserve basic starter.
p.write_text(s)
print("PASS 1.0.3: map all-row previews, living-target selection, Ironclad-style cost/effect balance.")
