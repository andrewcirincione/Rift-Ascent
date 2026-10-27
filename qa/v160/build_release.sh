#!/usr/bin/env bash
set -euo pipefail
baseline_eboot=${1:?Pass a complete 1.5 EBOOT}
release_eboot=${2:?Pass output EBOOT path}
: "${PSPDEV:?Set PSPDEV to the PSP SDK root}"
export PATH="$PSPDEV/bin:$PATH"
python3 qa/v160/build_source.py
python3 qa/v160/prepare_art.py
python3 qa/v160/star_contrast/prepare_art.py
python3 qa/v160/build_source.py
python3 qa/v160/memory_fix/pack_assets.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/fix_psp_elf_metadata.py portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v160/memory_fix/append_assets.py portable_build/EBOOT.PBP qa/v160/memory_fix/with-assets.PBP
python3 qa/v1311/pack_music.py "$baseline_eboot" qa/v160/memory_fix/with-assets.PBP "$release_eboot"
