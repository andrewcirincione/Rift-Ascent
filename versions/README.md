# Versioned Rift Ascent EBOOTs

Every explicitly named Rift Ascent version has its own `versions/<version>/` directory containing `PATCH_NOTES.md`, `EBOOT.PBP` and `SHA256.txt` after successful compilation. User-supplied media for 1.0.3 is compressed inside the delivered EBOOT; the GitHub build pipeline contains its reproducible packer but does not redistribute the original WAV assets.

- [1.0.0 baseline](1.0.0/PATCH_NOTES.md) — source commit and CI artifact from October 23, 2026.
- [1.0.1 candidate](1.0.1/PATCH_NOTES.md) — user feedback, tracked implementation, regression checks and remaining work.
- [1.0.2 candidate](1.0.2/PATCH_NOTES.md) — save/quit, clearer battle/deck/shop UI, previews, route inspection, CHURN battle music.
- [1.0.3 revision](1.0.3/PATCH_NOTES.md) — full soundtrack replacement, new XMB art, combat HUD fixes, map/dead-target repairs and wider card rebalancing.
- [1.1.0 revision](1.1.0/PATCH_NOTES.md) — integrated claymore hero sprite, smooth room icons, collision-free cards, wiki-accurate Inferno and boosted embedded soundtrack.

The released binaries are checked into the repository by the isolated successful GitHub Actions publishing job. Always preserve an independent copy of the last verified physical-PSP EBOOT.

- [1.1.0 87-card Ironclad expansion](1.1.0/IRONCLAD_87_RELEASE.md) — complete [87-card catalog](1.1.0/IRONCLAD_87_CATALOG.md), migrated saves, and verified PSP EBOOT.
