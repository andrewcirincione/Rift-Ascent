#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in progression storage effects expedition boundaries; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v181/tests/$suite.c" -o "/tmp/rift-181-$suite"
  "/tmp/rift-181-$suite" > "qa/v181/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/v181/tests/fixtures.c -o /tmp/rift-181-fixtures
/tmp/rift-181-fixtures
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/v181/tests/render.c qa/v181/source/asset_stream.c qa/v181/source/artwork.c qa/v181/source/motion_art.c qa/v181/source/enemy_art.c qa/v181/source/sound.c -lm -o /tmp/rift-181-render
/tmp/rift-181-render > qa/v181/evidence/native-render.txt
