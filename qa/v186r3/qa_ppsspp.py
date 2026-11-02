"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import signal,re
signal.signal(signal.SIGTERM,lambda *_: (_ for _ in ()).throw(SystemExit("QA stopped")))
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v186';E=R/'qa/v186r3/evidence';T=Path(os.environ.get('RIFT_TOOLCHAIN','/workspace/toolchain'));instance=os.environ.get('RIFT_INSTANCE','186r3');prefix=os.environ.get('RIFT_PREFIX','');extra_start=int(os.environ.get('RIFT_FROM_EXTRA','0'));display=os.environ.get('RIFT_DISPLAY','601');port=os.environ.get('RIFT_PORT','19601');C=T/('qa'+instance);C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\nFast-forward = 1-132\n')
D=M/'PSP/GAME/RIFT186R3VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('ASCENT18_*'):f.unlink()
assert (E/'EBOOT.PBP').read_bytes()[-160:-152]==b'RIFTAU19', 'Incomplete package: soundtrack footer missing'
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
assert hashlib.sha256((D/'EBOOT.PBP').read_bytes()).digest()==hashlib.sha256((E/'EBOOT.PBP').read_bytes()).digest()
if resume:
 for f in D.glob('ASCENT18_*'):f.unlink()
 shutil.copyfile(E/'legacy182-run.bin',D/'ASCENT18_RUN_1.0')
 shutil.copyfile(E/'legacy182-account.bin',D/'ASCENT18_ACCOUNT.0')
conf=C/'settings.ini';conf.write_text(f'[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = {port}\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\n[Graphics]\nInternalResolution = 1\nVerticalSync = False\nFrameSkip = 8\nAutoFrameSkip = True\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:'+display,LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',SDL_VIDEO_X11_XSHM='0',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
nm=subprocess.check_output([str(T/'pspdev/bin/psp-nm'),'-S',str(Q/'source/rift_ascent_crt_free.elf')],text=True)
syms={p[-1]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split()) in (3,4)}
L=json.loads((E/'state-layout.json').read_text());screens=L['screens'];seq=0
# Derive screen constants from the same source used for this EBOOT.
names=re.search(r'enum \{(TITLE,[^}]+)\}',(Q/'source/game.h').read_text()).group(1).split(',');screens.update({name:i for i,name in enumerate(names)})
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
  account=(E/('account-'+n+'.bin')).read_bytes();assert len(account)==L['account_size']
  await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode(account).decode())
  await req('memory.write',address=syms['ascent_memory'],base64=base64.b64encode(bytes(20)).decode())
  await put('ascent_memory_generation',0)
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','pending_card_ticks','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active','r170_story_queue']:
   if key in syms:await put(key,0)
  for key in ['rift120_played_card','rift120_auto_card','r140_potion_gained']:await put(key,-1)
  await command('cpu.resume');await ticks(8);await settle()
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
 os.environ['RIFT_FAST_PRESENTATION']='0'
 async def evaluate(expression):return (await req('cpu.evaluate',expression=expression))['uintValue']
 async def until_break(address):
  for _ in range(600):
   status=await req('cpu.status')
   if status['stepping']:
    assert status['pc']==address,status
    return
   await asyncio.sleep(.01)
  raise AssertionError('Frame boundary timeout')
 async def frame_result():
  # Entry is the outer game_render_frame call. Stop at its actual MIPS
  # return, then read the completed framebuffer before it is presented.
  address=await evaluate('ra');pointer=await evaluate('a1');stride=await evaluate('a2');tick=await evaluate('a3')
  assert stride==512 and pointer in (0x44000000,0x44088000),(hex(pointer),stride)
  await req('cpu.breakpoint.add',address=address);await command('cpu.resume');await until_break(address);await req('cpu.breakpoint.remove',address=address)
  pixels=base64.b64decode((await req('memory.read',address=pointer,size=stride*272*4))['base64'])
  picture=Image.frombytes('RGBA',(stride,272),pixels).crop((0,0,480,272)).convert('RGB')
  return tick,picture
 async def next_frame():
  address=syms['game_render_frame'];await req('cpu.breakpoint.add',address=address);await command('cpu.resume');await until_break(address);await req('cpu.breakpoint.remove',address=address)
 async def sequence(act,fast,leader_last):
  label='act'+str(act)+'-'+('fast' if fast else 'normal')+('-leader-last' if leader_last else '-minion-last')
  def patch(raw):
   if leader_last:
    for e in range(1,field(raw,'enemy_count')):struct.pack_into('<i',raw,L['enemies']+e*L['enemy_size']+L['enemy_hp'],0)
   else:
    struct.pack_into('<i',raw,L['enemies']+L['enemy_hp'],0)
    for e in range(1,field(raw,'enemy_count')-1):struct.pack_into('<i',raw,L['enemies']+e*L['enemy_size']+L['enemy_hp'],0)
  await fixture('finish'+str(act),patch);await put('ascent_fast_setting',fast);await pause_at_frame();await req('input.buttons.send',buttons={'cross':True})
  rows=[];pictures=[];started=False;release=False
  for index in range(100):
   tick,picture=await frame_result();finishing=await val('r1311_finishing');replay=await val('victory_replay');transition=await val('r161_transition_active');raw=await state()
   age=tick-await val('r137_start');kill_at=await val('r1311_kill_at');finish_end=kill_at+172
   luminosity=sum(sum(p) for p in picture.get_flattened_data());rows.append(dict(frame=index,tick=tick,age=age,kill_at=kill_at,finish_end=finish_end,screen=field(raw,'screen'),finishing=finishing,replay=replay,transition=transition,brightness=luminosity));pictures.append(picture)
   if finishing:
    started=True;assert not transition,(label,rows[-1])
    if not release:await req('input.buttons.send',buttons={'cross':False});release=True
    if age<finish_end-12:assert sum(sum(p) for p in picture.crop((0,0,480,17)).get_flattened_data())>0,(label,rows[-1])
    if age==finish_end:assert luminosity==0,(label,rows[-1])
    if age>=finish_end+12:
     assert not replay and luminosity>1000000 and field(raw,'screen')==screens[{3:'ASCENT_ENDING',4:'ASCENT_TRUE_END',7:'ABOMINATION_AFTER'}[act]],(label,rows[-1])
     break
   await next_frame()
  else:raise AssertionError('Ending did not finish '+label)
  assert started and await val('rift_asset_errors')==0
  folder=E/label;folder.mkdir(exist_ok=True)
  for index,picture in enumerate(pictures):picture.save(folder/('%03d.png'%index))
  (E/(label+'.json')).write_text(json.dumps(rows,indent=2)+'\n')
  # A normalized frame-sequence video is for visual inspection, not a wall-clock benchmark.
  subprocess.run(['ffmpeg','-y','-loglevel','error','-framerate','30','-i',str(folder/'%03d.png'),'-c:v','libx264','-pix_fmt','yuv420p',str(E/(label+'.mp4'))],check=True)
  picture.save(E/(label+'-ending.png'))
  selected=[0]+[next((i for i,r in enumerate(rows) if r['finishing'] and r['age']>=t),len(rows)-1) for t in [0,16,156,164,172,176,184]]
  board=Image.new('RGB',(960,544*2))
  for k,i in enumerate(selected):board.paste(pictures[i],((k%2)*480,(k//2)*272))
  board.save(E/(label+'-sequence.png'))
  print('PASS exact EBOOT sequence',label,'frames',len(rows),'one final fade, no generic fade/return, cutscene revealed; zero art errors',flush=True)
  await command('cpu.resume');await settle()
 if os.environ.get('RIFT_FADE_ONLY')=='1':
  for act,fast,leader_last in [(3,0,True),(3,0,False),(3,1,True),(4,0,True),(7,0,False),(7,1,False)]:await sequence(act,fast,leader_last)
 else:
  # Route/cost checks skip repeated presentation only after live resolution.
  # RIFT_FADE_ONLY separately captures every unmodified rendered frame.
  os.environ['RIFT_FAST_PRESENTATION']='1'
  for c in range(3):
   for prior in [0,8,9,10]:
    label=f'ending-{c}-{prior}'
    await fixture(label);await put('ascent_fast_setting',0);await button('cross');assert await screen()==screens['ASCENT_ENDING']
    account=await memory('ascent_account',L['account_size']);assert struct.unpack_from('<I',account,L['account_wins']+4*c)[0]==prior+1
    if prior in (0,9):await shot(label+'-resolution-1')
    await button('cross');await button('cross')
    if prior in (0,9):await shot(label+'-resolution-3')
    await button('cross');assert await screen()!=screens['ASCENT_REVELATION']
    for _ in range(3):await button('cross')
    assert await screen()==(screens['ASCENT_REVELATION'] if prior>=9 else screens['VICTORY']),(c,prior,await screen())
    if prior==9:await shot(label+'-secret-reveal')
    print('PASS exact PSP ending gate character',c,'win',prior+1,'secret',prior>=9,flush=True)
  for act in [1,3,5]:
   await fixture('forge-'+str(act));before=await state();await shot('forge-'+str(act))
   await button('square');assert await val('inspect_tab')==5;await shot('forge-deck-'+str(act));await button('circle');assert await state()==before
   await button('start');assert await val('inspect_open')==1;await button('circle');assert await state()==before
   await button('cross');assert await screen()==screens['BETA_FORGE_CARD'];await shot('forge-preview-'+str(act));await button('circle');assert field(await state(),'maxhp')==120
   await button('cross');await button('cross');after=await state();assert field(after,'maxhp')==120-max(0,5-act) and (field(after,'deck')>>13)&7==1
   print('PASS exact PSP Forge quote, preview, deck/inventory, cancel, Echo payment: Act',act,flush=True)
  for act in [1,3,5]:
   await fixture('gift-'+str(act));await shot('gift-'+str(act));before=await state();n=field(before,'deck_count');await button('cross');after=await state();assert field(after,'deck_count')==n+1
  await fixture('relic-3');await shot('relic-3');before=field(await state(),'relic_count');await button('cross');assert field(await state(),'relic_count')==before+1
  await fixture('mystery-forge-3');await shot('mystery-forge-3');await button('cross');assert await screen()==screens['EVENT_CARD'];await button('circle');assert field(await state(),'gold')==200;await button('cross');await button('cross');assert field(await state(),'gold')==144 and await screen()==screens['EVENT_RESULT']
  await fixture('mystery-cards-3');await shot('mystery-cards-3')
  await fixture('foreign-5');await shot('foreign-5');before=field(await state(),'deck_count');await button('cross');assert field(await state(),'deck_count')==before+1
  await fixture('combat-4');await shot('release-combat');await audible()
 assert await val('rift_asset_errors')==0
 assert 'Bad memory access' not in (E/(prefix+'ppsspp.log')).read_text()
 print('PASS final release EBOOT:', 'all Normal/Fast full-frame ending sequences' if os.environ.get('RIFT_FADE_ONLY')=='1' else 'all character first/ninth/tenth/eleventh ending routes, reward claims, Forge quotes/preview/inventory/cancel/payment', '; full soundtrack, zero art or memory errors',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);await command('cpu.stepping')

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
