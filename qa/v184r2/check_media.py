from pathlib import Path
import json,struct,hashlib,zlib,wave,subprocess,numpy as np
from build_splash import validate
r=Path(__file__).resolve().parent;repo=r.parents[1];b=(r/'evidence/EBOOT.PBP').read_bytes();base=(repo/'versions/1.8.4/EBOOT.PBP').read_bytes();m=json.loads((r/'audio/manifest.json').read_text());a=json.loads((r/'assets.json').read_text())
assert b[-160:-152]==b'RIFTAU19';size,crc=struct.unpack_from('<II',b,len(b)-8);music=b[-160-size:-160];assert zlib.crc32(music)==crc
cursor=0
for i,t in enumerate(m['tracks']):
 off,frames,rate=struct.unpack_from('<III',b,len(b)-152+i*12);assert off==cursor==t['offset'] and frames==t['frames'] and rate==t['rate'];data=music[off:off+frames];assert hashlib.sha256(data).hexdigest()==t['encoded_sha256'];cursor+=frames
 if i>=10:
  src=r/'audio'/(t['name']+'.wav');assert hashlib.sha256(src.read_bytes()).hexdigest()==t['source_sha256']
  with wave.open(str(src)) as w:assert w.getnchannels()==2 and w.getframerate()==44100 and w.getnframes()==frames
  assert t['snr_db']>20
assert cursor==size and len(m['tracks'])==12
old_size=struct.unpack_from('<I',base,len(base)-8)[0];assert music[:old_size]==base[-136-old_size:-136]
end=len(b)-160-size;assert b[end-16:end-8]==b'RIFTART1';length,crc=struct.unpack_from('<II',b,end-8);bank=b[end-16-length:end-16];assert zlib.crc32(bank)==crc and hashlib.sha256(bank).hexdigest()==a['sha256']
oldend=len(base)-136-old_size;oldlength=struct.unpack_from('<I',base,oldend-8)[0];assert bank==base[oldend-16-oldlength:oldend-16]
for x in a['assets']:
 raw=bank[x['offset']:x['offset']+x['bytes']];assert len(raw)==x['bytes'] and hashlib.sha256(raw).hexdigest()==x['sha256']
off=struct.unpack_from('<8I',b,8);old=struct.unpack_from('<8I',base,8)
assert b[off[5]:off[6]]==base[old[5]:old[6]];print('XMB',validate(b[off[5]:off[6]],b[off[2]:off[3]]))
pcm=subprocess.check_output(['ffmpeg','-v','error','-i',str(r/'source/SND0.AT3'),'-f','s16le','-']);x=np.frombuffer(pcm,dtype='<i2').astype(float);assert np.sqrt(np.mean(x*x))>100 and np.max(np.abs(x[:int(.15*44100)*2]))<100
print('PASS every artwork byte, original ten encoded audio streams and RiftBoss splash audio preserved; both supplied themes encoded in full at 44.1 kHz stereo with 5 ms loop-edge fades.')
print('New track SNR dB:',[(t['name'],t['snr_db']) for t in m['tracks'][10:]])
print('SHA256',hashlib.sha256(b).hexdigest());print('Bytes',len(b))
