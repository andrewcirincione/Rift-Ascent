from pathlib import Path
import subprocess,hashlib,json,zipfile,struct,zlib
ROOT=Path(__file__).resolve().parent
SRC=ROOT/'audio/source';OUT=ROOT/'audio';SRC.mkdir(parents=True,exist_ok=True)
ZIP=ROOT.parents[2]/'upload/Soundtrack(1).zip'
with zipfile.ZipFile(ZIP) as z:
 for n in z.namelist():
  if n.lower().endswith('.wav'):(SRC/Path(n).name).write_bytes(z.read(n))
names=['RiftBoss','RiftMenu','RiftFight','RiftFight2','RiftFight3','RiftFight4','RiftFight5','RiftMenu2','RiftMenu3','RiftMenu4']
manifest={'source_zip_sha256':hashlib.sha256(ZIP.read_bytes()).hexdigest(),'format':'Stereo IMA ADPCM, low nibble left / high nibble right; independent zero initial state','tracks':[]}; bank=bytearray();footer=bytearray(b'RIFTAU18')
for index,name in enumerate(names):
 source=SRC/(name+'.wav');rate=44100 if index==0 else 22050;pcm=OUT/(name+'.pcm');encoded=OUT/(name+'.ima')
 subprocess.run(['ffmpeg','-v','error','-y','-i',str(source),'-ar',str(rate),'-ac','2','-f','s16le',str(pcm)],check=True)
 frames,snr=subprocess.check_output(['/tmp/rift-ima',str(pcm),str(encoded)],text=True).split();data=encoded.read_bytes();assert len(data)==int(frames)
 footer+=struct.pack('<III',len(bank),len(data),rate);bank+=data
 manifest['tracks'].append(dict(index=index,name=name,rate=rate,frames=int(frames),seconds=int(frames)/rate,snr_db=float(snr),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),pcm_sha256=hashlib.sha256(pcm.read_bytes()).hexdigest(),encoded_sha256=hashlib.sha256(data).hexdigest()))
 print(name,len(data),snr,flush=True);pcm.unlink()
footer+=struct.pack('<II',len(bank),zlib.crc32(bank));assert len(footer)==136
(OUT/'soundtrack.bank').write_bytes(bank+footer);(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
