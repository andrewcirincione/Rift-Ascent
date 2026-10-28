#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in progression storage effects; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/rebuild180/tests/$suite.c" -o "/tmp/rift-180-$suite"
  "/tmp/rift-180-$suite" > "qa/rebuild180/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/rebuild180/tests/fixtures.c -o /tmp/rift-180-fixtures
/tmp/rift-180-fixtures
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/rebuild180/tests/render.c qa/rebuild180/source/asset_stream.c qa/rebuild180/source/artwork.c qa/rebuild180/source/motion_art.c qa/rebuild180/source/enemy_art.c qa/rebuild180/source/sound.c -lm -o /tmp/rift-180-render
/tmp/rift-180-render > qa/rebuild180/evidence/native-render.txt
