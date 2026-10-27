"""Compile the actual PSP button decoder with event/control invariant tests."""
from pathlib import Path
import os,re,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'qa/v150/source/psp_start.c').read_text()
decoder=re.search(r'static int button\(unsigned int down\) \{.*?\n\}',source,re.S).group()
constants={'SELECT':1,'START':8,'UP':16,'RIGHT':32,'DOWN':64,'LEFT':128,'LTRIGGER':256,'RTRIGGER':512,'TRIANGLE':4096,'CIRCLE':8192,'CROSS':16384,'SQUARE':32768}
with tempfile.TemporaryDirectory(prefix='rift150-controls-') as tmp:
    p=Path(tmp)
    text='#include "'+str(root/'qa/v150/source/game.h')+'"\n'
    text+=''.join(f'#define PSP_CTRL_{name} {value}\n' for name,value in constants.items())
    text+=decoder+'\n#include "'+str(root/'qa/v150/control_tests.c')+'"\n'
    (p/'audit.c').write_text(text)
    subprocess.run([os.environ.get('CC','gcc'),'-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer',str(p/'audit.c'),'-lm','-o',str(p/'audit')],check=True)
    subprocess.run([str(p/'audit')],cwd=root,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},check=True)
