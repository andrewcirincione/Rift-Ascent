# Rift Ascent 1.6.1

- Fixed Proxy's forward-jumping idle frame and reduced Vesper's idle drift with registered body/boot anchors.
- Added generated Warden idle animation with gentle breathing and a flowing cape. Removed the abrupt sword-raising frame at the end of his attack without extending its timing.
- Updated all twenty Vesper sprites with lighter weathered leather and muted gold armor, retaining her white cloak, hood, crouch and gritty style.
- Added a small golden particle burst when Proxy gains Stars.
- Matched Energy and Star cost badge sizes and numeral positioning on cards and previews.
- Repositioned spoils potion artwork, descriptions and Take/Leave choices inside the frame. Take/Leave respond to Up/Down. Unified their artwork with the main menu, other reward/relic actions, inventory/deck tabs, campfire and end-run buttons; centered labels within safe interiors.
- Aligned pause-menu icons and labels, with the appropriate controller hints.
- Added fades through black for Save & Quit, Resume and victory-to-spoils transitions.
- Fixed the brief battle-music restart between the Victory banner and spoils.
- Added a saved-run overwrite warning for normal/endless new runs, defaulting to keeping the save. Blocked the premature Save & Quit shortcut on character selection and title Help.

Retains the hardware-confirmed 1.6-r2 startup correction and bounded artwork cache. Existing SAVE.DAT and PROFILE.DAT files remain compatible. All five music tracks, RiftBoss home-screen audio, the animated icon and wallpaper are preserved.

Validated with native logic/rendering checks, ASan/UBSan and the exact complete EBOOT in PPSSPP's 32 MiB PSP mode, including controller tests and native-resolution screenshot review. Physical PSP testing of this patch remains pending.

Install: replace `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep `SAVE.DAT` and `PROFILE.DAT`.
