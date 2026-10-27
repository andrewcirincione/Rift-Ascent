# 1.6-r2 PSP startup and resource audit

The reported hardware error was 800200D9 (`SCE_KERNEL_ERROR_MEMBLOCK_ALLOC_FAILED`). No physical PSP is connected to this environment. The diagnosis is supported by the old binary's layout and the firmware/CFW loader model, with physical confirmation still pending.

## Loader correction

The original first PT_LOAD had `p_paddr == p_vaddr`, rather than a file offset to `.rodata.sceModuleInfo`. Firmware marks that locator invalid, while CFW can repair static ELF module info later. If `execSize` remains unset, ModuleMgr falls back to file end minus executable start. For the released 1.6 PBP this models a 26,921,216-byte allocation ending beyond `0x0A000000`.

`tools/fix_psp_elf_metadata.py` installs the actual module-info section file offset. It does not move code or data, change the entry point, or convert the executable to PRX. The new verifier requires this locator and reserves much greater memory headroom. The streamed executable's address span, including its cache, is 11,778,304 bytes; the conservative sum of resident span and ELF file is 18,830,088 bytes.

Primary source references:

- https://pspdev.github.io/pspsdk/pspkerror_8h_source.html
- https://github.com/uofw/uofw/blob/master/src/kd/loadcore/loadelf.c
- https://github.com/uofw/uofw/blob/master/src/kd/modulemgr/modulemgr.c
- https://github.com/PSP-Archive/ARK-4/blob/main/core/systemctrl/src/loadercore.c
- https://github.com/PSP-Archive/ARK-4/blob/main/core/systemctrl/src/modulemanager.c
- https://github.com/pspdev/pspsdk/blob/master/tools/psp-prxgen.c

## Artwork storage

`pack_assets.py` preserves the original texture arrays behind a non-streaming build branch and emits small descriptors for PSP builds. The bank is appended outside DATA.PSP, followed by its size/CRC footer, then the unchanged five-track music payload. The bank reader uses its own file handle so audio seeks cannot interfere. A fixed 64-page, 4 MiB FIFO cache validates sizes, offsets and per-texture CRCs. No heap allocation, image resampling, palette reduction or decompression occurs at runtime.

Every rendering path resolves an image before accessing its pixels; each pointer is consumed synchronously before the next resolve. `assets.json` is the deterministic manifest. `assets.bin` is regenerated and excluded from Git because the exact bytes are already in the source arrays and release EBOOT.

## Tests and rebuild

- `asset_audit.py`: all 299 resources in forward, reverse and shuffled order (897 byte comparisons), cache hits, eviction and malformed descriptors.
- `render_audit.c`: 1,768 rendered cases, including every base/upgraded card, all enemies and characters, rooms, overlays, maps and animation frames. Resident and streamed aggregate hashes match. The streamed pass uses AddressSanitizer and UndefinedBehaviorSanitizer. LeakSanitizer is disabled because this container cannot enumerate its processes; there are no heap allocations in the cache.
- `loader_audit.py`: old and corrected loader-allocation models, not hardware execution.
- `verify_media.py`: exact preservation of XMB sections and all five music streams.
- `../star_contrast/qa_ppsspp.py`: exact packaged EBOOT, PSPModel=0 (32 MiB), controller checks and native screenshots. `RIFT_CONTROL_AUDIT=resume` tests a fresh process; `RIFT_CONTROL_AUDIT=warden` tests the streamed Warden animations.

From the repo root, set `PSPDEV` and run:

```sh
bash qa/v160/build_release.sh versions/1.6/EBOOT.PBP qa/v160/star_contrast/evidence/EBOOT.PBP
bash qa/v160/memory_fix/run_native.sh
python3 qa/v160/memory_fix/loader_audit.py versions/1.6/EBOOT.PBP qa/v160/star_contrast/evidence/EBOOT.PBP
```

The public EBOOT and screenshots are at `versions/1.6-r2`.
