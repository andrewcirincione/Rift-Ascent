#!/usr/bin/env bash
set -euo pipefail
: "${PSPDEV:?Set PSPDEV to the PSP SDK root}"
export PATH="$PSPDEV/bin:$PATH"
python3 qa/v161/prepare_art.py
python3 qa/v161/build_source.py
python3 qa/v161/pack_assets.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/fix_psp_elf_metadata.py portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v161/append_assets.py portable_build/EBOOT.PBP qa/v161/evidence/with-assets.PBP
python3 qa/v1311/pack_music.py versions/1.6-r2/EBOOT.PBP qa/v161/evidence/with-assets.PBP qa/v161/evidence/EBOOT.PBP
