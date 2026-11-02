#!/usr/bin/env python3
"""Original distribution catalogue. Recipes are the sole source of rule text and effects.
Stable numeric slots preserve artwork indexing, not legacy card identities.
"""
from pathlib import Path
import json, re, hashlib
ROOT=Path(__file__).resolve().parents[2]
SRC=ROOT/'qa/v186/source'
OPS=['HIT','SWEEP','GUARD','WITHER','WEAK','EXPOSE','DRAW','ECHO','VENT_HIT','VENT_GUARD','VENT_WITHER','BRACE_HIT','MARK_HIT','THREAT_GUARD','LOW_GUARD','RESERVE_GUARD','TEMP_FORCE','NEXT_GUARD','NEXT_DRAW','HEAL','RECALL','FOLD','POWER','SHARD','STRIP']
POWERS=[
 ('First Attack/Skill switch each turn: gain {n} Guard and 1 Echo.', 'BRIDGE'),
 ('First Echo release each turn: gain {n} Guard.', 'WELD'),
 ('On your fourth card each turn: hit all foes for {n} and draw 1.', 'REFRAIN'),
 ('Turn start with 4+ Echo: gain {n} Guard and draw 1.', 'BEACON'),
 ('Turn end with unused Energy: gain {n} Guard.', 'RESERVE'),
 ('On turns 3 and 6: recover {n} HP.', 'MEND'),
 ('First Attack after a Skill each turn: +{n} damage per hit.', 'WHEEL'),
 ('First release of 3+ Echo each turn: gain {n} Guard; +1 Energy next turn.', 'RETURN'),
 ('Turn start with no Echo: gain 2 Echo and {n} Guard.', 'WELL'),
 ('First Attack each turn against a Withered foe: gain {n} Guard.', 'VEIL'),
 ('Turn end with 6+ Echo: apply {n} Wither to all foes.', 'CHORUS'),
 ('On your third Skill each turn: deal {n} damage to all foes.', 'BELLOWS'),
]
# op, base, upgrade increment, auxiliary (release cap, conditional bonus, or power index)
def op(k,n,u=0,p=0): return [k,n,u,p]
# New recipes combine guaranteed value, positioning, and a bounded Echo economy.
def recipe(k,t,c):
    if k==0:return 1,0,[op('HIT',7+t,3),op('ECHO',1)]
    if k==1:return 1,0,[op('BRACE_HIT',8+t,3,4),op('ECHO',1)]
    if k==2:return 1,0,[op('HIT',6+t,3),op('GUARD',3,2),op('ECHO',1)]
    if k==3:return 1,0,[op('MARK_HIT',8+t,3,5),op('WITHER',2,1)]
    if k==4:return 1,0,[op('HIT',6+t,3),op('VENT_HIT',3,1,3)]
    if k==5:return 2,0,[op('HIT',14+t,4),op('VENT_GUARD',3,1,3)]
    if k==6:return 1,0,[op('SWEEP',5+t,2),op('ECHO',2)]
    if k==7:return 2,0,[op('SWEEP',9+t,3),op('WEAK',1,0,1),op('ECHO',1)]
    if k==8:return 1,0,[op('HIT',7+t,2),op('EXPOSE',1),op('NEXT_GUARD',3,2)]
    if k==9:return 1,0,[op('HIT',4+t,2,2),op('VENT_WITHER',2,1,2)]
    if k==10:return 2,0,[op('HIT',5+t,1,3),op('ECHO',2)]
    if k==11:return 1,0,[op('STRIP',5+t,3),op('HIT',8+t,3),op('ECHO',1)]
    if k==12:return 2,0,[op('HIT',15+t,4),op('WITHER',4,2),op('NEXT_DRAW',1)]
    if k==13:return 0,0,[op('HIT',3+t,2),op('VENT_GUARD',2,1,2)]
    if k==14:return 3,0,[op('HIT',19+2*t,5),op('VENT_HIT',3,1,4),op('GUARD',3,2)]
    if k==15:return 2,0,[op('SWEEP',10+t,3),op('VENT_WITHER',1,1,3)]
    if k==16:return 1,0,[op('HIT',5+t,2),op('DRAW',1),op('NEXT_GUARD',3,2)]
    if k==17:return 1,0,[op('BRACE_HIT',7+t,3,6),op('VENT_HIT',2,1,2)]
    if k==18:return 2,0,[op('MARK_HIT',12+t,4,8),op('GUARD',5,2)]
    if k==19:return 1,0,[op('HIT',8+t,3),op('FOLD',4,2)]
    if k==20:return 1,1,[op('GUARD',7+t,3),op('ECHO',2)]
    if k==21:return 1,1,[op('THREAT_GUARD',6+t,3,4),op('ECHO',1)]
    if k==22:return 1,1,[op('GUARD',5+t,3),op('VENT_GUARD',3,1,3)]
    if k==23:return 1,1,[op('LOW_GUARD',7+t,3,5),op('NEXT_DRAW',1)]
    if k==24:return 1,1,[op('RESERVE_GUARD',5+t,3,2),op('ECHO',1)]
    if k==25:return 2,1,[op('GUARD',14+t,4),op('WITHER',3,1,1)]
    if k==26:return 1,1,[op('GUARD',5+t,2),op('WEAK',1),op('NEXT_GUARD',4,2)]
    if k==27:return 0,1,[op('GUARD',2+t,2),op('ECHO',1)]
    if k==28:return 1,1,[op('WITHER',4+t,2),op('VENT_WITHER',1,1,3)]
    if k==29:return 1,1,[op('GUARD',4+t,2),op('RECALL',1),op('ECHO',1)]
    if k==30:return 1,1,[op('GUARD',5+t,2),op('DRAW',1),op('ECHO',1)]
    if k==31:return 1,1,[op('NEXT_GUARD',9+t,3),op('GUARD',3,2),op('NEXT_DRAW',1)]
    if k==32:return 1,1,[op('TEMP_FORCE',2,1),op('GUARD',4+t,2),op('ECHO',2)]
    if k==33:return 2,1,[op('GUARD',8+t,3),op('WEAK',1,0,1),op('VENT_HIT',2,1,3)]
    if k==34:return 1,1,[op('FOLD',8+t,3),op('DRAW',1),op('ECHO',2)]
    if k==35:return 1,1,[op('WITHER',3+t,1,1),op('ECHO',2)]
    if k==36:return 2,1,[op('GUARD',10+t,3),op('HEAL',2,1),op('VENT_GUARD',2,1,3)]
    if k==37:return 1,1,[op('GUARD',4+t,2),op('SHARD',1),op('ECHO',1)]
    if k==38:return 2,0,[op('SWEEP',9+t,3),op('GUARD',6,2),op('VENT_HIT',2,1,2)]
    if k==39:return 2,1,[op('GUARD',12+t,4),op('EXPOSE',1,0,1),op('ECHO',2)]
    p=(k-40)%len(POWERS)
    strength=[3,4,4,4,5,3,3,3,3,4,2,5][p]+t
    return (2 if p in (2,3,6,10,11) else 1),2,[op('POWER',strength,1,p),op('ECHO',1+c%2)]

def description(ops,up=False):
    out=[]
    for k,b,u,p in ops:
        n=b+u*up; all='all foes' if p==1 else 'the target'
        text={
            'HIT':f'Hit for {n}'+(f', {p} times' if p>1 else '')+'.',
            'SWEEP':f'Hit all foes for {n}.',
            'GUARD':f'Gain {n} Guard.',
            'WITHER':f'Apply {n} Wither to {all}.',
            'WEAK':f'Apply {n} Falter to {all}.',
            'EXPOSE':f'Apply {n} Fracture to {all}.',
            'DRAW':f'Draw {n}.',
            'ECHO':f'Gain {n} Echo.',
            'VENT_HIT':f'Release up to {p} Echo: hit for {n} per Echo.',
            'VENT_GUARD':f'Release up to {p} Echo: gain {n} Guard per Echo.',
            'VENT_WITHER':f'Release up to {p} Echo: apply {n} Wither per Echo.',
            'BRACE_HIT':f'Hit for {n}; +{p} if you have Guard.',
            'MARK_HIT':f'Hit for {n}; +{p} if the target has Wither.',
            'THREAT_GUARD':f'Gain {n} Guard; +{p} if a foe is attacking.',
            'LOW_GUARD':f'Gain {n} Guard; +{p} below half HP.',
            'RESERVE_GUARD':f'Gain {n} Guard; +{p} per Echo (up to 4 Echo).',
            'TEMP_FORCE':f'Gain {n} Force for this turn.',
            'NEXT_GUARD':f'Gain {n} Guard next turn.',
            'NEXT_DRAW':f'Draw {n} more next turn.',
            'HEAL':f'Recover {n} HP. Exile this card.',
            'RECALL':'Return the lowest-cost discarded card to hand.',
            'FOLD':f'Put the rightmost hand card atop your draw pile; gain {n} Guard if moved.',
            'SHARD':f'Create {n} Needlewake'+('s' if n>1 else '')+'.',
            'STRIP':f'Remove {n} enemy Guard.',
            'POWER':POWERS[p][0].format(n=n) if p<len(POWERS) else '',
        }[k]
        out.append(text)
    return ' '.join(out)

FIRST=[
 'Kiln Cinder Rivet Iron Ash Furnace Soot Ember Anvil Quarry Chain Flue Charcoal Basalt Hammer Bellows Hearth Scoria Cobalt Foundry Slate Crucible'.split(),
 'Choir Hush Ivory Velvet Veil Censer Wisp Sable Murmur Saltglass Rime Whisper Chime Needle Moth Silt Tallow Requiem Plume Quiet Frost Hollow'.split(),
 'Orbit Parallax Brass Meridian Lumen Astral Horizon Zenith Prism Noonday Aperture Umbra Dawn Halo Sextant Sundial Eclipse Solstice Mosaic Opal Aether Crownless'.split(),
 'Rift Wayfarer Cairn Pilgrim Threshold Lantern Passage Shelter Witness Oathless Dreamer Wanderer'.split()]
LAST=['Tally','Reckoning','Measure','Witness','Ransom','Traverse','Shelter','Refrain','Vigil','Accord','Retort','Offering','Canticle','Compass','Promise','Spiral','Relay','Ledger','Cadence','Weft']

def name_for(c,j):return FIRST[c][j%len(FIRST[c])]+ ' '+LAST[(j//len(FIRST[c]))%len(LAST)]

def main():
    slots=[]
    counts=[0,0,0,0]
    for id in range(394):
        c=(id-334)//20 if id>=334 else (0 if id<96 else 1 if id<185 else 2 if id<281 else 3)
        j=counts[c];counts[c]+=1
        # Co-prime traversal deliberately breaks the former catalogue's ordering.
        k=(j*17+c*11)%52
        t=j//52
        cost,typ,ops=recipe(k,t,c)
        rarity=0 if k<14 or 20<=k<28 else 1 if k<40 else 2
        if id>=334:
            ix=(id-334)%20;rarity=ix//6 if ix<18 else 3
        for effect in ops:
            if effect[0] in ('HIT','SWEEP','GUARD','BRACE_HIT','MARK_HIT','THREAT_GUARD','LOW_GUARD','RESERVE_GUARD','NEXT_GUARD','FOLD'):
                effect[1]=max(1,(effect[1]*88+50)//100)
        if t and len(ops)<4 and typ<2:
            for effect in ops:
                if effect[0] in ('HIT','SWEEP','GUARD','BRACE_HIT','MARK_HIT','THREAT_GUARD','LOW_GUARD','RESERVE_GUARD'):
                    effect[1]=max(1,effect[1]-2*t);break
            ops.append(op('NEXT_GUARD',2*t,1))
        name=name_for(c,j)
        slots.append(dict(id=id,name=name,character=c if c<3 else -1,cost=cost,rarity=rarity,type=typ,ops=ops))
    # Twenty authored unlocks per character preserve the ten-win progression.
    unlock_names=[
      ['Ash Dividend','Rivet Refuge','Cinder Census','Furnace Dividend','Anvil Debtor','Quarry Psalm','Kiln Reprisal','Chain Sanctuary','Hammer Testament','Flue Procession','Basalt Toll','Soot Almanac','Hearth Ultimatum','Foundry Covenant','Ember Inquest','Iron Remembrance','Mason Mercy','Bellows Inheritance','The Unbroken Kiln','An Oath Still Warm'],
      ['Ivory Murmur','Choir Refuge','Veil Census','Rime Reprisal','Needle Debt','Silt Psalm','Hush Reprisal','Velvet Sanctuary','Censer Testament','Chime Procession','Frost Toll','Moth Almanac','Requiem Ultimatum','Plume Covenant','Tallow Inquest','Quiet Remembrance','Cantor Mercy','Saltglass Inheritance','The Unvoiced Oath','A Name Beneath Snow'],
      ['Dawn Dividend','Brass Refuge','Orbit Census','Zenith Reprisal','Meridian Debt','Halo Psalm','Prism Reprisal','Lumen Sanctuary','Sundial Testament','Aperture Procession','Solstice Toll','Opal Almanac','Horizon Ultimatum','Umbra Covenant','Noonday Inquest','Aether Remembrance','Sextant Mercy','Mosaic Inheritance','The Fractured Noon','A Crown Unclaimed']]
    for c in range(3):
      rows=[
       (1,0,[op('HIT',7+c,3),op('ECHO',2)]),
       (1,1,[op('GUARD',6+c,3),op('ECHO',2)]),
       (1,0,[op('HIT',6+c,3),op('WITHER',2+c,1)]),
       (1,1,[op('THREAT_GUARD',6+c,3,3),op('NEXT_DRAW',1)]),
       (1,1,[op('SHARD',1),op('GUARD',4+c,2),op('ECHO',1)]),
       (1,0,[op('SWEEP',5+c,2),op('ECHO',1)]),
       (2,0,[op('HIT',11+c,4),op('VENT_HIT',3,1,3)]),
       (1,1,[op('GUARD',5+c,3),op('VENT_GUARD',3,1,3)]),
       (2,2,[op('POWER',3+c,1,[0,9,8][c]),op('GUARD',4,2)]),
       (1,1,[op('NEXT_GUARD',8+c,3),op('NEXT_DRAW',2),op('ECHO',1)]),
       (2,1,[op('WITHER',4+c,2,1),op('DRAW',1),op('GUARD',3,2)]),
       (2,0,[op('HIT',13+c,4),op('GUARD',8,3)]),
       (3,0,[op('SWEEP',13+c,4),op('GUARD',7,3),op('VENT_HIT',3,1,3)]),
       (2,2,[op('POWER',4+c,1,[6,10,3][c]),op('GUARD',5,2)]),
       (2,0,[op('HIT',17+c,5),op('ECHO',2)]),
       (2,1,[op('GUARD',17+c,4),op('VENT_WITHER',2,1,3)]),
       (1,1,[op('HEAL',3+c,2),op('GUARD',9,3)]),
       (1,1,[op('FOLD',9+c,3),op('DRAW',2),op('ECHO',2)]),
       (3,0,[op('SWEEP',19+c,5),op('GUARD',12,4),op('VENT_WITHER',2,1,4)]),
       (3,2,[op('POWER',5+c,2,[0,9,3][c]),op('GUARD',16,5),op('ECHO',3)]),
      ]
      for j,(cost,typ,ops) in enumerate(rows):
       slots[334+c*20+j].update(name=unlock_names[c][j],cost=cost,type=typ,ops=ops)
    # Holdout decision review: these rewards were consistently passed over.
    # Give each a useful setup niche without inflating the high-damage finishers.
    revisions={
      129:(1,[op('HIT',6,3),op('ECHO',2),op('NEXT_DRAW',1)]),
      199:(1,[op('HIT',6,3),op('ECHO',1),op('NEXT_GUARD',5,2)]),
      328:(1,[op('HIT',6,3),op('ECHO',1),op('DRAW',1)]),
      181:(1,[op('HIT',5,3),op('ECHO',1),op('NEXT_GUARD',2,1),op('WEAK',1)]),
      251:(1,[op('STRIP',7,3),op('HIT',5,3),op('ECHO',2),op('NEXT_GUARD',4,2)]),
      151:(1,[op('HIT',3,1,2),op('ECHO',2),op('NEXT_GUARD',3,2)]),
      174:(1,[op('GUARD',4,2),op('SHARD',2),op('ECHO',1),op('NEXT_GUARD',2,1)]),
      244:(1,[op('GUARD',4,2),op('SHARD',2),op('ECHO',1),op('NEXT_GUARD',2,1)]),
      336:(1,[op('HIT',7,3),op('WITHER',2,1),op('NEXT_DRAW',1)]),
      355:(1,[op('GUARD',8,3),op('ECHO',3),op('NEXT_GUARD',3,1)]),
    }
    for id,(cost,ops) in revisions.items():slots[id].update(cost=cost,ops=ops)
    # Starters teach each character's economy with immediate, usable effects.
    special={
      0:('Cinder Knuckle',0,1,0,[op('HIT',5,3),op('ECHO',1)]),
      1:('Hearth Stance',0,1,1,[op('GUARD',4,3),op('ECHO',1)]),
      2:('Pressure Break',0,2,0,[op('HIT',12,4),op('VENT_HIT',3,1,2),op('GUARD',3,2)]),
      173:('Ivory Flick',1,1,0,[op('HIT',5,3),op('WITHER',1),op('ECHO',1)]),
      120:('Veil Posture',1,1,1,[op('GUARD',4,3),op('ECHO',1)]),
      147:('Silt Silencer',1,1,0,[op('HIT',5,2),op('WEAK',1),op('ECHO',1)]),
      176:('Quiet Interval',1,1,1,[op('GUARD',8,3),op('NEXT_GUARD',3,2)]),
      269:('Brass Knell',2,1,0,[op('HIT',5,3),op('ECHO',1)]),
      210:('Orbit Mantle',2,1,1,[op('GUARD',4,3),op('ECHO',1)]),
      278:('Horizon Loan',2,0,1,[op('ECHO',2,1),op('NEXT_GUARD',2,2)]),
      213:('Meridian Cast',2,1,0,[op('HIT',5,2),op('VENT_HIT',3,1,3)]),
      165:('Needlewake',1,0,0,[op('HIT',3,2),op('GUARD',1,1)]),
      91:('Quarry Fragment',0,1,0,[op('HIT',11,4),op('GUARD',2,2)]),
      208:('Spent Filament',2,1,1,[op('GUARD',3,2),op('ECHO',2)]),
    }
    # These IDs are read from the game's enums by audit_catalog.py, not assumed by tests.
    for id,(name,c,cost,typ,ops) in special.items():slots[id].update(name=name,character=c,cost=cost,type=typ,ops=ops,rarity=0)
    for i,n in enumerate(['Cinder Scar','Fog Burden','Stone Ache','Rift Debt']):
        slots[50+i].update(name=n,type=3,cost=-2,ops=[],rarity=0)
    names=set()
    for row in slots:
        assert row['name'] not in names,row['name'];names.add(row['name'])
        row['description']=description(row['ops']) if row['type']!=3 else ['Cannot play. Held at turn end: lose 1 HP and 1 Echo.','Cannot play. Held at turn end: lose 2 Echo, if held.','Cannot play. Held at turn end: lose 2 Guard, if held.','Cannot play. Held at turn end: spend 2 Echo, or lose 2 HP.'][row['id']-50]
        row['upgrade_description']=description(row['ops'],True) if row['type']!=3 else row['description']
        row['exile']=any(o[0]=='HEAL' for o in row['ops']) or row['id'] in (91,165,208)
        if row['exile'] and not any(o[0]=='HEAL' for o in row['ops']):
            row['description']+=' Exile.';row['upgrade_description']+=' Exile.'
    # Reject unchanged rules and visible-name matches against the local pre-rewrite catalogue.
    baseline=Path('/tmp/rift-pre-distribution-source')
    if baseline.exists():
        old='\n'.join((baseline/n).read_text() for n in ['game.c','ninja_cards.inc','regent_cards.inc','new_cards.inc'])
        oldnames=set(re.findall(r'\{\s*"([^"]+)",\s*"',old))
        for r in slots:assert r['name'] not in oldnames, r['name']
        (ROOT/'distribution/legacy-name-hashes.json').write_text(json.dumps(sorted(hashlib.sha256(n.lower().encode()).hexdigest() for n in oldnames),indent=2)+'\n')
    (ROOT/'distribution/cards.json').write_text(json.dumps(slots,indent=2)+'\n')
    q=lambda x:json.dumps(x,ensure_ascii=True)
    text='/* Generated by distribution/tools/make_catalog.py. */\n'
    text+='enum {'+', '.join('DC_'+x for x in OPS)+'};\n'
    text+='enum {'+', '.join('DP_'+x[1] for x in POWERS)+', DP_COUNT};\n'
    text+='typedef struct {short kind,n,u,aux;} DistOp;\ntypedef struct {signed char character,exile,count; DistOp op[4];} DistCard;\n'
    text+='const CardDef cards[]={\n'+''.join(' {'+','.join([q(r['name']),q(r['description']),str(r['cost']),str(r['rarity']),str(r['type'])])+'},\n' for r in slots)+'};\n'
    text+='const int CARD_COUNT=394;\nstatic const char *upgrade_descriptions[]={\n'+''.join(' '+q(r['upgrade_description'])+',\n' for r in slots)+'};\n'
    text+='static const DistCard dist_cards[]={\n'
    for r in slots:
        text+=' {'+f'{r["character"]},{int(r["exile"])},{len(r["ops"])},'+'{'+','.join('{DC_'+k+f',{n},{u},{p}'+'}' for k,n,u,p in r['ops'])+'}},\n'
    text+='};\n'
    (SRC/'distribution_catalog.inc').write_text(text)
    print(f'Generated {len(slots)} original card slots; {sum(r["type"]!=3 for r in slots)} playable; unique names.')
if __name__=='__main__':main()
