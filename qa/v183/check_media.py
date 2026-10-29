from pathlib import Path
import json,struct,hashlib,zlib
r=Path(__file__).resolve().parent;b=(r/'evidence/EBOOT.PBP').read_bytes();m=json.loads((r.parent/'rebuild180/audio/manifest.json').read_text());a=json.loads((r/'assets.json').read_text())
assert b[-136:-128]==b'RIFTAU18';size,crc=struct.unpack_from('<II',b,len(b)-8);music=b[-136-size:-136];assert zlib.crc32(music)==crc
cursor=0
for i,t in enumerate(m['tracks']):
 off,frames,rate=struct.unpack_from('<III',b,len(b)-128+i*12);assert off==cursor and frames==t['frames'] and rate==t['rate'];data=music[off:off+frames];assert hashlib.sha256(data).hexdigest()==t['encoded_sha256'];cursor+=frames
assert cursor==size and len(m['tracks'])==10 and m['tracks'][0]['rate']==44100
end=len(b)-136-size;assert b[end-16:end-8]==b'RIFTART1';length,crc=struct.unpack_from('<II',b,end-8);bank=b[end-16-length:end-16];assert zlib.crc32(bank)==crc
assert hashlib.sha256(bank[:a['base_bank_bytes']]).hexdigest()==a['base_bank_sha256']
for x in a['new_assets']:
 raw=bank[x['offset']:x['offset']+x['bytes']];assert len(raw)==x['bytes'] and zlib.crc32(raw)==int(x['crc32'],16)
assert not a['draft_missing'],a['draft_missing']
off=struct.unpack_from('<8I',b,8);assert b[off[5]:off[5]+4]==b'RIFF'
from build_splash import validate
print('XMB',validate(b[off[5]:off[6]],b[off[2]:off[3]]))
print('PASS ten exact supplied audio streams; stereo rates; 1.8.2 bank preserved; all new art checksums; no missing art; updated LP2 splash music')
print('SHA256',hashlib.sha256(b).hexdigest())

base=(r.parents[1]/'versions/1.8.2/EBOOT.PBP').read_bytes()
assert b[-136-size:]==base[-136-size:],'In-game audio changed'
old=struct.unpack_from('<8I',base,8)
validate(base[old[5]:old[6]],base[old[2]:old[3]])
assert b[off[5]:off[6]]==base[old[5]:old[6]]
import subprocess,numpy as np
pcm=subprocess.check_output(['ffmpeg','-v','error','-i',str(r/'source/SND0.AT3'),'-f','s16le','-'])
x=np.frombuffer(pcm,dtype='<i2').astype(float);assert len(x)>44100*2*13 and np.sqrt(np.mean(x*x))>100
assert np.max(np.abs(x[:int(.15*44100)*2]))<100,'Splash leading silence contains a click'
print('PASS splash decode non-silent stereo, unchanged safe combined-media budget; full soundtrack unchanged')
