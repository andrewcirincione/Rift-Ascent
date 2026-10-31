# Rift Ascent 1.8.5 — PSP startup correction (r2)

Fixes a PSP thread-stack overflow introduced by the 1.8.5 save-loading path.
The original EBOOT used 201,432 bytes of its 196,608-byte game-thread stack
even when scanning an empty save folder. An older save migration could use
229,952 bytes. The published EBOOT was reproduced byte for byte and the
empty-folder overflow was observed directly through the emulator debugger.
PPSSPP permitted the out-of-stack access, so the earlier gameplay tests missed it.

Save-and-quit snapshots and beta-import snapshots now live in separate,
non-inlined call frames. The permanent game-loop frame drops from 57,328 to
160 bytes. The largest audited startup/migration chain is 144,704 bytes;
the allocation remains 192 KiB. The build now rejects unsafe startup/save
stack use, including an explicit negative test against the original release.

The boot log now identifies revision 18502 and records worker entry before
save loading. HOME exit is registered before save initialization. A reported
storage initialization failure exits cleanly.

The game version remains 1.8.5. Combat balance, triple encounters, Forge/story
behavior, all artwork and all twelve complete soundtrack streams are preserved.
Existing save/account and beta files are retained.

Validation: exact packaged EBOOT playtested in PPSSPP, all three characters'
triple fights, targeting, enemy turns, save/quit/resume and story Forge; native
sanitizer regressions; cold starts with fresh, current, older stable, beta-only,
legacy and corrupt-save folders; explicit MIPS stack-boundary measurements.
Physical PSP confirmation is still required; emulator success is not hardware
certification.

Install: replace `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep all existing save/account
files. If another failure occurs, retain the new `ms0:/RIFT_BOOT.LOG`.
