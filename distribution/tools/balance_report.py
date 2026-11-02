from pathlib import Path
import csv,json,math,collections
R=Path(__file__).resolve().parents[2];E=R/'distribution/evidence'
cards=json.loads((R/'distribution/cards.json').read_text())
def read(tag):return [{k:int(v) for k,v in row.items()} for row in csv.DictReader((E/(tag+'.csv')).open())]
def interval(w,n):
 p=w/n;z=1.96;d=1+z*z/n;centre=(p+z*z/(2*n))/d;h=z*math.sqrt(p*(1-p)/n+z*z/(4*n*n))/d
 return f'{(centre-h)*100:.1f}–{(centre+h)*100:.1f}%'
def summary(r):
 w=sum(x['result']==1 for x in r);assert all(x['result'] in (0,1) for x in r)
 return f'{len(r):,} | {w:,} | {w/len(r)*100:.1f}% | {interval(w,len(r))}'
tags=['fresh-a','fresh-b','unlocked','defensive','secret'];groups={t:read('final-'+t) for t in tags};hold=read('holdout-fresh-holdout');standard=sum((groups[t] for t in tags[:-1]),[]);fresh=groups['fresh-a']+groups['fresh-b'];allstd=standard+hold
out=['# Distribution balance validation','',
'Final compiled ruleset: `'+json.loads((R/'release/current.json').read_text())['eboot_sha256']+'`.',
'', '## Scope and limits','',
'14,700 complete seeded simulations use the real game engine, ordinary starting decks, legal combat inputs, potions, rewards, route choices, shops, story events and forges. There are no debug kills or stat boosts. The first 11,700 runs compare the final tuning on fixed seeds; an additional 3,000 previously unused seeds were evaluated after tuning ended. Every run terminated in victory or defeat; no action-limit stalls occurred.',
'', 'These are heuristic simulated players, not human playtests or a guarantee of player win rate. Combat lookahead considers cards already known in hand; the reward policy is approximate and favors some effects. Wilson intervals describe the seeded sample only; character results sharing seeds and multiple policies are not independent human observations. Late-act endless balance beyond the secret-boss route was not estimated.',
'', '## Outcomes','', '| Cohort | Runs | Wins | Win rate | 95% Wilson interval |','|---|---:|---:|---:|---:|']
for label,r in [('Fresh profile, two fixed-seed batches',fresh),('All progression unlocked',groups['unlocked']),('More defensive policy, fresh profile',groups['defensive']),('Standard Act 3 aggregate',standard),('Independent fresh-seed holdout',hold),('All standard Act 3 samples',allstd),('Full secret-boss route, five prior wins',groups['secret'])]:out.append('| '+label+' | '+summary(r)+' |')
out+=['','The secret-route rate includes losses before reaching the secret boss; it is not the conditional win rate of that boss encounter. Unlocks raise expected power. The defensive policy is slightly above the 20–25% target, while the broad standard sample and independent holdout remain within it.','', '| Character | Standard runs including holdout | Wins | Win rate | 95% Wilson interval |','|---|---:|---:|---:|---:|']
for c,name in enumerate(['Warden','Vesper','Proxy']):out.append('| '+name+' | '+summary([r for r in allstd if r['character']==c])+' |')
out+=['','## Act progression','', '| Act | Runs reaching act | Defeats within act | Conditional defeat rate |','|---|---:|---:|---:|']
for a in range(1,4):
 r=[x for x in allstd if x['act']>=a];d=sum(x['act']==a and not x['result'] for x in r);out.append(f'| {a} | {len(r):,} | {d:,} | {d/len(r)*100:.1f}% |')
out+=['','Losses increase through the three standard acts. Act 2 remains the largest difficulty step for this policy; these data do not establish a uniformly gentle human difficulty curve. Enemy formations and the existing boss/minion tuning were preserved rather than retuned around one automated strategy.','', '## Decisions changed after playtesting','',
'Early candidates won too often (roughly 36–53% by character in the initial sample). Immediate card output, the strongest burst attacks and turn-economy loops were reduced. Starter Guard/damage, bounded Echo income, limited per-turn power triggers and Wither decay were then tested together. Later copies of template families trade immediate output for deferred defense rather than being strict numerical upgrades.',
'', 'A subsequent 11,700-run batch exposed ten reward cards with zero selections under the standard policies. They were given distinct setup benefits or lower-cost roles, then the same seeds were repeated. Starter Silt Silencer was not inflated merely because the policy declines additional copies: it already sees substantial use in its starting deck.',
'', '| Revised reward | Offered | Picked | Selection rate | Plays |','|---|---:|---:|---:|---:|']
stats=collections.defaultdict(collections.Counter)
for t in tags[:-1]:
 for row in csv.DictReader((E/('final-'+t+'-cards.csv')).open()):stats[int(row['id'])].update({k:int(v) for k,v in row.items() if k!='id'})
for i in [129,199,328,181,251,151,174,244,336,355]:
 s=stats[i];out.append(f"| {cards[i]['name']} | {s['offered']:,} | {s['picked']:,} | {100*s['picked']/s['offered']:.1f}% | {s['played']:,} |")
out+=['','Selection rates are not intended to be equal. Some cards remain situational and rarely selected by this policy. Pick rates compare reward offers only; play counts also include starter, shop and event acquisitions. A card appearing in winning decks does not by itself prove that it caused those wins. Strong direct-damage/Wither and banked-defense cards remain frequent choices; no universal must-pick claim is made.',
'', '## Reproduction','', 'Run `python3 distribution/tools/run_campaigns.py` for the 11,700 fixed-seed regressions, then `python3 distribution/tools/run_campaigns.py --holdout` for the 3,000 holdout runs. `--quick` is only a CI smoke sample. Raw per-run and per-card CSVs, cohort summaries, and source fingerprints are in `distribution/evidence/`. The recorded four-file rules fingerprint covers game.c plus the distribution catalog and card/relic engine includes; the release SHA-256 identifies the full compiled product.','']
(R/'distribution/BALANCE.md').write_text('\n'.join(out))
print('\n'.join(out[:27]))
