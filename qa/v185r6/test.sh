#!/usr/bin/env bash
set -euo pipefail
bash qa/v185r4/test.sh
gcc -O0 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer qa/v185r6/boss_damage.c -o /tmp/rift185r6-boss-damage
ASAN_OPTIONS=detect_leaks=0 /tmp/rift185r6-boss-damage > qa/v185r6/evidence/boss-damage.txt
