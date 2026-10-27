# Rift Ascent 1.4.0 validation

`source/` is canonical. `build_source.py` synchronizes GAME/src and portable_build; `prepare_art.py` packs the generated artwork. The unchanged existing art headers remain in those source folders.

Build the complete game using a PSP SDK:

```
PSPDEV=/path/to/pspdev bash qa/v140/build_release.sh versions/1.3.11/EBOOT.PBP qa/v140/evidence/EBOOT.PBP
python3 qa/v140/verify_media.py
```

The release used PSP GCC 15.2.0 (pspdev v20261001). `build_icon.py` regenerates the animated icon using Pillow, NumPy, FFmpeg and the included MIT-licensed PSP media encoder. It decodes the actual PMF and validates brightness continuity, the black background and frame count.

Native checks compile with GCC and `-fsanitize=address,undefined`:

- `qa.c`: inherited combat math, previews, descriptions, status icons, save migration, map and endless tests. 51,072 Attack cases; 30,240 Whirlwind cases; 348 Strength potion cases; 1,568 intent cases; all 184 base/upgraded card layouts.
- `relic_tests.c`: all 630 distinct relic pairs in both acquisition orders, all 36 together, potion interactions, Rust multi-hit/AoE and duplicate Compass migrations.
- `reward_endless_tests.c`: 84 independently referenced rarity definitions; 21,000 Rare boss offers across normal/endless acts; pending-offer repair; both normal-run ending choices; continue with preserved possessions, healing and Act IV/V progression.
- `potion_belt_tests.c`: 6,000 purchases with all six types, 20,000 full-belt denials without charge, 6,000 consumption/refill cases, map/shop healing and exact return navigation.
- `variety_tests.c`: 720,000 weighted selections and 60,000 shop offers; unseen/owned 3:1 weights, upgraded-card ownership, distinct shop inventory, unchanged rarity rates and deterministic saves.
- `map_choice_tests.c`: 40,000 maps and 120,000 forks; different rooms within two floors of every fork, before merging, compared with the old generator.
- `ashen_energy_tests.c`: 160 Ashen Strike combinations and 204 unaffordable plays, with no resource/card loss; dynamic free and X costs.
- `navigation_tests.c`: every position for 1–128-card grids, short last rows from all columns, page boundaries and actual upgrade/removal confirmation.
- `bully_tests.c`: 480 base/upgraded Bully cases with Vulnerable durations, Strength, Weak, Cruelty, Glass Emblem, enemy Block and exact live description.
- `presentation_tests.c`: all 36 idle cycles with fixed ground anchors, shortened 16-tick Power poses, once-only 650 ms chime, chest margins and reproducible screen fixtures.

For `qa.c` and `presentation_tests.c`, compile with `-I GAME/src` and link GAME/src/{game,artwork,motion_art,enemy_art,sound}.c and `-lm`. Other test files include game.c directly. Output logs are under `evidence/`; generated state fixtures and PPMs are excluded from git.

`qa_ppsspp.py` launches PPSSPP 1.20.4 with the complete packaged EBOOT, injects deterministic gameplay fixtures through its debugger, then uses actual controller input for navigation, purchases, potion use, power casting, ending choices and the killing-blow transition. Screenshots are PPSSPP's own 480x272 screenshots. The log records the exact tested EBOOT hash. Fixture injection makes the changed states reproducible; this is targeted emulator testing, not a claim of complete human-played runs or physical PSP testing.

Rarity reference: https://sts.wiki/cards (Ironclad table; values and mappings retained in rarity-reference.json), cross-checked against https://slaythespire2.wiki.gg/wiki/Bosses and individual card pages. Normal fights keep the existing no-Rare policy requested for this game; act bosses offer three distinct Rare cards.

The published EBOOT is copied without rebuilding after PPSSPP validation. `tools/prepare_github_release.py` verifies source parity, its hash, soundtrack CRC, screenshot and emulator validation before packaging the GitHub Release.

`ppsspp-full-regression.txt` preserves an earlier full presentation pass. The final release log is authoritative for the shipped EBOOT and includes the later map, Energy, sword, description and offer changes. Attack source prompts are in `art/extra-prompts.json`; `prepare_attack.py` extracts whole connected sprites and aligns measured boot anchors, without clipping swords at sheet cell boundaries.
