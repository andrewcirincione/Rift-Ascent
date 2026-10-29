# Rift Ascent 1.8.1

- **The Unbegotten** waits at the end of Act 7: an exceptionally hard, three-phase abomination of divinity, with new arena art and cutscenes before and after the fight. Defeat it to continue into Act 8.
- **Expect a Fight** now prevents further Energy gains that turn, including Bloodletting, Offering, potions and relic triggers. Normal Energy recovery resumes next turn.
- Enemy health and damage scale progressively faster after Act 3. The first three acts retain their previous scaling.
- **20 additional base relics**, available immediately, bringing the relic catalog to 94. New art also expands the shared artwork used by existing relics. The unlock gallery includes a Base Relics category.
- Relic capacity increases from 64 to 96. Exhausted boss, elite, treasure and story-relic pools offer repeatable Strength, Vitality or Fortune bonuses instead of blank rewards.
- **60 character story events: 20 each for Warden, Vesper and Proxy.** Normal and elite victories have a 5% chance to trigger an illustrated three-page encounter followed by a bonus card or relic. Events do not repeat until the character's 20-event run bag has been seen.
- **The Nameless Exchange**, a rare mystery-room encounter, offers a choice of rare cards from the other two characters. Taking a card is optional and does not change account unlocks.
- **PSP splash music fix:** the updated RiftBoss recording is retained in a 14-second high-quality stereo preview. Combined animated-icon and audio size is reduced to fit the PSP XMB budget. All ten complete in-game music streams remain unchanged.
- Existing 1.8 saves, shared unlock progress, character win counts and golden A badges are preserved. The Act 4 secret endings and Act 3 Endless availability rules remain unchanged. Other Endless bosses still come randomly from Acts 1-3.

## Installing

Extract the PSP ZIP to the Memory Stick root, or replace `PSP/GAME/RIFTASCENT/EBOOT.PBP` with the standalone EBOOT. Keep `SAVE.DAT`, `PROFILE.DAT`, and every `ASCENT18_ACCOUNT.*` / `ASCENT18_RUN_*` file. Version 1.8 run saves are upgraded automatically when saved again.

## Validation

Native sanitizer regressions cover the added mechanics, all 60 stories, all 20 relics, 100,000 story rolls, foreign-card resolution, existing unlock progression, old-save migration and relic overflow. The packaged EBOOT is also tested in PPSSPP with controller inputs and explicitly assisted late-game fixtures. XMB audio is separately decoded and checked for valid loop metadata and combined media size; physical PSP XMB playback has not been verified in this environment.
