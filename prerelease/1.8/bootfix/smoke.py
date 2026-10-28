"""Boot regression: original fails in renamed folder, corrected binary boots.
Read-only state inspection plus controller input; no game memory/ELF patches.
The damaged-package case modifies only a disposable copy's media trailer.
"""
from pathlib import Path
import os,sys,json,asyncio,subprocess,shutil,hashlib
import websockets
from PIL import ImageGrab
root=Path(sys.argv[1]).resolve();app=Path(sys.argv[2]).resolve();old=Path(sys.argv[3]).resolve()
ev=root/'evidence/bootfix';ev.mkdir(parents=True,exist_ok=True)
syms={x[3]:int(x[0],16) for line in (root/'source/symbols.txt').read_text().splitlines() if len(x:=line.split())==4}
async def case(mode):
 home=Path(os.environ.get('RUNNER_TEMP','/tmp'))/('rift-pre2-'+mode)
 if home.exists():shutil.rmtree(home)
 home.mkdir();mem=home/'.config/ppsspp';install=mem/'PSP/GAME'/('RIFTASCENT_A18' if mode=='canonical' else 'Rift Ascent Test');install.mkdir(parents=True)
 exe=old if mode=='old-renamed' else root/'release/EBOOT.PBP';shutil.copyfile(exe,install/'EBOOT.PBP')
 if mode=='damaged':
  with (install/'EBOOT.PBP').open('r+b') as f:f.seek(-56,2);f.write(b'BADMAGIC')
 conf=home/'config.ini';conf.write_text('[General]\nRemoteISOPort = 19518\nRemoteDebuggerOnStartup = True\nPauseOnLostFocus = False\nPauseWhenMinimized = False\nEnableLogging = True\n[Graphics]\nInternalResolution = 1\nVSyncInterval = False\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
 env=os.environ.copy();env.update(HOME=str(home),XDG_CONFIG_HOME=str(home/'.config'),LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy')
 log=(ev/(mode+'.log')).open('w');p=subprocess.Popen([str(app),'--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(install/'EBOOT.PBP')],env=env,stdout=log,stderr=subprocess.STDOUT);ws=None;n=0
 async def req(e,**kw):
  nonlocal n
  n+=1;t=n;await ws.send(json.dumps(dict(event=e,ticket=t,**kw)))
  while True:
   r=json.loads(await asyncio.wait_for(ws.recv(),30))
   if r.get('ticket')==t:return r
 async def value(s,offset=0):return (await req('memory.read_u32',address=syms[s]+offset)).get('value',0)
 try:
  for _ in range(90):
   await asyncio.sleep(1)
   assert p.poll() is None,'Emulator process exited'
   try:ws=await websockets.connect('ws://127.0.0.1:19518/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None);break
   except OSError:pass
  assert ws is not None,'No emulator debugger'
  text=''
  for _ in range(120):
   await asyncio.sleep(1);logs=list(home.rglob('RIFT_BOOT.LOG'));text=logs[0].read_text(errors='replace') if logs else ''
   if mode=='old-renamed':
    if 'FATAL artwork bank' in text:break
   elif mode=='damaged':
    if 'Cannot load artwork' in text:break
   elif await value('game')==0x52494654 and await value('animation_tick')>48 and 'five music tracks loaded' in text:break
  else:raise RuntimeError('Boot checkpoint timeout: '+text)
  await asyncio.sleep(1);ImageGrab.grab(xdisplay=os.environ['DISPLAY']).save(ev/(mode+'.png'));(ev/(mode+'-RIFT_BOOT.LOG')).write_text(text)
  if mode=='old-renamed':assert 'FATAL artwork bank: -8' in text
  elif mode=='damaged':assert 'Cannot load artwork' in text and 'game_new entering' not in text
  else:
   assert await value('game')==0x52494654 and await value('rift_asset_errors')==0
   assert 'streamed artwork bank: 1' in text and 'A1.8 profile journal: 1' in text
   assert str(install.name) in text and list(install.glob('A18_PROFILE.*'))
   await req('input.buttons.press',button='cross',duration=15);await asyncio.sleep(4)
   assert await value('game',16)==29,'Save-slot screen not reached'
   ImageGrab.grab(xdisplay=os.environ['DISPLAY']).save(ev/(mode+'-slots.png'))
   assert hashlib.sha256((install/'EBOOT.PBP').read_bytes()).digest()==hashlib.sha256(exe.read_bytes()).digest()
  print('PASS',mode,flush=True)
 finally:
  if ws:await ws.close()
  p.terminate()
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:p.kill();p.wait()
  log.close();await asyncio.sleep(1)
async def main():
 modes=['old-renamed','renamed','canonical','damaged']
 for mode in modes:await case(mode)
 (ev/'BOOT_SMOKE_PASSED.json').write_text(json.dumps(dict(sha256=hashlib.sha256((root/'release/EBOOT.PBP').read_bytes()).hexdigest(),cases=modes,scope='Exact final EBOOT; real emulator launch argument; no directory aliases; read-only inspection and controller input. Hardware not tested.'),indent=2))
if __name__=='__main__':asyncio.run(main())
