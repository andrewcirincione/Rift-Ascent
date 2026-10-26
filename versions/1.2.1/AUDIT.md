# Rift Ascent 1.2.1 — change request audit

**Status: gameplay/audio technical preview. NOT a completed art release.**

The 1.2.0 procedural-image generator was a poor fit for the art requirement. None of its paint-by-shape outputs count as new AI illustrations. A separate, genuine image-generation pass and PSP visual QA of those images are required before claiming 1.2.1 art completion.

| # | Request | Current status |
|---|---|---|
| 1 | Only real AI-generated art | **OPEN** — no new full-resolution illustrations approved |
| 2 | PSP XMB splash music | **IMPLEMENTED FOR PRIVATE PACKAGE** — proper ATRAC3 SND0.AT3 from RiftBoss, 18s looping; PSP hardware test still needed |
| 3 | All map paths stay visible | **CODED / CI-TESTED** — visible historical graph separated from current navigable choices |
| 4 | Quick rise/pulse/fade played card | **CODED / EMULATOR SMOKE** — 24 frames |
| 5 | Played-card squashing/overlap | **CODED / EMULATOR SMOKE** — fixed 104x132 full card format |
| 6 | New AI gothic regular, elite and unique act-boss backgrounds | **OPEN** — must be real artwork and three boss rooms |
| 7 | Action log newest-left, turn delimiter | **CODED** — newest action left, gold turn separators |
| 8 | No unnecessary highlight for solo enemy | **CODED** |
| 9 | Same Strike art across copies | **CODED CONSISTENCY, ART OPEN** |
| 10 | All card illustrations AI-generated and recognizable | **OPEN** |
| 11 | Same Defend art across copies | **CODED CONSISTENCY, ART OPEN** |
| 12 | Matching rounded, compact HP capsules / external block | **CODED / EMULATOR SMOKE** |
| 13 | Slow last killing blow, fade into rewards | **CODED**, needs targeted timing QA |
| 14 | Replace poor card art starting with common cards | **OPEN** |
| 15 | Remove random shapes around enemies | **PARTIAL** — hit particle clutter removed, legacy enemy sprite art remains |
| 16 | Remove store healing service | **CODED / CI-TESTED** — shop has seven actual options |
| 17 | Full card text | **CODED** — multi-page scrolling rule area, further manual page review needed |
| 18 | Enemy-facing consistency | **CODED**, additional full-roster visual verification needed |
| 19 | Name incoming enemies at battle start | **CODED** |
| 20 | More multi-enemy combat | **CODED / CI-TESTED** — roughly 38/47/56% of hallway fights by act |
| 21 | Harder act progression | **CODED / CI-TESTED** — act/floor HP and intent strength scaling; balance playtests needed |
| 22 | Remove rusted forge 'effects previewed' | **CODED** |
| 23 | Moon Spring partial payment | **CODED / CI-TESTED** — 2 Gold per HP up to 20 HP |
| 24 | Replace geometric bonfire with AI illustration | **OPEN** |
| 25 | Multi-enemy health/block bars overlap | **CODED / EMULATOR SMOKE** — reduced width and wider spacing |
| 26 | Whirlwind X-cost spends energy | **CODED / CI-TESTED** — uses full manual-play energy even with bad free flag |

**Validation scope:** GitHub Actions tests the executable with the PSPDEV toolchain, native regression assertions, and PPSSPP boot/screenshot smoke scenes. The privately packaged soundtrack/XMB image is checked for checksum and format, but not tested on the user's PSP hardware. An emulator smoke test is not equivalent to every scene being manually playtested.
