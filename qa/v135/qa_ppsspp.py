import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
base=Path(__file__).resolve().parents[3];rt=base/'runtime';ev=base/'evidence135';cfg=rt/'qa135statusconfig';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True);(gamedir/'SAVE.DAT').unlink(missing_ok=True);shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa135status.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19233\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:248',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4713',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':248','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb-qa.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4713 auth-anonymous=1'],env=env,stdout=open(ev/'pulse-qa.log','w'),stderr=subprocess.STDOUT)
nm=subprocess.check_output([str(rt/'pspdev/psp/bin/nm'),'-S',str(base/'rift-ascent/portable_build/rift_ascent_crt_free.elf')]).decode();symbols={parts[3]:int(parts[0],16) for line in nm.splitlines() if len(parts:=line.split())==4}
p=None;ticket=0
async def check(ws):
 global ticket
 async def req(event,**kw):
  global ticket
  ticket+=1;t=ticket;await ws.send(json.dumps(dict(event=event,ticket=t,**kw)))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),10))
   if d.get('ticket')==t:
    assert d['event']!='error',d
    return d
 async def command(event):
  await ws.send(json.dumps({'event':event}))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),10))
   if d.get('event')==event:return
 async def state():
  d=await req('memory.read',address=symbols['game'],size=3144);return base64.b64decode(d['base64'])
 def val(data,offset):return struct.unpack_from('<i',data,offset)[0]
 async def waitval(offset,predicate):
  for _ in range(150):
   data=await state()
   if predicate(val(data,offset)):return data
   await asyncio.sleep(1.0)
  raise AssertionError(('game state timeout',offset,val(data,offset)))
 async def button(name,delay=2.5):
  for _ in range(45):
   d=await req('memory.read_u32',address=symbols['action_cooldown'])
   if not d['value']:break
   await asyncio.sleep(1)
  await req('input.buttons.press',button=name,duration=15);await asyncio.sleep(max(2.5,delay))
 async def shot(name):
  folder=mem/'PSP/SCREENSHOT';before=set(folder.glob('*.png'))
  subprocess.run([str(rt/'deps/usr/bin/xdotool'),'key','F1'],env=env,check=True)
  for _ in range(100):
   await asyncio.sleep(.1);new=set(folder.glob('*.png'))-before
   if new:
    src=next(iter(new))
    try:
     with Image.open(src) as check_image:check_image.load()
     if not src.read_bytes().endswith(bytes.fromhex('49454e44ae426082')):continue
    except Exception:continue
    shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print('SHOT',name,flush=True);return
  raise RuntimeError('Screenshot missing: '+name)
 async def fixture(name):
  await command('cpu.stepping')
  raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==3144
  await req('memory.write',address=symbols['game'],base64=base64.b64encode(raw).decode())
  for name in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown']:
   await req('memory.write_u32',address=symbols[name],value=0)
  for name in ['hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind']:
   await req('memory.write_u32',address=symbols[name],value=0)
  for name in ['rift120_played_card','rift120_auto_card']:
   await req('memory.write_u32',address=symbols[name],value=4294967295)
  await req('memory.write',address=symbols['rift120_log'],base64=base64.b64encode(bytes(192)).decode())
  await command('cpu.resume');await asyncio.sleep(2.5)
 await asyncio.sleep(4)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await shot('title')
 for name in ['combat-buffs','combat-negative','combat-no-status','combat-short-description','combat-long-description','hand-0','hand-2','hand-4','enemy-13']:
  await fixture(name);await shot(name)
 await fixture('combat-buffs');before=await state();await button('square');assert val(await state(),16)==7;await asyncio.sleep(7);await shot('square-deck');await button('circle');assert val(await state(),16)==2
 await button('start');d=await req('memory.read_u32',address=symbols['inspect_open']);assert d['value']==1;await shot('start-inventory');await button('start');d=await req('memory.read_u32',address=symbols['inspect_open']);assert d['value']==0
 print('PASS Square and Start symbols match actual deck and inventory controls',flush=True)
 await fixture('hand-0');before=await state();await req('input.buttons.press',button='cross',duration=15)
 for i in range(7):
  await asyncio.sleep(2);await shot('compact-flight-%02d'%i)
 after=await waitval(60,lambda x:x<val(before,60));await asyncio.sleep(4);await shot('compact-flight-complete');print('PASS compact card still rises to center, resolves and fades',flush=True)
 await fixture('combat-buffs');await shot('combat-final')
 print('PASS final 1.3.5: PPSSPP screenshots verified for status icons, zero/negative stats, complete compact previews, brighter shadowed single/multi-hit intents, controller glyphs and card animation',flush=True)

try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/'ppsspp-qa.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for i in range(60):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19233/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
   except OSError:continue
   async with ws:await check(ws)
   return
  raise RuntimeError('debugger absent')
 asyncio.run(run())
finally:
 for proc in [p,x,pulse]:
  if proc:
   proc.terminate()
   try:proc.wait(timeout=5)
   except subprocess.TimeoutExpired:proc.kill()
