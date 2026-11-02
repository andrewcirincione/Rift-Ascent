# 1.8.6-r2 ending transition correction

Canonical source stays in `qa/v186/source`; `GAME/src` and `portable_build` are identical mirrors. The in-game version remains 1.8.6. Immutable correction tag: `v1.8.6-r2`.

## Cause and correction

Combat exits into the Act 3/4/7 endings scheduled both a generic story transition and the combat finishing replay. The generic transition ran first, fading early and then revealing combat again. After the finishing replay, the cutscene renderer returned before applying the existing destination reveal, causing an abrupt cut.

The finishing replay now cancels the generic transition when it takes ownership. Its existing destination reveal is shared with the cutscene renderers before their early returns. Animation durations, battle resolution, enemy balance, art, audio and saves are unchanged. Ordinary story-to-story transitions retain their original fade.

## Verification

- `timeline.c` resolves real combat actions and exercises the complete renderer at every tick through the Act 3, Act 4 and Act 7 endings. It checks no early fade, monotonic final fade, a black midpoint, exact destination reveal pixels, replay cleanup, leader-first/minion-last behavior, and ordinary story transitions. Sound output alone is stubbed. Streamed art comes from the built candidate; the media audit verifies that the 1.8.6 art bank is unchanged.
- Both optimized and AddressSanitizer/UndefinedBehaviorSanitizer runs pass. Negative controls intentionally fail: the published renderer starts the generic fade; cancelling only that fade still fails the cutscene reveal pixel comparison. See `evidence/negative-*.txt`.
- The exact complete release EBOOT was exercised in PPSSPP 1.20.4 with PSP-1000 memory settings. Assisted fixtures place the player at the final attack; controller input and the real PSP executable resolve it. The debugger captures every completed framebuffer at the outer renderer's return, including each intermediate transition frame, rather than checking only the settled ending.
- Six PSP sequences cover Act 3 boss-last and minion-last, Normal/Fast speed, the special Act 4 ending, and Act 7 minion-last at both speeds. JSON records timing, screen, replay flags and brightness for each frame; contact sheets and MP4 sequences support visual review. Videos normalize each captured frame to 30 fps and are not wall-clock performance measurements. Normal speed samples the exact black midpoint; Fast speed advances 12 ticks per frame and can step over it. The native test checks every intermediate tick.
- Visual inspection confirms the finishing attack and death animation, one final fade, then the ending reveal without returning to combat. Full audio output, zero streamed-art errors and no bad memory accesses were checked. Media and stack audits pass. No gameplay code changed, so the 1.8.6 balance results remain applicable; campaign simulations were not rerun for this rendering correction.

## Reproduce

Build with PSPDEV v20261001 and Pillow 12.3.0, then run the native checks:

```sh
PSPDEV=/path/to/pspdev bash qa/v186/build.sh
bash qa/v186r2/test.sh
cp qa/v186/evidence/EBOOT.PBP qa/v186r2/evidence/EBOOT.PBP
gcc -O2 -std=gnu11 qa/v186r2/fixtures.c -o /tmp/r186r2-fixtures
/tmp/r186r2-fixtures
gcc -O2 -std=gnu11 qa/v186/tests/layout.c -o /tmp/r186r2-layout
/tmp/r186r2-layout
cp qa/v186/evidence/state-layout.json qa/v186r2/evidence/state-layout.json
RIFT_TOOLCHAIN=/path/to/toolchain RIFT_INSTANCE=186r2 RIFT_DISPLAY=601 RIFT_PORT=19601 RIFT_DIRECT=1 python3 -u qa/v186r2/qa_ppsspp.py
```

The toolchain directory follows the existing QA layout: `pspdev`, extracted official PPSSPP 1.20.4 AppImage in `squashfs-root`, and Xvfb/xdotool dependencies in `sysroot`; Python requires websockets 16.0 and Pillow 12.3.0, with FFmpeg on PATH. Physical PSP testing was not performed.

Tested complete EBOOT: 99,129,667 bytes; SHA-256 `350c4407133058929083faa835533e6259c814ddfb99f71b5610c52deced34a4`.
