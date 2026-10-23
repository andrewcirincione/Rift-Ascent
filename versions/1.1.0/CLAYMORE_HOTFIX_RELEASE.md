# Rift Ascent 1.1.0 — Style-corrected Claymore EBOOT

- **All-in-one EBOOT size:** 14,433,090 bytes.
- **Full EBOOT SHA-256:** `8efb235dae4da021a2bab03715f003d34a168a50f951d09accbe007c512145e3`.
- **Base engine:** GitHub Actions [40030677220](https://github.com/andrewcirincione/Rift-Ascent/actions/runs/40030677220), successful, on `hotfix-1.1.0-claymore-art`.
- **Music:** The complete ten-track 1.1.0 ADPCM trailer is preserved **byte for byte** from the earlier release (11,533,436 bytes, verified CRC32 and all ten track indexes).
- **PSP preview:** Reuses exactly the previous 1.1.0 XMB PNG artwork. Only the in-combat hero sprite is modified.
- **Executable identity:** The complete PBP was repackaged from the exact successful PSPDEV-compiled `EBOOT.PBP`, without changing any of its executable bytes.
- **Controls, save compatibility, cards, enemy logic, and existing map icons:** unchanged.

The full packaged EBOOT and a real PPSSPP combat screenshot are downloadable in the ChatGPT conversation. They are not committed to the repo because the packaged binary includes user-supplied soundtrack audio absent from GitHub Actions. The downloadable GitHub artifact contains the engine-only executable; it is **not** the complete music-bearing release.

**Hardware verification:** PSPDEV compilation and PPSSPP tests passed. This is not a claim of successful testing on the user's physical PSP-3000/ARK-5; keep the previously working EBOOT and `SAVE.DAT` backed up.
