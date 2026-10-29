# Rift Ascent 1.8.4

Canonical source for 1.8.4. The immutable base is the published 1.8.3 release recorded in BASE_PROVENANCE.json. Its lineage is the fresh 1.8-r2 rebuild from 1.71-r2; no abandoned 1.8 prerelease content is used.

Changes are documented in ../../versions/1.8.4/PATCH_NOTES.md.

## Build and check

```sh
export PSPDEV=/path/to/pspdev
bash qa/v184/build.sh
bash qa/v184/test.sh
python3 qa/v184/check_media.py
RIFT_PREFIX=final- RIFT_FAST_PRESENTATION=1 python3 qa/v184/qa_ppsspp.py
RIFT_COLD=1 RIFT_INSTANCE=184cold RIFT_PREFIX=cold- RIFT_DISPLAY=532 RIFT_PORT=19532 RIFT_FAST_PRESENTATION=1 python3 qa/v184/qa_ppsspp.py
```

The packer preserves the complete published 1.8.3 artwork bank and all ten soundtrack streams, then appends the original gothic targeting crest. Full generation instructions and provenance are in art-specs.json and art/. The packaged splash music is retained unchanged.

Native ASan/UBSan suites cover progression, storage, all character card sources, custom loadouts, quantities, cross-character cards, account isolation, death receipts, speed-setting persistence, save migration, maps, species bags, boss rarity, mandatory-event escape safety, full-health healing, and simultaneous attack/block intent resolution. Renderer checks cover all 59 enemies, 25 intents, 42 mystery rooms, collection contents, custom menus, preview text layout and the removed victory notice. The drawing benchmark compares exact pixels against the original routines; its timings describe host CPU work, not physical PSP frame rates.

The PPSSPP harness runs the exact complete EBOOT and uses controller inputs. Targeted combat/menu/event states use explicit fixtures inserted at safe frame-entry breakpoints; this is targeted testing, not an unaided complete run. Presentation-only waits can be skipped except timing/music checks. Cold mode imports an actual serialized 1.8.3 slot before boot. Only isolated QA saves are used. Additional focused checks use RIFT_FOCUS184=1 (saved death and unskipped victory) RIFT_LATEST184=1 (full-health choices, mixed intent and speed), or RIFT_STARS184=1 (conditional resource HUD and actual cross-character Star gain, spending and retention). Set RIFT_TOOLCHAIN to a directory containing pspdev, the extracted official PPSSPP AppImage and sysroot Xvfb/xdotool binaries. This environment uses X11 TCP, dummy SDL audio output and disabled host vertical sync, host fast-forward and frontend frame skipping; PCM is checked inside the game's audio buffer. The AppImage's bundled .env must also use SDL_AUDIODRIVER=dummy. Frontend frame skipping does not skip the game's software render loop. Screenshots come from the PSP framebuffer; speed is compared over actual game render calls, not host wall time. Polling uses read-only debugger expressions to avoid repeatedly pausing the emulated CPU. Physical PSP hardware and XMB listening were not available.

evidence/final-tests.txt identifies the final tested binary and actual results. No test modifies the packaged executable.
