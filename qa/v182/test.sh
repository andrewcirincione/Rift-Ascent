#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in progression storage effects expedition boundaries revision; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v182/tests/$suite.c" -o "/tmp/rift-182-$suite"
  "/tmp/rift-182-$suite" > "qa/v182/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/v182/tests/fixtures.c -o /tmp/rift-182-fixtures
/tmp/rift-182-fixtures
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/v182/tests/render.c qa/v182/source/asset_stream.c qa/v182/source/artwork.c qa/v182/source/motion_art.c qa/v182/source/enemy_art.c qa/v182/source/sound.c -lm -o /tmp/rift-182-render
/tmp/rift-182-render > qa/v182/evidence/native-render.txt
