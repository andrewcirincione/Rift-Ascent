# Rift Ascent 1.7

Canonical source: `qa/v170/source`. This release adds character stories while preserving save format v23, the hardware-confirmed startup correction, the 32 MiB PSP target and the bounded 4 MiB artwork cache.

## Story and presentation

Warden, Proxy and Vesper have linked histories in the burning lower wards, blue star-court and white/gold summit. `STORY.md` records the backstory; `story_script.py` authors and validates the complete dialogue and emits `story.json` and `source/story170_data.h`.

Each character has ten entry statements for each of the three acts and ten for Endless: 120 statements total. Each character/boss pairing has five three-beat exchanges: 45 exchanges and 135 boss pages. Twelve new entry paintings and nine confrontation paintings appear in a centered 336x126 rectangle on black, with fully visible text below. All 255 pages fit the actual runtime font and four-line limit. There is no typewriter wait or pixelation filter.

The route is act splash, existing blessing selection, entry scene, map. X advances immediately. Boss selection leads to the matching dialogue, then its fight; acts four onward bypass boss dialogue. The Furnace Maw, Crimson Judge and Hollow King have new generated combat models matching the confrontation paintings. Their stats and AI are unchanged.

Entry scenes are silent apart from the X confirmation sound. Boss scenes start RiftBoss and keep the same decoder playing into combat. Scene boundaries fade through full black over 32 animation ticks; the audio route changes at the black midpoint. Dialogue pages within an exchange share one illustration and advance directly. Buffered X presses cannot choose a map room or play a card after leaving a scene. The title now offers Resume Run, Begin Ascent and How to Play; post-victory Endless and existing Endless saves remain available.

## Validation and rebuild

`evidence/final-tests.txt` identifies the exact released binary and completed checks. Native coverage includes every dialogue page, variant distribution, unchanged encounter RNG, correct boss identities, save/shortcut behavior, fade midpoint, bounded input buffering and scene sound isolation. Existing map, event, blessing, Regent combat and artwork recovery regressions also pass. Resident and streamed rendering agree across 2,287 cases, including all new dialogue pages.

`qa_ppsspp.py` drives the complete EBOOT in PPSSPP 1.20.4, 32 MiB PSP mode, using real controller edges and PPSSPP's native 480x272 screenshots. Fixtures establish starting states. Frame breakpoints sample actual fade framebuffers without changing game timing or code. Scene audio checks inspect the live mixer and track decoder. `qa_ppsspp_animation.py` retains the final-hit music regression. Physical PSP testing of 1.7 remains pending.

From the repository root, with PSPDEV set and Python/Pillow/numpy available:

```sh
bash qa/v170/build_release.sh
```

The build regenerates dialogue/art banks, mirrors canonical sources, builds the corrected executable and preserves all five soundtrack payloads, the animated icon, splash wallpaper and RiftBoss home-screen audio from 1.6.2. `make_ppsspp_driver.py` generates the emulator driver from the established harness plus the 1.7 control flow.
