"""Interactive PPSSPP launcher-preview capture for the corrected SND0."""
from pathlib import Path
import os,subprocess,shutil,time,json,sys
from PIL import ImageGrab
base=Path(__file__).resolve().parents[3];rt=base/'runtime';ev=base/'evidence1311-splash';cfg=rt/'qa1311splashconfig';mem=cfg/'ppsspp';system=mem/'PSP/SYSTEM';system.mkdir(parents=True,exist_ok=True)
game=mem/'PSP/GAME/RIFTASCENT';game.mkdir(parents=True,exist_ok=True);shutil.copyfile(base/'evidence1311/EBOOT.PBP',game/'EBOOT.PBP')
s=(rt/'qa1311finalconfig/ppsspp/PSP/SYSTEM/ppsspp.ini').read_text();s=s.replace('qa1311finalconfig','qa1311splashconfig');s=s.replace('RemoteDebuggerOnStartup = True','RemoteDebuggerOnStartup = False').replace('UIVolume = 75','UIVolume = 100').replace('GamePreviewVolume = 75','GamePreviewVolume = 100');(system/'ppsspp.ini').write_text(s)
env=os.environ.copy();env.update(DISPLAY='127.0.0.1:417',LIBGL_ALWAYS_SOFTWARE='1',SDL_VIDEODRIVER='x11',XDG_CONFIG_HOME=str(cfg),PULSE_SERVER='tcp:127.0.0.1:4817',LD_LIBRARY_PATH=str(rt/'deps/usr/lib/x86_64-linux-gnu')+':'+str(rt/'deps/usr/lib/x86_64-linux-gnu/pulseaudio')+':'+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'))
procs=[]
def launch(args,log):
 p=subprocess.Popen(args,env=env,stdout=open(ev/log,'w'),stderr=subprocess.STDOUT);procs.append(p);return p
try:
 launch([str(rt/'deps/usr/bin/Xvfb'),':417','-screen','0','960x544x24','-ac','-nolisten','unix','-listen','tcp','-extension','MIT-SHM','-xkbdir',str(rt/'deps/usr/share/X11/xkb')],'xvfb.log')
 launch([str(rt/'deps/usr/bin/pulseaudio'),'-n','--daemonize=no','--use-pid-file=no','--exit-idle-time=-1','--disable-shm','--dl-search-path='+str(rt/'deps/usr/lib/pulse-16.1+dfsg1/modules'),'--load=module-null-sink sink_name=rift','--load=module-native-protocol-tcp listen=127.0.0.1 port=4817 auth-anonymous=1'],'pulse.log');time.sleep(1)
 launch([str(rt/'AppDir/AppRun'),'--graphics=opengl','--windowed'],'ppsspp.log');time.sleep(4)
 ImageGrab.grab(xdisplay=env['DISPLAY']).save(ev/'launcher.png');print('READY launcher.png',flush=True)
 for line in sys.stdin:
  cmd=json.loads(line)
  if cmd['action']=='quit':break
  if cmd['action']=='keys':subprocess.run([str(rt/'deps/usr/bin/xdotool'),'key',*cmd['keys']],env=env,check=True)
  if cmd['action']=='mouse':subprocess.run([str(rt/'deps/usr/bin/xdotool'),'mousemove',str(cmd['x']),str(cmd['y'])],env=env,check=True)
  if cmd['action']=='click':subprocess.run([str(rt/'deps/usr/bin/xdotool'),'click',str(cmd.get('button',1))],env=env,check=True)
  if cmd['action']=='record':
   with open(ev/'launcher-audio.s16le','wb') as f:
    p=subprocess.Popen([str(rt/'deps/usr/bin/parec'),'--device=rift.monitor','--format=s16le','--rate=44100','--channels=2'],env=env,stdout=f,stderr=open(ev/'parec.log','w'));time.sleep(cmd.get('seconds',28));p.terminate();p.wait(timeout=5)
  time.sleep(1);ImageGrab.grab(xdisplay=env['DISPLAY']).save(ev/(cmd.get('shot','launcher')+'.png'));print('DONE',cmd,flush=True)
finally:
 for p in reversed(procs):
  p.terminate()
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:p.kill()
