# Rift Ascent 1.71-r2

- The act title sits on a solid black bottom bar throughout the splash fade.
- All acts now run: act splash, character cutscene, act relic choice, map.
- Act splashes and character cutscenes remain silent. RiftMenu begins as the relic screen appears. Boss dialogue keeps its existing RiftBoss behavior.
- Visible enemy feet are anchored to the combat floor, correcting small gaps from transparent padding and downsampling.
- Player idle cycles are 50% faster; enemy idle breathing is roughly 50% faster. Combat-action timing is unchanged.
- Forgotten Tome and Forgotten Armory previews fit entirely inside the mystery panel, with visible prices and complete card descriptions.
- Up/Down now selects Save & Quit or Resume in the pause menu; X activates the highlighted choice. Your underlying card or map selection is preserved.
- Removed the unintended Select-button symbol from the main-menu help trim.
- In-game music is 15% louder; sound effects and impact ducking remain intact.
- Existing saves made in an old post-relic cutscene continue to the map without granting a second relic. Rapid skipping cannot accidentally accept the act relic.

The game version remains 1.71, with a separate r2 release tag. All previous art, music, the Furnace Maw contrast improvement and the hardware-confirmed startup correction are retained.

Tested in PPSSPP 1.20.4 with a 32 MiB PSP configuration, native sanitizer checks, rendering comparisons and screenshot inspection. Physical PSP verification remains pending.

Install as `PSP/GAME/RIFTASCENT/EBOOT.PBP`, preserving `SAVE.DAT` and `PROFILE.DAT`.
