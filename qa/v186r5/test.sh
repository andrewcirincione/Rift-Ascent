#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in beta cards_three afterimage prior_regression inventory boss_damage; do
 gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v186/tests/$suite.c" -o "/tmp/rift186r5-$suite"
 "/tmp/rift186r5-$suite" > "qa/v186r5/evidence/$suite.txt"
done
for suite in revision185 prior186 relic_tests; do
 gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v186r5/$suite.c" -o "/tmp/rift186r5-$suite"
 "/tmp/rift186r5-$suite" > "qa/v186r5/evidence/$suite.txt"
done
gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer qa/v186r4/revision.c -o /tmp/r5-ending-regression
/tmp/r5-ending-regression > qa/v186r5/evidence/ending-regression.txt
sources=(qa/v186r5/timeline.c qa/v186/source/game.c qa/v186/source/asset_stream.c qa/v186/source/artwork.c qa/v186/source/motion_art.c qa/v186/source/enemy_art.c)
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer "${sources[@]}" -o /tmp/rift186r5-timeline
/tmp/rift186r5-timeline > qa/v186r5/evidence/timeline.txt
python3 qa/v186/check_media.py > qa/v186r5/evidence/media.txt
python3 qa/v186/check_stack.py > qa/v186r5/evidence/stack.txt
