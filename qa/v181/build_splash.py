"""Build a looping PSP XMB SND0 from the user's RiftBoss recording.

ATRAC3 LP2, 44.1kHz stereo, 14 seconds; fact/loop metadata must stay within
encoded capacity, including decoder delay and the PSP's tail margin.
Reference: https://github.com/TotalKommando/psp-media-toolkit
"""
from pathlib import Path
import argparse,struct,subprocess,tempfile

def chunks(blob):
 assert blob[:4]==b'RIFF' and blob[8:12]==b'WAVE'
 out={};p=12
 while p+8<=len(blob):
  key,size=struct.unpack_from('<4sI',blob,p);assert p+8+size<=len(blob)
  out[key]=blob[p+8:p+8+size];p+=8+size+(size&1)
 return out

def validate(blob,icon):
 c=chunks(blob);fmt,fact,smpl,data=(c[k] for k in (b'fmt ',b'fact',b'smpl',b'data'))
 tag,channels,rate=struct.unpack_from('<HHI',fmt);align=struct.unpack_from('<H',fmt,12)[0]
 assert tag==0x270 and channels==2 and rate==44100 and align==384
 samples,delay=struct.unpack_from('<II',fact);assert delay==1024 and samples>0
 assert len(data)%align==0;frames=len(data)//align
 assert samples+delay+368<=frames*1024,'XMB rejects overclaimed samples'
 assert len(smpl)==60 and struct.unpack_from('<I',smpl,28)[0]==1
 start,end=struct.unpack_from('<II',smpl,44)
 assert start==delay and end==delay+samples-1 and end<frames*1024
 assert len(blob)+len(icon)<=480*1024,'Combined ICON1 + SND0 exceeds safe XMB budget'
 return dict(samples=samples,frames=frames,delay=delay,loop_start=start,loop_end=end,bytes=len(blob),combined_bytes=len(blob)+len(icon))

def build(source,dest,encoder):
 with tempfile.TemporaryDirectory() as tmp:
  tmp=Path(tmp);wav=tmp/'preview.wav';oma=tmp/'encoded.oma';remux=tmp/'remux.wav'
  subprocess.run(['ffmpeg','-v','error','-y','-i',str(source),'-t','14','-ar','44100','-ac','2','-sample_fmt','s16','-af','afade=t=in:d=0.25,afade=t=out:st=13.25:d=0.75',str(wav)],check=True)
  w=chunks(wav.read_bytes());samples=len(w[b'data'])//4
  subprocess.run([str(encoder),'-e','atrac3','-i',str(wav),'-o',str(oma)],check=True,stdout=subprocess.DEVNULL)
  subprocess.run(['ffmpeg','-v','error','-y','-i',str(oma),'-c:a','copy','-f','wav',str(remux)],check=True)
  c=chunks(remux.read_bytes());fmt,data=c[b'fmt '],c[b'data'];align=struct.unpack_from('<H',fmt,12)[0]
  samples=min(samples,len(data)//align*1024-1024-368);assert samples>0
  # RIFF sampler header + one forward, infinitely repeating loop.
  smpl=struct.pack('<15I',0,0,22676,60,0,0,0,1,24,0,0,1024,1024+samples-1,0,0)
  body=b'WAVE'
  for key,val in [(b'fmt ',fmt),(b'fact',struct.pack('<II',samples,1024)),(b'smpl',smpl),(b'data',data)]:
   body+=key+struct.pack('<I',len(val))+val+(b'\0' if len(val)&1 else b'')
  blob=b'RIFF'+struct.pack('<I',len(body))+body;report=validate(blob,(dest.parent/"ICON1.PMF").read_bytes());dest.write_bytes(blob);print(report)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path);p.add_argument('--encoder',default='atracdenc');a=p.parse_args();build(a.source,a.output,a.encoder)
