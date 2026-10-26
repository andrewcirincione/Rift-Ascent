# Rift Ascent 1.3.11

Continues the delivered 1.3.10 build. Install by replacing `PSP/GAME/RIFTASCENT/EBOOT.PBP`. Keep `SAVE.DAT` and `PROFILE.DAT`; old v16–v18 saves migrate to v19. The widened enemy fields and persistent endless/music settings make a v19 save 3,196 bytes plus its checksum. Back up a save before using it with an older build.

## Changes

- Elite encounters use the supplied **RiftFight2**. RiftFight3 now uses the final supplied **RiftFight3(1).wav** recording (105.143 seconds). Normal encounters choose **RiftFight** or **RiftFight3** with equal probability at battle entry and keep that choice across saves and menus. A separate hash chooses music without consuming the gameplay RNG. Checkers remains the title/boss music; RiftMenu remains navigation music. The three original compressed recordings are preserved exactly.
- The normal Act III boss proceeds directly to victory, without Gold, healing, relic or card rewards. The last lethal contact enters quarter-speed playback. Music pauses at that contact while attack effects remain audible; a brief VICTORY banner follows, then the correct rewards or ending. The final timing pass shortens the banner hold from 36 to 12 rendered frames and its fade from 6 to 3; the quarter-speed killing hit is retained. Navigation music starts after the sequence. Poison-only finishes also retain the finishing pause.
- **Endless Mode** is a separate main-menu choice. Every boss gives normal boss rewards, heals the player, and begins another 15-floor act. Act IV onward uses the late-act enemy pool; HP continues by 36 percentage points per act, boss HP by 30 points, and damage by 33 points. There is no final act condition. Enemy stats use 32-bit values instead of the former 999-HP/16-bit limit. Large HP meter calculations use wide intermediate arithmetic.
- Existing entrance blessings remain available until taken. When the unowned pool runs low, repeatable permanent Strength, maximum-HP/full-heal and Gold blessings keep the choice usable indefinitely. These repeatable boons do not consume relic slots.
- A generated cathedral-gate illustration fades in from black, displays the actual act number, then fades out before the existing blessing choice. The sequence lasts 120 game frames and also appears before Act I.
- The XMB `ICON0.PNG` uses the exact same painted logo as the main menu. `SND0.AT3` contains a 24-second excerpt from the supplied RiftBoss.wav, with gentle fades. Re-encoded as ATRAC3 LP4 (44.1 kHz stereo, 66 kbps), with a forward infinite loop and sample-count metadata that reserves decoder delay and tail capacity. The previous stream overclaimed available samples and lacked a loop chunk. The new section is 198,864 bytes.
- Enemy sprites now use connected-silhouette ground bounds, excluding detached stray pixels below the feet. Removed vertical idle displacement. All 36 models were inspected in native renders and representative actors cross-checked in PPSSPP.
- Block is quieter and shorter. Attacks add a stronger, longer dry crack and stronger mid-frequency body, with a soft limiter and deeper momentary music ducking. First-100-ms RMS changes against 1.3.10: Block −9.09 dB; low/medium/high attacks +5.21/+3.58/+1.78 dB. The largest measured single-impact peak is 26,529, below the PCM clipping limit.
- Violet Orb explicitly lists the upgraded potion results: Healing 23 HP, Strength +3, Guard 25 Block, Fire 25 base damage, Energy +3 and Insight drawing 4 cards. Fire's existing combat modifiers still apply to its base damage. All six results have resolver tests.
- A newly generated, compact black-iron shop selection plate replaces the oversized ornament. The shop and reward footers use a generated ivory Triangle on the same dark metal and bronze button style as X and Square. The full Violet Orb text fits the shop panel.
- Plating has a generated armor-scale icon beside the player. Ending the turn first applies Plating and presents a brief guard hold with visible Block. Only after that pause do enemies act. Saving/reloading during this phase does not grant Block twice. Its next-turn decay and the established damage rules are preserved.

- The hand continuation arrows use a newly generated bright silver/ivory asset with a restrained warm halo. Both directions retain the original gothic shape.
- All six Power poses are registered to a fixed boot anchor; the extra vertical translation has been removed. The stance stays within one pixel of the idle position while the casting poses animate.

## Validation

- Sanitized native suite: all inherited card arithmetic, preview/layout, map, reward rarity, relic, status, animation-observer and save checks pass. This includes 81,312 independent Attack/Whirlwind modifier cases, 348 Strength-potion cases and 1,568 incoming-intent cases.
- New suite: 1,000 consecutive endless act transitions; repeatable blessings after exhaustion; 42 combinations of Plating and Barricade with split resolution and save/resume; reward-free final boss; standard and poison-only finishing holds; all six Violet Orb potion values; music-selection persistence; all 36 model renders; extreme enemy HP arithmetic; v16/v17/v18 migration; endless lifetime-record persistence.
- A separate 1,000-run campaign simulation won 372 runs (37.2%) with no stalls. This is a heuristic simulation, not a measured human win rate or 1,000 manually played runs.
- PPSSPP 1.20.4: compiled PSP code, real controller input, memory assertions, actual audio-worker stream selection and PPSSPP's own 480×272 screenshots. Checked menu → endless intro → blessing → map; Plating's intermediate Block and subsequent decay; both final-boss branches; Act IV; new elite/normal music; shop and grounding. A second run checks the final EBOOT and records its SHA-256 in `evidence/ppsspp-final.txt`.
- Follow-up regressions: 12 normal/elite/final/endless single-hit, repeated-hit and area-hit finishes; exact mute boundary, quarter-speed clock, victory hold, once-only sound dispatch and unchanged nonlethal timing. All six Power foot anchors remain within one pixel. PPSSPP follow-up captures both hand arrows, matching buttons, actual Power play and all three ending branches; the soundtrack decoder sample count is unchanged while paused.
- The final PBP has valid imports, two ELF segments, original artwork integrity, five CRC-checked music streams, a matching logo icon, and a successfully decoded, non-silent ATRAC3 splash section. No physical PSP was available; XMB playback was validated by its packaged format/decoder rather than a console.

## Rebuild

Canonical sources are in `qa/v1311/source`; runtime copies are in `GAME/src` and `portable_build`. `build_source.py` synchronizes them and packs the shipped generated images. The one-time `patch_rules.py` and `patch_visual.py` document the transition; do not rerun them against already patched sources.

With `PSPDEV` set to the PSP SDK root, run from the project root:

```sh
bash qa/v1311/build_release.sh /path/to/complete-1.3.11-EBOOT.PBP /path/to/output/EBOOT.PBP
```

Alternatively, use a complete 1.3.10 EBOOT plus the user's new recordings:

```sh
bash qa/v1311/build_release.sh /path/to/1.3.10-EBOOT.PBP /path/to/output/EBOOT.PBP /path/to/RiftFight2.wav /path/to/RiftFight3.wav
```

The source ZIP includes the converted splash section. Full private music recordings are supplied by the existing EBOOT and/or the user's WAV files, not duplicated in the source ZIP.

Generated originals and prompts are in `qa/v1311/art` and `art-prompts.json`. The five assets use the built-in OpenAI image-generation tool, including revised Triangle and hand-arrow edits. The XMB logo reuses the existing generated menu logo. ATRAC encoding used the open-source [atracdenc](https://github.com/dcherednik/atracdenc) encoder, not a proprietary Sony tool.

To regenerate the XMB audio from the supplied recording, run `python qa/v1311/build_splash.py /path/to/RiftBoss.wav portable_build/SND0.AT3 --encoder /path/to/atracdenc` before building. The shipped source already includes the corrected SND0.

To replace only RiftFight3 in an existing complete 1.3.11 release: `python qa/v1311/replace_fight3.py /path/to/EBOOT.PBP /path/to/replacement.wav /path/to/output/EBOOT.PBP`. The replacement retains the executable, splash and four other audio streams byte for byte; its full decoded signal and checksum are validated.
