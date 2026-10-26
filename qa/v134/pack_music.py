"""Append the unchanged private soundtrack from an existing complete EBOOT."""
from pathlib import Path
import struct,zlib,hashlib,sys
if len(sys.argv)!=4:raise SystemExit('Usage: pack_music.py ORIGINAL_EBOOT NEW_COMPILED_EBOOT OUTPUT_EBOOT')
old=Path(sys.argv[1]).read_bytes();size,crc=struct.unpack('<II',old[-8:]);tail=old[-40-size:]
assert old[-40:-32]==b'RIFTM119' and zlib.crc32(tail[:-40])==crc
assert hashlib.sha256(tail).hexdigest()=='77c0d56c3d352eaf79e3d902d7c911dc254efe79dc8c282f870549b92640401a'
new=Path(sys.argv[2]).read_bytes();assert new[:4]==b'\x00PBP' and new[-40:-32]!=b'RIFTM119'
Path(sys.argv[3]).write_bytes(new+tail)
print('PASS: all three original music tracks preserved byte for byte')
