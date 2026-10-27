# Rift Ascent 1.5.1 work

User requested all following changes, compiled complete EBOOT, PPSSPP tests/screenshots and GitHub release.

- Entirely new Nightshade sprites: white hood/cape, crouched silhouette, stronger background contrast. New idle/attack/block/power/death artwork; do not reuse old Nightshade sprites. Fewer attack poses.
- Warden attack: same total duration, quick overhead lift and downstroke, longer grounded follow-through, concise recovery; remove awkward holds.
- Smaller combat Victory artwork.
- Remove redundant one-modification-per-card wording.
- Restrained generated mystery-option selection artwork, no text overlap.
- Ten inventive relics with explicit triggers and tested synergies.
- Inventory/potion discard, deck and relic access on reward, combat, shop, map, mystery and campfire screens; preserve selection and pending choices.
- Potion reward icon inset; independent card/potion take/skip; allow freeing belt slots.
- Potion selection artwork must not overlap icon or text.
- Upgrade previews must reflect upgraded Energy costs, including cost-changing modifications.

Audit correction v1.5b-r2 is published and its public EBOOT SHA verified against the tested binary.

Baseline source: qa/v150b/source, audit correction commit 1ce66c01eab7b864c4bafcd120206c7c2bea3113. GitHub main already points to it. v1.5b-r2 Publish PSP release run 40517850900 completed successfully; verified public EBOOT digest 819af839606a893bb00cb6848de43140f071d3292e556b8469fc9bef8dc32fe5 (24,926,578 bytes). Do not overwrite old tags/assets. Current release/current.json still points to r2 until 1.5.1 is ready.

Current potion behavior: check_win automatically deposits potion if belt space and records ic[37]; skipping card does not remove potion. 1.5.1 needs separate explicit offer.

Canonical implementation for 1.5.1: this source/ directory; copy to GAME/src and portable_build through new build script after changes. Existing v150b card art can remain; all Nightshade character atlases must be fresh generated assets.

Implemented (October 27):
- Appended v22 save extension; v21 prefix migrates without re-granting already awarded potions.
- Independent explicit potion offer and card reward, full-belt recovery via inventory.
- Shared native/PSP inventory controller: six tabs, Triangle discards potions, Square opens deck outside combat; no underlying menu cursor mutation. Mandatory card decisions remain locked.
- Ten shared relics (IDs 53–62), including deferred triggers across Headbutt/True Grit choices, bounded once-per-turn/combat effects, and persistent Grave Ledger skip count.
- New generated white-hooded crouched Nightshade idle/attack/block/power/death atlases, fixed body/ground anchors; no old Nightshade model used.
- Warden contact timing skips frame 3 and holds sword-down frame 4; duration unchanged. Nightshade attack uses windup/contact/follow-through, omits extra recovery pose.
- Generated selection tablet replaces ornate mystery and inventory selections, icons inset outside ornament areas.
- Upgrade side preview is actual upgraded card with correct Energy badge and current-to-upgraded cost label.
- Smaller combat Victory artwork, ten generated relic icons.

Native checks currently pass: 39 relic/reward/menu scenarios, 756 valid upgrade/mod cost combinations (62 cost-changing), 174 Nightshade card resolutions, 15,952 independent damage/Block cases, 18,000 rarity/class reward draws, previous save migrations. Three thousand matched-seed automated runs complete without invalid states, stalls or divergent save continuation. Automated policy outcomes are evidence of functionality, not equal character balance. PPSSPP exact-EBOOT controls and screenshots completed; final SHA and evidence recorded below.

Final build SHA-256: 164f2ef589c5fe794d5923111cf6728f9b6dc71eee03854c907a67f623025c25 (25,418,418 bytes). All five soundtrack payloads and XMB assets verified. Canonical copies match; the ELF contains new r151 Nightshade sprites and no old r150b Nightshade sprites. Native ASan/UBSan checks pass. 1,522 upgrade/inventory layouts fit fully (maximum height 149px). PPSSPP complete animation input/capture pass succeeded on this SHA; reviewed Warden sword-down hold, grounded Nightshade attack, power, block, complete death and smaller Victory banner before rewards. Final full control pass completed successfully on the same SHA. Upgrade previews, independent rewards, full belt discard/take, all menu overlays and forced-choice safety passed.
