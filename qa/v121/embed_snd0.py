#!/usr/bin/env python3
"""Package a real PSP XMB preview in SND0.AT3 without modifying DATA.PSP.
Usage: python qa/v121/embed_snd0.py compiled_EBOOT.PBP SND0.AT3 final_EBOOT.PBP
The three privately embedded music tracks remain at end of the PBP.
"""
from pathlib import Path
import struct,sys,zlib
if len(sys.argv)!=4:raise SystemExit(__doc__)
src=Path(sys.argv[1]).read_bytes();sound=Path(sys.argv[2]).read_bytes()
if src[:4]!=b'\0PBP':raise ValueError('Invalid PBP')
if sound[:4]!=b'RIFF' or b'WAVE' not in sound[:16]:raise ValueError('SND0 must be real RIFF ATRAC3')
if not any(sound[i:i+2]==b'\x70\x02' for i in range(12,min(88,len(sound)))):
 raise ValueError('Not ATRAC3 codec 0x0270')
offs=list(struct.unpack_from('<8I',src,8))
if offs!=sorted(offs):raise ValueError('Nonmonotonic PBP offsets')
if offs[5]!=offs[6]:raise ValueError('PBP already has XMB audio')
if len(sound)>500*1024:raise ValueError('XMB audio > 500 KB')
# PBP entries: SND0 at index 5, DATA.PSP index 6, DATA.PSAR index 7.
data=bytearray(src[:offs[6]]+sound+src[offs[6]:])
for i in (6,7):struct.pack_into('<I',data,8+i*4,offs[i]+len(sound))
new=struct.unpack_from('<8I',data,8)
assert data[new[5]:new[6]]==sound
assert data[new[6]:new[7]]==src[offs[6]:offs[7]]
if src[-40:-32]==b'RIFTM119':
 length,crc=struct.unpack_from('<II',src,len(src)-8)
 assert zlib.crc32(src[-40-length:-40])==crc
 assert data[-40-length:]==src[-40-length:]
Path(sys.argv[3]).write_bytes(data)
print('PASS: playable ATRAC3 SND0 inserted, ELF and private music preserved')
