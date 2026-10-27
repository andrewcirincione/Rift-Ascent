#!/usr/bin/env bash
set -euo pipefail
mkdir -p ../runtime
common=( -std=c99 -Iportable_build qa/v160/memory_fix/render_audit.c portable_build/game.c portable_build/artwork.c portable_build/enemy_art.c portable_build/motion_art.c portable_build/sound.c portable_build/asset_stream.c )
gcc -O2 "${common[@]}" -o ../runtime/render-resident
../runtime/render-resident > qa/v160/memory_fix/evidence/resident-frames.txt 2> qa/v160/memory_fix/evidence/resident-tests.txt
gcc -DRIFT_STREAMED_ART -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer "${common[@]}" -o ../runtime/render-sanitized
# LeakSanitizer cannot enumerate processes in this container. ASan/UBSan stay on.
ASAN_OPTIONS=detect_leaks=0 ../runtime/render-sanitized > qa/v160/memory_fix/evidence/streamed-frames.txt 2> qa/v160/memory_fix/evidence/streamed-tests.txt
cmp qa/v160/memory_fix/evidence/resident-frames.txt qa/v160/memory_fix/evidence/streamed-frames.txt
python3 qa/v160/memory_fix/asset_audit.py > qa/v160/memory_fix/evidence/asset-tests.txt
echo 'PASS identical resident/streamed rendered frames and texture bytes'
