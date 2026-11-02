from pathlib import Path
import json
R=Path(__file__).resolve().parents[2];S=R/'qa/v186/source'
# Trigger, magnitude, companion magnitude. Each relic has a compound contract.
recipes=[
('START',7,2),('TURN2',6,1),('SWITCH',4,1),('RELEASE',5,1),('FOURTH',4,1),('TURN3',6,1),
('WIN',3,9),('POTION',7,2),('HURT',8,2),('POWER',6,1),('EMPTY',6,1),('BANK',5,1),
('SKIP',13,1),('REST',5,11),('SHOP',17,1),('WITHER',3,5),('TURN4',7,1),('PICKHP',9,23),
('FIRST',5,1),('THIRD_SKILL',5,2),('SIXTH',8,2),('RELEASE3',7,1),('LOWTURN',6,1),('TWOFOES',7,1)]
prefix='Kiln Saltglass Meridian Censer Basalt Tallow Solstice Rime Cairn Opal Scoria Ivory'.split()
nouns=['Token','Vessel','Clasp','Spindle','Gauge','Seal','Lens','Buckle']
rows=[]
for i in range(94):
 k,a,b=recipes[(i*7)%len(recipes)];a+=i//len(recipes)
 name=prefix[i%12]+' '+nouns[i//12]
 if i==0:name,k,a,b='Hearth Remnant','WARDEN',3,2
 if i==44:name,k,a,b='Choir Keepsake','VESPER',3,1
 if i==63:name,k,a,b='Unclaimed Diadem','PROXY',2,1
 if i==11:name,k,a,b='Volatile Charter','ENERGY',2,7
 if i==25:name,k,a,b='Dawn Obligation','ENERGY',3,11
 if i==23:name,k,a,b='Unspent Farewell','REVIVE',17,2
 if i==30:name,k,a,b='Passage Impression','TURN2',6,1 # unoffered compatibility slot, still distinct name
 if i==37:name,k,a,b='Lasting Resolve','PICKFORCE',1,6
 if i==38:name,k,a,b='Lasting Shelter','PICKHP',7,29
 if i==39:name,k,a,b='Lasting Passage','PICKGOLD',113,4
 desc={
 'START':f'Combat start: gain {a} Guard and {b} Echo.',
 'TURN2':f'Turn 2: gain {a} Guard and draw {b}.',
 'SWITCH':f'First Attack/Skill switch each turn: {a} Guard and {b} Echo.',
 'RELEASE':f'First Echo release each turn: {a} Guard and {b} Echo after the card.',
 'FOURTH':f'Fourth card each turn: {a} Guard and {b} Echo.',
 'TURN3':f'Turn 3: gain {a} Guard and {b} Energy.',
 'WIN':f'After victory: recover {a} HP and gain {b} Gold.',
 'POTION':f'Using a potion in combat: gain {a} Guard and {b} Echo.',
 'HURT':f'First enemy hit that costs HP each combat: {a} Guard and {b} Echo.',
 'POWER':f'First Power each combat: {a} Guard and draw {b}.',
 'EMPTY':f'Turn end with no Echo: gain {a} Guard and {b} Echo.',
 'BANK':f'Turn end with unused Energy: {a} Guard and {b} Wither to all foes.',
 'SKIP':f'Skip a card reward: gain {a} Gold and recover {b} HP.',
 'REST':f'Resting recovers {a} extra HP and grants {b} Gold.',
 'SHOP':f'Shop prices fall by {a} Gold. Combat start: {b} Echo.',
 'WITHER':f'Combat start: apply {a} Wither to all foes; gain {b} Guard.',
 'TURN4':f'Turn 4: gain {a} Guard and {b} Force.',
 'PICKHP':f'On pickup: gain {a} max HP, recover {a} HP, gain {b} Gold.',
 'FIRST':f'First Attack each combat: {a} Guard and {b} Echo.',
 'THIRD_SKILL':f'Third Skill each turn: {a} Guard and {b} Wither to all foes.',
 'SIXTH':f'Sixth card each combat: {a} Guard and {b} Echo.',
 'RELEASE3':f'First release of 3+ Echo each combat: {a} Guard and {b} Energy.',
 'LOWTURN':f'Turn start below half HP: {a} Guard and {b} Echo. Once per combat.',
 'TWOFOES':f'Combat start with 2+ foes: {a} Guard and {b} Force.',
 'WARDEN':f'Combat start: {b} Echo. After victory: recover {a} HP.',
 'VESPER':f'Combat start: draw {b}. First switch each turn: {a} Guard.',
 'PROXY':f'Combat start: {a} Echo. Turn 3: draw {b}.',
 'ENERGY':f'On pickup: +1 Energy per turn. Combat start: {a} Falter and {b} Guard.',
 'REVIVE':f'Once per act, lethal damage restores {a} HP and grants {b} Echo.',
 'PICKFORCE':f'On pickup: gain {a} permanent Force and recover {b} HP. Repeatable gift.',
 'PICKGOLD':f'On pickup: gain {a} Gold and recover {b} HP. Repeatable gift.',
 }[k]
 rows.append(dict(id=i,name=name,trigger=k,a=a,b=b,description=desc))
q=json.dumps
(R/'distribution/relics.json').write_text(q(rows,indent=2)+'\n')
keys=sorted(set(r['trigger'] for r in rows))
s='/* Generated original relic contracts. */\nenum {'+', '.join('DR_'+x for x in keys)+'};\n'
s+='typedef struct {short trigger,a,b;} DistRelic;\nconst RelicDef relic_defs[]={\n'+''.join(' {'+q(r['name'])+','+q(r['description'])+'},\n' for r in rows)+'};\nconst int RELIC_COUNT=94;\n'
s+='static const DistRelic dist_relics[]={\n'+''.join(' {DR_'+r['trigger']+f',{r["a"]},{r["b"]}'+'},\n' for r in rows)+'};\n'
(S/'distribution_relics.inc').write_text(s)
print('Generated 94 relic contracts.')
