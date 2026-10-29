# Rift Ascent 1.8.1

Canonical source: `qa/v181/source`. This update starts from the published
fresh v1.8-r2 tree (606e0f4819a37efff6d3a0b32cd7c268cdac9838), which was rebuilt
from v1.71-r2. No abandoned 1.8 prerelease content is imported.

## Build

With the PSP SDK installed, from the repository root:

```sh
PSPDEV=/path/to/pspdev bash qa/v181/build.sh
python3 qa/v181/check_media.py
bash qa/v181/test.sh
RIFT_FAST_PRESENTATION=1 python3 qa/v181/qa_ppsspp.py
RIFT_FAST_PRESENTATION=1 RIFT_COLD=1 python3 qa/v181/qa_ppsspp.py
```

The final EBOOT is produced at `qa/v181/evidence/EBOOT.PBP`. Packaging preserves
v1.8-r2's artwork prefix and all ten complete soundtrack streams byte-for-byte,
then appends the 25 new illustrations. `pack_art.py` refuses missing art unless
explicitly invoked with `--draft`; draft builds cannot pass release validation.
PNG sources, prompts and generation records are in `art/`. Each of the 60 story
events has three authored dialogue pages; five events share each of four
character-specific setting paintings. Eight new relic paintings extend the
shared icon library for both new and existing relics.

The checked-in `SND0.AT3` is the updated RiftBoss recording encoded as a
14-second LP2 stereo preview. ICON1 + SND0 now total 471,312 bytes, under the
480 KiB safe packaging budget. The prior 1.8 combination was 637,200 bytes.
`build_splash.py` can recreate it from the user's original RiftBoss WAV using
atracdenc. In-game RiftBoss remains the full, unchanged 44.1 kHz recording.
The XMB limitation was checked against the primary toolkit documentation:
https://github.com/TotalKommando/psp-media-toolkit

## Mechanics and compatibility

- Act 7 always ends at The Unbegotten: 610 base HP, three escalating phases,
  separate entrance/aftermath scenes and a new arena. At Act 7 its scaled HP
  is 2,672. This is independent of the character-specific Act 4 secret boss.
- After Act 3, HP scaling gains 5 percentage points times `(act - 3)^2` and
  damage scaling gains 2 points times `(act - 3)^2`. Acts 1-3 are unchanged.
- Normal/elite wins roll once for a 5% bonus story, presented after ordinary
  spoils. Each character has a 20-entry non-repeating bag, reset after all 20.
  Story gifts and the seen bag persist in the run save.
- Rare mystery: 4% of mystery rooms offer three distinct rare cards, including
  at least one from each other character. Offers use cards whose immediate
  effects are usable by the receiving character; taking a card does not alter
  account unlocks. Leaving is free. Full decks can leave without becoming stuck.
- Relics 74-93 are available immediately. Exhausted boss, elite, treasure and
  bonus-story relic pools offer repeatable Strength, Vitality or Fortune.
- The entire 28,224-byte 1.8 run prefix remains intact. New fields are appended;
  the first 64 relic slots retain their offsets, with 32 overflow slots in the
  extension. Alternating old/new snapshots can both be loaded. Account format,
  unlocks, win counts, secret endings and four save slots retain their identities.

## Validation scope

Native address/undefined-behavior sanitizer suites exercise all 20 relics,
60 stories/180 pages, 100,000 event rolls, 900 foreign-card offers and every
eligible foreign card for both other heroes, overflow saves, old-save recovery,
full-deck choices and Act 7 transitions. Existing progression/card/potion/storage
regressions remain included. All new dialogue fits the four-line scene area.

PPSSPP tests operate the exact release EBOOT using controller inputs for the
Act 7 entrance, combat, full victory hold, aftermath, rewards, Act 8 and a
second-process cold load of the saved pre-boss scene. Broader gameplay checks
were completed on the prior build with identical gameplay code; the final
change corrects a render-only victory hold. `evidence/final-tests.txt` records
both hashes, coverage and a corrected QA fixture error from the first pass.
Late-game states are seeded fixtures, not a claimed unaided seven-act run.
Repeated presentation fades may be skipped after state resolution to accommodate
slow software rendering. Screenshots and logs are kept in `evidence/`. XMB audio decoding, loop metadata and combined media size are
validated separately: PPSSPP does not reproduce a physical PSP's XMB browser.
