# Rift Ascent 1.1.0 — Ironclad audit candidate EBOOT

- **Download:** provided in ChatGPT as `Rift-Ascent-v1.1.0-Card-Audit-Candidate.zip` and direct `EBOOT.PBP`. Full user-supplied music is embedded only in the ChatGPT attachment.
- **Full EBOOT size:** 14,435,442 bytes
- **SHA-256:** `f003b1eaa2a7f24f0a9ee97f9748023aedefa3d361af5b14a940a95ddd372c4f`
- **Music:** same ten original compressed tracks as the user's existing 1.1.0 EBOOT (11,533,436 bytes). No new music assets are checked into the repository.
- **XMB artwork:** same key art as 1.1.0.
- **Compiled engine:** [GitHub Actions success #40032086884](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40032086884). PSPDEV, native ASAN/UBSAN deterministic card regression, PPSSPP render and music-index check passed.
- **Audited source:** `overhaul/v110/audit_cards_patch.py`.
- **Full card audit:** [IRONCLAD_CARD_AUDIT.md](IRONCLAD_CARD_AUDIT.md).
- **Change scope:** Battle Trance turn-long draw lock, Pommel Strike+ accurate draw/damage, Body Slam+ zero cost and damage, Armaments+ Block, Dominate Vulnerable/Strength/Exhaust and metadata, several corrected Skill classifications.
- **Known remaining:** choice UI for Brand, Burning Pact and Armaments base; Conflagration attack-only scaling; more than half the wiki's Ironclad roster absent. This is **not** full STS2 Ironclad parity.
- **Hardware:** PSP-3000 / ARK-5 not physically verified; preserve `SAVE.DAT` and the last stable EBOOT. This is a separate candidate branch, **not a silent replacement** for 1.1.0.

The binary is intentionally not committed into the public GitHub repository because the actual ten user-provided music tracks are not source assets present in GitHub Actions. The binary's SHA-256 is retained for traceability.
