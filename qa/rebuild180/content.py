"""Fresh 1.8 content. Authored from 1.71-r2; no earlier 1.8 assets/code."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parent
# name, cost, type, rule, upgraded rule, C effect, art subject
WARDEN=[
('Ash Oath',1,0,'Deal 8 damage. Gain 3 Block.','Deal 11 damage. Gain 5 Block.','deal(g,e,8+3*u);card_block(g,3+2*u);','a black iron gauntlet gripping an ember-filled oath chain'),
('Rivet Guard',1,1,'Gain 7 Block. If the foe is Vulnerable, gain 4 more.','Gain 10 Block. If the foe is Vulnerable, gain 4 more.','card_block(g,7+3*u+(g->enemies[e].vulnerable?4:0));','a battered tower shield with glowing red rivets'),
('Chainbreaker',1,0,'Deal 7 damage. Apply 1 Vulnerable.','Deal 10 damage. Apply 2 Vulnerable.','deal(g,e,7+3*u);apply_vulnerable(g,&g->enemies[e],1+u);','a massive sword severing a prison chain amid sparks'),
('Blood Tithe',0,1,'Lose 2 HP. Gain 1 Strength. Draw 1.','Lose 1 HP. Gain 1 Strength. Draw 1.','lose_hp(g,2-u);if(g->hp>0){g->strength++;draw_card(g);}','a drop of blood burning on a black iron altar'),
('Furnace Breath',1,0,'Deal 6 to ALL. If you lost HP this turn, deal 4 more.','Deal 9 to ALL. If you lost HP this turn, deal 4 more.','all_damage(g,6+3*u+(g->ic[IC_HPLOSS_TURN]?4:0));','red furnace fire spilling through a knight helmet'),
('Grave Weight',1,0,'Deal 10 damage. Gain 2 Block per exhausted card, up to 10.','Deal 14 damage. Gain 2 Block per exhausted card, up to 10.','deal(g,e,10+4*u);card_block(g,MIN(10,2*g->exhaust_count));','a stone funeral bell chained to a greatsword'),
('Iron Testament',2,1,'Gain 14 Block and 1 Strength.','Gain 18 Block and 2 Strength.','card_block(g,14+4*u);g->strength+=1+u;','a scorched steel book sheltering a red coal'),
('Scalding Reprisal',1,0,'Deal 9 damage, plus twice the HP lost this turn.','Deal 13 damage, plus twice the HP lost this turn.','deal(g,e,9+4*u+2*MIN(100,g->ic[IC_HPLOSS_TURN]));','a wounded armored knight delivering a fiery counterstroke'),
('Grave Toll',1,1,'Exhaust the rightmost card in hand. Draw 2.','Exhaust the rightmost card in hand. Draw 3.','if(g->hand_count)exhaust_card(g,g->hand[--g->hand_count]);for(int k=0;k<2+u;k++)draw_card(g);','a rusted bell consuming a burning parchment'),
('Ruin March',2,0,'Deal 12 damage twice.','Deal 16 damage twice.','for(int k=0;k<2;k++)deal(g,e,12+4*u);','two immense sword arcs cutting through a ruined gate'),
('Red Bastion',1,1,'Lose 2 HP. Gain 18 Block.','Lose 2 HP. Gain 24 Block.','lose_hp(g,2);if(g->hp>0)card_block(g,18+6*u);','a crimson-caped knight braced behind a red stone bulwark'),
('Oath Reforged',1,1,'Heal 4 HP. Draw 1. Exhaust.','Heal 7 HP. Draw 1. Exhaust.','heal(g,4+3*u);draw_card(g);','a broken iron oath ring being welded in an ember forge'),
('Prison Sun',2,0,'Lose 3 HP. Deal 24 to ALL.','Lose 3 HP. Deal 31 to ALL.','lose_hp(g,3);if(g->hp>0)all_damage(g,24+7*u);','a miniature red sun behind a fractured prison gate'),
('Sovereign Anvil',2,1,'Gain 22 Block and 2 Strength. Exhaust.','Gain 28 Block and 3 Strength. Exhaust.','card_block(g,22+6*u);g->strength+=2+u;','a crown crushed on a monumental black anvil'),
('Ninefold Chain',2,0,'Deal 4 damage five times.','Deal 5 damage six times.','for(int k=0;k<5+u;k++)deal(g,e,4+u);','a knot of glowing iron chains lashing from a gauntlet'),
('Last Ward',1,1,'Gain 10 Block plus missing HP, up to 40 total. Exhaust.','Gain 15 Block plus missing HP, up to 50 total. Exhaust.','card_block(g,MIN(40+10*u,10+5*u+g->maxhp-g->hp));','a final shield of embers protecting a kneeling knight'),
('Crown Breaker',2,0,'Deal 27 damage. Apply 2 Vulnerable.','Deal 35 damage. Apply 3 Vulnerable.','deal(g,e,27+8*u);apply_vulnerable(g,&g->enemies[e],2+u);','a giant sword striking through a crowned statue'),
('Ashen Inheritance',1,1,'Exhaust up to 2 rightmost cards. Gain 2 Strength for each.','Exhaust up to 2 rightmost cards. Gain 3 Strength for each.','for(int k=0;k<2&&g->hand_count;k++){exhaust_card(g,g->hand[--g->hand_count]);g->strength+=2+u;}','two burnt banners yielding a glowing iron sigil'),
('Unmake Heaven',3,0,'Lose 6 HP. Deal 45 to ALL. Exhaust.','Lose 4 HP. Deal 58 to ALL. Exhaust.','lose_hp(g,6-2*u);if(g->hp>0)all_damage(g,45+13*u);','a charcoal knight cleaving the foundation of a golden heaven'),
('The First Ember',2,1,'Gain 5 Strength. Heal 8 HP. Exhaust.','Gain 7 Strength. Heal 12 HP. Exhaust.','g->strength+=5+2*u;heal(g,8+4*u);','the first living ember held inside an ancient armored hand')]
VESPER=[
('Ivory Nick',1,0,'Deal 6 damage. Apply 3 Poison.','Deal 9 damage. Apply 4 Poison.','deal(g,e,6+3*u);ninja_poison(g,e,3+u);','an ivory-handled knife with a green venom bead'),
('Choir Veil',1,1,'Gain 7 Block. Add a Shiv to hand.','Gain 10 Block. Add a Shiv to hand.','card_block(g,7+3*u);add_generated(g,N_SHIV);','a white hood and narrow dagger beneath a silent choir arch'),
('Quiet Step',0,1,'Gain 3 Block. Draw 1.','Gain 6 Block. Draw 1.','card_block(g,3+3*u);draw_card(g);','white-cloaked feet crossing a moonlit golden balustrade'),
('Hushed Verdict',1,0,'Deal 8 damage. Apply 1 Weak.','Deal 11 damage. Apply 2 Weak.','deal(g,e,8+3*u);g->enemies[e].weak+=1+u;','a curved dagger cutting a gilded judgment seal'),
('Needle Rain',1,1,'Add 2 Shivs to hand. Gain 3 Block.','Add 3 Shivs to hand. Gain 3 Block.','for(int k=0;k<2+u;k++)add_generated(g,N_SHIV);card_block(g,3);','small silver knives falling through pale cathedral light'),
('Bitter Hymn',1,1,'Apply 4 Poison to ALL.','Apply 6 Poison to ALL.','ninja_poison_all(g,4+2*u);','a poisoned incense censer beneath a white choir canopy'),
('Choirless',1,0,'Deal 11 damage. Draw 1 for each poisoned foe.','Deal 16 damage. Draw 1 for each poisoned foe.','deal(g,e,11+5*u);for(int k=0;k<g->enemy_count;k++)if(g->enemies[k].poison)draw_card(g);','a hooded assassin in a deserted immense cathedral choir'),
('Gilded Venom',1,1,'Apply 7 Poison. Gain 6 Block.','Apply 10 Poison. Gain 9 Block.','ninja_poison(g,e,7+3*u);card_block(g,6+3*u);','green poison flowing over gold filigree on a knife'),
('Saints Silence',1,1,'Apply 2 Weak to ALL. Draw 1.','Apply 3 Weak to ALL. Draw 2.','for(int k=0;k<g->enemy_count;k++)g->enemies[k].weak+=2+u;for(int k=0;k<1+u;k++)draw_card(g);','a veiled saint statue with its golden mouth sealed'),
('White Wake',1,0,'Deal 5 damage three times.','Deal 7 damage three times.','for(int k=0;k<3;k++)deal(g,e,5+2*u);','three knife arcs trailing a white cloak in darkness'),
('Threadcutter',1,1,'Discard the rightmost card. Gain 2 Energy. Exhaust.','Discard the rightmost card. Gain 3 Energy. Exhaust.','if(g->hand_count){int discarded=g->hand[--g->hand_count];ninja_discard_effects(g,discarded,preview_depth);}gain_energy(g,2+u);','a silver knife severing a taut golden fate thread'),
('Merciless Mercy',1,0,'Deal 12 damage. If the foe is poisoned, heal 3.','Deal 17 damage. If the foe is poisoned, heal 5.','{int p=g->enemies[e].poison;deal(g,e,12+5*u);if(p)heal(g,3+2*u);}','a white-cloaked assassin offering a venomous blade like a blessing'),
('False Benediction',2,1,'Double the foe\'s Poison. Gain 10 Block. Exhaust.','Double the foe\'s Poison. Gain 16 Block. Exhaust.','ninja_poison(g,e,g->enemies[e].poison);card_block(g,10+6*u);','a poisonous emerald halo behind a cracked ivory mask'),
('Choir of Knives',2,1,'Add 4 upgraded Shivs to hand.','Add 5 upgraded Shivs to hand.','for(int k=0;k<4+u;k++)add_generated(g,N_SHIV|UPBIT);','four ornate silver daggers hovering like a cathedral choir'),
('Pale Intercession',1,1,'Gain 22 Block. Clear Weak and Vulnerable. Exhaust.','Gain 30 Block. Clear Weak and Vulnerable. Exhaust.','card_block(g,22+8*u);g->weak=g->vulnerable=0;','a white cloak turning aside black arrows in holy light'),
('Last Confession',2,0,'Deal 18 damage plus the foe\'s Poison.','Deal 25 damage plus the foe\'s Poison.','deal(g,e,18+7*u+g->enemies[e].poison);','a hooded assassin confronting a shadow behind confession lattice'),
('Poisoned Crown',2,1,'Apply 15 Poison to ALL. Exhaust.','Apply 21 Poison to ALL. Exhaust.','ninja_poison_all(g,15+6*u);','emerald venom dripping from an abandoned gold crown'),
('Unwritten Prayer',0,1,'Draw 3. Gain 1 Dexterity. Exhaust.','Draw 4. Gain 2 Dexterity. Exhaust.','for(int k=0;k<3+u;k++)draw_card(g);g->dexterity+=1+u;','a blank prayer ribbon winding around a white-gloved hand'),
('Heaven Has Teeth',3,0,'Deal 8 damage five times. Apply 10 Poison. Exhaust.','Deal 11 damage five times. Apply 14 Poison. Exhaust.','for(int k=0;k<5;k++)deal(g,e,8+3*u);ninja_poison(g,e,10+4*u);','a white assassin beneath a monstrous cathedral arch shaped like fangs'),
('The Unheard Saint',2,1,'Gain 30 Block and 3 Dexterity. Add 2 Shivs. Exhaust.','Gain 40 Block and 4 Dexterity. Add 2 Shivs. Exhaust.','card_block(g,30+10*u);g->dexterity+=3+u;for(int k=0;k<2;k++)add_generated(g,N_SHIV);','a faceless ivory saint holding two gleaming silver knives')]
PROXY=[
('Borrowed Dawn',1,0,'Deal 7 damage. Gain 2 Stars.','Deal 10 damage. Gain 3 Stars.','deal(g,e,7+3*u);regent_gain_stars(g,2+u);','a bronze crown cupping a tiny captured dawn'),
('Brass Orbit',1,1,'Gain 7 Block and 2 Stars.','Gain 10 Block and 3 Stars.','card_block(g,7+3*u);regent_gain_stars(g,2+u);','a bronze astrolabe ring sheltering two golden stars'),
('Throne Shard',1,0,'Deal 9 damage. Forge 3.','Deal 12 damage. Forge 5.','deal(g,e,9+3*u);regent_forge(g,3+2*u);','a broken blue throne shard being forged into a gold blade'),
('Starlit Writ',0,1,'Gain 2 Stars. Draw 1.','Gain 3 Stars. Draw 1.','regent_gain_stars(g,2+u);draw_card(g);','gold star script appearing on a blue royal decree'),
('Exile\'s Mantle',1,1,'Gain 6 Block, plus 1 per Star, up to 12 extra.','Gain 10 Block, plus 1 per Star, up to 12 extra.','card_block(g,6+4*u+MIN(12,g->regent.stars));','a blue royal mantle lined with tiny golden stars'),
('Fallen Meridian',1,0,'Deal 5 to ALL. Gain 1 Star per living foe.','Deal 8 to ALL. Gain 1 Star per living foe.','{int n=0;for(int k=0;k<g->enemy_count;k++)n+=g->enemies[k].hp>0;all_damage(g,5+3*u);regent_gain_stars(g,n);}','a gold celestial meridian crashing through a blue observatory'),
('Crown Kiln',1,1,'Forge 8. Gain 4 Block.','Forge 12. Gain 6 Block.','regent_forge(g,8+4*u);card_block(g,4+2*u);','a bronze crown glowing inside a blue celestial forge'),
('Astral Levy',1,0,'Deal 12 damage. Gain 4 Stars.','Deal 17 damage. Gain 5 Stars.','deal(g,e,12+5*u);regent_gain_stars(g,4+u);','a wizard king summoning a tax of golden constellations'),
('Empty Court',1,1,'Gain 12 Block. Draw 2.','Gain 17 Block. Draw 2.','card_block(g,12+5*u);draw_card(g);draw_card(g);','a drowned empty royal court illuminated by starlight'),
('Royal Cinder',0,1,'Spend up to 3 Stars. Forge 3 for each spent.','Spend up to 3 Stars. Forge 4 for each spent.','{int n=MIN(3,g->regent.stars);g->regent.stars-=n;regent_forge(g,n*(3+u));}','three star embers falling into a sovereign sword mold'),
('Blue Mandate',1,0,'Deal 8 damage twice. Gain 2 Stars.','Deal 11 damage twice. Gain 3 Stars.','for(int k=0;k<2;k++)deal(g,e,8+3*u);regent_gain_stars(g,2+u);','a blue-robed king making twin golden sword strokes'),
('Broken Zodiac',1,1,'Gain 6 Stars. Apply 1 Weak to ALL.','Gain 8 Stars. Apply 2 Weak to ALL.','regent_gain_stars(g,6+2*u);for(int k=0;k<g->enemy_count;k++)g->enemies[k].weak+=1+u;','a shattered zodiac wheel releasing bright gold fragments'),
('Sunless Coronation',2,1,'Gain 10 Stars and 16 Block. Exhaust.','Gain 14 Stars and 22 Block. Exhaust.','regent_gain_stars(g,10+4*u);card_block(g,16+6*u);','a bronze crown beneath an eclipsed sun over blue mist'),
('Sovereign Rupture',2,0,'Deal 18 damage plus Forge, up to 30 extra.','Deal 25 damage plus Forge, up to 40 extra.','deal(g,e,18+7*u+MIN(30+10*u,g->regent.forge));','a vast gold sovereign blade shattering an astronomical ring'),
('The Last Decree',1,1,'Spend up to 5 Stars. Draw 1 for each spent. Exhaust.','Spend up to 5 Stars. Draw 1 each. Gain 1 Energy. Exhaust.','{int n=MIN(5,g->regent.stars);g->regent.stars-=n;for(int k=0;k<n;k++)draw_card(g);if(u)gain_energy(g,1);}','a final royal scroll burning into five gold stars'),
('Crownless Comet',2,0,'Deal 22 to ALL. Forge 6.','Deal 30 to ALL. Forge 9.','all_damage(g,22+8*u);regent_forge(g,6+3*u);','a crown-shaped gold comet ripping through a blue void'),
('Kingdom of One',1,1,'Gain 18 Block. If only one foe lives, gain 3 Strength.','Gain 25 Block. If only one foe lives, gain 4 Strength.','card_block(g,18+7*u);{int n=0;for(int k=0;k<g->enemy_count;k++)n+=g->enemies[k].hp>0;if(n==1)g->strength+=3+u;}','a solitary blue king on a narrow island beneath a vast empty sky'),
('Starless Forge',1,1,'Forge 16. If you have no Stars, gain 6. Exhaust.','Forge 23. If you have no Stars, gain 8. Exhaust.','regent_forge(g,16+7*u);if(!g->regent.stars)regent_gain_stars(g,6+2*u);','a cold blue forge igniting with the first golden spark'),
('Uncrown the Sky',3,0,'Spend up to 10 Stars. Deal 25 to ALL plus 3 per Star spent. Exhaust.','Spend up to 10 Stars. Deal 35 to ALL plus 4 per Star spent. Exhaust.','{int n=MIN(10,g->regent.stars);g->regent.stars-=n;all_damage(g,25+10*u+n*(3+u));}','a blue wizard king slicing the crown from the heavens with a golden sword'),
('The Unborrowed Sun',2,1,'Gain 15 Stars. Forge 20. Heal 8. Exhaust.','Gain 20 Stars. Forge 28. Heal 12. Exhaust.','regent_gain_stars(g,15+5*u);regent_forge(g,20+8*u);heal(g,8+4*u);','a newborn gold sun resting above the hand of an uncrowned blue king')]
POTIONS=[
('Cinderblood','Heal 12 HP and gain 1 Strength.','heal(g,12+bonus);g->strength++;','ruby blood in a scorched iron vial'),
('Rimeglass','Gain 28 Block. Apply 1 Weak to ALL.','relic_block(g,28+bonus);for(int k=0;k<g->enemy_count;k++)g->enemies[k].weak++;','frosted blue glass flask'),
('Widowlight','Apply 9 Poison to ALL.','ninja_poison_all(g,9+bonus);','emerald poison in a webbed silver vial'),
('Star Nectar','Gain 7 Stars and 1 Energy.','regent_gain_stars(g,7+bonus);gain_energy(g,1);','golden starlight in a bronze crystal vial'),
('Tempered Rain','Forge 12. Gain 10 Block.','regent_forge(g,12+bonus);relic_block(g,10);','liquid steel in a crowned blue bottle'),
('Mercy Ash','Heal 24 HP. Can be used outside combat.','heal(g,24+bonus);','warm ash glowing in a white ceramic flask'),
('Glass Thorn','Deal 32 damage to the selected foe.','damage_enemy(g,g->target,32+bonus,1);','red thorn inside clear black-rimmed glass'),
('Black Incense','Clear Weak, Vulnerable and Poison. Gain 8 Block.','g->weak=g->vulnerable=g->poison=0;relic_block(g,8+bonus);','black smoke inside an ivory censer bottle'),
('Second Breath','Draw 4 cards. Gain 4 Block.','for(int k=0;k<4;k++)draw_card(g);relic_block(g,4+bonus);','pale wind spiraling in a silver flask'),
('Rift Salt','Gain 3 Energy. Lose 4 HP.','lose_hp(g,4);if(g->hp>0)gain_energy(g,3+(bonus>0));','violet salt crystals in a jagged bottle'),
('Choir Oil','Add 4 upgraded Shivs to hand.','for(int k=0;k<4;k++)add_generated(g,N_SHIV|UPBIT);','ivory oil bottle with four tiny blade ornaments'),
('Oath Resin','Gain 3 Strength and 1 Dexterity.','g->strength+=3+(bonus>0);g->dexterity++;','amber resin holding an iron oath seal'),
('Gravewater','Apply 3 Weak and 3 Vulnerable to ALL.','for(int k=0;k<g->enemy_count;k++){g->enemies[k].weak+=3;apply_vulnerable(g,&g->enemies[k],3);}','grave mist in a dark blue stone vial'),
('Goldroot','Gain 40 Gold. Can be used outside combat.','g->gold+=40+bonus;','gold roots curling through green glass'),
('Ember Crown','Deal 14 to ALL. Gain 14 Block.','all_damage(g,14+bonus);relic_block(g,14+bonus);','a crown-shaped flask holding red embers'),
('Moon Marrow','Gain 4 maximum HP and heal 10. Use anywhere.','g->maxhp+=4;heal(g,10+bonus);','white lunar marrow inside a bone flask'),
('Deep Recollection','Draw 2. Gain 2 Energy.','draw_card(g);draw_card(g);gain_energy(g,2);','blue memory motes in an old crystal bottle'),
('Severed Dawn','Remove all enemy Block. Deal 10 to ALL.','for(int k=0;k<g->enemy_count;k++)g->enemies[k].block=0;all_damage(g,10+bonus);','severed sunrise glowing through cracked amber glass'),
('Pale Aegis','Gain 36 Block. Lose 1 Strength (minimum zero).','relic_block(g,36+bonus);g->strength=MAX(0,g->strength-1);','white enamel shield-shaped potion bottle'),
('Heaven\'s Bile','Double the selected foe\'s Poison. Apply 2 Vulnerable.','ninja_poison(g,g->target,g->enemies[g->target].poison);apply_vulnerable(g,&g->enemies[g->target],2);','green-gold bile in a blasphemous halo bottle')]
RELICS=[
('Gatekeeper\'s Coal','Start combat with 6 Block.','a living ember locked in an iron cage'),
('Widow\'s Bell','At combat start, apply 2 Poison to ALL foes.','a tarnished silver bell with green venom in its mouth'),
('Orphan Star','Start combat with 3 additional Stars.','a tiny golden star chained to a bronze medallion'),
('Ash Tally','After combat, gain 8 extra Gold.','a charred bone tally stick bound in red cloth'),
('White Reliquary','After combat, heal 3 HP.','an ivory reliquary with a warm gold heart'),
('Drowned Lens','Draw 1 extra card on the first turn.','a fractured blue observatory lens'),
('Judgment Nail','Start combat with 1 Strength.','an immense iron nail engraved with a broken crown'),
('Unworn Halo','Start combat with 1 Dexterity.','a damaged gold halo lying on white cloth'),
('Furnace Heart','On pickup, gain 10 maximum HP and heal 10.','a red furnace core in an anatomical iron heart'),
('Last Astrolabe','Every third turn, gain 1 Energy.','a small shattered bronze astrolabe with a surviving star')]
def write():
 rows=[]
 for c,group in enumerate([WARDEN,VESPER,PROXY]):
  assert len(group)==20
  for n,(name,cost,t,desc,up,effect,subject) in enumerate(group):
   rows.append(dict(id=334+c*20+n,character=c,index=n,name=name,cost=cost,type=t,rarity=0 if n<6 else 1 if n<12 else 2 if n<18 else 3,description=desc,upgrade=up,effect=effect,exhaust='Exhaust.' in desc,subject=subject))
 out={'cards':rows,'potions':[dict(id=i+6,name=n,description=d,effect=e,subject=s) for i,(n,d,e,s) in enumerate(POTIONS)],'relics':[dict(id=i+64,name=n,description=d,subject=s) for i,(n,d,s) in enumerate(RELICS)]}
 (ROOT/'content.json').write_text(json.dumps(out,indent=2)+'\n')
 src=ROOT/'source'
 (src/'new_cards.inc').write_text(''.join('{%s,%s,%d,%d,%d},\n'%(json.dumps(r['name']),json.dumps(r['description']),r['cost'],r['rarity'],r['type']) for r in rows))
 (src/'new_upgrades.inc').write_text(''.join(json.dumps(r['upgrade'])+',\n' for r in rows))
 (src/'new_effects.inc').write_text(''.join('case %d: {%s} break;\n'%(r['id'],r['effect'].replace('u','u')) for r in rows))
 (src/'new_relics.inc').write_text(''.join('{%s,%s},\n'%(json.dumps(n),json.dumps(d)) for n,d,s in RELICS))
 (src/'new_potions.h').write_text('static const char*new_potion_names[20]={'+','.join(json.dumps(p[0]) for p in POTIONS)+'};\nstatic const char*new_potion_rules[20]={'+','.join(json.dumps(p[1]) for p in POTIONS)+'};\n')
 (src/'new_potion_effects.inc').write_text(''.join('case %d: {%s} break;\n'%(i+6,e) for i,(n,d,e,s) in enumerate(POTIONS)))
 (src/'new_card_flags.h').write_text('static const unsigned char new_exhaust[60]={'+','.join(str(int(r['exhaust'])) for r in rows)+'};\nstatic const unsigned char new_block[60]={'+','.join(str(int('Block' in r['description'])) for r in rows)+'};\n')
 print('Fresh content:',len(rows),'cards,',len(POTIONS),'potions,',len(RELICS),'relics')
if __name__=='__main__':write()
