"""Play the exact packaged EBOOT; save PPSSPP's own native-resolution screenshots."""
import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
import numpy as np
smoke=os.environ.get("RIFT_FINAL_SMOKE")=="1"
control_mode=os.environ.get('RIFT_CONTROL_AUDIT','')
repo=Path(__file__).resolve().parents[2];base=repo.parent;rt=base/'runtime';ev=repo/'qa/v171r2/evidence';cfg=rt/'qa171r2config';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True)
if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)
shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa171r2.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19482\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
modules=next((rt/'deps/usr/lib').glob('pulse-*/modules'))
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:482',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4882',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(modules))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':482','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(modules),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4882 auth-anonymous=1'],env=env,stdout=open(ev/'pulse.log','w'),stderr=subprocess.STDOUT)
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
  for key in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active','r1311_finishing','r140_chime_done','enemy_turn_delay','plating_delay','hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind','r161_transition_active','r170_story_queue']:
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
   if await val('action_cooldown')==0 and (not await val('r161_transition_active') or await val('animation_tick')-await val('r161_transition_tick')>=await val('r161_transition_length')):break
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
   if await val('animation_tick')>=end and not await val('r161_transition_active') and await val('action_cooldown')==0 and await val('enemy_turn_delay')==0 and await val('plating_delay')==0:break
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
 async def ticks(n):
  start=await val('animation_tick')
  for _ in range(900):
   if await val('animation_tick')>=start+n:return
   await asyncio.sleep(.15)
  raise AssertionError('frames stopped')
 async def pcm():return base64.b64decode((await req('memory.read',address=symbols['audio_buffer'],size=2048))['base64'])
 async def settled():
  for _ in range(900):
   if not await val('r161_transition_active') and await val('action_cooldown')==0:return
   await asyncio.sleep(.15)
  raise AssertionError('transition stuck')
 async def fade_action(key,label):
  # Stop before the next frame renders, preserving each actual PSP framebuffer.
  # Wall-clock polling can miss an eight-frame fade on a fast emulator.
  await settled();await command('cpu.stepping')
  previous=await val('r161_transition_tick')
  await req('input.buttons.send',buttons={key:True})
  for _ in range(6):
   await next_frame()
   if await val('r161_transition_tick')!=previous:break
  start=await val('r161_transition_tick');length=await val('r161_transition_length')
  assert start!=previous and await val('r161_transition_active'),(label,field(await state(),'screen'),start,previous)
  await req('input.buttons.send',buttons={key:False})
  for j,age in enumerate([8,length//2,length-8,length+4]):
   await next_frame(start+age+4)
   await shot(label+'-'+str(j))
  await req('cpu.breakpoint.remove',address=symbols['game_render_frame']);await command('cpu.resume');await settled()
 frame_target=0
 async def next_frame(target=None):
  nonlocal frame_target
  if target is None:target=max(await val('animation_tick'),frame_target)+4
  frame_target=target
  await req('cpu.breakpoint.add',address=symbols['game_render_frame'],condition=f'a3 >= 0x{target:x}',enabled=True)
  await command('cpu.resume')
  for _ in range(900):
   if (await req('cpu.status')).get('stepping'):return
   await asyncio.sleep(.15)
  raise AssertionError('frame breakpoint did not stop')
 await shot(control_mode+'-boot' if control_mode else 'boot')
 if control_mode=='resume':
  await button('cross');assert field(await state(),'screen')==28 and await val('music_route')&256
  await shot('cold-resume-story');await button('cross');assert field(await state(),'screen')==18 and await val('music_route')==1
  print('PASS cold resume preserves pre-relic story and silence, then offers relic exactly once',flush=True);return
 if control_mode=='fade':
  await fixture('intro-0-2');await fade_action('cross','fade-splash-story')
  assert field(await state(),'screen')==28 and await val('music_route')&256
  await fade_action('cross','fade-story-relic')
  assert field(await state(),'screen')==18 and await val('music_route')==1
  await button('cross');assert field(await state(),'screen')==1
  print('PASS exact packaged EBOOT: screenshot phases of splash-to-story and story-to-relic fades, silent entry and RiftMenu handoff',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 # Exercise the real pause controller, including both X destinations.
 await fixture('event-0-2');await button('down');original=await state()
 await button('select');assert field(await state(),'screen')==13 and await val('paused_selection')==0
 await shot('pause-save');await button('down');assert await val('paused_selection')==1
 await shot('pause-resume');await button('cross');assert field(await state(),'screen')==6 and field(await state(),'selection')==field(original,'selection')
 await button('select');await button('up');assert await val('paused_selection')==1
 await button('down');assert await val('paused_selection')==0
 await button('circle');assert field(await state(),'screen')==6
 for c in range(3):
  for event in [2,6]:
   await fixture(f'event-{c}-{event}');await shot(f'event-{c}-{event}-0')
   await button('down');assert field(await state(),'selection')==1;await shot(f'event-{c}-{event}-1')
   count=field(await state(),'deck_count');await button('cross');assert field(await state(),'deck_count')==count+1
   await shot(f'event-{c}-{event}-received')
 # Music actually reaches the PSP output; act tests below still require zero PCM.
 await fixture('event-0-2');await ticks(24);assert any(await pcm())
 print('PASS pause D-pad selection and X Resume, Circle resume, all-character Tome/Armory reward navigation and receipt; music PCM audible',flush=True)
 await fixture('characters');await button('cross');await ticks(160);assert field(await state(),'screen')==20 and await val('music_route')&256
 assert not any(await pcm());await shot('new-run-intro')
 for c,a in [(0,1),(0,2),(0,3),(0,4),(1,1),(1,3),(2,1),(2,2)]:
  await fixture(f'intro-{c}-{a}');await ticks(12);assert await val('music_route')&256 and not any(await pcm())
  await shot(f'intro-{c}-{a}');await button('cross');assert field(await state(),'screen')==28 and await val('music_route')&256
  assert not any(await pcm());await shot(f'story-{c}-{a}');await button('cross');assert field(await state(),'screen')==18 and await val('music_route')==1
  await shot(f'relic-{c}-{a}');await button('cross');assert field(await state(),'screen')==1 and await val('music_route')==1
 # Natural splash completion takes the same route without any input.
 await fixture('intro-0-2');await ticks(500);await settled();assert field(await state(),'screen')==28 and await val('music_route')&256
 # Queue X during the splash->story fade; it must not accept the relic.
 await fixture('intro-0-1');await command('cpu.stepping')
 for edge in range(8):
  await req('input.buttons.send',buttons={'cross':edge%2==0});await next_frame()
 await req('cpu.breakpoint.remove',address=symbols['game_render_frame']);await command('cpu.resume');await ticks(80);await settled()
 assert field(await state(),'screen')==18;await shot('rapid-x-relic-safe')
 for e in [0,1,3,9,10,12,14,17,20,26,31,41,42,43]:
  await fixture(f'enemy-{e:02}');await shot(f'ground-{e:02}')
 for c in range(3):
  await fixture(f'idle-{c}')
  for i in range(6):await shot(f'idle-{c}-{i}');await ticks(32)
 await fixture('story-2-2');await button('select');await button('cross');assert field(await state(),'screen')==0
 await button('cross');assert field(await state(),'screen')==28 and await val('music_route')&256
 await shot('resumed-story')
 print('PASS exact 1.71-r2 EBOOT: silent act splashes/stories, relic-before-map music handoff, all act palettes/characters, natural and X progression, safe rapid-X, 14 grounded enemy examples, faster player idle cycles and save/resume',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)

try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/('ppsspp-'+control_mode+'.log' if control_mode else 'ppsspp.log'),'w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19482/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
