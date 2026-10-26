#!/usr/bin/env python3
"""Generate fixed-size UI bitmap slots for post-link AI texture installation."""
from pathlib import Path
import sys
out=Path(sys.argv[1])/"rift130_ui.h"
slots=[("hand",480,80),("hp",110,36),("energy",52,52),
       ("attack",70,91),("skill",70,91),("power",70,91)]
with out.open("w") as f:
 f.write("#pragma once\n#include <stdint.h>\n")
 for i,(name,w,h) in enumerate(slots):
  # Each 64-pixel unique fingerprint is verified and replaced with real
  # AI-source texture before any build is distributed.
  vals=[0x5A11+i]*64+[(0x0810+i)]*(w*h-64)
  f.write(f"static const uint16_t rift130_ui_{name}[{w*h}]={{\n")
  for j in range(0,len(vals),96):
   f.write(",".join(str(v) for v in vals[j:j+96])+",\n")
  f.write("};\n")
print("PASS: six 1.3.0 UI art slots allocated")
