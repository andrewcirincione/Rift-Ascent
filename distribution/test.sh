#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in rules progression integration; do
 gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "distribution/tests/$suite.c" -o "/tmp/rift-dist-$suite"
 "/tmp/rift-dist-$suite" > "distribution/evidence/$suite.txt"
done
for suite in ui timeline; do
 sources=("distribution/tests/$suite.c" qa/v186/source/game.c qa/v186/source/asset_stream.c qa/v186/source/artwork.c qa/v186/source/motion_art.c qa/v186/source/enemy_art.c)
 gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer "${sources[@]}" -o "/tmp/rift-dist-$suite"
 "/tmp/rift-dist-$suite" > "distribution/evidence/$suite.txt"
done
