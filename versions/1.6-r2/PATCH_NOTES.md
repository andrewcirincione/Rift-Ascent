# Rift Ascent 1.6 — Revision 2

This revision targets the physical-PSP `800200D9` startup failure reported for the original 1.6 release. The in-game version remains 1.6.

- Corrected the first ELF load header's PSP module-info locator. The old header depended on custom-firmware repair; the modeled fallback includes appended media in its allocation and exceeds the standard PSP user-memory range.
- Moved larger textures into a resource bank inside the same EBOOT, using a bounded 4 MiB cache. Resident executable memory fell from 24,083,504 to 11,778,304 bytes (about 23.0 to 11.2 MiB). Texture resolution, color and alpha bytes are unchanged. No separate asset files are needed.
- Added stricter build checks for the module-info locator, address span, startup headroom and every packaged artwork resource.
- Replaced Proxy's Stars icon with generated dark bronze artwork, bright outlined numbers and an amber glow. The HUD glow switches off at zero and returns when Stars are gained. Card Star-cost badges are slightly larger, use the same high-contrast treatment and lose their glow when unaffordable. Free and X costs remain explicit.
- Preserved all cards, mechanics, saves, the five-track soundtrack and all XMB media, including RiftBoss audio.

Validation: the exact packaged EBOOT boots and plays in PPSSPP 1.20.4 configured as a 32 MiB PSP. Controller tests cover zero-to-positive and positive-to-zero Stars, unaffordable cards, X/free costs, upgrades, both other characters, Warden attack/Block/power, saving and a fresh-launch Resume. There were no streamed-resource errors. All 897 texture comparisons and 1,768 resident-versus-streamed rendered cases passed, including ASan/UBSan checks. The loader model reproduces the original allocation overflow and accepts this revision. These tests do not constitute physical PSP verification; confirmation on the affected console remains pending.

Replace `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep `SAVE.DAT` and `PROFILE.DAT`. If startup still fails, retain the error code and `RIFT_BOOT.LOG` from the Memory Stick root if one was created.

Evidence and rebuild tools: `qa/v160/memory_fix` and `qa/v160/star_contrast`.
