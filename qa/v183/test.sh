#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in progression storage effects expedition boundaries revision events_extra revision183; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v183/tests/$suite.c" -o "/tmp/rift-183-$suite"
  "/tmp/rift-183-$suite" > "qa/v183/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/v183/tests/fixtures.c -o /tmp/rift-183-fixtures
/tmp/rift-183-fixtures
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/v183/tests/render.c qa/v183/source/asset_stream.c qa/v183/source/artwork.c qa/v183/source/motion_art.c qa/v183/source/enemy_art.c qa/v183/source/sound.c -lm -o /tmp/rift-183-render
/tmp/rift-183-render > qa/v183/evidence/native-render.txt
