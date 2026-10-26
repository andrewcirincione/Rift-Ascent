#!/usr/bin/env bash
set -euo pipefail
baseline_eboot=${1:?Pass your complete 1.3.9 EBOOT.PBP to preserve the soundtrack}
release_eboot=${2:-../evidence1310/EBOOT.PBP}
: "${PSPDEV:?Set PSPDEV to your PSP SDK root}"
export PATH="$PSPDEV/bin:$PATH"
python3 qa/v1310/build_source.py
make -C portable_build -j2
python3 qa/v115/import_114_art.py GAME/src portable_build/EBOOT.PBP
python3 tools/audit_psp_imports.py portable_build/EBOOT.PBP
python3 GAME/verify_eboot.py portable_build/EBOOT.PBP
python3 qa/v134/pack_music.py "$baseline_eboot" portable_build/EBOOT.PBP "$release_eboot"
