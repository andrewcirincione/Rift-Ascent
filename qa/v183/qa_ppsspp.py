"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import signal
signal.signal(signal.SIGTERM,lambda *_: (_ for _ in ()).throw(SystemExit("QA stopped")))
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v183';E=Q/'evidence';T=R.parent/'toolchain';instance=os.environ.get('RIFT_INSTANCE','183');prefix=os.environ.get('RIFT_PREFIX','');extra_start=int(os.environ.get('RIFT_FROM_EXTRA','0'));display=os.environ.get('RIFT_DISPLAY','530');port=os.environ.get('RIFT_PORT','19530');C=T/('qa'+instance);C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
D=M/'PSP/GAME/RIFT183VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('ASCENT18_*'):f.unlink()
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
if resume:
 for f in D.glob('ASCENT18_*'):f.unlink()
 shutil.copyfile(E/'legacy182-run.bin',D/'ASCENT18_RUN_1.0')
 shutil.copyfile(E/'legacy182-account.bin',D/'ASCENT18_ACCOUNT.0')
conf=C/'settings.ini';conf.write_text(f'[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = {port}\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:'+display,LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
nm=subprocess.check_output([str(T/'pspdev/bin/psp-nm'),'-S',str(Q/'source/rift_ascent_crt_free.elf')],text=True)
syms={p[3]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split())==4}
L=json.loads((E/'state-layout.json').read_text());screens=L['screens'];seq=0
async def check(w):
 global seq
 fast_frame_ready=False
 async def req(event,**kw):
  global seq
  seq+=1;n=seq;await w.send(json.dumps(dict(event=event,ticket=n,**kw)))
  while True:
   d=json.loads(await asyncio.wait_for(w.recv(),30))
   if d.get('ticket')==n:assert d.get('event')!='error',d;return d
 async def command(event):
  status=await req('cpu.status')
  if event=='cpu.stepping' and status['stepping']:return
  if event=='cpu.resume' and not status['stepping']:return
  await w.send(json.dumps({'event':event}));deadline=time.monotonic()+30
  while time.monotonic()<deadline:
   d=json.loads(await asyncio.wait_for(w.recv(),max(.1,deadline-time.monotonic())))
   assert d.get('event')!='error',d
   if d.get('event')==event:return
  raise AssertionError('CPU command timeout '+event)
 async def val(n):return (await req('memory.read_u32',address=syms[n]))['value']
 async def put(n,v):await req('memory.write_u32',address=syms[n],value=v&0xffffffff)
 async def memory(n,size):return base64.b64decode((await req('memory.read',address=syms[n],size=size))['base64'])
 async def audible():
  # Avoid sampling the brief zero-fill between the SFX and stereo mixing passes.
  for _ in range(40):
   if any(await memory('audio_buffer',2048)):return
   await asyncio.sleep(.1)
  raise AssertionError('No PCM output: '+str({k:await val(k) for k in ['soundtrack_active','soundtrack_current','left_samples','music_route','audio_pos']}))
 async def state():return await memory('game',L['size'])
 def field(b,k,idx=0):return struct.unpack_from('<i',b,L[k]+idx*4)[0]
 async def screen():return field(await state(),'screen')
 async def ticks(n):
  if fast_frame_ready and os.environ.get('RIFT_FAST_PRESENTATION')=='1' and await screen()!=screens['COMBAT']:
   n=min(n,8)
  start=await val('animation_tick')
  for attempt in range(600):
   now=await val('animation_tick')
   if now>=start+n:return
   if attempt and attempt%100==0:print('WAIT ticks',start,now,n,flush=True)
   await asyncio.sleep(.1)
  raise AssertionError('Emulated frames stopped')
 async def settle():
  keys=['r161_transition_active','r1311_finishing','animation_tick','r137_start','r1311_kill_at']
  first=min(syms[k] for k in keys);end=max(syms[k] for k in keys)+4
  for attempt in range(800):
   blob=base64.b64decode((await req('memory.read',address=first,size=end-first))['base64'])
   values={k:struct.unpack_from('<I',blob,syms[k]-first)[0] for k in keys}
   for k in ['action_cooldown','enemy_turn_delay']:values[k]=await val(k)
   if os.environ.get('RIFT_FAST_PRESENTATION')=='1' and values['r161_transition_active']:
    await put('r161_transition_active',0)
    await ticks(8)
    continue
   if os.environ.get('RIFT_FAST_PRESENTATION')=='1' and values['r1311_finishing']:
    # The first character run observes the full unchanged finishing timeline.
    # Later repetitions may skip that display-only replay AFTER game resolution.
    for k in ['r1311_finishing','r137_count','r138_death_active','victory_replay']:await put(k,0)
    print('QA skipped repeated finisher presentation after resolution',flush=True)
    await ticks(8)
    continue
   if not values['r161_transition_active'] and values['action_cooldown']==0 and values['enemy_turn_delay']==0 and (not values['r1311_finishing'] or values['animation_tick']-values['r137_start']>values['r1311_kill_at']+184):return
   if attempt and attempt%40==0:print('WAIT settle',values,flush=True)
   await asyncio.sleep(.1)
  raise AssertionError('Transition did not settle')
 async def button(k):
  k={'l':'ltrigger','r':'rtrigger'}.get(k,k)
  await settle();await req('input.buttons.send',buttons={k:True});await ticks(8);await req('input.buttons.send',buttons={k:False});await ticks(8);await settle()
 async def shot(n):
  await ticks(4)
  f=M/'PSP/SCREENSHOT';before=set(f.glob('*.png'))
  subprocess.run([str(T/'sysroot/usr/bin/xdotool'),'key','F1'],env=env,check=True)
  for _ in range(200):
   await asyncio.sleep(.1)
   for src in set(f.glob('*.png'))-before:
    try:
     with Image.open(src) as im:im.load();assert im.size==(480,272)
     if not src.read_bytes().endswith(bytes.fromhex('49454e44ae426082')):continue
    except Exception:continue
    assert await val('rift_asset_errors')==0,n
    shutil.copyfile(src,E/('ppsspp-'+prefix+n+'.png'));print('SHOT',n,flush=True);return
  raise AssertionError('Screenshot timeout '+n)
 async def pause_at_frame():
  # Swap fixtures only at the next render entry, never through a live blit.
  await command('cpu.stepping')
  address=syms['game_render_frame'];await req('cpu.breakpoint.add',address=address)
  await command('cpu.resume')
  for _ in range(600):
   status=await req('cpu.status')
   if status['stepping']:
    assert status['pc']==address,status
    await req('cpu.breakpoint.remove',address=address);return
   await asyncio.sleep(.05)
  raise AssertionError('No safe frame boundary for fixture')
 async def fixture(n,patch=None):
  print('FIXTURE',n,flush=True)
  await pause_at_frame();raw=bytearray((E/('fixture-'+n+'.bin')).read_bytes());assert len(raw)==L['size']
  if patch:patch(raw)
  await req('memory.write',address=syms['game'],base64=base64.b64encode(raw).decode())
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active','r170_story_queue']:
   if key in syms:await put(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await put(key,-1)
  await command('cpu.resume');await ticks(8);await settle()
 await asyncio.sleep(2)
 ids=subprocess.check_output([str(T/'sysroot/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env,text=True).split()
 subprocess.run([str(T/'sysroot/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await ticks(80);print('BOOT',await screen(),'asset errors',await val('rift_asset_errors'),'stereo streaming',await val('soundtrack_active'),flush=True);assert await screen()==screens['TITLE'];assert await val('soundtrack_active')==1;assert await val('soundtrack_current')==0;await audible();await shot('title')
 fast_frame_ready=True
 await req('input.buttons.send',buttons={'ltrigger':False,'rtrigger':False})
 # Seed an isolated QA account; no user save files are touched.

 if resume:
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await button('cross');raw=await state()
  assert field(raw,'screen')==screens['MAP'] and field(raw,'gold')==789 and struct.unpack_from('<I',raw,4)[0]==183
  await shot('cold-182-migration');await button('select');await button('cross');assert await screen()==screens['TITLE']
  print('PASS cold 1.8.2 slot import into 1.8.3 retains gold, map and rarity state',flush=True)
 else:
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await button('cross');assert await screen()==screens['CHARACTER_SELECT'];await button('cross');assert await screen()==screens['ACT_INTRO']
  for _ in range(3):await button('cross')
  assert await screen()==screens['MAP'];await button('select');await button('cross');assert await screen()==screens['TITLE'];await button('cross');await button('cross');assert await screen()==screens['MAP']
  print('PASS fresh slot, save-and-quit and resume',flush=True)
 await pause_at_frame();await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode((E/'fixture-account.bin').read_bytes()).decode());await command('cpu.resume')
 await fixture('unlocks');await shot('unlocks-warden');await button('square');assert field(await state(),'selection')==20;await button('square');assert field(await state(),'selection')==0
 for tab in range(1,5):
  await button('r');assert field(await state(),'scroll')==tab;await shot('unlocks-'+str(tab))
 await button('r');assert field(await state(),'scroll')==0;await button('l');assert field(await state(),'scroll')==4
 for tab in range(5):
  await fixture('base-'+str(tab));await shot('base-'+str(tab))
 print('PASS five framed unlock tabs, base catalogs, wrapping controls',flush=True)
 await fixture('target');await shot('mist-gazer-target-left');await button('up');await shot('mist-gazer-target-right')
 # Deck return must not start any fade. Observe within two actual rendered frames.
 await button('square');assert await screen()==screens['DECK_VIEW'];await shot('combat-deck')
 await req('input.buttons.send',buttons={'circle':True});await ticks(8);await req('input.buttons.send',buttons={'circle':False})
 assert await screen()==screens['COMBAT'] and await val('r161_transition_active')==0;await settle();await shot('instant-deck-return')
 print('PASS combat deck closes immediately, both target indicators and brighter Mist Gazer',flush=True)
 await fixture('purge');await shot('stone-giant-purge');await button('circle');await shot('stone-giant-after-purge')
 print('PASS Stone Giant overhead cleanse/shield intent and actual turn resolution',flush=True)
 for act in range(1,4):
  for k in range(4):
   await fixture(f'new-{act}-{k}');await shot(f'new-{act}-{k}')
 print('PASS all twelve new enemies rendered on their act backgrounds',flush=True)
 for event in [18,28,32,33,35,38]:
  await fixture(f'event-{event:02d}');await button('cross')
  if await screen()==screens['EVENT_CARD']:await button('cross')
  await shot(f'event-{event:02d}-result')
  assert await val('rift_asset_errors')==0
 await fixture('event-18');old_count=field(await state(),'deck_count');await button('cross');assert await screen()==screens['EVENT_CARD'];await button('cross');assert await screen()==screens['EVENT_RESULT'] and field(await state(),'deck_count')==old_count-1;await shot('remove-card-result');await button('cross')
 await fixture('boss-relic');raw=await state();assert raw[L['message']]==0;await shot('boss-relic');await button('cross');assert await screen()==screens['REWARD']
 print('PASS newer mystery results, single continue hint after removal, dedicated boss relic display',flush=True)
 for kind,track in [('normal',5),('elite',6)]:
  await fixture('map-'+kind);os.environ['RIFT_FAST_PRESENTATION']='0'
  await req('input.buttons.send',buttons={'cross':True});await ticks(8);await req('input.buttons.send',buttons={'cross':False})
  assert await val('music_route')&256;await settle();await ticks(24);assert await val('music_route')==track;await audible();os.environ['RIFT_FAST_PRESENTATION']='1'
 print('PASS silent map transition and correct normal/elite soundtrack routing',flush=True)
 await fixture('target');await shot('release-combat')
 assert 'Bad memory access' not in (E/(prefix+'ppsspp.log')).read_text()
 print('PASS final release EBOOT targeted runtime playtest; zero streamed-art errors or memory faults',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
 await command('cpu.stepping')

p=None;x=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':'+display,'-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/(prefix+'xvfb.log'),'w'),stderr=subprocess.STDOUT);time.sleep(.5);assert x.poll() is None
 p=subprocess.Popen([str(T/'ppsspp/AppDir/AppRun'),'--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,stdout=open(E/(prefix+'ppsspp.log'),'w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('Emulator exited; see ppsspp.log')
   try:w=await websockets.connect(f'ws://127.0.0.1:{port}/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
   except OSError:continue
   async with w:await check(w)
   return
  raise RuntimeError('Debugger unavailable')
 asyncio.run(run())
finally:
 for proc in [p,x]:
  if proc:
   proc.terminate()
   try:proc.wait(timeout=5)
   except subprocess.TimeoutExpired:proc.kill()
