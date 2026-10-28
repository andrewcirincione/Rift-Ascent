# Rift Ascent 1.8 Pre-release — startup correction (pre.2)

This revision fixes a reproduced boot failure in pre.1: art, music and saves
were opened from a hardcoded RIFTASCENT_A18 directory, and a failed art open
returned silently to the PSP home screen before displaying a frame.

The executable now captures the PSP launch path, opens its own embedded art
and soundtrack, and keeps A18 profile/run files beside the actual executable.
Spaces and renamed game folders are supported. ef0 paths are supported by the
path parser; a physical PSP Go test is not claimed. A missing/damaged package
or unwritable profile produces an on-screen diagnostic instead of a silent
exit. RIFT_BOOT.LOG includes the selected path and file-operation status.

All game artwork, all 60 custom card paintings, soundtrack, XMB audio and
progression mechanics are preserved. No prior release assets/tags are replaced.

## Install
Replace only EBOOT.PBP in your existing 1.8 game folder. Do not delete A18_*
profile or run files. The installation ZIP uses PSP/GAME/RIFTASCENT_A18, but
that exact folder name is no longer required. Keep it separate from stable
versions. Do not move saves unless moving the whole 1.8 installation.

## Verification and limitations
Fresh PSPDEV compile, native progression tests, sanitized launch-path tests,
and exact-EBOOT PPSSPP boot tests. The old binary fails in a renamed directory;
the corrected binary reaches title/save-slot screens with zero asset errors
in both normal and renamed directories, and loads all five music streams.
A deliberately damaged disposable package displays the diagnostic screen.
These are emulator checks, not a physical-PSP confirmation. The user's newly
mentioned boot log was not attached/available during this correction; this
fix addresses a reproduced startup defect, not a proven diagnosis of every
possible black-screen failure.
