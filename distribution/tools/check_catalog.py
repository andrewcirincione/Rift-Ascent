"""Known-name regression and reproducibility checks, not a legal clearance test."""
from pathlib import Path
import hashlib,json,subprocess
R=Path(__file__).resolve().parents[2];S=R/'qa/v186/source'
known=set(json.loads((R/'distribution/legacy-name-hashes.json').read_text()))
all_names=set()
for file,count in [('cards.json',394),('relics.json',94),('potions.json',26)]:
 rows=json.loads((R/'distribution'/file).read_text());assert len(rows)==count
 for i,r in enumerate(rows):
  assert r['id']==i,(file,i)
  name=r['name'].casefold();assert name not in all_names,(file,name);all_names.add(name)
  assert hashlib.sha256(name.encode()).hexdigest() not in known,(file,name)
  if file=='cards.json':assert len(r['ops'])<=4 and r['description'] and r['upgrade_description']
 print('PASS',file,count,'unique names, no exact inherited-name matches')
files=['distribution/cards.json','distribution/relics.json','distribution/potions.json','qa/v186/source/distribution_catalog.inc','qa/v186/source/distribution_relics.inc','qa/v186/source/new_potions.h','qa/v186/source/new_potion_effects.inc','qa/v186/source/game.c']
before={f:(R/f).read_bytes() for f in files}
for name in ['make_catalog.py','make_relics.py','make_potions.py']:subprocess.run(['python3',str(R/'distribution/tools'/name)],check=True)
assert all((R/f).read_bytes()==b for f,b in before.items()),'Regeneration changed committed rules'
for name in ['ninja_cards.inc','regent_cards.inc','new_cards.inc','ninja_engine.inc','regent_engine.inc','new_relics.inc','expedition_relics.inc']:
 assert not (S/name).exists(),name
print('PASS deterministic catalog/rules regeneration; retired card engines and catalogs absent')
