from pathlib import Path
import struct,hashlib,json,argparse,os,tempfile
r=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--draft',action='store_true');a=p.parse_args()
manifest=json.loads((r/'assets.json').read_text());assert a.draft or not manifest['draft_missing']
source=(r/'source/EBOOT.PBP').read_bytes();assert source[:4]==b'\0PBP'
out=source+(r/'assets.bin').read_bytes()+(r/'soundtrack.bank').read_bytes()
# Publish a complete package atomically: readers must never see a partial bank.
with tempfile.NamedTemporaryFile(dir=r/'evidence',prefix='package-',suffix='.tmp',delete=False) as f:
 f.write(out);f.flush();os.fsync(f.fileno());temporary=Path(f.name)
assert temporary.stat().st_size==len(out)
os.replace(temporary,r/'evidence/EBOOT.PBP')
print(len(out),hashlib.sha256(out).hexdigest())
