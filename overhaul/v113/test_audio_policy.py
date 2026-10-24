#!/usr/bin/env python3
"""Audit compiled-source music policy, fail if a procedural score is restored."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
for required in ("void rift_audio_generate(", "void rift_sound_effect(", "fx_age", "out[i*2]", "out[i*2+1]"):
    assert required in s, required
for forbidden in ("lead_calm", "lead_battle", "bass_calm", "bass_battle",
                  "step_samples", "pad_chords", "music_theme", "sin(", "synthesizer"):
    assert forbidden not in s, forbidden
assert "(void)battle;" in s
assert "(void)accent;" in s
assert "out[i*2]=limit(sfx);" in s
assert "out[i*2+1]=limit(sfx);" in s
print("PASS: native PSP audio generates SFX only and cannot synthesize background music")
