"""Preserve all published 1.8.4 artwork and append only the two supplied themes."""
from pathlib import Path
import hashlib,json,struct,zlib,subprocess,numpy as np
r=Path(__file__).resolve().parent;repo=r.parents[1]
b=(repo/'versions/1.8.4/EBOOT.PBP').read_bytes()
assert hashlib.sha256(b).hexdigest()=='a299459763b2712f8a065de9b488f66bd2754d95b0ea27bb4248154d9395942a'
assert b[-136:-128]==b'RIFTAU18';size,crc=struct.unpack_from('<II',b,len(b)-8);end=len(b)-136-size
length,art_crc=struct.unpack_from('<II',b,end-8);art=b[end-16-length:end]
assert art[-16:-8]==b'RIFTART1' and zlib.crc32(art[:-16])==art_crc
(r/'assets.bin').write_bytes(art);(r/'assets.json').write_bytes((repo/'qa/v184/assets.json').read_bytes())
bank=bytearray(b[end:-136]);assert zlib.crc32(bank)==crc
tracks=[]
old=json.loads((repo/'qa/rebuild180/audio/manifest.json').read_text())['tracks']
for i in range(10):
 off,frames,rate=struct.unpack_from('<III',b,len(b)-128+i*12)
 data=bytes(bank[off:off+frames]);assert hashlib.sha256(data).hexdigest()==old[i]['encoded_sha256']
 tracks.append(dict(index=i,name=old[i]['name'],offset=off,frames=frames,rate=rate,encoded_sha256=hashlib.sha256(data).hexdigest(),preserved_from='v1.8.4'))
encoder=Path('/tmp/rift184r2-encode-ima');subprocess.run(['gcc','-O2',str(repo/'qa/rebuild180/encode_ima.c'),'-lm','-o',str(encoder)],check=True)
for name in ['RiftMain','RiftAbomination']:
 src=r/'audio'/(name+'.wav');pcm=r/'evidence'/(name+'.pcm');ima=r/'evidence'/(name+'.ima')
 raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(src),'-ar','44100','-ac','2','-f','s16le','-'])
 x=np.frombuffer(raw,dtype='<i2').reshape(-1,2).astype(np.float64);n=220
 x[:n]*=np.linspace(0,1,n)[:,None];x[-n:]*=np.linspace(1,0,n)[:,None]
 pcm.write_bytes(np.rint(x).astype('<i2').tobytes())
 frames,snr=subprocess.check_output([str(encoder),str(pcm),str(ima)],text=True).split();data=ima.read_bytes();assert len(data)==int(frames)==len(x) and float(snr)>20
 tracks.append(dict(index=len(tracks),name=name,offset=len(bank),frames=len(data),rate=44100,encoded_sha256=hashlib.sha256(data).hexdigest(),source_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),snr_db=float(snr),loop_edge_fade_ms=5))
 bank.extend(data)
footer=b'RIFTAU19'+b''.join(struct.pack('<III',t['offset'],t['frames'],t['rate']) for t in tracks)+struct.pack('<II',len(bank),zlib.crc32(bank))
assert len(footer)==160
(r/'soundtrack.bank').write_bytes(bank+footer)
(r/'audio/manifest.json').write_text(json.dumps({'format':'RIFTAU19','tracks':tracks},indent=2)+'\n')
print('Preserved all 1.8.4 artwork and ten existing audio streams; appended both 44.1 kHz stereo themes.',len(bank),'audio bytes')
