#!/usr/bin/env python3
"""Make a short ten-track ADPCM test trailer with no private soundtrack files."""
from pathlib import Path
import math, struct, sys, zlib, runpy
packer=runpy.run_path("tools/embed_soundtrack_103.py")
encoder=runpy.run_path("tools/embed_churn.py")
base=Path(sys.argv[1]).read_bytes()
encoder["inspect"](base)
length=0;streams=[];entries=[]
for i in range(10):
    count=2205+i*10
    samples=[int(14000*math.sin((j*(180+i*23)*6.283185307179586)/11025))
             for j in range(count)]
    data=encoder["encode"](samples)
    entries.append((length,count))
    length+=len(data);streams.append(data)
payload=b"".join(streams)
footer=b"RIFTM103"+b"".join(struct.pack("<II",*e) for e in entries)+struct.pack("<II",len(payload),zlib.crc32(payload))
assert len(footer)==96
out=Path(sys.argv[2]);out.write_bytes(base+payload+footer)
assert packer["verify"](out)==len(payload)
print("PASS synthetic tracks: 10 soundbanks, indexed footer, CPP PBP intact.")
