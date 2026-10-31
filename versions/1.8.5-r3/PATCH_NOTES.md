# Rift Ascent 1.8.5 — story and challenge balance

- Forge is now one possible story outcome: four of each character's twenty
  stories offer card work when eligible. Ordinary gifts offer only Take/Leave.
- Cinder Gambit: 150 Gold for 6 HP, or 180 Gold plus Injury. Leaving stays free.
- Red Communion: its offensive bargain costs 3 HP and adds only 1 Vulnerable,
  while retaining 3 Strength for the next combat.
- Candle for a Stranger: its next-combat boon now costs 4 HP instead of 6.
- Elites have 8% more health; bosses have 10% more. Their displayed attack
  values, phase cycles and recovery turns are preserved.
- Preserve the 1.8.5-r2 startup stack fix, existing saves, full artwork and
  all twelve soundtrack tracks. Old pending third-choice gift selections
  become Leave; an already-open Forge remains resumable.

The established bot won 282 of 1,200 candidate campaigns (23.5%). A separate
900-run holdout finished at 24.4%, compared with 30.7% on the previous build.
These are comparative automated results, not predicted human win rates.
A stronger decision-aware bot won 46.3%, demonstrating the effect of better
choices. Event evaluation also replayed 300 identical states across every
affordable option, including 60 Cinder encounters.

Native sanitizer tests and the exact complete EBOOT are checked separately.
See `qa/v185r3/README.md` and its evidence for methods and limitations.
Physical PSP confirmation remains outstanding.

In-game version: **1.8.5**. Release tag: **v1.8.5-r3**.
Install the complete `RIFTASCENT` folder under `PSP/GAME`, retaining existing
save files when replacing the EBOOT. The install archive includes music.

EBOOT SHA-256:
`0c07974fed35463b76aa012cc7b9b5493a87de997085e177a995e4b886e4323b`
