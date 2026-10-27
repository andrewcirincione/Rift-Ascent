from pathlib import Path
import subprocess,os
q=Path('qa/v160');(q/'regression').mkdir(exist_ok=True)
for p in Path('qa/v151/regression').iterdir():
 if p.suffix in ('.c','.h'):
  t=p.read_text().replace('qa/v151/evidence/','qa/v160/evidence/').replace('g.version==22','g.version==23').replace('GAME_VERSION==22','GAME_VERSION==23').replace('CARD_COUNT==185&&RELIC_COUNT==63','CARD_COUNT==334&&RELIC_COUNT==64')
  (q/'regression'/p.name).write_text(t)
p=q/'regression/save_tests.c';s=p.read_text().replace('qa/v160/evidence/pending-sly.dat','qa/v151/evidence/pending-sly.dat');p.write_text(s)
t=Path('qa/v151/engine_tests.c').read_text().replace('RELIC_COUNT==63','RELIC_COUNT==64').replace('g.version==22','g.version==23').replace('GAME_VERSION==22','GAME_VERSION==23').replace('CARD_COUNT==185&&RELIC_COUNT==63','CARD_COUNT==334&&RELIC_COUNT==64').replace('qa/v151/evidence/','qa/v160/evidence/');(q/'engine_tests.c').write_text(t)
for name,src in [('regression-engine','regression/engine_tests.c'),('engine','engine_tests.c'),('ninja','regression/ninja_audit.c'),('save','regression/save_tests.c')]:
 out=Path('../runtime')/('audit160-'+name)
 p=subprocess.run(['cc','-O2','-std=c99',str(q/src),'-o',str(out)],capture_output=True,text=True)
 assert p.returncode==0,p.stderr
 p=subprocess.run([str(out)],capture_output=True,text=True)
 (q/'evidence'/(name+'-tests.txt')).write_text(p.stdout+p.stderr);print(name,p.stdout[-1000:],p.stderr[-1000:],flush=True);assert p.returncode==0
