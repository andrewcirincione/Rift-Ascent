"""Play the exact packaged EBOOT; save PPSSPP's own native-resolution screenshots."""
import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
import numpy as np
smoke=os.environ.get("RIFT_FINAL_SMOKE")=="1"
control_mode=os.environ.get('RIFT_CONTROL_AUDIT','')
repo=Path(__file__).resolve().parents[2];base=repo.parent;rt=base/'runtime';ev=repo/'qa/v162/evidence';cfg=rt/'qa162config';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True)
if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)
shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa162.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19464\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
modules=next((rt/'deps/usr/lib').glob('pulse-*/modules'))
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:464',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4864',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(modules))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':464','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(modules),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4864 auth-anonymous=1'],env=env,stdout=open(ev/'pulse.log','w'),stderr=subprocess.STDOUT)
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
    assert await val('rift_asset_errors')==0,('asset load error',name);shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print('SHOT',name,flush=True);return
  raise RuntimeError('Screenshot missing: '+name)
 async def fixture(name,overlay=None):
  await command('cpu.stepping');raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==layout["size"]
  await req('memory.write',address=symbols['game'],base64=base64.b64encode(raw).decode())
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active']:
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
  assert struct.unpack_from('<i',raw,16)[0]==20 or (await state())[:100]==raw[:100],('fixture changed before input',name)
 async def button(name):
  for _ in range(150):
   if await val('action_cooldown')==0 and (not await val('r161_transition_active') or await val('animation_tick')-await val('r161_transition_tick')>=48):break
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
 async def transition_button(key,label):
  await req('input.buttons.send',buttons={key:True});start=await val('animation_tick')
  for _ in range(300):
   if await val('animation_tick')>=start+8:break
   await asyncio.sleep(.1)
  await req('input.buttons.send',buttons={key:False})
  began=await val('r161_transition_tick');assert await val('r161_transition_active')
  for phase,age in enumerate([12,24,40,56]):
   for _ in range(600):
    if await val('animation_tick')>=began+age:break
    await asyncio.sleep(.1)
   await shot(label+'-'+str(phase))
  assert not await val('r161_transition_active')
 await asyncio.sleep(3)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 def field(raw,key,index=0):return struct.unpack_from('<i',raw,layout[key]+4*index)[0]
 await shot('boot')
 if control_mode!='resume':
  await button('square');await button('select');assert field(await state(),'screen')==10
  await button('circle');assert field(await state(),'screen')==0
 if control_mode=='resume':
  assert await val('resume_available')==1 and field(await state(),'screen')==0
  await button('cross');assert field(await state(),'screen')==2
  await shot('cold-resume');print('PASS cold Resume on complete 32 MiB PSP EBOOT',flush=True);return
 for name in ['costs','idle-0','idle-1','idle-2','new-run-cancel','new-run-replace','belt-full','rest-heal','rest-upgrade','endless-choice','pile-tabs','status-arrow','arrow-left','long-preview','upgrade-grid','upgrade-last-row','modify-grid','characters-0','characters-1','characters-2','blessing-1','blessing-2','blessing-3','blessing-4','reward-card','reward-taken','act-1','act-2','act-3','act-4']:
  await fixture(name);await shot(name)
 await fixture('rift-menu');await shot('rift-menu')
 for act in range(1,4):
  for stage in range(5):
   name=f'world-{act}-char-{(act+stage)%3}-fight-{stage}';await fixture(name);await shot(name)
  for kind in ['map','rest','shop']:
   name=f'world-{act}-{kind}';await fixture(name);await shot(name)
 await fixture('inventory-3',overlay=(3,0,0));await shot('inventory-tabs')
 await fixture('inventory-5',overlay=(5,0,0));await shot('inventory-deck');await button('right');assert await val('inspect_scroll')==1;await button('down');assert await val('inspect_scroll')==4;await shot('inventory-deck-selected')
 for id in range(6):
  await fixture(f'potion-{id}-take');await shot(f'potion-{id}-take')
  await button('down');assert field(await state(),'selection')==5;await shot(f'potion-{id}-leave')
  await button('up');assert field(await state(),'selection')==4
 await fixture('potion-0-take');before=field(await state(),'potion_count');await button('cross');assert field(await state(),'potion_count')==before+1 and field(await state(),'screen')==3
 await fixture('potion-0-take');before=field(await state(),'potion_count');await button('down');await button('cross');assert field(await state(),'potion_count')==before and field(await state(),'screen')==1
 # Actual potion shortcuts preserve the underlying reward/rest/combat cursor.
 for name in ['rest-heal','reward-card','status-arrow']:
  await fixture(name);before=await state();await button('triangle');assert await val('inspect_open')==1 and await val('inspect_tab')==3;await shot(name+'-potions');await button('circle');assert await val('inspect_open')==0 and await state()==before
 await fixture('upgrade-grid');await button('down');assert field(await state(),'selection')==5;await button('down');assert field(await state(),'selection')==6;await shot('upgrade-short-row-control')
 # Simulate a stale PSP file descriptor. A new texture must recover without
 # save/quit; subsequent screenshots and cache reads remain error-free.
 await fixture('status-arrow');recoveries=await val('rift_asset_recoveries');await setval('asset_fd',-1)
 for name in ['costs','upgrade-grid','blessing-4','long-preview']:
  await fixture(name);await shot('recovery-'+name)
 # Force an uncached bank image using a valid existing fixture after eviction.
 for name in ['inventory-3','potion-1-take','modify-grid','idle-1','idle-2','status-arrow']:
  await fixture(name)
 if await val('rift_asset_recoveries')==recoveries:
  await req('memory.write',address=symbols['entries'],base64=base64.b64encode(bytes(64*20)).decode())
  await req('memory.write',address=symbols['owners'],base64=base64.b64encode(bytes(64)).decode());await setval('cursor',0)
  await fixture('costs')
 assert await val('rift_asset_recoveries')>recoveries and await val('rift_asset_errors')==0
 await shot('recovered-combat')
 await fixture('play-2');before=field(await state(),'regent');await button('cross');assert field(await state(),'regent')==before+2;await shot('stars-gained')
 await button('select');await shot('pause');await transition_button('circle','pause-resume-fade');assert field(await state(),'screen')==2
 await button('select');await transition_button('cross','save-quit-fade');assert field(await state(),'screen')==0
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];await shot('saved-title')
 await button('down');await button('cross');assert field(await state(),'screen')==25;await shot('new-run-warning')
 await button('cross');assert field(await state(),'screen')==0 and (gamedir/'SAVE.DAT').read_bytes()[4:]==saved
 await button('cross');await button('down');await button('cross');assert field(await state(),'screen')==22
 await button('select');assert field(await state(),'screen')==22 and (gamedir/'SAVE.DAT').read_bytes()[4:]==saved
 await button('circle');await button('up');await transition_button('cross','resume-fade');assert (await state())==saved;await shot('resumed')
 print('PASS exact 1.6.2 EBOOT: stacked rewards, potion shortcuts, card grids, short last row controls, 6 potions, art-handle recovery, Stars, pause/save/resume',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)

try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/'ppsspp.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19464/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
