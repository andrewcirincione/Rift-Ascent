"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import signal
signal.signal(signal.SIGTERM,lambda *_: (_ for _ in ()).throw(SystemExit("QA stopped")))
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v184beta1';E=Q/'evidence';T=Path(os.environ.get('RIFT_TOOLCHAIN','/workspace/toolchain'));instance=os.environ.get('RIFT_INSTANCE','184r2');prefix=os.environ.get('RIFT_PREFIX','');extra_start=int(os.environ.get('RIFT_FROM_EXTRA','0'));display=os.environ.get('RIFT_DISPLAY','601');port=os.environ.get('RIFT_PORT','19601');C=T/('qa'+instance);C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\nFast-forward = 1-132\n')
D=M/'PSP/GAME/RIFT184VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('BETA184_*'):f.unlink()
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
if resume:
 for f in D.glob('BETA184_*'):f.unlink()
 shutil.copyfile(E/'legacy182-run.bin',D/'ASCENT18_RUN_1.0')
 shutil.copyfile(E/'legacy182-account.bin',D/'ASCENT18_ACCOUNT.0')
conf=C/'settings.ini';conf.write_text(f'[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = {port}\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\n[Graphics]\nInternalResolution = 1\nVerticalSync = False\nFrameSkip = 8\nAutoFrameSkip = True\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:'+display,LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',SDL_VIDEO_X11_XSHM='0',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
nm=subprocess.check_output([str(T/'pspdev/bin/psp-nm'),'-S',str(Q/'source/rift_ascent_crt_free.elf')],text=True)
syms={p[-1]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split()) in (3,4)}
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
 async def val(n):return (await req('cpu.evaluate',expression=f'[0x{syms[n]:08x}]'))['uintValue']
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
 async def screen():return (await req('cpu.evaluate',expression=f'[0x{syms["game"]+L["screen"]:08x}]'))['uintValue']
 async def ticks(n):
  if (E/'STOP_QA').exists():raise SystemExit('QA stopped by operator')
  if fast_frame_ready and os.environ.get('RIFT_FAST_PRESENTATION')=='1' and await screen()!=screens['COMBAT']:
   n=min(n,8)
  start=await val('animation_tick')
  for attempt in range(600):
   now=await val('animation_tick')
   if now>=start+n:return
   if attempt and attempt%100==0:print('WAIT ticks',start,now,n,await req('cpu.status'),flush=True)
   await asyncio.sleep(.1)
  raise AssertionError('Emulated frames stopped')
 async def settle():
  keys=['r161_transition_active','r1311_finishing','animation_tick','r137_start','r1311_kill_at','r137_duration','r137_count','r138_death_active','r137_last_screen']
  first=min(syms[k] for k in keys);end=max(syms[k] for k in keys)+4
  for attempt in range(800):
   values={k:await val(k) for k in keys}
   for k in ['action_cooldown','enemy_turn_delay','plating_delay']:values[k]=await val(k)
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
   fx_end=values['r137_duration']
   if values['r137_last_screen'] in [screens[k] for k in ['REWARD','RELIC_RESULT','VICTORY','DEFEAT']]:fx_end=max(fx_end,30)+10
   fx_busy=(values['r137_count'] or values['r138_death_active']) and values['animation_tick']-values['r137_start']<fx_end
   if not values['r161_transition_active'] and values['action_cooldown']==0 and values['enemy_turn_delay']==0 and values['plating_delay']==0 and not fx_busy and (not values['r1311_finishing'] or values['animation_tick']-values['r137_start']>values['r1311_kill_at']+184):return
   if attempt and attempt%40==0:print('WAIT settle',values,flush=True)
   await asyncio.sleep(.1)
  raise AssertionError('Transition did not settle')
 async def button(k):
  k={'l':'ltrigger','r':'rtrigger'}.get(k,k)
  await settle();await req('input.buttons.send',buttons={k:True});await ticks(8);await req('input.buttons.send',buttons={k:False});await ticks(8)
  # Card plays intentionally resolve after five rendered presentation frames.
  # That countdown is a local in main(), not an exported debugger symbol.
  if k=='cross' and await screen()==screens['COMBAT']:await ticks(24)
  await settle()
 async def shot(n):
  await ticks(36)
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
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','pending_card_ticks','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active','r170_story_queue']:
   if key in syms:await put(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await put(key,-1)
  await command('cpu.resume');await ticks(8);await settle()
 async def latest_checks():
  for event in [1,30]:
   await fixture('full-heal-'+str(event));before=field(await state(),'hp');await shot('full-health-event-'+str(event));await button('cross');raw=await state();assert field(raw,'screen')==screens['EVENT_RESULT'] and field(raw,'hp')==field(raw,'maxhp') and field(raw,'hp')==before-(5 if event==30 else 0);await button('cross');assert await screen()==screens['MAP']
  await fixture('braced-strike');await shot('braced-strike-intent');await button('circle');raw=await state();assert field(raw,'hp')==74 and struct.unpack_from('<i',raw,L['enemies']+32)[0]==6;await shot('braced-strike-after')
  print('PASS full-health healing choices and actual simultaneous enemy attack/block with both overhead icons',flush=True)
  rates=[]
  for fast in [0,1]:
   await fixture('options');await put('ascent_fast_setting',1-fast);await button('cross');assert await val('ascent_fast_setting')==fast;await shot('speed-'+str(fast))
   await fixture('braced-strike');os.environ['RIFT_FAST_PRESENTATION']='0';await pause_at_frame();start_tick=await val('animation_tick')
   for frame in range(6):
    await command('cpu.resume');await pause_at_frame()
   end_tick=await val('animation_tick');rates.append(end_tick-start_tick);await command('cpu.resume')
  assert rates==[24,72],rates
  print('PASS Normal/Fast options, persistent setting, exact 3x combat presentation clock:',rates,flush=True)
  os.environ['RIFT_FAST_PRESENTATION']='1'
 await asyncio.sleep(2)
 ids=subprocess.check_output([str(T/'sysroot/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env,text=True).split()
 subprocess.run([str(T/'sysroot/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 subprocess.run([str(T/'sysroot/usr/bin/xdotool'),'keydown','F2'],env=env,check=True)
 for _ in range(240):
  try:
   await val('animation_tick');break
  except AssertionError as error:
   if 'CPU not started' not in str(error):raise
   await asyncio.sleep(.25)
 if os.environ.get('RIFT_BOOT_ONLY')=='1':
  start=await val('animation_tick');await asyncio.sleep(10);print('BOOT BENCHMARK ticks in ten seconds:',(await val('animation_tick'))-start,flush=True);return
 await ticks(16)
 for _ in range(600):
  if await val('soundtrack_active')==1 and await val('soundtrack_current')==10:break
  await asyncio.sleep(.1)
 print('BOOT',await screen(),'asset errors',await val('rift_asset_errors'),'stereo streaming',await val('soundtrack_active'),'track',await val('soundtrack_current'),flush=True);assert await screen()==screens['TITLE'];assert await val('soundtrack_active')==1;assert await val('soundtrack_current')==10;await audible();await shot('title')
 if os.environ.get('RIFT_BOOT_ONLY')=='1':
  start=await val('animation_tick');await asyncio.sleep(5);print('BOOT BENCHMARK ticks in five seconds:',(await val('animation_tick'))-start,flush=True);return
 fast_frame_ready=True
 await fixture('recover');await shot('recover-intent')
 assert field(await state(),'hp')==80
 await button('circle');raw=await state();assert field(raw,'hp')==80
 assert struct.unpack_from('<i',raw,L['enemies']+44)[0]==1
 await shot('recover-opening');print('PASS Recover: no damage and exactly one Vulnerable turn exposed',flush=True)
 for i in range(4):
  await fixture('boss-'+str(i));await shot('boss-'+str(i)+'-intent')
  await button('start');await button('left');await shot('boss-'+str(i)+'-cycle');await button('circle')
 await fixture('forge');await shot('late-spoils')
 await button('down');await button('cross');assert await screen()==screens['BETA_FORGE'];await shot('forge-menu')
 await button('cross');assert await screen()==screens['BETA_FORGE_CARD'];await shot('forge-upgrade-preview')
 await button('select');await button('cross');assert await screen()==screens['TITLE']
 await button('cross');await button('cross');assert await screen()==screens['BETA_FORGE_CARD']
 await button('cross');raw=await state();assert field(raw,'screen')==screens['REWARD'] and field(raw,'deck')&512
 await shot('forge-upgrade-complete');print('PASS upgrade choice, result preview and mid-choice Save & Quit/resume',flush=True)
 await fixture('forge');hp=field(await state(),'maxhp')
 await button('down');await button('cross');await button('down');await button('down');await button('cross')
 await shot('forge-echo-preview');await button('circle');assert field(await state(),'maxhp')==hp
 await button('cross');await button('cross');raw=await state();assert field(raw,'maxhp')==hp-4 and (field(raw,'deck')>>13)&7==1
 await shot('forge-echo-complete');print('PASS Echo cancel costs nothing; confirming spends exactly 4 max HP and replaces one card pick',flush=True)
 await fixture('death');await button('circle');assert await screen()==screens['DEFEAT'];await shot('recap-death')
 raw=await state();assert b'HOLLOW KING' in raw[L['subtitle']:L['subtitle']+72]
 await button('square');await shot('recap-deck');await button('right');await shot('recap-deck-selected')
 await button('triangle');await shot('recap-relics');await button('down');await shot('recap-relic-selected')
 await button('circle');await button('cross');assert await screen()==screens['TITLE']
 await button('cross');await button('cross');assert await screen()!=screens['COMBAT'];print('PASS death recap, final deck/relic browsing, killing enemy attribution and retired run cannot resume',flush=True)
 await fixture('victory');await shot('recap-victory');await button('square');await button('circle');await button('cross')
 assert field(await state(),'act')==4;print('PASS victory recap retains endless continuation',flush=True)
 await fixture('release');await shot('release-combat')
 assert 'Bad memory access' not in (E/(prefix+'ppsspp.log')).read_text()
 print('PASS final release EBOOT: beta Recover, boss pattern inspection, forge upgrade/Echo/cancel/save-resume, defeat/victory recap and endless continuation; zero streamed-art errors or memory faults',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
 await command('cpu.stepping')

p=None;x=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':'+display,'-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/(prefix+'xvfb.log'),'w'),stderr=subprocess.STDOUT);time.sleep(.5);assert x.poll() is None
 launch=[str(T/'squashfs-root/lib/ld-linux-x86-64.so.2'),'--library-path',str(T/'squashfs-root/lib'),str(T/'squashfs-root/shared/bin/PPSSPPSDL')] if os.environ.get('RIFT_DIRECT')=='1' else [str(T/'squashfs-root/AppRun')]
 p=subprocess.Popen(launch+['--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,stdout=open(E/(prefix+'ppsspp.log'),'w'),stderr=subprocess.STDOUT)
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
