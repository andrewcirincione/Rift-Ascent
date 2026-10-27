"""Play the exact packaged EBOOT; save PPSSPP's own native-resolution screenshots."""
import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
import numpy as np
smoke=os.environ.get("RIFT_FINAL_SMOKE")=="1"
repo=Path(__file__).resolve().parents[2];base=repo.parent;rt=base/'runtime';ev=repo/'qa/v140/evidence';cfg=rt/'qa140config';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True);(gamedir/'SAVE.DAT').unlink(missing_ok=True);shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa140.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19421\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n')
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
 async def state():return base64.b64decode((await req('memory.read',address=symbols['game'],size=3196))['base64'])
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
  await command('cpu.stepping');raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==3196
  await req('memory.write',address=symbols['game'],base64=base64.b64encode(raw).decode())
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind']:
   if key in symbols:await setval(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await setval(key,-1)
  await req('memory.write',address=symbols['r1310_potion_notice'],base64=base64.b64encode(bytes(72)).decode())
  if overlay:
   await setval('inspect_open',1);await setval('inspect_tab',overlay[0]);await setval('inspect_scroll',overlay[1]);await setval('inspect_potion',overlay[2])
  await command('cpu.resume');await asyncio.sleep(2)
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
 await asyncio.sleep(3)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await shot('title')
 if os.environ.get('RIFT_SHOP_ONLY')=='1':
  # Buy repeatedly using the real shop controller. Every slot must be preserved.
  for repeat in range(1 if smoke else 4):
   await fixture('shop-purchase-ready')
   await command('cpu.stepping')
   await req('memory.write_u32',address=symbols['game']+12,value=140+repeat*641)
   await command('cpu.resume')
   prior=[]
   for n in range(3):
    before=await state();await button('cross');after=await state()
    count=struct.unpack_from('<i',after,100)[0];pots=list(struct.unpack_from('<3i',after,104))[:count]
    assert count==n+1 and pots[:n]==prior and struct.unpack_from('<i',after,68)[0]==struct.unpack_from('<i',before,68)[0]-50
    prior=pots
    if repeat==0:await shot(f'shop-purchase-{n+1}')
   before=await state();await button('cross');after=await state()
   assert after[100:116]==before[100:116] and after[68:72]==before[68:72]
   if repeat==0:
    assert await val('r140_potion_gained') in [-1,0xffffffff], (await val('r140_potion_gained'),after[2940:3012])
    await shot('shop-belt-full-notice')
   await button('triangle');assert struct.unpack_from('<i',await state(),16)[0]==17
   if repeat==0:await shot('shop-full-belt')
   await button('circle');assert struct.unpack_from('<i',await state(),16)[0]==5
  print('PASS',3 if smoke else 12,'real shop purchases: prior slots retained, three-slot capacity, full-belt refusal without charge, belt browse/return',flush=True)
  return
 for name in (['combat','bully-preview','ashen-preview','no-energy','low-energy','boss-rewards','map-chest','map-routes-1-0','map-routes-1-6','map-routes-2-12','endless-choice'] if smoke else ['combat','hand-middle','hand-left','camp-rest','camp-upgrade','potions','relic-orb','potion-reward-0','potion-reward-5','shop-orb','map-chest','boss-rewards','endless-choice','menu-choice','card-top-48','card-top-84','bully-preview','ashen-preview','no-energy','low-energy','map-routes-1-0','map-routes-1-6','map-routes-2-12']):
  await fixture(name);await shot(name)
 # Actual controller navigation, then verify state changed.
 await fixture('camp-rest');await button('right');assert struct.unpack_from('<i',await state(),36)[0]==1;await shot('camp-controller')
 await fixture('combat');await button('start')
 await button('right')
 await shot('inventory-potions')
 await button('right');await shot('inventory-relics')
 # State contains the items of the native inventory fixture; open through real Start.
 await fixture('inventory-potions');await button('start')
 await button('right')
 await button('down');await shot('inventory-potions-selected')
 await button('right');await button('down');await shot('inventory-relics-selected')
 # Reach the incomplete final row from the right column with real D-pad input.
 for name,last in [('upgrade-short-row',9),('remove-short-row',9),('discard-short-row',6)]:
  await fixture(name);await button('down');assert struct.unpack_from('<i',await state(),36)[0]==last
  await shot(name+'-selected')
  if name=='upgrade-short-row':
   await button('cross');after=await state();assert struct.unpack_from('<i',after,16)[0]==1 and struct.unpack_from('<i',after,284+9*4)[0]==(35|128)
 print('PASS real D-pad Down reaches short final row in upgrade/removal/discard; correct Bully upgraded',flush=True)
 # Buy repeatedly using the real shop controller. Every slot must be preserved.
 for repeat in range(1 if smoke else 4):
  await fixture('shop-purchase-ready')
  await command('cpu.stepping')
  await req('memory.write_u32',address=symbols['game']+12,value=140+repeat*641)
  await command('cpu.resume')
  prior=[]
  for n in range(3):
   before=await state();await button('cross');after=await state()
   count=struct.unpack_from('<i',after,100)[0];pots=list(struct.unpack_from('<3i',after,104))[:count]
   assert count==n+1 and pots[:n]==prior and struct.unpack_from('<i',after,68)[0]==struct.unpack_from('<i',before,68)[0]-50
   prior=pots
   if repeat==0:await shot(f'shop-purchase-{n+1}')
  before=await state();await button('cross');after=await state()
  assert after[100:116]==before[100:116] and after[68:72]==before[68:72]
  if repeat==0:
   assert await val('r140_potion_gained') in [-1,0xffffffff], (await val('r140_potion_gained'),after[2940:3012])
   await shot('shop-belt-full-notice')
  await button('triangle');assert struct.unpack_from('<i',await state(),16)[0]==17
  if repeat==0:await shot('shop-full-belt')
  await button('circle');assert struct.unpack_from('<i',await state(),16)[0]==5
 print('PASS',3 if smoke else 12,'real shop purchases: prior slots retained, three-slot capacity, full-belt refusal without charge, belt browse/return',flush=True)
 # Map potion access: healing with Violet Orb, no combat potion consumption.
 await fixture('map-potions-ready');await button('triangle');await shot('map-potion-belt')
 await button('cross');after=await state();assert struct.unpack_from('<i',after,48)[0]==48 and struct.unpack_from('<i',after,100)[0]==1
 await shot('map-potion-used');before=after;await button('cross');after=await state();assert after[100:116]==before[100:116] and after[80:84]==before[80:84]
 await button('circle');assert struct.unpack_from('<i',await state(),16)[0]==1;await shot('map-after-healing')
 print('PASS map Triangle potion belt, +23 HP with Violet Orb, consumption of only Healing, other potions retained, return to map',flush=True)
 # Normal victory keeps possessions and starts Act IV; alternative returns to title.
 await fixture('endless-choice');before=await state();await button('cross');after=await state()
 assert struct.unpack_from('<i',after,24)[0]==4 and struct.unpack_from('<i',after,3188)[0]==1
 assert after[104:280]==before[104:280]
 await shot('continued-endless')
 await fixture('menu-choice');await button('cross');assert struct.unpack_from('<i',await state(),16)[0]==0
 print('PASS compiled normal-victory choices: Continue Endless at Act IV with belongings, or Return to Menu',flush=True)
 # Insufficient energy must keep the selected card, hand, energy and enemy HP.
 await fixture('no-energy');before=await state();await button('cross');after=await state()
 assert before[:284]==after[:284] and before[284:2808]==after[284:2808]
 await shot('no-energy-denied')
 print('PASS compiled no-energy feedback and rejection without spending or discarding',flush=True)
 # Actual heavy sword swing; the resolver and impact timing are unchanged.
 await fixture('heavy-ready');await req('input.buttons.send',buttons={'cross':True})
 for _ in range(300):
  if await val('rift120_played_card') not in [-1,0xffffffff]:break
  await asyncio.sleep(.1)
 await req('input.buttons.send',buttons={'cross':False});await shot('sword-windup')
 for _ in range(300):
  if await val('r137_count')>0:break
  await asyncio.sleep(.1)
 duration=await val('r137_duration');assert duration==24,duration
 await shot('sword-impact');await asyncio.sleep(1);await shot('sword-follow-through')
 print('PASS new heavy sword sprites in actual attack; duration remains 24 ticks',flush=True)
 # All enemy idle phases are sampled in the compiled PSP renderer.
 for art in ([28] if smoke else [0,5,9,16,19,28,35]):
  await fixture(f'idle-{art:02d}-0');await shot(f'idle-{art:02d}-a')
  a=np.array(Image.open(ev/f'ppsspp-idle-{art:02d}-a.png'))[85:174,265:440]
  first_tick=await val('animation_tick');changed=0
  for attempt in range(10):
   await asyncio.sleep(1.5);await shot(f'idle-{art:02d}-b')
   b=np.array(Image.open(ev/f'ppsspp-idle-{art:02d}-b.png'))[85:174,265:440]
   changed=int(np.any(a!=b,axis=2).sum())
   if changed>100:break
  assert changed>100,(art,first_tick,await val('animation_tick'),changed)
  print('PASS natural idle motion',art,'changed sprite-region pixels',changed,'ticks',first_tick,await val('animation_tick'),flush=True)
 # Play a power with the actual PSP controller and verify shortened FX duration.
 await fixture('power-ready');await req('input.buttons.send',buttons={'cross':True})
 for _ in range(200):
  if await val('r137_count')>0:break
  await asyncio.sleep(.1)
 await req('input.buttons.send',buttons={'cross':False})
 assert await val('r137_duration')==16;await shot('power-cast')
 print('PASS compiled Power duration: 16 ticks / four rendered frames',flush=True)
 # Final contact pauses music; Victory art dispatches one independent chime.
 await fixture('finisher');initial=await val('victory_serial');await req('input.buttons.send',buttons={'cross':True})
 paused=False;chime=False
 for _ in range(500):
  paused|=bool(await val('music_paused'))
  if await val('victory_serial')==initial+1:
   chime=True;await req('input.buttons.send',buttons={'cross':False});await shot('victory');break
  await asyncio.sleep(.1)
 assert paused and chime,(paused,chime)
 for _ in range(500):
  if not await val('music_paused') and struct.unpack_from('<i',await state(),16)[0]==3:break
  await asyncio.sleep(.1)
 assert await val('victory_serial')==initial+1;await shot('victory-rewards')
 print('PASS final contact music pause; generated Victory art; exactly one chime; prompt rewards',flush=True)
 pcm=base64.b64decode((await req('memory.read',address=symbols['audio_buffer'],size=2048))['base64']);samples=struct.unpack('<1024h',pcm);assert sum(abs(v) for v in samples)>1000
 print('PASS non-silent compiled PSP audio',flush=True)
 print('PASS final release EBOOT 1.4.0: PPSSPP screenshots, controller navigation, power timing, enemy idles, Victory/chime/audio and reward transition',flush=True)
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
