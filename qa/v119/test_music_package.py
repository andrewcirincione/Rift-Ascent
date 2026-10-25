#!/usr/bin/env python3
"""Public test: exercise the exact new three-track ADPCM EBOOT format.

Test audio is synthetic and used ONLY in disposable emulator QA images.
No synth music is ever included in actual user release EBOOT files.
"""
from pathlib import Path
import math,struct,sys,zlib,runpy
base=Path(sys.argv[1]).read_bytes()
out=Path(sys.argv[2])
adpcm=runpy.run_path("tools/embed_churn.py")
adpcm["inspect"](base)
assert not base.endswith(b"RIFTM119")
assert base[-96:-88]!=b"RIFTM103"
assert base[-40:-32]!=b"RIFTM119"
segments=[];entry=[];length=0
for i in range(3):
 count=2205+i*271
 samples=[int(13000*math.sin((j*(160+37*i)*2*math.pi)/11025))
          for j in range(count)]
 encoded=adpcm["encode"](samples)
 assert len(encoded)==(count+1)//2
 entry.append((length,count));segments.append(encoded);length+=len(encoded)
payload=b"".join(segments)
footer=b"RIFTM119"+b"".join(struct.pack("<II",*e) for e in entry)
footer+=struct.pack("<II",len(payload),zlib.crc32(payload))
assert len(footer)==40
out.write_bytes(base+payload+footer)
final=out.read_bytes()
assert final[-40:-32]==b"RIFTM119"
assert struct.unpack_from("<I",final,len(final)-8)[0]==len(payload)
assert zlib.crc32(final[-40-len(payload):-40])==struct.unpack_from("<I",final,len(final)-4)[0]
for offset,count in entry:
 assert 0<=offset<len(payload) and (count+1)//2 <= len(payload)-offset
# The user's source music is *not* present in this disposable public QA build.
print("PASS: exactly three audio streams and a CRC-valid 40-byte RIFTM119 footer")
