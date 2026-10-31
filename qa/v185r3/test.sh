#!/usr/bin/env bash
set -euo pipefail
bash qa/v185/test.sh
gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer qa/v185r3/regression.c -o /tmp/rift185r3-regression
ASAN_OPTIONS=detect_leaks=0 /tmp/rift185r3-regression > qa/v185r3/evidence/regression.txt
