from pathlib import Path
import struct,hashlib,json,argparse
r=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--draft',action='store_true');a=p.parse_args()
manifest=json.loads((r/'assets.json').read_text());assert a.draft or not manifest['draft_missing']
source=(r/'source/EBOOT.PBP').read_bytes();assert source[:4]==b'\0PBP'
out=source+(r/'assets.bin').read_bytes()+(r/'soundtrack.bank').read_bytes()
(r/'evidence/EBOOT.PBP').write_bytes(out);print(len(out),hashlib.sha256(out).hexdigest())
