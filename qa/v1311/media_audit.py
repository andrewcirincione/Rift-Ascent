from pathlib import Path
import struct,json,hashlib,subprocess,runpy,numpy as np
q=Path(__file__).parent;root=q.parents[1];ev=root.parent/'evidence1311';data=(ev/'EBOOT.PBP').read_bytes();codec=runpy.run_path(str(root/'tools/embed_churn.py'))
off=struct.unpack_from('<8I',data,8);snd=data[off[5]:off[6]];assert snd==(root/'portable_build/SND0.AT3').read_bytes();assert data[off[1]:off[2]]==(root/'portable_build/ICON0.PNG').read_bytes()
from build_splash import validate
splash=validate(snd)
assert splash['samples']==24*44100
subprocess.run(['ffmpeg','-v','error','-y','-i',str(root/'portable_build/SND0.AT3'),'-f','s16le',str(ev/'splash-decoded.s16le')],check=True)
pcm=np.fromfile(ev/'splash-decoded.s16le',dtype='<i2');assert len(pcm)>44100*2*11 and np.sqrt(np.mean(pcm.astype(float)**2))>500
size=struct.unpack_from('<I',data,len(data)-8)[0];payload=data[-56-size:-56];tracks=[]
for i,name in enumerate(['Checkers','RiftMenu','RiftFight','RiftFight2','RiftFight3']):
 start,n=struct.unpack_from('<II',data,len(data)-48+i*8);v=np.asarray(codec['decode'](payload[start:start+(n+1)//2],n),dtype=np.float64);assert len(v)==n
 tracks.append({'name':name,'seconds':n/11025,'rms':float(np.sqrt(np.mean(v*v))),'peak':int(np.max(np.abs(v)))})
report={'splash_codec':'ATRAC3, 44100 Hz stereo','splash_bytes':len(snd),'splash_source':'RiftBoss.wav','splash_metadata':splash,'splash_decoded_non_silent':True,'icon_matches_main_menu_logo':True,'tracks':tracks,'eboot_sha256':hashlib.sha256(data).hexdigest()};(ev/'media-audit.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
