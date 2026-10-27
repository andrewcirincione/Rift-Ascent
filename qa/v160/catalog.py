from pathlib import Path
import json,re
q=Path(__file__).parent
snapshot=q.parents[1].parent/'runtime/stsdata.json'
if snapshot.exists():D=json.loads(snapshot.read_text())
else:
 saved=json.loads((q/'reference/regent.json').read_text());neutral=json.loads((q/'reference/colorless.json').read_text())
 D={'meta':saved['meta'],'cards':saved['cards']+neutral['cards']}
reg=[c for c in D['cards'] if c.get('character')=='regent' or c['id'] in ['SovereignBlade','MinionDiveBomb','MinionSacrifice','MinionStrike','Debris']]
neutral=[c for c in D['cards'] if c['pool']=='colorless' and 'multiplayer' not in (c.get('sourceNotes') or '').lower()]
cs=reg+neutral
(q/'reference/regent.json').write_text(json.dumps({'meta':D['meta'],'cards':reg},indent=2)+'\n')
(q/'reference/colorless.json').write_text(json.dumps({'meta':D['meta'],'cards':neutral},indent=2)+'\n')
ids={c['id']:185+i for i,c in enumerate(cs)}
def enum(c):return ('R_' if c in reg else 'C_')+re.sub('([a-z0-9])([A-Z])',r'\1_\2',c['id']).upper()
def esc(x):return json.dumps(x,ensure_ascii=True)
def desc(c,up):
 s=(c['desc'].get('up') if up else c['desc']['base']) or c['desc']['base'];s=s['plain'].replace('\n',' ').replace('gain Star','gain 1 Star').replace('spend or gain Star','spend or gain Stars')
 if c['id']=='HammerTime':s='Solo: Forge twice whenever you Forge. Does not trigger itself.'
 if 'multiplayer' in (c.get('sourceNotes') or '').lower():s=s.replace('Another player','You').replace('another player','yourself').replace('all allies','you').replace('ALL players','You')+' (Solo adaptation.)'
 return s.replace('You draws','You draw').replace('You adds','You add').replace('You chooses','You choose').replace('gains 1 Energy, and gains','gain 1 Energy, and gain')
def val(c,k,up):
 v=c['vars'].get(k,{})
 return v.get('up',v.get('base',0)) if up else v.get('base',0)
def flags(c,up):
 s=desc(c,up);return sum(b for k,b in [('Exhaust',1),('Retain',2),('Innate',4),('Ethereal',8)] if re.search(r'(?:^|\.\s+)'+k+r'\.',s))
def cost(c,up):
 x=c.get('upCost',c['cost']) if up else c['cost'];return -1 if x=='X' else -2 if x is None else x
def star(c,up):
 x=c.get('upStarCost',c.get('starCost')) if up else c.get('starCost');return -1 if x=='X' else x or 0
h=f'#define REGENT_FIRST 185\n#define REGENT_COUNT {len(reg)}\n#define REGENT_LAST {185+len(reg)}\n#define COLORLESS_FIRST REGENT_LAST\n#define COLORLESS_COUNT {len(neutral)}\n#define EXTRA_LAST {185+len(cs)}\nenum {{\n'+',\n'.join(f' {enum(c)}={ids[c["id"]]}' for c in cs)+'\n};\n'
(q/'source/regent_ids.h').write_text(h)
base=[];up=[];meta=[]
for c in cs:
 rarity={'Common':0,'Uncommon':1,'Rare':2}.get(c['rarity'],3)
 # All five ally cards use explicit local solo adaptations and normal rarity.
 base.append('{'+','.join([esc(c['title']),esc(desc(c,0)),str(cost(c,0)),str(rarity),str({'Attack':0,'Skill':1,'Power':2,'Status':3}[c['type']])])+'},')
 up.append(esc(desc(c,1))+',')
 meta.append('{'+','.join(str(v) for v in [cost(c,1),star(c,0),star(c,1),flags(c,0),flags(c,1),val(c,'Damage',0),val(c,'Damage',1),val(c,'Block',0),val(c,'Block',1)])+'}, // '+c['id'])
(q/'source/regent_cards.inc').write_text('\n'.join(base)+'\n');(q/'source/regent_upgrades.inc').write_text('\n'.join(up)+'\n')
(q/'source/regent_meta.inc').write_text('typedef struct {int upcost,stars,upstars,flags,upflags,damage,updamage,block,upblock;} RegentCardDef;\nstatic const RegentCardDef regent_cards[]={\n'+'\n'.join(meta)+'\n};\n')
(q/'reference/ids.json').write_text(json.dumps({enum(c):ids[c['id']] for c in cs},indent=2)+'\n')
print('Cards:',len(reg),'Regent/tokens,',len(neutral),'Colorless. Total',185+len(cs))
