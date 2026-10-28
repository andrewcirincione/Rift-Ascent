#!/usr/bin/env bash
set -euo pipefail
: "${PSPDEV:?Set PSPDEV to the PSP SDK root}"
export PATH="$PSPDEV/bin:$PATH"
python3 qa/rebuild180/pack_audio.py
python3 qa/rebuild180/pack_art.py "$@"
make -C qa/rebuild180/source -j4
python3 qa/v115/import_114_art.py GAME/src qa/rebuild180/source/EBOOT.PBP
python3 tools/fix_psp_elf_metadata.py qa/rebuild180/source/EBOOT.PBP
python3 tools/audit_psp_imports.py qa/rebuild180/source/EBOOT.PBP
python3 GAME/verify_eboot.py qa/rebuild180/source/EBOOT.PBP
python3 qa/rebuild180/package.py "$@"
