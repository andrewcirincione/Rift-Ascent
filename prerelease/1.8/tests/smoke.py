"""Assisted exact-EBOOT tests: real keyboard input and read-only debugger checks.
No game executable bytes or live game memory are modified by this harness.
"""
from pathlib import Path
import os,sys,json,subprocess,asyncio,base64,struct,hashlib,shutil,time
from PIL import Image
from Xlib import display,X,XK
from Xlib.ext import xtest
import websockets
root=Path(sys.argv[1]).resolve();app=Path(sys.argv[2]).resolve();ev=root/'evidence';ev.mkdir(exist_ok=True)
layout=json.loads((ev/'state-layout.json').read_text());syms={q[3]:int(q[0],16) for line in (ev/'symbols.txt').read_text().splitlines() if len(q:=line.split())==4}
sha=hashlib.sha256((root/'release/EBOOT.PBP').read_bytes()).hexdigest()
home=root/'qa-home';home.mkdir(exist_ok=True);install=home/'.config/ppsspp/PSP/GAME/RIFTASCENT_A18';install.mkdir(parents=True,exist_ok=True)
config=root/'qa.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19518\nPauseOnLostFocus = False\nPauseWhenMinimized = False\nEnableLogging = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(HOME=str(home),XDG_CONFIG_HOME=str(home/'.config'),LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy')
class Q:
 def __init__(self):self.i=0
 async def req(self,event,**kw):
  self.i+=1;t=self.i
  async with websockets.connect('ws://127.0.0.1:19518/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000) as w:
   await w.send(json.dumps(dict(event=event,ticket=t,**kw)))
   while True:
    r=json.loads(await asyncio.wait_for(w.recv(),20))
    if r.get('ticket')==t:
     if r.get('event')=='error':raise RuntimeError(r)
     return r
 async def val(self,s):return (await self.req('memory.read_u32',address=syms[s]))['value']
 async def memory(self,a,n):return base64.b64decode((await self.req('memory.read',address=a,size=n))['base64'])
 async def state(self):return await self.memory(syms['game'],layout['size'])
 async def field(self,s):return struct.unpack_from('<i',await self.state(),layout[s])[0]
 async def ticks(self,n):await asyncio.sleep(max(.35,n/80))
 async def screen(self,n):
  end=time.monotonic()+40
  while time.monotonic()<end:
   if await self.field('screen')==n:return
   await asyncio.sleep(1)
  raise RuntimeError('Expected screen %d, actual %d'%(n,await self.field('screen')))
 async def button(self,b):
  d=display.Display();name={'cross':'z','right':'Right','left':'Left','up':'Up','down':'Down','circle':'x'}[b]
  for window in d.screen().root.query_tree().children:
   try:
    if 'PPSSPP' in str(window.get_wm_class()):window.set_input_focus(X.RevertToParent,X.CurrentTime)
   except Exception:pass
  k=d.keysym_to_keycode(XK.string_to_keysym(name));xtest.fake_input(d,X.KeyPress,k);d.sync();await asyncio.sleep(.35);xtest.fake_input(d,X.KeyRelease,k);d.sync();d.close();await self.ticks(80)
 async def shot(self,name):
  await self.ticks(24);p=(await self.val('pix'))&0x1fffffff
  raw=await self.memory(p,512*272*4);im=Image.frombytes('RGBA',(512,272),raw).crop((0,0,480,272)).convert('RGB');im.save(ev/(name+'.png'))
  assert len(im.getcolors(130561))>64,'Blank game image'
  assert await self.val('rift_asset_errors')==0,'Streamed asset error'
  print('PASS screenshot',name,'screen',await self.field('screen'),flush=True)
 async def profile(self):return await self.memory(syms['a18_account'],layout['profile_size'])
async def case(mode,c):
 fixture=root/'fixtures'/f'{mode}-{c}';fixture.mkdir(parents=True,exist_ok=True)
 for f in fixture.glob('A18_*'):f.unlink()
 subprocess.run([str(root/'tests/fixtures'),str(fixture),mode,str(c)],check=True)
 for f in install.glob('A18_*'):f.unlink()
 for f in fixture.iterdir():shutil.copyfile(f,install/f.name)
 shutil.copyfile(root/'release/EBOOT.PBP',install/'EBOOT.PBP')
 alternate=home/'.config/ppsspp/GAME';alternate.mkdir(parents=True,exist_ok=True)
 if not (alternate/'RIFTASCENT_A18').exists():(alternate/'RIFTASCENT_A18').symlink_to(install,target_is_directory=True)
 log=(ev/f'ppsspp-{mode}-{c}.log').open('w');p=subprocess.Popen([str(app),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(install/'EBOOT.PBP')],env=env,stdout=log,stderr=subprocess.STDOUT)
 try:
  q=Q();ready=False
  for _ in range(180):
   await asyncio.sleep(.25)
   if p.poll() is not None:raise RuntimeError('PPSSPP quit: '+str(p.returncode))
   try:ready=await q.val('game')==0x52494654
   except (OSError,RuntimeError):ready=False
   if ready:break
  assert ready,'No game startup'
  d=display.Display()
  for window in d.screen().root.query_tree().children:
   try:
    if 'PPSSPP' in str(window.get_wm_class()):window.configure(stack_mode=X.Above);window.set_input_focus(X.RevertToParent,X.CurrentTime);d.sync()
   except Exception:pass
  d.close();await q.ticks(200);await q.screen(0);await q.shot(f'title-{mode}-{c}')
  await q.button('cross');await q.screen(29);await q.shot(f'slots-{mode}-{c}');await q.button('cross');await q.ticks(200)
  if mode=='tenth':
   await q.screen(2);await q.shot(f'act3-before-{c}');await q.button('cross');await q.screen(32);await q.ticks(300);await q.shot(f'act3-ending-{c}')
   for _ in range(3):await q.button('cross')
   await q.screen(33);await q.shot(f'unlock-cards-{c}');await q.button('cross');await q.screen(34);await q.shot(f'unlock-potions-{c}');await q.button('cross');await q.screen(35);await q.shot(f'unlock-relic-{c}');await q.button('cross');await q.screen(36);await q.shot(f'beyond-{c}')
   data=await q.profile();assert struct.unpack_from('<I',data,layout['profile_wins']+4*c)[0]==10
   for _ in range(3):await q.button('cross')
   assert await q.field('act')==4;await q.shot(f'act4-arrival-{c}')
  elif mode=='boss4':
   await q.screen(37);await q.shot(f'act4-intro-{c}')
   for _ in range(3):await q.button('cross')
   await q.screen(2);await q.ticks(120);await q.shot(f'act4-fight-{c}')
   data=await q.state();n=struct.unpack_from('<i',data,layout['hand_count'])[0];hand=struct.unpack_from('<'+str(n)+'i',data,layout['hand']);ids=json.loads((ev/'attack-ids.json').read_text());idx=next(i for i,raw in enumerate(hand) if (raw&511) in ids)
   for _ in range(idx):await q.button('right')
   await q.button('cross');await q.screen(38);await q.ticks(300);await q.shot(f'true-ending-{c}')
   data=await q.profile()
   assert struct.unpack_from('<I',data,layout['profile_act4_clears']+4*c)[0]==1
   assert struct.unpack_from('<I',data,layout['profile_wins']+4*c)[0]==10
   assert struct.unpack_from('<I',data,layout['profile_best_acts']+4*c)[0]==4
   assert struct.unpack_from('<I',data,layout['profile_true_endings'])[0]&(1<<c)
   for _ in range(3):await q.button('cross')
   await q.screen(8);await q.shot(f'act4-record-{c}')
  elif mode=='archive':
   await q.screen(31);await q.shot('collection-cards');await q.button('right');await q.shot('collection-potions');await q.button('right');await q.shot('collection-relics')
  else:await q.screen(2);await q.shot('two-enemy-combat')
  assert sha==hashlib.sha256((install/'EBOOT.PBP').read_bytes()).hexdigest()
  print('PASS exact release',mode,c,sha,flush=True)
 finally:
  p.terminate()
  try:p.wait(timeout=8)
  except subprocess.TimeoutExpired:p.kill();p.wait()
  log.close();await asyncio.sleep(.5)
async def main():
 cases=sys.argv[3:] or ['tenth:0','boss4:0','tenth:1','boss4:1','tenth:2','boss4:2','archive:0','combat:0']
 for value in cases:
  mode,c=value.split(':');await case(mode,int(c))
 (ev/'SMOKE_PASSED.json').write_text(json.dumps(dict(sha256=sha,cases=cases,scope='Exact packaged PSP executable; assisted saved-game fixtures, read-only debugger and real keyboard input. No executable modification.'),indent=2))
if __name__=='__main__':asyncio.run(main())
