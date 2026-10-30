#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in beta afterimage; do
 gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v184beta1/tests/$suite.c" -o "/tmp/rift-beta-$suite"
 "/tmp/rift-beta-$suite" > "qa/v184beta1/evidence/$suite-native.txt"
done
gcc -O1 -std=gnu11 qa/v184beta1/tests/layout.c -o /tmp/rift-beta-layout
/tmp/rift-beta-layout
python3 qa/v184beta1/check_media.py
