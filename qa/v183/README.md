# Rift Ascent 1.8.3

Canonical source for the 1.8.3 release. The immutable base is published 1.8.2,
recorded in BASE_PROVENANCE.json. Its lineage is the fresh 1.8-r2 rebuild from
1.71-r2; no abandoned 1.8 prerelease content is used.

Changes are documented in ../../versions/1.8.3/PATCH_NOTES.md.

## Build and check

```sh
export PSPDEV=/path/to/pspdev
bash qa/v183/build.sh
bash qa/v183/test.sh
python3 qa/v183/check_media.py
RIFT_FAST_PRESENTATION=1 python3 qa/v183/qa_ppsspp.py
RIFT_COLD=1 RIFT_INSTANCE=183cold RIFT_DISPLAY=532 RIFT_PORT=19532 RIFT_FAST_PRESENTATION=1 python3 qa/v183/qa_ppsspp.py
```

The packer preserves the complete 1.8.2 artwork bank and all ten soundtrack
streams, then appends twelve new original enemy textures. --draft is only for
local build checks; release validation rejects missing assets. Source images,
full generation prompts and output provenance are in art/ and art-specs.json.
Brightness and targeting adjustments are renderer changes, preserving the
original source images.

Native suites use ASan/UBSan. Rendering checks cover all 59 enemy sprites,
24 overhead intents, 39 event-result mappings, unlock/base collection items,
and the combat-deck transition. revision183.c checks 665,280 card sources over
all characters and zero/single/full masks, collection navigation, boss notices,
and both 1.8.2 save formats. Existing progression, storage, combat, rarity,
event and story suites remain in test.sh.

The PPSSPP harness tests the exact complete EBOOT with real controller input.
Targeted combat/menu/event states are assisted fixtures inserted at a safe
render-entry breakpoint; they are not an unaided whole-game playthrough.
Presentation-only delays can be skipped except the measured combat transitions.
The cold mode imports a genuine serialized 1.8.2 slot before boot. No user saves
are used. evidence/final-tests.txt records the final hash and actual results.
Physical PSP hardware/XMB listening was not available; splash audio is decoded
and checked structurally, and in-game PCM output is checked in PPSSPP.
