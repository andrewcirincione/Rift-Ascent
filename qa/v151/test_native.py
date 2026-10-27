from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2];rt=root.parent/'runtime'
def run(cmd,path):
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True);(root/path).write_text(p.stdout+p.stderr);print(p.stdout[-5000:],p.stderr[-1000:]);assert p.returncode==0,cmd
for name,source in [('engine','engine_tests.c'),('ninja','regression/ninja_audit.c'),('save','regression/save_tests.c')]:
 run(['cc','-O2','-std=c99','qa/v151/'+source,'-o',str(rt/('test151-'+name))],Path('qa/v151/evidence')/(name+'-compile.txt'))
 run([str(rt/('test151-'+name))],Path('qa/v151/evidence')/(name+'-tests.txt'))
