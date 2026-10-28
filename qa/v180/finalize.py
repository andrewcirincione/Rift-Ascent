#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,shutil
from PIL import Image
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1];ev=HERE/'evidence'
exe=HERE/'release/EBOOT.PBP';sha=hashlib.sha256(exe.read_bytes()).hexdigest()
smoke=json.loads((ev/'FEEDBACK_SMOKE_PASSED.json').read_text());assert smoke['sha256']==sha
assert len(smoke['cases'])==13
assert 'ALL 1.8 FEEDBACK TESTS PASSED' in (ev/'feedback-native.txt').read_text()
assert 'ALL 1.8 PRERELEASE NATIVE TESTS PASSED' in (ev/'native-regression.txt').read_text()
for p in ev.glob('*.ppm'):
 Image.open(p).save(p.with_suffix('.png'));p.unlink()
for p in (HERE/'source').iterdir():
 if p.suffix in ('.o','.elf','.PBP') or p.name in ('PARAM.SFO','symbols.txt','game.map','build.log'):
  p.unlink()
assert not any(p.is_dir() for p in (HERE/'source').iterdir())
for folder in ('GAME/src','portable_build'):
 dest=ROOT/folder;dest.mkdir(parents=True,exist_ok=True)
 for p in (HERE/'source').iterdir():shutil.copyfile(p,dest/p.name)
version=ROOT/'versions/1.8';version.mkdir(parents=True,exist_ok=True)
shutil.copyfile(exe,version/'EBOOT.PBP');(version/'EBOOT.PBP.sha256').write_text(sha+'  EBOOT.PBP\n')
shutil.copyfile(HERE/'assets.json',version/'resources.json');shutil.copyfile(HERE/'release/PACKAGE_AUDIT.json',version/'PACKAGE_AUDIT.json')
Image.open(ev/'psp-act4-random.png').resize((480,272),Image.Resampling.LANCZOS).save(version/'combat.png')
Image.open(ev/'psp-title.png').resize((480,272),Image.Resampling.LANCZOS).save(version/'title.png')
notes='''# Rift Ascent 1.8

Promotes the working 1.8 prerelease to the main release with the owner's hardware feedback addressed. Earlier published tags and downloads are preserved.

## Changes
- Replaced How To Play with Options. Debug shortcuts default OFF and persist in a separate A18_OPTIONS.DAT file. With debug enabled, L clears all living enemies through the normal reward flow; R advances one act without granting a boss clear. Inventory shoulder-button navigation is unchanged. No cheat-control hints were added to combat HUDs.
- Audited the legacy roster and replaced six designs with separate AI-generated full-body assets: Ash Crawler, Bone Sentinel, Glimmer Moth, Mire Oracle, Ashborne Monk, and Entropy Knight. Detached props, alternate poses, labels, and backgrounds are excluded from the sprites. Acts 1-3 keep their themed normal pools. Act 4 and beyond can draw any of the 30 normal enemies, excluding elites/bosses and duplicate pairs.
- Corrected the Infernal Colossus and Rift Behemoth facing direction.
- Normalized shared footer order to Potions, then Deck.
- Health fills are bounded by the painted frame aperture and use clamped proportional arithmetic, including increased maximum health. Very large numbers use compact labels rather than crossing the frame.
- Moon Spring now heals a flat 20 HP for free, up to maximum health. Its alternative costs 30 Gold to copy an eligible deck card with its upgrade/modification. Cancellation does not charge Gold.
- Removed unlock totals from the title screen; they remain in Unlocks.
- Removed developer/loading-test text from ordinary startup. Genuine package errors still display diagnostics. Preserved the actual-launch-path fix confirmed on the owner's PSP.
- Inset both save-slot text lines inside the highlight's decorative borders.
- Removed Act 4 clear totals from character selection and stopped incrementing those obsolete counters. Their reserved bytes remain for save compatibility. Secret-boss completion is shown only by the golden A at the top left. Act 3 wins and longest completed-act record sit at the top right.
- Removed the extra X marker identifying the selected pause option. The selection highlight and footer confirmation hint remain.

## Preserved
All 60 custom character cards and their individual generated illustrations; all new potions and relics; shared unlock progression; four independent run saves; endings and secret bosses; the purple Act 4+ arena; the complete existing five-track soundtrack and XMB preview audio. Existing A18 run/profile journals remain layout-compatible.

## Installation
For an existing 1.8 installation, replace ONLY EBOOT.PBP in its current folder. Keep every A18_* file and any older SAVE.DAT/PROFILE.DAT. The ZIP uses PSP/GAME/RIFTASCENT_A18 so it can coexist with older versions. The startup code uses the actual launch directory, including renamed folders.

## Verification scope
Fresh PSPDEV compilation; native ASAN/UBSAN progression and feedback regressions; 900 progression sequences; 10,000 Act 4/12 normal encounters; all 60 new card base/upgrade/mod cases; proportional-health assertions; full native-resolution review screenshots and all new card layout checks. The exact complete release EBOOT is tested using emulated controller input and read-only state assertions in PPSSPP, from an installation directory containing spaces. Saved-game fixtures are assisted test setups, not unassisted balance playthroughs. The owner confirmed prerelease 2 on real PSP hardware; this new release has not separately received that hardware confirmation.
'''
(version/'PATCH_NOTES.md').write_text(notes)
log='1.8 validation evidence\n'+(ev/'native-regression.txt').read_text()+'\n'+(ev/'feedback-native.txt').read_text()+'\n'+(ev/'render-tests.txt').read_text()+'\n'+(ev/'feedback-smoke.txt').read_text()+'\nPASS final release EBOOT '+sha+'\n'
(version/'validation.log').write_text(log)
source_paths=['AGENTS.md','release','versions/1.8','qa/v180','prerelease/1.8/source','prerelease/1.8/tests','prerelease/1.8/release/EBOOT.PBP','tools','GAME/src','GAME/verify_eboot.py','portable_build']
meta=dict(version='1.8',tag='v1.8',canonical_source='qa/v180/source',eboot='versions/1.8/EBOOT.PBP',eboot_sha256=sha,notes='versions/1.8/PATCH_NOTES.md',screenshot='versions/1.8/combat.png',validation='versions/1.8/validation.log',resources_manifest='versions/1.8/resources.json',install_folder='PSP/GAME/RIFTASCENT_A18',source_paths=source_paths,supersedes='v1.71-r2',basis='v1.8-pre.2',notes_summary='Debug Options, six generated enemy refreshes, proportional HP, Moon Spring choices, consistent footer, save/character/pause UI fixes, launch diagnostics hidden.')
(ROOT/'release/current.json').write_text(json.dumps(meta,indent=2)+'\n')
print('PASS stable 1.8 source, media, exact smoke hash and canonical release metadata:',sha)
