from pathlib import Path
import json,re
R=Path(__file__).resolve().parents[2];S=R/'qa/v186/source'
rows=[
 ('Hearth Sip','Recover 15 HP. Gain 7 Gold.','heal(g,15);g->gold+=7;'),
 ('Rivet Tonic','Gain 1 Force and 8 Guard.','g->strength++;relic_block(g,8);'),
 ('Slate Draught','Gain 17 Guard and 2 Echo.','relic_block(g,17);regent_gain_stars(g,2);'),
 ('Censer Spark','Hit all foes for 13. Apply 2 Wither to all.','all_damage(g,13);ninja_poison_all(g,2);'),
 ('Lantern Cordial','Gain 1 Energy and 3 Echo.','gain_energy(g,1);regent_gain_stars(g,3);'),
 ('Murmur Ink','Draw 2. Gain 6 Guard.','draw_card(g);draw_card(g);relic_block(g,6);'),
 ('Kiln Nectar','Recover 9 HP. Gain 2 Echo.','heal(g,9);regent_gain_stars(g,2);'),
 ('Rime Decanter','Gain 19 Guard. Apply 2 Falter to the target.','relic_block(g,19);g->enemies[g->target].weak+=2;'),
 ('Silt Essence','Apply 6 Wither to all. Gain 4 Guard.','ninja_poison_all(g,6);relic_block(g,4);'),
 ('Meridian Milk','Gain 5 Echo. Draw 1.','regent_gain_stars(g,5);draw_card(g);'),
 ('Brass Infusion','Gain 3 Echo and 13 Guard.','regent_gain_stars(g,3);relic_block(g,13);'),
 ('Pilgrim Dew','Recover 21 HP. Gain 5 Gold. Use anywhere.','heal(g,21);g->gold+=5;'),
 ('Flue Extract','Hit the target for 27. Gain 1 Echo.','damage_enemy(g,g->target,27,0);regent_gain_stars(g,1);'),
 ('Opal Solvent','Clear Falter and Fracture. Gain 11 Guard.','g->weak=g->vulnerable=0;relic_block(g,11);'),
 ('Choir Distillate','Draw 3. Gain 2 Echo.','for(int k=0;k<3;k++)draw_card(g);regent_gain_stars(g,2);'),
 ('Scoria Elixir','Gain 2 Energy and 7 Guard. Lose 3 HP.','lose_hp(g,3);if(g->hp>0){gain_energy(g,2);relic_block(g,7);}'),
 ('Needle Phial','Create 3 Needlewakes. Gain 5 Guard.','for(int k=0;k<3;k++)add_generated(g,RIFT_CARD_165);relic_block(g,5);'),
 ('Foundry Resin','Gain 2 Force and 2 Echo.','g->strength+=2;regent_gain_stars(g,2);'),
 ('Veil Solution','Apply 2 Falter to all. Gain 9 Guard.','for(int k=0;k<g->enemy_count;k++)g->enemies[k].weak+=2;relic_block(g,9);'),
 ('Wayfarer Oil','Gain 31 Gold. Recover 4 HP. Use anywhere.','g->gold+=31;heal(g,4);'),
 ('Twilight Syrup','Hit all for 11. Gain 15 Guard.','all_damage(g,11);relic_block(g,15);'),
 ('Cairn Marrow','Gain 3 max HP. Recover 9 HP. Use anywhere.','g->maxhp+=3;heal(g,9);'),
 ('Horizon Condensate','Draw 1. Gain 1 Energy and 2 Echo.','draw_card(g);gain_energy(g,1);regent_gain_stars(g,2);'),
 ('Saltwash','Remove 12 Guard from all foes. Apply 5 Wither to all.','for(int k=0;k<g->enemy_count;k++)g->enemies[k].block=MAX(0,g->enemies[k].block-12);ninja_poison_all(g,5);'),
 ('Basalt Suspension','Gain 29 Guard. Lose 2 Echo, if held.','relic_block(g,29);g->regent.stars=MAX(0,g->regent.stars-2);'),
 ('Echo Bile','Apply 7 Wither and 1 Fracture to the target.','ninja_poison(g,g->target,7);apply_vulnerable(g,&g->enemies[g->target],1);'),
]
(R/'distribution/potions.json').write_text(json.dumps([dict(id=i,name=n,description=d) for i,(n,d,e) in enumerate(rows)],indent=2)+'\n')
q=json.dumps
(S/'new_potions.h').write_text('static const char*new_potion_names[20]={'+','.join(q(n) for n,d,e in rows[6:])+'};\nstatic const char*new_potion_rules[20]={'+','.join(q(d) for n,d,e in rows[6:])+'};\n')
(S/'new_potion_effects.inc').write_text(''.join(f'case {i}: {{{e}}} break;\n' for i,(n,d,e) in enumerate(rows)))
p=S/'game.c';s=p.read_text();a=s.index('static const char*potion_titles');b=s.index('const char *game_potion_name',a);s=s[:a]+'static const char*potion_titles[6]={'+','.join(q(n) for n,d,e in rows[:6])+'};\nstatic const char*potion_details[6]={'+','.join(q(d) for n,d,e in rows[:6])+'};\n'+s[b:]
p.write_text(s)
print('Generated 26 original potion contracts.')
