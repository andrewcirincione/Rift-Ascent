"""Play the exact packaged EBOOT; save PPSSPP's own native-resolution screenshots."""
import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
import numpy as np
smoke=os.environ.get("RIFT_FINAL_SMOKE")=="1"
control_mode=os.environ.get('RIFT_CONTROL_AUDIT','')
repo=Path(__file__).resolve().parents[2];base=repo.parent;rt=base/'runtime';ev=repo/'qa/v150b/evidence';cfg=rt/'qa150bpublishconfig';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True)
if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)
shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa150bpublish.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19426\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n')
modules=next((rt/'deps/usr/lib').glob('pulse-*/modules'))
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:426',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4826',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(modules))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':426','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(modules),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4826 auth-anonymous=1'],env=env,stdout=open(ev/'pulse.log','w'),stderr=subprocess.STDOUT)
nm=subprocess.check_output([str(rt/'pspdev/psp/bin/nm'),'-S',str(repo/'portable_build/rift_ascent_crt_free.elf')]).decode();symbols={p[3]:int(p[0],16) for l in nm.splitlines() if len(p:=l.split())==4}
p=None;ticket=0
async def check(ws):
 global ticket
 layout=json.loads((ev/'state-layout.json').read_text())
 async def req(event,**kw):
  global ticket
  ticket+=1;t=ticket;await ws.send(json.dumps(dict(event=event,ticket=t,**kw)))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),20))
   if d.get('ticket')==t:
    assert d['event']!='error',d
    return d
 async def command(event):
  await ws.send(json.dumps({'event':event}))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),20))
   if d.get('event')==event:return
 async def val(name):return (await req('memory.read_u32',address=symbols[name]))['value']
 async def setval(name,value):await req('memory.write_u32',address=symbols[name],value=value&0xffffffff)
 async def state():return base64.b64decode((await req('memory.read',address=symbols['game'],size=layout["size"]))['base64'])
 async def shot(name):
  folder=mem/'PSP/SCREENSHOT';before=set(folder.glob('*.png'))
  subprocess.run([str(rt/'deps/usr/bin/xdotool'),'key','F1'],env=env,check=True)
  for _ in range(150):
   await asyncio.sleep(.1);new=set(folder.glob('*.png'))-before
   if new:
    src=next(iter(new))
    try:
     with Image.open(src) as im:im.load();assert im.size==(480,272),im.size
     if not src.read_bytes().endswith(bytes.fromhex('49454e44ae426082')):continue
    except Exception:continue
    shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print('SHOT',name,flush=True);return
  raise RuntimeError('Screenshot missing: '+name)
 async def fixture(name,overlay=None):
  await command('cpu.stepping');raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==layout["size"]
  await req('memory.write',address=symbols['game'],base64=base64.b64encode(raw).decode())
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind']:
   if key in symbols:await setval(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await setval(key,-1)
  await req('memory.write',address=symbols['r1310_potion_notice'],base64=base64.b64encode(bytes(72)).decode())
  if overlay:
   await setval('inspect_open',1);await setval('inspect_tab',overlay[0]);await setval('inspect_scroll',overlay[1]);await setval('inspect_potion',overlay[2])
  start=await val('animation_tick');await command('cpu.resume')
  # Wait for complete game frames, not wall time. Software-rendered PSP frames
  # can take longer than two seconds, and the display is double buffered.
  for _ in range(600):
   if await val('animation_tick')-start>=24:break
   await asyncio.sleep(.1)
  else:raise AssertionError('fixture did not render: '+name)
  assert (await state())[:100]==raw[:100],('fixture changed before input',name)
 async def button(name):
  for _ in range(150):
   if await val('action_cooldown')==0:break
   await asyncio.sleep(.2)
  # Hold/release across actual game frames so slow software rendering cannot
  # miss the key edge or leave the previous press logically held.
  start=await val('animation_tick');await req('input.buttons.send',buttons={name:True})
  for _ in range(300):
   if await val('animation_tick')-start>=8:break
   await asyncio.sleep(.1)
  await req('input.buttons.send',buttons={name:False});start=await val('animation_tick')
  for _ in range(300):
   if await val('animation_tick')-start>=8:break
   await asyncio.sleep(.1)
  # Allow the five-frame card ascent plus the short contact timeline to settle.
  end=await val('animation_tick')+(32 if struct.unpack_from('<i',await state(),16)[0]==2 else 12)
  for _ in range(400):
   if await val('animation_tick')>=end and await val('action_cooldown')==0 and await val('enemy_turn_delay')==0 and await val('plating_delay')==0:break
   await asyncio.sleep(.1)
 await asyncio.sleep(3)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await shot('boot')
 def field(raw,key,index=0):return struct.unpack_from('<i',raw,layout[key]+4*index)[0]
 if control_mode=='resume':
  before=await state();assert field(before,'screen')==23 and field(before,'character')==1
  await shot('resumed-choice');await button('cross');after=await state()
  assert field(after,'screen')==2 and field(after,'energy')==field(before,'energy')+1
  await shot('resumed-sly-resolved')
  print('PASS final release EBOOT: pending Sly choice resumes and resolves once',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 await button('cross');assert field(await state(),'screen')==22
 await button('right');assert field(await state(),'selection')==1
 await shot('character-selection')
 await button('cross')
 for _ in range(600):
  if field(await state(),'screen')==18:break
  await asyncio.sleep(.1)
 assert field(await state(),'character')==1
 assert field(await state(),'hp')==70
 await button('cross');assert field(await state(),'screen')==1
 await shot('new-run-map')
 for name in ['combat','poison','shivs','sly-choice','rewards','reward-preview','shop']:
  await fixture(name);await shot(name)
 await fixture('sly-choice');before=await state()
 for key in ['circle','square','triangle','start']:
  await button(key);after=await state();assert field(after,'screen')==23 and field(after,'hand_count')==field(before,'hand_count')
 await button('cross');after=await state();assert field(after,'screen')==2 and field(after,'energy')==field(before,'energy')+1
 await shot('sly-resolved')
 await fixture('shivs');before=await state();await button('cross');after=await state()
 assert field(after,'hand_count')==field(before,'hand_count')+2 and field(after,'exhaust_count')==field(before,'exhaust_count')+1
 await shot('generated-shivs')
 await fixture('attack');before=await state();await button('cross');after=await state();assert field(after,'energy')==field(before,'energy')-1
 await shot('after-attack')
 await fixture('block');before=await state();await button('cross');after=await state();assert field(after,'block')==field(before,'block')+7
 await shot('after-block')
 await fixture('power');await button('cross');assert field(await state(),'hand_count')==0;await shot('after-power')
 await fixture('retain');await button('circle');assert field(await state(),'screen')==23;await shot('retain-choice');await button('circle');assert field(await state(),'turn')==2
 await fixture('status',overlay=(1,0,0));await shot('enemy-status')
 await fixture('status',overlay=(2,0,0));await shot('player-status')
 await fixture('status',overlay=(2,5,0));await shot('active-powers')
 await fixture('status',overlay=(4,0,0));await shot('relic-inventory')
 await fixture('sly-choice');await button('select');assert field(await state(),'screen')==13
 await button('circle');assert field(await state(),'screen')==23
 await button('select')
 try:
  await button('cross')
  raise AssertionError('Save & Quit did not exit the game')
 except AssertionError as error:
  assert 'CPU not started' in str(error),error
 assert (gamedir/'SAVE.DAT').exists()
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];assert field(saved,'screen')==23
 (ev/'ppsspp-pending-save.dat').write_bytes((gamedir/'SAVE.DAT').read_bytes())
 print('PASS final release EBOOT 1.5b: character selection, new run, combat, poison, Shivs, Sly controls, Retain, powers, Block, attacks, rewards, shop, status, inventory and pending-choice Save & Quit',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)

try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/'ppsspp.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19426/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
