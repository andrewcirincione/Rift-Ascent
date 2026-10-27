from pathlib import Path
import sys,struct,zlib
compiled,out=map(Path,sys.argv[1:3]);q=Path(__file__).parent
data=compiled.read_bytes();bank=(q/'assets.bin').read_bytes()
assert data[:4]==b'\0PBP' and data[-56:-48]!=b'RIFTM311'
out.write_bytes(data+bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank)))
print('Appended lossless resource bank; original DATA.PSP boundary preserved')
