#!/usr/bin/env bash
set -euo pipefail
: "${PSPDEV:?Set PSPDEV to the PSPDEV v20261001 toolchain root}"
export PATH="$PSPDEV/bin:$PATH"
python3 distribution/tools/resources.py
for file in ICON0.PNG ICON1.PMF PIC1.PNG SND0.AT3; do
 cp "distribution/resources/$file" "qa/v186/source/$file"
done
make -C qa/v186/source -j4
python3 distribution/tools/package.py
