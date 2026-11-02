#!/usr/bin/env bash
set -euo pipefail
sources=(qa/v186r2/timeline.c qa/v186/source/game.c qa/v186/source/asset_stream.c qa/v186/source/artwork.c qa/v186/source/motion_art.c qa/v186/source/enemy_art.c)
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer "${sources[@]}" -o /tmp/rift186r2-timeline
ASAN_OPTIONS=detect_leaks=0 /tmp/rift186r2-timeline > qa/v186r2/evidence/timeline-sanitizers.txt
python3 qa/v186/check_media.py > qa/v186r2/evidence/media.txt
python3 qa/v186/check_stack.py > qa/v186r2/evidence/stack.txt
