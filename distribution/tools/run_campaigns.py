#!/usr/bin/env python3
"""Reproducible legal-action balance probes; simulated players, not human win-rate claims."""
from pathlib import Path
import concurrent.futures,subprocess,hashlib,json,shutil,csv,argparse,tempfile
R=Path(__file__).resolve().parents[2];E=R/'distribution/evidence'
p=argparse.ArgumentParser();p.add_argument('--quick',action='store_true');p.add_argument('--holdout',action='store_true');a=p.parse_args()
tmp=Path(tempfile.mkdtemp(prefix='rift-dist-holdout-'));exe=tmp/'campaigns'
subprocess.run(['gcc','-O2','-std=gnu11',str(R/'distribution/tests/campaigns.c'),'-o',str(exe)],check=True)
jobs=[('fresh-a',1000,60000,0,0,3),('fresh-b',1000,70000,0,0,3),('unlocked',1000,80000,1,0,3),('defensive',600,90000,0,1,3),('secret',300,100000,1,0,4)]
if a.holdout:jobs=[('fresh-holdout',1000,110000,0,0,3)]
if a.quick:jobs=[(name,10,off,u,pol,goal) for name,n,off,u,pol,goal in jobs]
files=['game.c','distribution_catalog.inc','distribution_engine.inc','distribution_relic_engine.inc']
sha=hashlib.sha256(b''.join((R/'qa/v186/source'/n).read_bytes() for n in files)).hexdigest()
def run(j):
 name,n,offset,unlocked,policy,goal=j;out=tmp/(name+'.csv');stats=tmp/(name+'-cards.csv');log=tmp/(name+'.txt')
 with out.open('w') as f,log.open('w') as l:
  p=subprocess.run([str(exe),str(n),str(offset),str(unlocked),'-1',str(policy),str(stats),str(goal)],stdout=f,stderr=l)
 rows=list(csv.DictReader(out.open()));assert len(rows)==n*3,(name,len(rows));assert p.returncode==0,(name,log.read_text())
 assert all(int(x['result']) in (0,1) for x in rows)
 for file in (out,stats,log):shutil.copyfile(file,E/(('smoke-' if a.quick else 'holdout-' if a.holdout else 'final-')+file.name))
 result=dict(name=name,runs=len(rows),wins=sum(int(x['result'])==1 for x in rows),source_sha256=sha,goal=goal,policy=policy,unlocked=unlocked)
 print(json.dumps(result),flush=True);return result
with concurrent.futures.ThreadPoolExecutor(max_workers=5) as pool:results=list(pool.map(run,jobs))
(E/('campaign-smoke.json' if a.quick else 'campaign-holdout.json' if a.holdout else 'campaign-summary.json')).write_text(json.dumps(results,indent=2)+'\n')
