"""Observe real MIPS stack use in exact published/fixed EBOOT cold boots.

PPSSPP permits accesses outside the individual thread's allocation. Check the
SP against that allocation explicitly, then verify fixed startup, music and
the imported slot. Reset/breakpoints observe execution; no game state is edited.
"""
from pathlib import Path
import argparse, asyncio, hashlib, json, os, shutil, struct, subprocess, time
import websockets

P=argparse.ArgumentParser(description=__doc__)
P.add_argument('--baseline',action='store_true');P.add_argument('--scenario',default='fresh')
a=P.parse_args();R=Path(__file__).resolve().parents[2];Q=R/'qa/v186';E=Path(os.environ.get('RIFT_BOOT_EVIDENCE',str(R/'qa/v186/evidence')));F=R/'qa/v186/evidence/boot-fixtures'
T=Path(os.environ['RIFT_TOOLCHAIN']);name=('baseline-' if a.baseline else 'v186-')+a.scenario
C=T/('boot-'+name);M=C/'ppsspp';D=M/'PSP/GAME/RIFTBOOT';D.mkdir(parents=True,exist_ok=True)
for f in D.iterdir():
 if f.is_file():f.unlink()
for f in (F/a.scenario).iterdir():shutil.copyfile(f,D/f.name)
original={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in D.iterdir() if f.name.startswith('BETA') or f.name=='SAVE.DAT'}
pbp=R/'versions/1.8.5/EBOOT.PBP' if a.baseline else Q/'evidence/EBOOT.PBP'
elf=Path(os.environ.get('RIFT_BASELINE_ELF','/tmp/rift185-baseline.elf')) if a.baseline else Q/'source/rift_ascent_crt_free.elf'
assert (pbp).read_bytes()[-160:-152]==b'RIFTAU19', 'Incomplete package: soundtrack footer missing'
shutil.copyfile(pbp,D/'EBOOT.PBP')
assert hashlib.sha256((D/'EBOOT.PBP').read_bytes()).digest()==hashlib.sha256((pbp).read_bytes()).digest()
nm=subprocess.check_output([str(T/'pspdev/bin/psp-nm'),str(elf)],text=True)
syms={p[-1]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split())==3}
display='603';port=19603
conf=C/'settings.ini';conf.write_text(f'[General]\nRemoteDebuggerOnStartup = True\nRemoteISOPort = {port}\n[Graphics]\nInternalResolution = 1\nVerticalSync = False\nFrameSkip = 8\nAutoFrameSkip = True\n[Sound]\nEnable = True\n[SystemParam]\nPSPModel = 0\n')
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:'+display,LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy',SDL_VIDEO_X11_XSHM='0',XDG_CONFIG_HOME=str(C),LD_LIBRARY_PATH=str(T/'sysroot/usr/lib/x86_64-linux-gnu'))
async def probe(w):
 ticket=0
 async def req(event,**kw):
  nonlocal ticket
  ticket+=1;t=ticket;await w.send(json.dumps(dict(event=event,ticket=t,**kw)))
  while True:
   d=json.loads(await asyncio.wait_for(w.recv(),30))
   if d.get('ticket')==t:
    assert d.get('event')!='error',d
    return d
 async def status():return await req('cpu.status')
 async def value(expr):return (await req('cpu.evaluate',expression=expr))['uintValue']
 async def resume():
  await w.send(json.dumps({'event':'cpu.resume'}))
  while True:
   d=json.loads(await asyncio.wait_for(w.recv(),30))
   if d.get('event')=='cpu.resume':return
 async def stopped():
  for _ in range(600):
   s=await status()
   if s['stepping']:return s['pc']
   await asyncio.sleep(.05)
  raise AssertionError('No breakpoint reached')
 for _ in range(200):
  s=await status()
  if s['pc']:break
  await asyncio.sleep(.1)
 await asyncio.sleep(2)
 await req('game.reset',**{'break':True});await asyncio.sleep(.5);await stopped()
 await req('cpu.breakpoint.add',address=syms['run_game']);await resume()
 assert await stopped()==syms['run_game']
 # The initial unobserved boot may have imported beta/SAVE.DAT files. Restore
 # the fixture while stopped before storage opens, so THIS measured reset is
 # a true first migration too. No user directories are used by this tool.
 for f in D.iterdir():
  if f.is_file() and f.name!='EBOOT.PBP':f.unlink()
 for f in (F/a.scenario).iterdir():shutil.copyfile(f,D/f.name)
 top=await value('sp');threads=(await req('hle.thread.list'))['threads']
 thread=next(t for t in threads if t['name']=='Rift Bare Game');assert thread['currentStackSize']==192*1024
 # Entry SP already includes a small firmware argument/trampoline reservation;
 # using top-stack is conservative: the actual bottom is slightly HIGHER.
 bottom=top-thread['currentStackSize'];minimum=top
 await req('cpu.breakpoint.remove',address=syms['run_game'])
 for symbol in ['ascent_read','memcpy','game_render_frame']:
  kw={'condition':f'sp < 0x{top-64*1024:x}'} if symbol=='memcpy' else {}
  await req('cpu.breakpoint.add',address=syms[symbol],**kw)
 # PPSSPP may replace memcpy with an HLE shortcut. Break immediately after
 # the converter's known MIPS addiu sp,sp,-28520 prologue as well.
 disasm=subprocess.check_output([str(T/'pspdev/bin/psp-objdump'),'-d','--disassemble=migrate_layout184',str(elf)],text=True)
 assert 'addiu\tsp,sp,-28520' in disasm
 migrate_probe=syms['migrate_layout184']+4
 await req('cpu.breakpoint.add',address=migrate_probe)
 await resume();overflow=False
 for _ in range(2000):
  pc=await stopped();sp=await value('sp');minimum=min(minimum,sp)
  if sp<bottom:
   overflow=True;print('OBSERVED thread stack overflow',json.dumps({'top':top,'bottom':bottom,'sp':sp,'bytes_used':top-sp,'allocated':192*1024,'at':hex(pc)}),flush=True);break
  if pc==syms['game_render_frame']:break
  if pc==syms['memcpy']:
   await req('cpu.breakpoint.update',address=syms['memcpy'],condition=f'sp < 0x{minimum:x}')
  await resume()
 else:raise AssertionError('Startup did not reach renderer')
 assert overflow==a.baseline,('Unexpected stack result',overflow,a.baseline)
 if a.baseline:
  print('PASS negative control: published 1.8.5 exceeds its PSP thread stack even with an empty save folder',flush=True)
  return
 for symbol in ['ascent_read','memcpy','game_render_frame']:await req('cpu.breakpoint.remove',address=syms[symbol])
 await req('cpu.breakpoint.remove',address=migrate_probe)
 await resume()
 for _ in range(600):
  bootlog=M/'RIFT_BOOT.LOG'
  if await value(f'[0x{syms["soundtrack_active"]:x}]')==1 and bootlog.exists() and 'frame 60 reached: 0' in bootlog.read_text():break
  await asyncio.sleep(.1)
 else:raise AssertionError('Boot did not reach frame 60 with soundtrack')
 assert await value(f'[0x{syms["game"]+16:x}]')==0
 present=await value(f'[0x{syms["ascent_slots"]:x}]')
 assert present==(0 if a.scenario in ('fresh','corrupt') else 1),(a.scenario,present)
 assert await value(f'[0x{syms["rift_asset_errors"]:x}]')==0
 for filename,digest in original.items():assert hashlib.sha256((D/filename).read_bytes()).hexdigest()==digest
 log=(M/'RIFT_BOOT.LOG').read_text();assert 'frame 60 reached: 0' in log
 (E/(name+'-boot.log')).write_text(log)
 print('PASS',name,'observed startup stack bytes',top-minimum,'of',192*1024,'slot present',present,'music active; frame 60; original beta/legacy files unchanged',flush=True)
 print('SHA256',hashlib.sha256(pbp.read_bytes()).hexdigest(),flush=True)

x=None;emu=None
try:
 x=subprocess.Popen([str(T/'sysroot/usr/bin/Xvfb'),':'+display,'-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir','/usr/share/X11/xkb'],env=env,stdout=open(E/(name+'-xvfb.log'),'w'),stderr=subprocess.STDOUT)
 time.sleep(.5);assert x.poll() is None
 emu=subprocess.Popen([str(T/'squashfs-root/lib/ld-linux-x86-64.so.2'),'--library-path',str(T/'squashfs-root/lib'),str(T/'squashfs-root/shared/bin/PPSSPPSDL'),'--appendconfig='+str(conf),'--graphics=opengl','--windowed',str(D/'EBOOT.PBP')],env=env,stdout=open(E/(name+'-ppsspp.log'),'w'),stderr=subprocess.STDOUT)
 async def run():
  for _ in range(120):
   await asyncio.sleep(.5)
   assert emu.poll() is None,'Emulator exited'
   try:w=await websockets.connect(f'ws://127.0.0.1:{port}/debugger',subprotocols=['debugger.ppsspp.org'],proxy=None,max_size=10000000)
   except OSError:continue
   async with w:await probe(w)
   return
  raise AssertionError('Debugger unavailable')
 asyncio.run(run())
finally:
 for proc in [emu,x]:
  if proc:
   proc.terminate()
   try:proc.wait(timeout=5)
   except subprocess.TimeoutExpired:proc.kill()
