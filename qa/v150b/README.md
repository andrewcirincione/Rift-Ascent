# Rift Ascent 1.5b — Nightshade

Adds a second, selectable character: Nightshade, a gothic dagger-wielding ninja based on the Silent's solo card mechanics. Normal and endless runs share the character picker. Existing saves continue as the Warden.

## Content and rules

- 70 HP, 99 Gold, 3 Energy; five Strikes, five Defends, Neutralize and Survivor. Serpent Ring draws two extra cards on the first turn.
- 89 card definitions and distinct generated illustrations: 80 ordinary solo reward cards, four starters, two Ancient blessings, Shiv, and two multiplayer reference entries excluded from solo generation.
- Poison, Shivs, active-discard Sly plays, temporary/permanent Sly, Retain, Dexterity, Thorns, Intangible, Burst, card-specific cost reductions, generated copies, and the Silent powers.
- Nine character relics/blessings, with shared Rift relics still available. Rewards, shops and random-card effects respect the character pool and existing encounter rarity rules.
- Five original four-pose animation atlases, generated dagger contact effects, and generated relic/status emblems. Player position uses measured hip/foot anchors. All card modifications remain visible and functional.
- Save version 21 preserves the pending choice/effect stack, physical card instance tags, cost reductions, and Nightmare copies. Legacy saves migrate by field, including the changed card-bit layout.

Reference: the extracted Silent catalog in `ptrlrd/spire-codex`, revision `c9c4854a4df21e8bc3e8c371f5a99a5f87ce0257` (July 15, 2026). The `reference` directory freezes the data used for this implementation. Upgrade calculation variables take precedence over stale preformatted upgrade text for Memento Mori and Precise Cut. All visual assets are original generated artwork; source illustrations are high-quality JPEG exports and runtime cards are 180×120 RGB565.

## Rebuild

`source/` is canonical. `build_source.py` mirrors it to `GAME/src` and `portable_build`; `prepare_art.py` deterministically emits the compiled artwork header from the supplied source assets. The one-time implementation scripts are development history, not rebuild entry points.

```bash
export PSPDEV=/path/to/pspdev
bash qa/v150b/build_release.sh versions/1.5-r2/EBOOT.PBP qa/v150b/evidence/EBOOT.PBP
python qa/v150b/verify_media.py
```

Never pass `--partial` for a release. Missing original art is a fatal build error. `evidence/art-manifest.json` proves all 89 card illustrations are distinct. The complete EBOOT retains the existing five music tracks and XMB graphics/music.

## Validation

- `ninja_audit.c`: 174 base/upgraded solo resolutions; 15,952 independent arithmetic cases; Sly, Burst, copies, Retain, poison, all 18 intents, full 128-Shiv replay, and 18,000 reward-pool draws. Includes the killing-hit/Strangle retarget case and Afterimage's pre-existing-power timing.
- `save_tests.c`: real legacy saves, v20 raw-bit migration, modified cards, current pending-choice roundtrip.
- `legacy_regression.c`: retained Warden arithmetic, animation/audio, 2,000 maps, 1,000 endless acts, shop/potion/relic/progression checks. Versioned ABI and the extra character-selection input are the intentional test adaptations.
- `preview_height_tests.c`: 12,128 layouts, including Sly, Inky, all eligible modifications, extreme status values; 2,800 rendered HUD-preservation frames. Descriptions fit within the 162-pixel height ceiling.
- `playtest.c`: 300 deterministic complete-run simulations across three heuristic policies, 13–20% wins, zero stalls. These are automated policies rather than human win-rate measurements.
- `animation_audit.c`: actual native render timelines for attacks, block, powers and death, plus contact audio assertions.
- `qa_ppsspp_animation.py` and `qa_ppsspp_status.py`: actual-input animation sequences, scrolled power status and generated relic icons, captured through PPSSPP screenshots.
- `qa_ppsspp.py`: exact packaged EBOOT, PPSSPP native 480×272 screenshots, physical controller commands, character selection, new runs, combat, rewards, shop, forced choices, Retain, status, save/quit/resume.

Run sanitizers with `ASAN_OPTIONS=detect_leaks=0`; AddressSanitizer and UndefinedBehaviorSanitizer remain enabled. LeakSanitizer cannot run under this execution environment's process tracing.
