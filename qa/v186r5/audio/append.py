"""Append Abomination to the unchanged 12-track bank produced by pack_art.py."""
from pathlib import Path
import hashlib,json,struct,zlib
q=Path(__file__).resolve().parent;repo=q.parents[2];bank=repo/'qa/v186/soundtrack.bank';old=bank.read_bytes()
assert old[-160:-152]==b'RIFTAU19'
n,crc=struct.unpack_from('<II',old,len(old)-8);payload=old[:-160];assert n==len(payload) and zlib.crc32(payload)==crc
new=(q/'RiftAbomination.ima').read_bytes();m=json.loads((q/'RiftAbomination.json').read_text());assert hashlib.sha256(new).hexdigest()==m['sha256'] and len(new)==m['frames']
entries=old[-152:-8]+struct.pack('<III',len(payload),m['frames'],m['rate'])
payload+=new;footer=b'RIFTAU20'+entries+struct.pack('<II',len(payload),zlib.crc32(payload));assert len(footer)==172
bank.write_bytes(payload+footer);print('PASS full Abomination track appended; all prior 12 streams preserved:',len(new),'bytes')
