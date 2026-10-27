# Rift Ascent 1.6.1

Canonical source: `qa/v161/source`. Save format remains v23; existing 1.6 saves and profiles are compatible. The user confirmed the 1.6-r2 startup correction on physical PSP. This patch retains its module-info locator fix, streamed artwork, fixed 4 MiB cache, 32 MiB PSP target and conservative startup-memory checks.

## Presentation and controls

- Proxy idle frames are registered against boot/lower-body pixels, removing the cumulative forward offset. Vesper's new generated twenty-frame sheet preserves her white hood and cloak, with lighter weathered leather and antique-gold armor. Her idle is registered to the same lower-body anchor. Warden has six generated breathing/cape frames with fixed boots and sword orientation. All idle loops use a slow ping-pong sequence. Packing offsets and registration measurements are in `evidence/anchors.json`.
- Warden's contact/recovery holds the sword-down pose and no longer plays the final sword-raising frame. Attack timing is unchanged.
- Stars gains emit an observer-only `GFX_STARS` event, including gains from delayed effects. A short, bounded burst of existing generated golden spark art surrounds Proxy; no combat state or preview calculation is changed.
- Card Energy and Star costs share equal dimensions, a common baseline and the same visible-glyph centering/outline implementation.
- Spoils potion art, descriptions and vertically stacked Take/Leave choices sit inside the panel's safe area. The shared `button()` now uses the existing main-menu artwork, including reward/relic actions, inventory/deck tabs, campfire choices and end-run actions; labels use visible-glyph centering. Up/Down follows their visible arrangement; left/right remains for card choices. Potion and card decisions remain independent, including a full belt.
- Pause icons occupy a common column; both action labels share a left edge. The footer displays the relevant pause controls.
- Save & Quit, title Resume and pause Resume crossfade through full black. Input is suppressed during the transition, and a resumed pending combat task waits until the fade completes. The victory clip fades fully to black before spoils fades in.
- The audio worker reads track and mute state from one atomic route word. It cannot briefly resume the old battle track while the main thread is switching to spoils music. Music stays muted from the final contact through the victory/reveal interval.
- Normal and endless new runs display a default-cancel overwrite warning only when a saved run is available. Cancel/Back preserves that save. Character selection and title Help cannot open Save & Quit before a new run has actually begun.

## Validation

The scripts and reports in this directory cover native navigation and text layout, ASan/UBSan, a 6,764-assertion Regent regression, 897 texture comparisons and 1,768 identical resident-versus-streamed rendered cases. PPSSPP drivers install the complete EBOOT in `PSP/GAME/RIFTASCENT` and use real controller input plus PPSSPP's native 480x272 screenshots. Fixture injection is limited to setting up reproducible scenarios; subsequent actions use the game controls.

`evidence/final-tests.txt` identifies the exact final binary and the completed checks. `qa_ppsspp.py` covers all six potion descriptions and both selection states, pause alignment, overwrite/cancel and character-selection shortcuts, save/resume fades and resource costs. `qa_ppsspp_animation.py` captures complete idle/action/Stars/finisher sequences and checks the actual audio route. Physical PSP testing of 1.6.1 remains with the user.

## Rebuild

From the repository root, with PSPDEV set to a PSP SDK and Python with Pillow/numpy/scipy:

```sh
bash qa/v161/build_release.sh
```

This produces `qa/v161/evidence/EBOOT.PBP` with the unchanged five-track soundtrack and XMB media from the hardware-confirmed 1.6-r2 package. `build_source.py` mirrors the canonical source into GAME/src and portable_build. `prepare_art.py` crops, registers, resizes and indexes the generated pixels; it does not draw replacement art. `pack_assets.py` and `append_assets.py` reproduce the lossless resource bank and maintain the executable's memory budget.
