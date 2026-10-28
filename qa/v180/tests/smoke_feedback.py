"""Exact-EBOOT controller-driven review. No game code or live-memory modification.
Only checksum-valid saved-game fixtures are prepared before boot.
"""
from pathlib import Path
import os,sys,json,subprocess,asyncio,hashlib,shutil,time
from PIL import ImageGrab
import websockets
root=Path(sys.argv[1]).resolve();app=Path(sys.argv[2]).resolve();ev=root/'evidence';ev.mkdir(exist_ok=True)
syms={q[3]:int(q[0],16) for line in (root/'source/symbols.txt').read_text().splitlines() if len(q:=line.split())==4};layout=json.loads((ev/'review-layout.json').read_text())
home=root/'review-home'
if home.exists():shutil.rmtree(home)
install=home/'.config/ppsspp/PSP/GAME/Rift Ascent 1.8';install.mkdir(parents=True)
subprocess.run([str(root/'tests/review_fixtures'),str(install)],check=True)
exe=root/'release/EBOOT.PBP';shutil.copyfile(exe,install/'EBOOT.PBP');digest=hashlib.sha256(exe.read_bytes()).hexdigest()
config=home/'config.ini';config.write_text('[General]\nRemoteISOPort = 19528\nRemoteDebuggerOnStartup = True\nPauseOnLostFocus = False\nPauseWhenMinimized = False\nEnableLogging = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(HOME=str(home),XDG_CONFIG_HOME=str(home/'.config'),LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy')
log=(ev/'ppsspp-feedback.log').open('w');proc=subprocess.Popen([str(app),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(install/'EBOOT.PBP')],env=env,stdout=log,stderr=subprocess.STDOUT)
class QA:
 def __init__(self):self.ws=None;self.ticket=0
 async def req(self,event,**kw):
  self.ticket+=1;t=self.ticket;await self.ws.send(json.dumps(dict(event=event,ticket=t,**kw)))
  while True:
   r=json.loads(await asyncio.wait_for(self.ws.recv(),30))
   if r.get('ticket')==t:
    if r.get('event')=='error':raise RuntimeError(r)
    return r
 async def value(self,s,offset=0):return (await self.req('memory.read_u32',address=syms[s]+offset))['value']
 async def field(self,s):return await self.value('game',layout[s])
 async def connect(self):
  for _ in range(100):
   await asyncio.sleep(.5);assert proc.poll() is None,'Emulator exited'
   try:
    if not self.ws:self.ws=await websockets.connect('ws://127.0.0.1:19528/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=2000000)
    if await self.value('game')==0x52494654 and await self.value('animation_tick')>50:return
   except (OSError,RuntimeError,websockets.exceptions.ConnectionClosed):self.ws=None
  raise RuntimeError('Game did not boot')
 async def idle(self):
  for _ in range(500):
   tick=await self.value('animation_tick');busy=False
   if await self.value('r161_transition_active'):busy=((tick-await self.value('r161_transition_tick'))&0xffffffff)<await self.value('r161_transition_length')
   if await self.value('r1311_finishing'):busy|=((tick-await self.value('r137_start'))&0xffffffff)<await self.value('r1311_kill_at')+200
   if not busy:return
   await asyncio.sleep(.2)
  raise RuntimeError('Visual transition stalled')
 async def button(self,b):
  await self.idle();await self.req('input.buttons.press',button=b,duration=15);await asyncio.sleep(1.5)
 async def screen(self,n):
  for _ in range(80):
   if await self.field('screen')==n:await self.idle();return
   await asyncio.sleep(.25)
  raise RuntimeError(f'Expected screen {n}, got {await self.field("screen")}')
 async def select(self,n):
  for _ in range(10):
   if await self.field('selection')==n:return
   await self.button('down')
  raise RuntimeError('Could not select '+str(n))
 async def shot(self,name):
  await self.idle();await asyncio.sleep(.4)
  ImageGrab.grab(xdisplay=os.environ['DISPLAY']).save(ev/('psp-'+name+'.png'))
  assert await self.value('rift_asset_errors')==0
  print('PASS screen',name,'id',await self.field('screen'),flush=True)
 async def savequit(self):
  await self.button('select');await self.screen(13);await self.button('cross');await self.screen(0)
async def main():
 q=QA()
 try:
  await q.connect();await q.screen(0);assert await q.value('r180_debug_enabled')==0;await q.shot('title')
  await q.select(3);await q.button('cross');await q.screen(10);await q.shot('options-off');await q.button('cross');assert await q.value('r180_debug_enabled')==1;await q.shot('options-on');await q.button('circle');await q.screen(0)
  await q.select(0);await q.button('cross');await q.screen(29);await q.shot('save-slots');await q.button('cross');await q.screen(2);assert await q.field('hp')==500;await q.shot('combat-maxhp-facing')
  await q.button('triangle');assert await q.value('inspect_open')==1;act=await q.field('act');await q.button('rtrigger');assert await q.field('act')==act;await q.button('circle');assert await q.value('inspect_open')==0
  await q.button('select');await q.screen(13);await q.shot('pause-no-cross');
  for _ in range(4):
   if await q.field('screen')!=13:break
   await q.button('circle')
  await q.screen(2)
  kills=await q.field('kill_count');await q.button('ltrigger');await q.screen(15);assert await q.field('kill_count')==kills+2;await q.shot('debug-kill-reward')
  await q.button('rtrigger');await q.screen(20);assert await q.field('act')==4;await q.shot('debug-next-act');await q.savequit()
  await q.select(3);await q.button('cross');await q.screen(10);await q.button('cross');assert await q.value('r180_debug_enabled')==0;await q.button('circle');await q.screen(0)
  await q.select(0);await q.button('cross');await q.screen(29);await q.select(1);await q.button('cross');await q.screen(6);await q.shot('moon-heal');await q.button('down');await q.shot('moon-reflection');await q.button('up');await q.button('cross');await q.screen(14);assert await q.field('hp')==50;await q.savequit()
  await q.select(0);await q.button('cross');await q.screen(29);await q.select(2);await q.button('cross');await q.screen(2);act=await q.field('act');await q.button('ltrigger');assert await q.field('screen')==2;await q.button('rtrigger');assert await q.field('act')==act;await q.shot('act4-random');await q.savequit()
  await q.select(1);await q.button('cross');await q.screen(29);await q.select(3);await q.button('cross');await q.screen(22);await q.shot('character-records')
  assert hashlib.sha256((install/'EBOOT.PBP').read_bytes()).hexdigest()==digest
  result=dict(sha256=digest,cases=['options-toggle','save-safe-inset','high-maxhp','boss-facing','pause-no-cross','debug-L','debug-R','inventory-shoulder-preserved','debug-off-noop','moon-fixed-heal','moon-reflection','act4-random','character-corners'],scope='Exact complete EBOOT, renamed install directory, controller inputs and read-only state assertions; seeded saved-game fixtures; no live-memory or executable patches.')
  (ev/'FEEDBACK_SMOKE_PASSED.json').write_text(json.dumps(result,indent=2));print('PASS final release EBOOT',digest,flush=True)
 finally:
  if q.ws:await q.ws.close()
  proc.terminate()
  try:proc.wait(timeout=8)
  except subprocess.TimeoutExpired:proc.kill();proc.wait()
  log.close()
if __name__=='__main__':asyncio.run(main())
