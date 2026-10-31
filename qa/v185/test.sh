#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in revision185 beta cards_three afterimage; do
 gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v185/tests/$suite.c" -o "/tmp/rift185-$suite"
 "/tmp/rift185-$suite" > "qa/v185/evidence/$suite.txt"
done
gcc -O0 -std=gnu11 qa/v185/tests/layout.c -o /tmp/rift185-layout
/tmp/rift185-layout
python3 qa/v185/check_media.py
