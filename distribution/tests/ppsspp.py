"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import signal,re
signal.signal(signal.SIGTERM,lambda *_: (_ for _ in ()).throw(SystemExit("QA stopped")))
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/v186';E=R/'distribution/evidence';T=Path(os.environ.get('RIFT_TOOLCHAIN','/workspace/toolchain'));instance=os.environ.get('RIFT_INSTANCE','186r4');prefix=os.environ.get('RIFT_PREFIX','');extra_start=int(os.environ.get('RIFT_FROM_EXTRA','0'));display=os.environ.get('RIFT_DISPLAY','601');port=os.environ.get('RIFT_PORT','19601');C=T/('qa'+instance);C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\nFast-forward = 1-132\n')
D=M/'PSP/GAME/RIFT_DIST_VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_COLD')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume and os.environ.get('RIFT_KEEP')!='1':
 for f in D.glob('RIFTD186_*'):f.unlink()
assert (E/'EBOOT.PBP').read_bytes()[-160:-152]==b'RIFTAU19', 'Incomplete package: soundtrack footer missing'
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
assert hashlib.sha256((D/'EBOOT.PBP').read_bytes()).digest()==hashlib.sha256((E/'EBOOT.PBP').read_bytes()).digest()
if resume:
 for f in D.glob('RIFTD186_*'):f.unlink()
 shutil.copyfile(E/'legacy182-run.bin',D/'RIFTD186_RUN_1.0')
 shutil.copyfile(E/'legacy182-account.bin',D/'RIFTD186_ACCOUNT.0')
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
  if fast_frame_ready:
   # F1 may sample a software backbuffer while the next frame is clearing it.
   # Read only after the complete game_render_frame return instead.
   await pause_at_frame();tick,picture=await frame_result()
   if await screen()==screens['ASCENT_FINALE']:
    manifest=json.loads((Q/'assets.json').read_text());item=next(a for a in manifest['assets'] if a['name']=='ascent_finale_art')
    bank=(R/'distribution/resources/art.bank').read_bytes();rgb=[]
    for (p,) in struct.iter_unpack('<H',bank[item['offset']:item['offset']+item['bytes']]):
     r=p>>11;g=(p>>5)&63;b=p&31;rgb.extend([(r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2)])
    assert picture.crop((72,38,408,164)).tobytes()==bytes(rgb),'Finale art mismatch '+n
   assert await val('rift_asset_errors')==0,n
   picture.save(E/('ppsspp-'+prefix+n+'.png'));await command('cpu.resume');print('SHOT completed frame',n,tick,flush=True);return
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
  for f in D.glob('RIFTD186_*'):f.unlink()
  await req('memory.write',address=syms['game'],base64=base64.b64encode(raw).decode())
  account=(E/('account-'+n+'.bin')).read_bytes();assert len(account)==L['account_size']
  await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode(account).decode())
  await req('memory.write',address=syms['ascent_memory'],base64=base64.b64encode(bytes(20)).decode())
  await put('ascent_memory_generation',0)
  finale=(E/('finale-'+n+'.bin')).read_bytes();assert len(finale)==20
  await req('memory.write',address=syms['ascent_finale'],base64=base64.b64encode(finale).decode())
  await put('ascent_finale_generation',0)
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
  for _ in range(1800):
   status=await req('cpu.status')
   if status['stepping']:
    assert status['pc']==address,status
    return
   await asyncio.sleep(.01)
  raise AssertionError('Frame boundary timeout '+str(await req('cpu.status'))+' expected '+hex(address))
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
 async def finale_sequence():
  os.environ['RIFT_FAST_PRESENTATION']='1'
  await fixture('order-0-2');await put('ascent_fast_setting',0);await button('cross');await button('cross');await button('cross')
  assert await screen()==screens['ASCENT_TRUE_END'] and field(await state(),'selection')==2
  os.environ['RIFT_FAST_PRESENTATION']='0'
  await pause_at_frame();await req('input.buttons.send',buttons={'cross':True})
  pictures=[];rows=[];started=False;black=False
  for index in range(40):
   tick,picture=await frame_result();transition=await val('r161_transition_active');scene=await screen();age=tick-await val('r161_transition_tick')
   luminosity=sum(sum(p) for p in picture.get_flattened_data());rows.append(dict(frame=index,tick=tick,screen=scene,transition=transition,age=age,brightness=luminosity));pictures.append(picture)
   assert scene in [screens['ASCENT_TRUE_END'],screens['ASCENT_FINALE']]
   if transition and not started:started=True;await req('input.buttons.send',buttons={'cross':False})
   if started and age==16:assert luminosity==0;black=True
   if started and not transition and scene==screens['ASCENT_FINALE']:assert luminosity>1000000;break
   await next_frame()
  else:raise AssertionError('Shared finale transition did not finish')
  assert started and black and await val('rift_asset_errors')==0
  folder=E/'act4-shared-finale';folder.mkdir(exist_ok=True)
  for i,picture in enumerate(pictures):picture.save(folder/('%03d.png'%i))
  (E/'act4-shared-finale.json').write_text(json.dumps(rows,indent=2)+'\n')
  subprocess.run(['ffmpeg','-y','-loglevel','error','-framerate','30','-i',str(folder/'%03d.png'),'-c:v','libx264','-pix_fmt','yuv420p',str(E/'act4-shared-finale.mp4')],check=True)
  board=Image.new('RGB',(960,1088))
  for k in range(8):board.paste(pictures[k*(len(pictures)-1)//7],((k%2)*480,(k//2)*272))
  board.save(E/'act4-shared-finale-sequence.png')
  await command('cpu.resume');await settle();await shot('shared-finale-destination')
  print('PASS exact PSP Act 4 individual ending to one-time shared finale:',len(rows),'completed frames, black midpoint, correct shared destination, no combat return',flush=True)
 if os.environ.get('RIFT_FADE_ONLY')=='1':
  for act,fast,leader_last in [(3,0,True),(3,0,False),(3,1,True),(4,0,True),(7,0,False),(7,1,False)]:await sequence(act,fast,leader_last)
  await finale_sequence()
 else:
  # Assisted positions exercise real resolution/input; frame checks run separately.
  os.environ['RIFT_FAST_PRESENTATION']='1'
  phase=os.environ.get('RIFT_PHASE','gates')
  async def account_value(name,c=0):return struct.unpack_from('<I',await memory('ascent_account',L['account_size']),L[name]+4*c)[0]
  async def finale_value(index):return struct.unpack_from('<I',await memory('ascent_finale',20),index*4)[0]
  async def click_n(n):
   for _ in range(n):await button('cross')
  if phase=='gates':
   for c in range(3):
    for prior in [3,4,9,10]:
     label=f'gate-{c}-{prior}'
     await fixture(label);await button('cross');assert await screen()==screens['ASCENT_ENDING']
     assert await account_value('account_wins',c)==prior+1
     assert (await account_value('account_cards',c)).bit_count()==min(prior+1,10)*2
     if prior in (3,4):await shot(label+'-resolution')
     await click_n(3);assert await screen()==screens['ASCENT_CARDS']
     if prior in (4,9,10):await shot(label+'-unlocks')
     await click_n(3);assert await screen()==(screens['ASCENT_REVELATION'] if prior>=4 else screens['VICTORY'])
     if prior==4:await shot(label+'-revelation')
     print('PASS exact PSP character',c,'Act 3 win',prior+1,'secret',prior>=4,'unlocked cards',min(prior+1,10)*2,flush=True)
   for wins in [4,5]:
    await fixture('badges-'+str(wins));await shot('badges-'+str(wins));assert await account_value('account_ascended')==0
   await fixture('combat');await shot('release-combat');await audible()
  elif phase=='cards':
   async def compare(c,step):
    actual=await state();expected=(E/f'fixture-cards-{c}-expected-{step}.bin').read_bytes()
    for key in ['hp','maxhp','block','energy','regent.stars','hand_count','draw_count','discard_count','exhaust_count','strength','turn']:
     assert field(actual,key)==field(expected,key),(c,step,key,field(actual,key),field(expected,key))
    for e in range(field(actual,'enemy_count')):
     at=L['enemies']+e*L['enemy_size']+L['enemy_hp']
     assert actual[at:at+4]==expected[at:at+4],(c,step,e)
    assert await screen()==screens['COMBAT'] and await val('rift_asset_errors')==0
   for c in range(3):
    await fixture(f'cards-{c}');await shot(f'cards-{c}-three-foes')
    for step in range(1,5):await button('cross');await compare(c,step)
    await button('triangle');assert await val('inspect_open')==1 and await val('inspect_tab')==3
    await shot(f'cards-{c}-potion-menu');await button('cross');await compare(c,5)
    assert await val('inspect_open')==0 and (await state())[L['message']]==0
    await shot(f'cards-{c}-after-potion');await button('circle');await compare(c,6)
    print('PASS exact PSP character',c,'four cards including Twinwoven and Power, potion, turn end match native oracle; no potion text box',flush=True)
   await fixture('forge')
   for choice in range(3):
    assert field(await state(),'selection')==choice
    await shot('forge-choice-'+str(choice));await button('square')
    assert await val('inspect_open')==1 and await val('inspect_tab')==5
    await shot('forge-deck-'+str(choice));await button('circle')
    assert await val('inspect_open')==0 and field(await state(),'selection')==choice and await screen()==screens['BETA_FORGE']
    if choice<2:await button('down')
   await button('cross');assert await screen()==screens['BETA_FORGE_CARD'];await shot('forge-card-preview')
   print('PASS exact PSP three forge choices, deck overlay preserves each cursor, forged-card preview',flush=True)
  elif phase=='orders':
   orders=[(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0)]
   for order,characters in enumerate(orders):
    completed=0
    for i,c in enumerate(characters):
     if i:assert await account_value('account_ascended')==completed
     await fixture(f'order-{order}-{i}');assert await account_value('account_ascended')==completed
     await button('cross');assert await screen()==screens['ASCENT_TRUE_END'];completed|=1<<c
     assert await account_value('account_ascended')==completed
     assert await account_value('account_wins',c)==5
     if order==0:await shot(f'order-{order}-{i}-individual-ending')
     await click_n(3)
     if i<2:assert await screen()==screens['VICTORY'] and await finale_value(3)==0
     else:
      assert await screen()==screens['ASCENT_FINALE'] and await finale_value(3)==2
      for page in range(5):
       assert field(await state(),'selection')==page and await screen()==screens['ASCENT_FINALE']
       if order==0 or page==0:await shot(f'finale-order-{order}-page-{page+1}')
       await button('cross')
      assert await screen()==screens['VICTORY'] and await finale_value(3)==3
     print('PASS exact PSP order',characters,'first-cleared characters',i+1,'shared finale',i==2,flush=True)
    await fixture('repeat-'+str(order));await button('cross');assert await screen()==screens['ASCENT_TRUE_END'];await click_n(3)
    assert await screen()==screens['VICTORY'] and await finale_value(3)==3
    print('PASS exact PSP repeat victory skips consumed finale, order',characters,flush=True)
   # Leave an actual persisted finale at page 3 for an independent cold boot.
   await fixture('order-0-2');await button('cross');await click_n(3);await click_n(2)
   assert await screen()==screens['ASCENT_FINALE'] and field(await state(),'selection')==2
   await shot('cold-before-exit-page-3')
   assert list(D.glob('RIFTD186_FINALE.*')) and list(D.glob('RIFTD186_ACCOUNT.*')) and list(D.glob('RIFTD186_RUN_3.*'))
   print('PASS saved real third-character finale at page 3 for cold restart',flush=True)
  elif phase in ('resume','done'):
   await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW']
   await button('down');await button('down');await button('cross')
   if phase=='resume':
    assert await screen()==screens['ASCENT_FINALE'] and field(await state(),'selection')==2 and await finale_value(3)==2
    await shot('cold-resumed-page-3')
    for page in range(2,5):
     assert field(await state(),'selection')==page;await button('cross')
    assert await screen()==screens['VICTORY'] and await finale_value(3)==3
    await shot('cold-finale-completed')
    print('PASS cold boot resumes at saved finale page 3 and commits completion',flush=True)
   else:
    assert await screen()==screens['VICTORY'] and await finale_value(3)==3
    await shot('cold-completed-no-replay')
    print('PASS second cold boot resumes Victory, finale permanently consumed',flush=True)
  else:raise AssertionError('Unknown test phase '+phase)
 assert await val('rift_asset_errors')==0
 assert 'Bad memory access' not in (E/(prefix+'ppsspp.log')).read_text()
 print('PASS final release EBOOT:', 'all Normal/Fast full-frame ending sequences' if os.environ.get('RIFT_FADE_ONLY')=='1' else 'five-win gates and ten-win unlocks / all six finale orders and repeats / cold-restart persistence: '+os.environ.get('RIFT_PHASE','gates'), '; full soundtrack, zero art or memory errors',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);await command('cpu.stepping')

p=None;x=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':'+display,'-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/(prefix+'xvfb.log'),'w'),stderr=subprocess.STDOUT);time.sleep(.5);assert x.poll() is None
 launch=[str(T/'squashfs-root/lib/ld-linux-x86-64.so.2'),'--library-path',str(T/'squashfs-root/lib'),str(T/'squashfs-root/shared/bin/PPSSPPSDL')] if os.environ.get('RIFT_DIRECT')=='1' else [str(T/'squashfs-root/AppRun')]
 p=subprocess.Popen(launch+['--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,cwd=str(T/'squashfs-root/bin'),stdout=open(E/(prefix+'ppsspp.log'),'w'),stderr=subprocess.STDOUT)
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
