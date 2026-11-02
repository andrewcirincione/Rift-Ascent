"""Encode the supplied RiftAbomination WAV as the game's stereo IMA stream.
No looping, trimming or synthesised replacement; one byte per stereo frame.
"""
from pathlib import Path
import array,hashlib,importlib.util,json,subprocess,sys
root=Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('ima',root/'tools/embed_churn.py');ima=importlib.util.module_from_spec(spec);spec.loader.exec_module(ima)
p=Path(sys.argv[1]);rate=22050
pcm=subprocess.check_output(['ffmpeg','-nostdin','-hide_banner','-loglevel','error','-i',str(p),'-ac','2','-ar',str(rate),'-f','s16le','-acodec','pcm_s16le','-'])
a=array.array('h');a.frombytes(pcm)
if sys.byteorder!='little':a.byteswap()
left=ima.encode(a[::2]);right=ima.encode(a[1::2]);frames=len(a)//2
out=bytes(((left[i//2]>>(4*(i%2)))&15)|(((right[i//2]>>(4*(i%2)))&15)<<4) for i in range(frames))
q=Path(__file__).resolve().parent;(q/'RiftAbomination.ima').write_bytes(out)
meta={'source_filename':p.name,'source_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'pcm_sha256':hashlib.sha256(pcm).hexdigest(),'rate':rate,'frames':frames,'seconds':frames/rate,'channels':2,'bytes':len(out),'sha256':hashlib.sha256(out).hexdigest(),'format':'continuous stereo IMA ADPCM, low nibble left, high nibble right; predictor/index start at zero','track_index':12}
(q/'RiftAbomination.json').write_text(json.dumps(meta,indent=2)+'\n');print(json.dumps(meta,indent=2))
