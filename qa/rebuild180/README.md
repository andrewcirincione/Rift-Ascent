# Fresh Rift Ascent 1.8 rebuild

Base: v1.71-r2, commit 16717db18a040f717acce2e63529621554cf0439. `BASE_PROVENANCE.json` records the stable source and EBOOT hashes. Nothing was copied from the preceding 1.8 prerelease. Canonical runtime source is `source/`; generated content, paintings, audio, build scripts and validation evidence are kept alongside it.

## Build

Requires a PSPDEV SDK (validated with v20261001 / GCC 15.2), Python 3, Pillow and NumPy. From the repository root:

```sh
export PSPDEV=/path/to/pspdev
python3 qa/rebuild180/content.py
python3 qa/rebuild180/story.py
bash qa/rebuild180/build.sh
bash qa/rebuild180/test.sh
python3 qa/rebuild180/check_media.py
```

The complete executable is `qa/rebuild180/evidence/EBOOT.PBP`. The intermediate `source/EBOOT.PBP` lacks the appended streaming banks and must not be distributed. The normal packer refuses missing artwork; `--draft` is only for development. The strict release contains no draft assets.

`pack_art.py` preserves the original stable bank byte-for-byte and appends the new textures. `pack_audio.py` reconstructs all ten soundtrack streams from checked-in `.ima` files, verifying their hashes. Rebuilding does not require the original WAV attachment. `audio.py` and `encode_ima.c` document initial conversion; `audio/manifest.json` records source, PCM and encoded hashes. `source/SND0.AT3` is the updated RiftBoss splash clip, validated as ATRAC3 LP2.

`art/processing.json` records original and optimized generated-image hashes. `shared_item_art.json` documents all forty legacy relic assignments and six legacy potion assignments. Icon sharing changes appearance only, never item IDs or behavior.

## Verification

`test.sh` runs native suites with AddressSanitizer and UndefinedBehaviorSanitizer:

- 36 completed runs, exact 20-card sets, idempotent account receipts, character-specific tenth-win and true-ending routes.
- 6,000 seeded normal fights (1,500 two-enemy encounters), plus 900 boss selections that cover all three original bosses in Endless while reserving the secret Act 4 encounter.
- All 120 base/upgraded custom card resolutions and preview isolation; Vesper discard/Sly/Pounce/Burst interactions and Proxy replay integration.
- All 20 new potion effects and ten relic effects.
- Four independent slots, torn-write recovery, failed-write rollback, debug persistence, stable v23 save import, 500 ten-win rarity schedules.
- Rendering all cards, items, 36 dialogue pages, 45 enemy sprites, collection views and large HP values, with streaming errors asserted zero.

`qa_ppsspp.py` drives the exact packaged executable in PPSSPP 1.20.4 / 32 MiB mode. It checks normal new-run/save/resume inputs, Options, all three fixture-assisted tenth-win paths, three pre-boss scenes, actual secret boss encounters, three true endings, the Endless gates, completed collections, all act music routes, active PCM output and reused legacy item icons. `RIFT_RESUME=1` separately verifies persistence after a cold process restart. Its external emulator/SDK/display paths are configured near the top of the script for this workspace and may need adapting elsewhere.

The fixture-assisted tests intentionally prepare late-game state in debugger memory; they exercise actual runtime progression and rendering without claiming full natural playthroughs. Native tests supply the broader repeated unlock/save coverage. No physical PSP test is claimed for 1.8.

Character checks can run as separate processes (`RIFT_CHARACTERS=0`, then 1 and 2), retaining account files with `RIFT_KEEP=1` and `RIFT_SEED_ACCOUNT=0` after the first. The first character observes the complete finisher timing. `RIFT_FAST_PRESENTATION=1` skips repeated screen fades and display replays after the game has resolved later fixture actions; it does not advance progression or grant unlocks. This avoids replaying the same long presentation repeatedly in the software-rendered test environment.

## Release

The exact tested binary, screenshot, patch notes and SHA-256 live under `versions/1.8-r2/`. `release/current.json` binds them to this source and the final evidence. `tools/prepare_github_release.py --validate-only` checks the source copies, all art/audio checksums, PSP imports, evidence and release metadata. The publish workflow packages and verifies the public download digests before making the release public.
