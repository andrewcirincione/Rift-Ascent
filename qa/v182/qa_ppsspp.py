"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v182';E=Q/'evidence';T=R.parent/'toolchain';instance=os.environ.get('RIFT_INSTANCE','182');prefix=os.environ.get('RIFT_PREFIX','');extra_start=int(os.environ.get('RIFT_FROM_EXTRA','0'));display=os.environ.get('RIFT_DISPLAY','519');port=os.environ.get('RIFT_PORT','19519');C=T/('qa'+instance);C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
D=M/'PSP/GAME/RIFT182VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('ASCENT18_*'):f.unlink()
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
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
  print('PASS cold restart of Act 7 cutscene into exact boss fight; saved run retained',flush=True)
  def release_combat(raw):
   struct.pack_into('<i',raw,L['hp'],80);struct.pack_into('<i',raw,L['maxhp'],80)
  await fixture('visual-0-1',release_combat);await shot('release-combat')
  assert 'Bad memory access' not in (E/(prefix+'ppsspp.log')).read_text()
  print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);await command('cpu.stepping');return
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
 if os.environ.get('RIFT_STAGE','all') in ('all','revision','final'):
  for c in (range(3) if not extra_start else []):
   for act in range(1,4):
    await fixture(f'visual-{c}-{act}');await shot(f'actors-{c}-act-{act}');assert await val('rift_asset_errors')==0
  for id in ([12,16] if not extra_start else []):
   await fixture(f'visual-extra-{id}');await shot(f'actor-extra-{id}')
  for event in ([6,17,18,19,20,21,22,23] if not extra_start else []):
   await fixture(f'event-{event:02d}');await shot(f'event-{event:02d}')
   if event in [18,19,22]:
    await button('down');await button('down');assert field(await state(),'selection')==2;await shot(f'event-{event:02d}-third')
   if event==18:
    await button('cross');assert await screen()==screens['EVENT_CARD'];await shot('event-third-picker');await button('circle');assert await screen()==screens['EVENT'] and field(await state(),'selection')==2
   await button('circle');assert await screen() in [screens['MAP'],screens['EXPEDITION_STORY']]
  for event in [n for n in [24,25,28,32,33,34,37,38] if n>=extra_start]:
   await fixture(f'event-{event:02d}');await shot(f'extra-event-{event:02d}');before=await state()
   await button('cross')
   if event in [24,25,33,37]:
    assert await screen()==screens['EVENT_CARD'];await shot(f'extra-picker-{event:02d}')
    await button('circle');assert await screen()==screens['EVENT']
    assert field(await state(),'gold')==field(before,'gold') and field(await state(),'potion_count')==field(before,'potion_count')
    await button('cross');await button('cross')
   expected=screens['RELIC_RESULT'] if event==32 else screens['EVENT_RESULT']
   assert await screen()==expected,(event,await screen(),expected);after=await state();await shot(f'extra-result-{event:02d}')
   if event==33:assert field(after,'gold')==field(before,'gold')-20 and field(after,'potion_count')==1
   if event==37:assert field(after,'deck')!=field(before,'deck') and field(after,'deck_count')==field(before,'deck_count')
   await button('cross');assert await screen()==screens['MAP']
   if event in [28,38]:
    await button('cross');assert await screen()==screens['COMBAT'];raw=await state()
    assert field(raw,'strength')==(3 if event==28 else 2)
    assert field(raw,'vulnerable')==(2 if event==28 else 0)
    assert field(raw,'block')==(0 if event==28 else 10)
    assert not any(raw[L['revision182']+16:L['revision182']+36]);await shot(f'extra-boon-combat-{event:02d}')
  if extra_start<39:print('PASS extra mysteries: controller choices, card picker cancellation/commit, potion payment, transform, relic/card gifts and next-fight Strength/Block/Vulnerable',flush=True)
  if extra_start<39:
   await fixture('mystery-0')
   for _ in range(3):await button('cross')
   await shot('exchange-selected-left');await button('right');await shot('exchange-selected-middle');assert field(await state(),'selection')==1
  for kind,track in [('normal',5),('elite',6)]:
   await fixture('map-'+kind);os.environ['RIFT_FAST_PRESENTATION']='0'
   await req('input.buttons.send',buttons={'cross':True})
   await ticks(8)
   await req('input.buttons.send',buttons={'cross':False})
   sample={k:await val(k) for k in ['animation_tick','music_route','r182_combat_transition','r161_transition_active','r161_transition_tick','r161_transition_length']}
   sample['screen']=await screen();print('TRANSITION',kind,sample,flush=True)
   assert sample['music_route']&256,(kind,sample)
   await shot('transition-'+kind);await settle();assert await screen()==screens['COMBAT']
   await ticks(24);assert await val('music_route')==track;await audible();await shot('music-'+kind)
   print('PASS silent map transition and correct fight track',kind,track,sample,flush=True)
   os.environ['RIFT_FAST_PRESENTATION']='1'
  await fixture('map-1');await button('select');await button('cross');assert await screen()==screens['TITLE']
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await shot('save-slots');await button('circle')
  await button('down');await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await button('cross')
  if await screen()!=screens['CHARACTER_SELECT']:
   await button('down');await button('cross')
  assert await screen()==screens['CHARACTER_SELECT'];await shot('character-select');await button('right');await shot('character-select-vesper');await button('circle')
  if not extra_start:print('PASS new actor shadows/art on all act backgrounds and seven event variants',flush=True)
  print('PASS save selection and character dimming; Exchange verified when included in this stage',flush=True)
 if os.environ.get('RIFT_STAGE','all') in ('all','god','final'):
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
