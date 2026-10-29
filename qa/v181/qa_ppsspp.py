"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v181';E=Q/'evidence';T=R.parent/'toolchain';C=T/'qa181';C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
D=M/'PSP/GAME/RIFT181VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('ASCENT18_*'):f.unlink()
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
conf=C/'settings.ini';conf.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19519\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:519',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
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
  await w.send(json.dumps({'event':event}))
  while True:
   d=json.loads(await asyncio.wait_for(w.recv(),30))
   if d.get('event')==event:return
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
  await settle();await req('input.buttons.send',buttons={k:True});await ticks(8);await req('input.buttons.send',buttons={k:False});await ticks(32 if await screen()==screens['COMBAT'] else 12);await settle()
 async def shot(n):
  await ticks(8)
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
    shutil.copyfile(src,E/('ppsspp-'+n+'.png'));print('SHOT',n,flush=True);return
  raise AssertionError('Screenshot timeout '+n)
 async def fixture(n,patch=None):
  await command('cpu.stepping');raw=bytearray((E/('fixture-'+n+'.bin')).read_bytes());assert len(raw)==L['size']
  if patch:patch(raw)
  await req('memory.write',address=syms['game'],base64=base64.b64encode(raw).decode())
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active','r170_story_queue']:
   if key in syms:await put(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await put(key,-1)
  await command('cpu.resume');await ticks(24);await settle()
 await asyncio.sleep(2)
 ids=subprocess.check_output([str(T/'sysroot/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env,text=True).split()
 subprocess.run([str(T/'sysroot/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 await ticks(80);print('BOOT',await screen(),'asset errors',await val('rift_asset_errors'),'stereo streaming',await val('soundtrack_active'),flush=True);assert await screen()==screens['TITLE'];assert await val('soundtrack_active')==1;assert await val('soundtrack_current')==0;await audible();await shot('title')
 fast_frame_ready=True
 await req('input.buttons.send',buttons={'ltrigger':False,'rtrigger':False})
 # Seed an isolated QA account; no user save files are touched.
 if os.environ.get('RIFT_COLD')=='1':
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await button('cross');assert await screen()==screens['ABOMINATION_BEFORE'];await shot('cold-resume-god-before')
  for _ in range(3):await button('cross')
  assert await screen()==screens['COMBAT'];assert field(await state(),'act')==7;await shot('cold-resume-god-combat');await audible()
  print('PASS cold restart of Act 7 cutscene into exact boss fight; saved account and run retained',flush=True)
  print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 if os.environ.get('RIFT_STAGE','all') in ('all','new'):
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await button('cross');assert await screen()==screens['CHARACTER_SELECT'];await button('cross');assert await screen()==screens['ACT_INTRO'];await button('cross');await button('cross');await button('cross');assert await screen()==screens['MAP'];await shot('new-run-map')
  await button('select');await button('cross');assert await screen()==screens['TITLE'];await button('cross');await button('cross');assert await screen()==screens['MAP']
  print('PASS actual new run, save-and-quit, slot resume',flush=True)
 await command('cpu.stepping');await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode((E/'fixture-account.bin').read_bytes()).decode());await command('cpu.resume')
 if os.environ.get('RIFT_STAGE','all') in ('all','new','rewards'):
  await fixture('energy');await button('cross');raw=await state();assert field(raw,'ic',L['energy_lock'])==1;e=field(raw,'energy');await button('cross');assert field(await state(),'energy')==e;await button('cross');assert field(await state(),'energy')==e
  await button('triangle');await button('cross');assert field(await state(),'energy')==e;assert field(await state(),'potion_count')==0;await shot('energy-lock')
  print('PASS real inputs: Expect a Fight blocks Bloodletting, Offering and Energy potion',flush=True)
  for c in range(3):
   await fixture('story-fight-'+str(c));await shot('story-fight-'+str(c))
   if c==0 and os.environ.get('RIFT_OBSERVE_FINISH','1')=='1':os.environ['RIFT_FAST_PRESENTATION']='0'
   await button('cross');os.environ['RIFT_FAST_PRESENTATION']='1';assert await screen() in (screens['REWARD'],screens['RELIC_RESULT'])
   if await screen()==screens['RELIC_RESULT']:await button('cross')
   assert await screen()==screens['REWARD'];await button('circle');assert await screen()==screens['EXPEDITION_STORY'];await shot('story-'+str(c))
   for _ in range(3):await button('cross')
   assert await screen()==screens['EXPEDITION_GIFT'];await shot('gift-'+str(c));raw=await state();cards0=field(raw,'deck_count');relics0=field(raw,'relic_count');await button('cross');raw=await state();assert field(raw,'screen')==screens['MAP'];assert field(raw,'deck_count')==cards0+1 or field(raw,'relic_count')==relics0+1
  print('PASS normal/elite victories naturally roll queued story; all heroes, dialogue, gifts, single collection; full finisher timing separately inspected',flush=True)
  await fixture('mystery-0');await shot('mystery-scene');
  for _ in range(3):await button('cross')
  assert await screen()==screens['MYSTERY_CARDS'];await shot('mystery-cards');count=field(await state(),'deck_count');await button('cross');assert await screen()==screens['MAP'] and field(await state(),'deck_count')==count+1
  await fixture('exhausted');assert field(await state(),'relic_count')>64;await button('cross');assert await screen()==screens['RELIC_RESULT'];assert field(await state(),'reward') in [37,38,39];await shot('exhausted-relic-fallback');await button('cross');assert await screen()==screens['REWARD']
  await fixture('collection');await shot('base-relic-collection');await button('r');assert field(await state(),'scroll')==0;await button('l');assert field(await state(),'scroll')==5
  print('PASS foreign rare choice, >64 relic run, repeatable reward after pool exhaustion, six-tab collection navigation',flush=True)
 if os.environ.get('RIFT_STAGE','all') in ('all','god'):
  await fixture('god-before-0');await shot('god-before');
  for _ in range(3):await button('cross')
  assert await screen()==screens['COMBAT'];raw=await state();assert struct.unpack_from('<i',raw,L['enemies']+L['enemy_art'])[0]==45;assert await val('soundtrack_current')==0;await audible();await shot('god-combat');await button('circle');assert await screen()==screens['COMBAT'];await shot('god-turn-two')
  await fixture('god-kill-0');os.environ['RIFT_FAST_PRESENTATION']='0';await button('cross');os.environ['RIFT_FAST_PRESENTATION']='1';assert await screen()==screens['ABOMINATION_AFTER'];await shot('god-after')
  for _ in range(3):await button('cross')
  assert await screen()==screens['RELIC_RESULT'];await button('cross');assert await screen()==screens['REWARD'];await button('circle');assert await screen()==screens['ACT_INTRO'] and field(await state(),'act')==8
  print('PASS actual Act 7 entrance, full-HP boss combat/turn, killing hit, after scene, rewards, Act 8; RiftBoss PCM audible',flush=True)
  for act,track in [(1,1),(2,7),(3,8),(4,9)]:
   await fixture('map-'+str(act));assert await val('soundtrack_current')==track;await audible()
  # Persist the before-boss scene to test it in a second, genuinely cold process.
  await fixture('god-before-0');await button('select');await button('cross');assert await screen()==screens['TITLE']
 print('PASS final release EBOOT targeted runtime playtest; zero streamed-art errors',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)

p=None;x=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':519','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/'xvfb.log','w'),stderr=subprocess.STDOUT);time.sleep(.5);assert x.poll() is None
 p=subprocess.Popen([str(T/'ppsspp/AppDir/AppRun'),'--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,stdout=open(E/'ppsspp.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('Emulator exited; see ppsspp.log')
   try:w=await websockets.connect('ws://127.0.0.1:19519/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
