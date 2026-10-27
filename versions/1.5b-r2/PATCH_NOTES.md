# Rift Ascent 1.5b — functionality audit correction

Keeps the in-game version at 1.5b. Published as v1.5b-r2 to preserve the original release.

Fixes Forgotten Armory offering cards from the other character's pool and the Warden's random-hand replacement generating Nightshade/multiplayer cards. Existing saves and previously acquired cards are preserved.

Validated with 3,000 additional full runs on the corrected engine, 89,562 exact save/resume continuations, all 17 mystery events, 1,000 shops, 990 shared-relic singles/pairs, and 160,000 mixed-deck actions covering all 87 solo Nightshade cards. Both characters pass normal/endless progression and choice-control checks. The complete packaged EBOOT is tested in PPSSPP, including the corrected event and replacement behavior.

The automated policies win less often with Nightshade (12.6–24.6%) than Warden (51.8–63.6%). Policy bias and character balance remain separate from these functionality results; no balance values changed.

Includes the complete soundtrack. Install EBOOT.PBP in PSP/GAME/RIFTASCENT/ and keep SAVE.DAT and PROFILE.DAT. See qa/v150b/audit_followup/README.md for the audit, reproduction commands and limitations.
