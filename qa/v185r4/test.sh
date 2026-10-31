#!/usr/bin/env bash
set -euo pipefail
bash qa/v185r3/test.sh
gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer qa/v185r4/inventory.c -o /tmp/rift185r4-inventory
ASAN_OPTIONS=detect_leaks=0 /tmp/rift185r4-inventory > qa/v185r4/evidence/inventory.txt
