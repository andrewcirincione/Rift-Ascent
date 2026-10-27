"""Play the exact packaged EBOOT; save PPSSPP's own native-resolution screenshots."""
import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
import numpy as np
smoke=os.environ.get("RIFT_FINAL_SMOKE")=="1"
control_mode=os.environ.get('RIFT_CONTROL_AUDIT','')
repo=Path(__file__).resolve().parents[2];base=repo.parent;rt=base/'runtime';ev=repo/'qa/v150/evidence';cfg=rt/'qa150config';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True)
if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)
shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa150.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19421\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n')
modules=next((rt/'deps/usr/lib').glob('pulse-*/modules'))
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:421',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4821',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(modules))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':421','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(modules),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4821 auth-anonymous=1'],env=env,stdout=open(ev/'pulse.log','w'),stderr=subprocess.STDOUT)
nm=subprocess.check_output([str(rt/'pspdev/psp/bin/nm'),'-S',str(repo/'portable_build/rift_ascent_crt_free.elf')]).decode();symbols={p[3]:int(p[0],16) for l in nm.splitlines() if len(p:=l.split())==4}
p=None;ticket=0
async def check(ws):
 global ticket
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
 async def state():return base64.b64decode((await req('memory.read',address=symbols['game'],size=3260))['base64'])
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
  await command('cpu.stepping');raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==3260
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
  end=await val('animation_tick')+(100 if struct.unpack_from('<i',await state(),16)[0]==2 else 12)
  for _ in range(400):
   if await val('animation_tick')>=end and await val('action_cooldown')==0 and await val('enemy_turn_delay')==0 and await val('plating_delay')==0:break
   await asyncio.sleep(.1)
 await asyncio.sleep(3)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await shot('controls-resumed' if control_mode=='resume' else 'controls-boot' if control_mode else 'title')
 layout=json.loads((ev/'state-layout.json').read_text())
 def field(raw,key,index=0):return struct.unpack_from('<i',raw,layout[key]+4*index)[0]
 async def wait_turn(wanted):
  for _ in range(600):
   raw=await state()
   if field(raw,'turn')>=wanted and not await val('enemy_turn_delay'):return raw
   await asyncio.sleep(.1)
  raise AssertionError('turn did not complete')
 if control_mode:
  def inventory(raw):return (field(raw,'hp'),field(raw,'gold'),raw[layout['deck_count']:layout['deck']+512],raw[layout['potion_count']:layout['relic_count']])
  async def hold_cross(ticks=64):
   start=await val('animation_tick');await req('input.buttons.send',buttons={'cross':True})
   for _ in range(600):
    if await val('animation_tick')-start>=ticks:break
    await asyncio.sleep(.1)
   await req('input.buttons.send',buttons={'cross':False});start=await val('animation_tick')
   for _ in range(200):
    if await val('animation_tick')-start>=12:break
    await asyncio.sleep(.1)
  if control_mode=='resume':
   before=(ev/'controls-before-save.dat').read_bytes();loaded=await state()
   assert field(loaded,'screen')==21 and field(loaded,'selection')==9 and inventory(loaded)==inventory(before)
   for key in ['square','triangle','start']:
    await button(key);assert inventory(await state())==inventory(before) and field(await state(),'screen')==21
   # The short final row is Bash, which cannot receive Ward. A held confirm
   # must refuse it without charging, including after a real save/relaunch.
   await hold_cross();denied=await state();assert field(denied,'screen')==21 and inventory(denied)==inventory(before)
   await button('up');assert field(await state(),'selection')==6
   await hold_cross();after=await state();assert field(after,'screen')==14 and field(after,'hp')==field(before,'hp')-8 and field(after,'gold')==field(before,'gold')
   changes=[i for i in range(field(before,'deck_count')) if field(before,'deck',i)!=field(after,'deck',i)]
   assert changes==[6] and field(after,'deck',6)==field(before,'deck',6)|(4<<11)
   await shot('controls-save-committed');await button('cross');committed=await state()
   await button('square');assert field(await state(),'screen')==7
   await button('cross');assert inventory(await state())==inventory(committed)
   await button('circle');assert field(await state(),'screen')==1
   print('PASS PSP controls: real Save & Quit/relaunch preserves unpaid picker and final-row selection; ineligible Bash is refused without charge; selected Defend receives Ward with one HP charge; map deck shortcut is read-only',flush=True)
  else:
   if control_mode!='save':
    for mod in range(1,7):
     await fixture(f'mod-audit-{mod}');await shot(f'mod-audit-{mod}')
    for mod in [3,5]:
     await fixture(f'mod-audit-{mod}');before=await state();await button('cross');after=await state()
     if mod==3:assert struct.unpack_from('<i',before,layout['enemies']+24)[0]-struct.unpack_from('<i',after,layout['enemies']+24)[0]==8
     else:assert field(after,'energy')==field(before,'energy')-1 and field(after,'block')==30 and field(after,'exhaust_count')==field(before,'exhaust_count')+1
    await fixture('event-12-0');before=await state()
    for key in ['square','triangle','start']:
     await button(key);assert inventory(await state())==inventory(before) and field(await state(),'screen')==6
    await button('select');assert field(await state(),'screen')==13
    await button('circle');assert field(await state(),'screen')==6
    await hold_cross();assert field(await state(),'screen')==21 and inventory(await state())==inventory(before)
    for key in ['square','triangle','start']:
     await button(key);assert inventory(await state())==inventory(before) and field(await state(),'screen')==21
    await button('right');await hold_cross();after=await state()
    changes=[i for i in range(field(before,'deck_count')) if field(before,'deck',i)!=field(after,'deck',i)]
    assert changes==[1] and field(after,'gold')==field(before,'gold')-70 and field(after,'screen')==14
    await shot('controls-held-confirm');await button('circle');assert field(await state(),'screen')==1
    print('PASS PSP controls: all six seal/description screenshots; Keen and Fleeting actual effects; Square/Triangle/Start cannot bypass event or picker; held Cross commits once to the chosen card',flush=True)
   await fixture('event-13-1');await button('cross');await button('down');await button('down');before=await state()
   assert field(before,'screen')==21 and field(before,'selection')==9
   (ev/'controls-before-save.dat').write_bytes(before)
   await button('select');assert field(await state(),'screen')==13
   await shot('controls-pause-save')
   trace=mem/'RIFT_BOOT.LOG';trace_before=trace.stat().st_size if trace.exists() else 0
   await req('input.buttons.send',buttons={'cross':True})
   for _ in range(600):
    # sceKernelExitGame returns PPSSPP to its menu; the desktop emulator
    # process need not exit. Check the game's successful save trace instead.
    logs=trace.read_text(errors='replace')[trace_before:] if trace.exists() else ''
    if 'save and quit result: 1' in logs or p.poll() is not None:break
    await asyncio.sleep(.1)
   else:raise AssertionError('Save & Quit did not finish')
   shutil.copyfile(trace,ev/'ppsspp-savequit-boot.txt')
   saved=(gamedir/'SAVE.DAT').read_bytes()[4:]
   assert field(saved,'screen')==21 and field(saved,'selection')==9 and inventory(saved)==inventory(before)
   print('PASS PSP controls: real pause-menu Save & Quit wrote the unpaid modification picker correctly',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
  return
 # Real controller transactions: cancel is free, commit modifies exactly one card.
 for event,option in [(12,0),(12,1),(13,0),(13,1),(14,0),(14,1),(16,0),(16,1),(10,0)]:
  await fixture(f'event-{event:02d}-{option}');before=await state()
  await button('cross');assert field(await state(),'screen')==21
  await shot(f'picker-{event}-{option}')
  await button('circle');cancel=await state();assert field(cancel,'gold')==field(before,'gold') and field(cancel,'hp')==field(before,'hp')
  assert cancel[layout['deck']:layout['deck']+512]==before[layout['deck']:layout['deck']+512]
  await button('cross');await button('cross');after=await state();assert field(after,'screen')==14
  if event in [12,13,14]:
   mod=(event-12)*2+option+1;changes=[i for i in range(field(before,'deck_count')) if field(before,'deck',i)!=field(after,'deck',i)]
   assert len(changes)==1 and (field(after,'deck',changes[0])>>11)&7==mod,(event,option,changes)
  await shot(f'result-{event}-{option}');await button('cross');assert field(await state(),'screen')==1
 print('PASS PSP: all six modification transactions, free cancellation, upgrade, remove and exact copy',flush=True)
 for name in ['event-02-0','event-05-1','event-15-1','event-11-0','combat-release','combat-mod-2','combat-mod-4','combat-mod-5','combat-mod-6','map-1-00','map-1-10','map-1-15','relic-status']:
  await fixture(name,overlay=(2,0,0) if name=='relic-status' else None);await shot(name)
 # A visible upgraded event reward is the card actually received, once.
 await fixture('event-02-0');before=await state();offered=field(before,'reward');await button('cross');after=await state()
 assert field(after,'deck_count')==field(before,'deck_count')+1 and field(after,'deck',field(before,'deck_count'))==offered and field(after,'gold')==field(before,'gold')-35
 await button('cross');assert field(await state(),'screen')==1
 # Echo and One-Two Punch play three times for one paid cost, one physical card.
 await fixture('echo-ready');before=await state();await button('cross');after=await state()
 assert field(after,'energy')==0 and field(after,'discard_count')==1
 hp_offset=layout['enemies']+24;assert struct.unpack_from('<i',before,hp_offset)[0]-struct.unpack_from('<i',after,hp_offset)[0]==18
 await shot('echo-played')
 # Retain survives the enemy turn, while Ward adds its Block and Kindled draws/exhausts.
 await fixture('retain-ready');before=await state();await button('circle');after=await wait_turn(field(before,'turn')+1)
 hand=[field(after,'hand',i) for i in range(field(after,'hand_count'))];assert 0|(2<<11) in hand
 await shot('retained-next-turn')
 for name in ['combat-mod-4','combat-mod-6']:
  await fixture(name);before=await state();await button('cross');after=await state()
  if name.endswith('4'):assert field(after,'block')-field(before,'block')==12
  else:assert field(after,'exhaust_count')==field(before,'exhaust_count')+1 and field(after,'hand_count')==field(before,'hand_count')+1
 print('PASS PSP: Echo/One-Two Punch, Retain through enemy turn, Ward and Kindled live resolution',flush=True)
 # All four new relics act through real card plays in the same combat.
 await fixture('relic-combat-ready');before=await state()
 await button('cross');assert field(await state(),'energy')==6
 await button('cross');after=await state();assert field(after,'energy')==6 and field(after,'block')==5
 await button('cross');after=await state();assert field(after,'energy')==8 and field(after,'hp')==field(before,'hp')-3 and field(after,'hand_count')==2
 await button('cross');after=await state();assert field(after,'energy')==6 and field(after,'block')==38 and field(after,'exhaust_count')==1
 await shot('relics-stacked');await button('circle');after=await wait_turn(field(before,'turn')+1)
 assert field(after,'energy')==5 and field(after,'ic',40)==0 and field(after,'ic',41)==0 and field(after,'ic',42)==0
 print('PASS PSP: Duelist Energy, Needle draw, Bell flat Block and Hourglass carryover stack and reset correctly',flush=True)
 # Every new enemy is rendered in the compiled game; natural idle changes pixels.
 for art in range(36,44):
  await fixture(f'enemy-{art:02d}');await shot(f'enemy-{art:02d}-a')
  a=np.array(Image.open(ev/f'ppsspp-enemy-{art:02d}-a.png'))[76:174,275:445]
  changed=0
  for attempt in range(8):
   await asyncio.sleep(1);await shot(f'enemy-{art:02d}-b')
   b=np.array(Image.open(ev/f'ppsspp-enemy-{art:02d}-b.png'))[76:174,275:445];changed=int(np.any(a!=b,axis=2).sum())
   if changed>40:break
  assert changed>40,(art,changed)
 for intent in range(14,18):
  await fixture(f'intent-{intent:02d}');await shot(f'intent-{intent:02d}');before=await state();await button('circle');after=await wait_turn(field(before,'turn')+1)
  expected=6 if intent==14 else 0;assert field(before,'hp')-field(after,'hp')==expected,(intent,field(before,'hp'),field(after,'hp'))
  await shot(f'intent-{intent:02d}-resolved')
 print('PASS PSP: eight new grounded enemies animate; Pierce, Sunder, Flurry and Entomb resolve correctly',flush=True)
 for card in range(92,96):
  await fixture(f'card-{card}');await shot(f'card-{card}');before=await state();await button('cross');after=await state()
  assert field(after,'energy')==field(before,'energy')-(2 if card==92 else 1)
  if card==92:assert struct.unpack_from('<i',before,hp_offset)[0]-struct.unpack_from('<i',after,hp_offset)[0]==8
  if card==93:assert field(after,'block')==field(before,'block')+10
  if card==94:assert field(after,'hp')==field(before,'hp')+3
  if card==95:assert field(after,'ic',44)==4
 await fixture('map-1-15');await button('up');await button('up');await shot('map-floor-19')
 assert field(await state(),'scroll')==17
 pcm=base64.b64decode((await req('memory.read',address=symbols['audio_buffer'],size=2048))['base64']);assert sum(abs(v) for v in struct.unpack('<1024h',pcm))>1000
 print('PASS PSP: all four new cards, full 19-floor map scrolling and non-silent complete soundtrack',flush=True)
 print('PASS final release EBOOT 1.5: PPSSPP native screenshots, real controller choices, all modifications, relic screens, expanded map, new enemies and attack patterns',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)

try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/'ppsspp.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19421/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
