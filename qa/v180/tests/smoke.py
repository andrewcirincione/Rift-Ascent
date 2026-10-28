"""Exact EBOOT smoke: assisted saves, controller input, read-only state.
No CPU breakpoints, live-memory writes or executable patches.
"""
from pathlib import Path
import os,sys,json,subprocess,asyncio,struct,hashlib,shutil,time
from PIL import ImageGrab,Image
import websockets
root=Path(sys.argv[1]).resolve();app=Path(sys.argv[2]).resolve();ev=root/'evidence';ev.mkdir(exist_ok=True)
layout=json.loads((ev/'state-layout.json').read_text());sha=hashlib.sha256((root/'release/EBOOT.PBP').read_bytes()).hexdigest()
syms={q[3]:int(q[0],16) for line in (ev/'symbols.txt').read_text().splitlines() if len(q:=line.split())==4}
home=root/'qa-home';home.mkdir(exist_ok=True);install=home/'.config/ppsspp/PSP/GAME/RIFTASCENT_A18';install.mkdir(parents=True,exist_ok=True)
config=root/'qa.ini';config.write_text('[General]\nRemoteISOPort = 19518\nRemoteDebuggerOnStartup = True\nPauseOnLostFocus = False\nPauseWhenMinimized = False\nEnableLogging = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(HOME=str(home),XDG_CONFIG_HOME=str(home/'.config'),LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy')
def journal(name,size):
 found=[]
 for f in install.glob(name+'.[01]'):
  b=f.read_bytes()
  if len(b)!=20+size:continue
  magic,version,generation,n,h=struct.unpack_from('<5I',b)
  if magic!=0x4131384a or version!=1 or n!=size:continue
  v=2166136261
  for x in b[20:]:v=((v^x)*16777619)&0xffffffff
  if v==h:found.append((generation,b[20:]))
 assert found,'No valid journal '+name
 return max(found,key=lambda a:a[0])[1]
class Q:
 def __init__(self):self.ws=None;self.ticket=0
 async def connect(self):
  for _ in range(60):
   try:
    self.ws=await websockets.connect('ws://127.0.0.1:19518/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=1000000)
    if await self.value('game')==0x52494654:return
    await self.ws.close()
   except (OSError,RuntimeError):pass
   await asyncio.sleep(.5)
  raise RuntimeError('Emulator game did not boot')
 async def req(self,event,**params):
  self.ticket+=1;t=self.ticket;await self.ws.send(json.dumps(dict(event=event,ticket=t,**params)))
  while True:
   result=json.loads(await asyncio.wait_for(self.ws.recv(),30))
   if result.get('ticket')==t:
    if result['event']=='error':raise RuntimeError(result)
    return result
 async def value(self,s):return (await self.req('memory.read_u32',address=syms[s]))['value']
 async def field(self,s):return (await self.req('memory.read_u32',address=syms['game']+layout[s]))['value']
 async def idle(self):
  for _ in range(120):
   tick=await self.value('animation_tick');busy=False
   if await self.value('r161_transition_active'):
    busy=((tick-await self.value('r161_transition_tick'))&0xffffffff)<await self.value('r161_transition_length')
   if await self.value('r1311_finishing'):
    busy=busy or ((tick-await self.value('r137_start'))&0xffffffff)<await self.value('r1311_kill_at')+200
   if not busy:return
   await asyncio.sleep(.5)
  raise RuntimeError('Game visual transition did not finish')
 async def button(self,b):
  await self.idle();await self.req('input.buttons.press',button=b,duration=15);await asyncio.sleep(1.5)
 async def advance(self,target):
  for _ in range(12):
   if await self.field('screen')==target:return
   await self.button('cross')
  raise RuntimeError('Expected %s, actual %s'%(target,await self.field('screen')))
 async def depart(self,old):
  for _ in range(8):
   if await self.field('screen')!=old:return
   await self.button('cross')
  raise RuntimeError('Could not leave '+str(old))
 async def shot(self,name):
  await self.idle()
  for attempt in range(15):
   await asyncio.sleep(1);im=ImageGrab.grab(xdisplay=os.environ['DISPLAY']).resize((480,272),Image.Resampling.NEAREST).convert('RGB')
   if len(im.getcolors(130561))>64:break
  im.save(ev/(name+'.png'));assert len(im.getcolors(130561))>64,'Blank screenshot: '+name
  assert await self.value('rift_asset_errors')==0,'Asset streaming error'
  print('SCREENSHOT',name,'screen',await self.field('screen'),flush=True)
 async def profile(self):return journal('A18_PROFILE',layout['profile_size'])
async def case(mode,c):
 fixture=root/'fixtures'/f'{mode}-{c}';fixture.mkdir(parents=True,exist_ok=True)
 for f in fixture.glob('A18_*'):f.unlink()
 subprocess.run([str(root/'tests/fixtures'),str(fixture),mode,str(c)],check=True)
 for f in install.glob('A18_*'):f.unlink()
 for f in fixture.iterdir():shutil.copyfile(f,install/f.name)
 shutil.copyfile(root/'release/EBOOT.PBP',install/'EBOOT.PBP')
 alternate=home/'.config/ppsspp/GAME';alternate.mkdir(parents=True,exist_ok=True)
 if not (alternate/'RIFTASCENT_A18').exists():(alternate/'RIFTASCENT_A18').symlink_to(install,target_is_directory=True)
 logfile=(ev/f'ppsspp-{mode}-{c}.log').open('w');p=subprocess.Popen([str(app),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(install/'EBOOT.PBP')],env=env,stdout=logfile,stderr=subprocess.STDOUT)
 q=Q()
 try:
  await asyncio.sleep(6);assert p.poll() is None,'Emulator exited at boot';await q.connect()
  await q.shot(f'title-{mode}-{c}');await q.advance(29);await q.shot(f'slots-{mode}-{c}');await q.button('cross');await asyncio.sleep(3)
  if mode=='tenth':
   assert await q.field('screen')==2;await q.shot(f'act3-before-{c}');await q.advance(32);await q.shot(f'act3-ending-{c}');await q.advance(33);await q.shot(f'unlock-cards-{c}');await q.advance(34);await q.shot(f'unlock-potions-{c}');await q.advance(35);await q.shot(f'unlock-relic-{c}');await q.advance(36);await q.shot(f'beyond-{c}')
   data=await q.profile();assert struct.unpack_from('<I',data,layout['profile_wins']+4*c)[0]==10
   await q.depart(36);assert await q.field('act')==4;await q.shot(f'act4-arrival-{c}')
  elif mode=='boss4':
   assert await q.field('screen')==37;await q.shot(f'act4-intro-{c}');await q.advance(2);await q.shot(f'act4-fight-{c}')
   data=journal('A18_SLOT_1',layout['size']);n=struct.unpack_from('<i',data,layout['hand_count'])[0];hand=struct.unpack_from('<'+str(n)+'i',data,layout['hand']);ids=json.loads((ev/'attack-ids.json').read_text());idx=next(i for i,raw in enumerate(hand) if (raw&511) in ids)
   for _ in range(idx):await q.button('right')
   await q.advance(38);await q.shot(f'true-ending-{c}');data=await q.profile()
   assert struct.unpack_from('<I',data,layout['profile_act4_clears']+4*c)[0]==0
   assert struct.unpack_from('<I',data,layout['profile_wins']+4*c)[0]==10
   assert struct.unpack_from('<I',data,layout['profile_best_acts']+4*c)[0]==4
   assert struct.unpack_from('<I',data,layout['profile_true_endings'])[0]&(1<<c)
   await q.advance(8);await q.shot(f'act4-record-{c}')
  elif mode=='archive':
   assert await q.field('screen')==31;await q.shot('collection-cards');await q.button('right');await q.shot('collection-potions');await q.button('right');await q.shot('collection-relics')
  else:assert await q.field('screen')==2;await q.shot('two-enemy-combat')
  assert p.poll() is None;assert sha==hashlib.sha256((install/'EBOOT.PBP').read_bytes()).hexdigest();print('PASS exact EBOOT',mode,c,sha,flush=True)
 finally:
  if q.ws:await q.ws.close()
  p.terminate()
  try:p.wait(timeout=8)
  except subprocess.TimeoutExpired:p.kill();p.wait()
  logfile.close();await asyncio.sleep(.5)
async def main():
 cases=sys.argv[3:] or ['tenth:0','boss4:0','tenth:1','boss4:1','tenth:2','boss4:2','archive:0','combat:0']
 for val in cases:
  mode,c=val.split(':');await case(mode,int(c))
 (ev/'SMOKE_PASSED.json').write_text(json.dumps(dict(sha256=sha,cases=cases,scope='Exact PSP binary; assisted saved-game fixtures; controller input; read-only running state; checksum-validated persistent records. No executable/live-memory patches or CPU breakpoints.'),indent=2))
if __name__=='__main__':asyncio.run(main())
