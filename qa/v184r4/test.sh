#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
gcc -O1 -std=gnu11 -DBASELINE qa/v184r4/tests/combat_tuning.c -o /tmp/rift184r4-baseline
/tmp/rift184r4-baseline > qa/v184r4/evidence/baseline-damage.txt
for suite in combat_tuning afterimage reward_tokens proxy_energy; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v184r4/tests/$suite.c" -o "/tmp/rift184r4-$suite"
  "/tmp/rift184r4-$suite" > "qa/v184r4/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/v184r4/tests/fixtures.c -o /tmp/rift184r4-fixtures
/tmp/rift184r4-fixtures
