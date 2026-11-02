#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in revision185 beta cards_three afterimage prior_regression inventory boss_damage revision186; do
 gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v186/tests/$suite.c" -o "/tmp/rift186-$suite"
 "/tmp/rift186-$suite" > "qa/v186/evidence/$suite.txt"
done
for suite in layout fixtures; do
 gcc -O0 -std=gnu11 "qa/v186/tests/$suite.c" -o "/tmp/rift186-$suite"
 "/tmp/rift186-$suite" > "qa/v186/evidence/$suite.txt"
done
python3 qa/v186/check_media.py
