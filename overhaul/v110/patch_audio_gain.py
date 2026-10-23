#!/usr/bin/env python3
"""Mix louder *normalized* user soundtrack while keeping impacts audible.

Separate full music comes from embedded IMA ADPCM. Synth soundtrack disabled.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
for a,b in [
 ("int l=audio_last*4/5+(int)audio_buffer[i*2];",
  "int l=audio_last+(int)audio_buffer[i*2]*2/3;"),
 ("int r=audio_last*4/5+(int)audio_buffer[i*2+1];",
  "int r=audio_last+(int)audio_buffer[i*2+1]*2/3;")
]:
 if a not in s:raise RuntimeError("1.1.0 audio gain anchor missing: "+a)
 s=s.replace(a,b,1)
p.write_text(s)
print("PASS 1.1.0: full-volume normalized tracks with 2/3-level sound effects and safe limiter")
