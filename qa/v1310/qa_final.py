import os,subprocess,time,asyncio,json,base64,shutil,struct,hashlib
from pathlib import Path
import websockets
from PIL import Image
base=Path(os.environ.get('RIFT_WORKSPACE',str(Path(__file__).resolve().parents[3])));rt=base/'runtime';ev=base/'evidence1310';cfg=rt/'qa1310finalconfig';mem=cfg/'ppsspp';sys=mem/'PSP/SYSTEM';sys.mkdir(parents=True,exist_ok=True)
(sys/'controls.ini').write_text('[ControlMapping]\nScreenshot = 1-131\n')
gamedir=mem/'PSP/GAME/RIFTASCENT';gamedir.mkdir(parents=True,exist_ok=True);(gamedir/'SAVE.DAT').unlink(missing_ok=True);shutil.copyfile(ev/'EBOOT.PBP',gamedir/'EBOOT.PBP')
config=rt/'qa1310final.ini';config.write_text('[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = 19412\nEnableLogging = True\nScreenshotsAsPNG = True\nScreenshotMode = 1\nDumpAudio = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:412',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4812',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'))
x=subprocess.Popen([str(rt/'deps/usr/bin/Xvfb'),':412','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],env=env,stdout=open(ev/'xvfb-qa.log','w'),stderr=subprocess.STDOUT)
pulse=subprocess.Popen([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4812 auth-anonymous=1'],env=env,stdout=open(ev/'pulse-qa.log','w'),stderr=subprocess.STDOUT)
nm=subprocess.check_output([str(rt/'pspdev/psp/bin/nm'),'-S',str(base/'rift-ascent-1310/portable_build/rift_ascent_crt_free.elf')]).decode();symbols={parts[3]:int(parts[0],16) for line in nm.splitlines() if len(parts:=line.split())==4}
p=None;ticket=0
async def check(ws):
 global ticket
 async def req(event,**kw):
  global ticket
  ticket+=1;t=ticket;await ws.send(json.dumps(dict(event=event,ticket=t,**kw)))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),10))
   if d.get('ticket')==t:
    assert d['event']!='error',d
    return d
 async def command(event):
  await ws.send(json.dumps({'event':event}))
  while True:
   d=json.loads(await asyncio.wait_for(ws.recv(),10))
   if d.get('event')==event:return
 async def state():
  d=await req('memory.read',address=symbols['game'],size=3144);return base64.b64decode(d['base64'])
 def val(data,offset):return struct.unpack_from('<i',data,offset)[0]
 async def waitval(offset,predicate):
  for _ in range(150):
   data=await state()
   if predicate(val(data,offset)):return data
   await asyncio.sleep(1.0)
  raise AssertionError(('game state timeout',offset,val(data,offset)))
 async def button(name,delay=2.5):
  for _ in range(45):
   d=await req('memory.read_u32',address=symbols['action_cooldown'])
   if not d['value']:break
   await asyncio.sleep(1)
  await req('input.buttons.press',button=name,duration=15);await asyncio.sleep(max(2.5,delay))
 async def shot(name):
  folder=mem/'PSP/SCREENSHOT';before=set(folder.glob('*.png'))
  subprocess.run([str(rt/'deps/usr/bin/xdotool'),'key','F1'],env=env,check=True)
  for _ in range(100):
   await asyncio.sleep(.1);new=set(folder.glob('*.png'))-before
   if new:
    src=next(iter(new))
    try:
     with Image.open(src) as check_image:check_image.load()
     if not src.read_bytes().endswith(bytes.fromhex('49454e44ae426082')):continue
    except Exception:continue
    shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print('SHOT',name,flush=True);return
  raise RuntimeError('Screenshot missing: '+name)
 async def fixture(name):
  await command('cpu.stepping')
  raw=(ev/('state-'+name+'.dat')).read_bytes()[4:];assert len(raw)==3144
  await req('memory.write',address=symbols['game'],base64=base64.b64encode(raw).decode())
  for name in ['inspect_open','inspect_tab','inspect_scroll','action_cooldown','r137_count','r137_held_valid','r138_death_active']:
   if name in symbols:await req('memory.write_u32',address=symbols[name],value=0)
  for name in ['hand_exchange','fx_transition','victory_replay','fx_player_damage','fx_enemy_damage','fx_heal','fx_block','effect_kind']:
   if name in symbols:await req('memory.write_u32',address=symbols[name],value=0)
  if 'fx_enemy_damage' in symbols:await req('memory.write_u32',address=symbols['fx_enemy_damage']+4,value=0)
  for name in ['rift120_played_card','rift120_auto_card']:
   await req('memory.write_u32',address=symbols[name],value=4294967295)
  await req('memory.write',address=symbols['rift120_log'],base64=base64.b64encode(bytes(192)).decode())
  await req('memory.write',address=symbols['r1310_potion_notice'],base64=base64.b64encode(bytes(72)).decode())
  await command('cpu.resume');await asyncio.sleep(2.5)
 await asyncio.sleep(4)
 ids=subprocess.check_output([str(rt/'deps/usr/bin/xdotool'),'search','--onlyvisible','--name','.*'],env=env).decode().split();subprocess.run([str(rt/'deps/usr/bin/xdotool'),'windowfocus',ids[-1]],env=env,check=True)
 async def waitsymbol(name,expected):
  for _ in range(80):
   d=await req('memory.read_u32',address=symbols[name])
   if d['value']==expected:return
   await asyncio.sleep(.5)
  raise AssertionError((name,d,expected))
 async def waithp(expected):
  for _ in range(100):
   data=await state();hp=struct.unpack_from('<h',data,2660)[0]
   if hp==expected:return data
   await asyncio.sleep(.5)
  raise AssertionError(('enemy hp',hp,expected))
 async def symbols_values(names):
  return {name:(await req('memory.read_u32',address=symbols[name]))['value'] for name in names}
 async def play_capture(name,expected_hp=None,kind=None):
  audio_before=(await req('memory.read_u32',address=symbols['impact_serial']))['value']
  await req('input.buttons.press',button='cross',duration=30)
  for attempt in range(80):
   await asyncio.sleep(1)
   count=(await req('memory.read_u32',address=symbols['r137_count']))['value']
   if count:break
  assert count>0,(name,count)
  raw=await req('memory.read',address=symbols['r137_events'],size=24*count)
  values=struct.unpack('<'+'i'*6*count,base64.b64decode(raw['base64']))
  if kind is not None:assert values[0]==kind,(name,values)
  await shot(name)
  await asyncio.sleep(2)
  audio_after=(await req('memory.read_u32',address=symbols['impact_serial']))['value']
  expected_audio=sum(values[i*6] in (1,4,5) and values[i*6+3]>0 for i in range(count))
  if expected_audio:assert audio_after-audio_before==expected_audio,(name,audio_before,audio_after,expected_audio)
  print('AUDIO',name,'contacts',audio_after-audio_before,flush=True)
  if expected_hp is not None:await waithp(expected_hp)
  print('PASS action',name,'events',count,'first event',values[:6],flush=True)
  await asyncio.sleep(8)
 await fixture('strength-potion');await button('triangle');await button('up');await button('cross');await waitval(80,lambda x:x==2)
 await shot('final-strength-potion')
 key_name=next(n for n in symbols if n.startswith('keys.'));count_name=next(n for n in symbols if n.startswith('count.'));value_name=next(n for n in symbols if n.startswith('values.'))
 count=(await req('memory.read_u32',address=symbols[count_name]))['value'];assert 0<count<=32
 keys=struct.unpack('<'+'i'*count,base64.b64decode((await req('memory.read',address=symbols[key_name],size=count*4))['base64']))
 values=base64.b64decode((await req('memory.read',address=symbols[value_name],size=320*count))['base64'])
 strike=[values[i*320:(i+1)*320].split(bytes([0]))[0].decode() for i,k in enumerate(keys) if k==0]
 assert strike and all('Deal 8 damage' in s for s in strike),strike
 print('PASS rendered PSP card cache immediately displays:',strike,flush=True)
 for case in ['release-combat','release-combat-end','ancient-lesson-reveal','potion-reward']:
  await fixture(case);await shot('final-'+case)
 print('PASS final release screenshots and PSP rendered Strength-potion description',flush=True)



try:
 time.sleep(1);assert x.poll() is None
 p=subprocess.Popen([str(rt/'AppDir/AppRun'),'--appendconfig='+str(config),'--graphics=opengl','--windowed',str(gamedir/'EBOOT.PBP')],env=env,stdout=open(ev/'ppsspp-qa.log','w'),stderr=subprocess.STDOUT)
 async def run():
  for i in range(60):
   await asyncio.sleep(.5)
   if p.poll() is not None:raise RuntimeError('emulator exited')
   try:ws=await websockets.connect('ws://127.0.0.1:19412/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
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
