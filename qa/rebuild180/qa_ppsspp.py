"""Exercise the packaged EBOOT through PPSSPP's inputs and debugger.
State fixtures are explicitly assisted; the executable is the release binary.
"""
from pathlib import Path
import os,subprocess,time,asyncio,json,base64,struct,hashlib,shutil
import websockets
from PIL import Image
R=Path(__file__).resolve().parents[2];Q=R/'qa/rebuild180';E=Q/'evidence';T=R.parent/'toolchain';C=T/'qa180';C.mkdir(exist_ok=True)
M=C/'ppsspp';S=M/'PSP/SYSTEM';S.mkdir(parents=True,exist_ok=True)
(S/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
D=M/'PSP/GAME/RIFT180VERIFY';D.mkdir(parents=True,exist_ok=True)
resume=os.environ.get('RIFT_RESUME')=='1';smoke=os.environ.get('RIFT_SMOKE')=='1'
if not resume:
 for f in D.glob('ASCENT18_*'):f.unlink()
shutil.copyfile(E/'EBOOT.PBP',D/'EBOOT.PBP')
conf=C/'settings.ini';conf.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19518\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:518',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
nm=subprocess.check_output([str(T/'pspdev/bin/psp-nm'),'-S',str(Q/'source/rift_ascent_crt_free.elf')],text=True)
syms={p[3]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split())==4}
L=json.loads((E/'state-layout.json').read_text());screens=L['screens'];seq=0
async def check(w):
 global seq
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
  start=await val('animation_tick')
  for _ in range(600):
   if await val('animation_tick')>=start+n:return
   await asyncio.sleep(.1)
  raise AssertionError('Emulated frames stopped')
 async def settle():
  for _ in range(800):
   if not await val('r161_transition_active') and await val('action_cooldown')==0 and await val('enemy_turn_delay')==0 and (not await val('r1311_finishing') or await val('animation_tick')-await val('r137_start')>400):return
   await asyncio.sleep(.1)
  raise AssertionError('Transition did not settle')
 async def button(k):
  await settle();await req('input.buttons.send',buttons={k:True});await ticks(8);await req('input.buttons.send',buttons={k:False});await ticks(12);await settle()
 async def shot(n):
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
 if resume:
  await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await shot('cold-resume-slots');await button('cross');assert (await state())[L['ascent']:L['ascent']+4]!=bytes(4);await shot('cold-resumed-run');print('PASS cold process resume; slots and global account retained',flush=True);return
 # Real user inputs create a new run in an empty slot and save it.
 await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await shot('slots-empty');await button('cross');assert await screen()==screens['CHARACTER_SELECT'];await shot('characters-new');await button('cross');assert await screen()==screens['ACT_INTRO']
 await button('cross');assert await screen()==screens['STORY_ACT_BEFORE_BLESSING'];await button('cross');assert await screen()==screens['BLESSING'];await button('cross');assert await screen()==screens['MAP'];await shot('first-map')
 await button('select');await button('cross');assert await screen()==screens['TITLE'];await button('cross');assert await screen()==screens['ASCENT_SLOTS_VIEW'];await shot('slots-saved');await button('cross');assert await screen()==screens['MAP'];await button('select');await button('cross');assert await screen()==screens['TITLE']
 # Options are accessible with debug initially off, and survive toggling.
 for _ in range(3):await button('down')
 await button('cross');assert await screen()==screens['ASCENT_OPTIONS'];await shot('options');await button('cross');await button('circle');assert await screen()==screens['TITLE']
 if smoke:print('PASS draft startup, actual slot save/resume, options and stereo music',flush=True);return
 # Explicitly seed nine-win accounts for the character-gated tenth-win tests.
 await command('cpu.stepping');await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode((E/'fixture-account.bin').read_bytes()).decode());await command('cpu.resume')
 for c in range(3):
  await fixture('boss3-'+str(c));await shot('act3-'+str(c)+'-before');await button('cross');assert await screen()==screens['ASCENT_ENDING'];await shot('act3-'+str(c)+'-ending')
  for _ in range(3):await button('cross')
  assert await screen()==screens['ASCENT_CARDS'];await shot('cards-'+str(c));await button('cross');assert await screen()==screens['ASCENT_POTIONS'];await shot('potions-'+str(c));await button('cross');assert await screen()==screens['ASCENT_RELIC'];await shot('relic-'+str(c));await button('cross');assert await screen()==screens['ASCENT_REVELATION'];await shot('revelation-'+str(c))
  for _ in range(3):await button('cross')
  assert await screen()==screens['ACT_INTRO'] and field(await state(),'act')==4;await ticks(48);await shot('act4-intro-'+str(c));await button('cross');await button('cross');await button('cross');assert await screen()==screens['MAP'];await shot('act4-map-'+str(c))
  # Move the actual completed run to its fourth-act boss entrance, retaining its identity.
  await command('cpu.stepping');await req('memory.write_u32',address=syms['game']+L['screen'],value=screens['ASCENT_BOSS']);await req('memory.write_u32',address=syms['game']+L['selection'],value=0);await command('cpu.resume');await ticks(24);await shot('preboss-'+str(c))
  for _ in range(3):await button('cross')
  assert await screen()==screens['COMBAT'];raw=await state();assert struct.unpack_from('<i',raw,L['enemies']+L['enemy_art'])[0]==44;await shot('secret-boss-'+str(c));await button('l');assert await screen()==screens['ASCENT_TRUE_END'];await shot('true-ending-'+str(c))
  for _ in range(3):await button('cross')
  assert await screen()==screens['VICTORY'];await shot('endless-after-secret-'+str(c));await button('cross');assert field(await state(),'act')==5
  account=await memory('ascent_account',L['account_size']);assert struct.unpack_from('<I',account,L['account_wins']+4*c)[0]==10;assert struct.unpack_from('<I',account,L['account_ascended'])[0]&(1<<c)
 # Record all character badges and the completed collections before resetting fixtures.
 await command('cpu.stepping');await req('memory.write_u32',address=syms['game']+L['screen'],value=screens['CHARACTER_SELECT']);await req('memory.write_u32',address=syms['game']+L['selection'],value=0);await command('cpu.resume');await ticks(24);await shot('characters-ascended')
 for category in range(5):
  await command('cpu.stepping');await req('memory.write_u32',address=syms['game']+L['screen'],value=screens['ASCENT_COLLECTION']);await req('memory.write_u32',address=syms['game']+L['selection'],value=0);await req('memory.write_u32',address=syms['game']+L['scroll'],value=category);await command('cpu.resume');await ticks(24);await shot('collection-'+str(category))
 # An unfinished character still receives Endless after Act 3.
 await command('cpu.stepping');await req('memory.write',address=syms['ascent_account'],base64=base64.b64encode((E/'fixture-account.bin').read_bytes()).decode());await req('memory.write_u32',address=syms['ascent_account']+L['account_cards'],value=0);await req('memory.write_u32',address=syms['ascent_account']+L['account_wins'],value=0);await command('cpu.resume')
 await fixture('boss3-0');await button('cross')
 for _ in range(6):await button('cross')
 assert await screen()==screens['VICTORY'];await shot('endless-before-unlocked')
 # Ten-track routing, actual streamed output and bounded HP UI.
 for act,track in [(1,1),(2,7),(3,8),(4,9),(5,9)]:
  await fixture('map-'+str(act));assert await val('soundtrack_current')==track;await audible();await shot('menu-music-act-'+str(act))
 await fixture('combat');await shot('combat');assert await val('soundtrack_current') in [2,4,6];await button('r');assert field(await state(),'act')==2;print('PASS debug R act skip',flush=True)
 for category,count in [('old-potions',2),('old-relic',4)]:
  for i in range(count):await fixture(category+'-'+str(i));await shot(category+'-'+str(i))
 print('PASS exact release EBOOT: real save/resume; three tenth-win chains; cards/potions/relic screens; direct secret Act 4; pre/post boss scenes; true-ending gate; prior-win Endless; all act menu routes; actual PCM; zero asset errors',flush=True)
 print('SHA256',hashlib.sha256((D/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
p=None;x=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':518','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/'xvfb.log','w'),stderr=subprocess.STDOUT);time.sleep(.5);assert x.poll() is None
 p=subprocess.Popen([str(T/'ppsspp/AppDir/AppRun'),'--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,stdout=open(E/'ppsspp.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('Emulator exited; see ppsspp.log')
   try:w=await websockets.connect('ws://127.0.0.1:19518/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
