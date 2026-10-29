#!/usr/bin/env bash
set -euo pipefail
export ASAN_OPTIONS=detect_leaks=0
for suite in ${RIFT_SUITES:-progression storage effects expedition boundaries revision events_extra revision183 custom184 balance184 revision184r2 catalogs afterimage combat_tuning}; do
  gcc -O1 -g -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer "qa/v184r3/tests/$suite.c" -o "/tmp/rift-184r3-$suite"
  "/tmp/rift-184r3-$suite" > "qa/v184r3/evidence/$suite.txt"
done
gcc -O1 -std=gnu11 qa/v184r3/tests/fixtures.c -o /tmp/rift-184r3-fixtures
/tmp/rift-184r3-fixtures
gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/v184r3/tests/render.c qa/v184r3/source/asset_stream.c qa/v184r3/source/artwork.c qa/v184r3/source/motion_art.c qa/v184r3/source/enemy_art.c qa/v184r3/source/sound.c -lm -o /tmp/rift-184r3-render
/tmp/rift-184r3-render > qa/v184r3/evidence/native-render.txt

gcc -O0 -std=gnu11 -ffunction-sections -fdata-sections qa/v184r3/tests/collection_layout.c -Wl,--gc-sections -lm -o /tmp/rift-184r3-layout
/tmp/rift-184r3-layout > qa/v184r3/evidence/collection-layout.txt

gcc -O1 -g -std=gnu11 -DRIFT_STREAMED_ART -fsanitize=address,undefined -fno-omit-frame-pointer qa/v184r3/tests/render184.c qa/v184r3/source/asset_stream.c qa/v184r3/source/artwork.c qa/v184r3/source/motion_art.c qa/v184r3/source/enemy_art.c qa/v184r3/source/sound.c -lm -o /tmp/rift-184r3-render184
/tmp/rift-184r3-render184 > qa/v184r3/evidence/native-render184.txt
gcc -O2 -std=gnu11 -DRIFT_STREAMED_ART qa/v184r3/tests/performance.c qa/v184r3/source/asset_stream.c qa/v184r3/source/artwork.c qa/v184r3/source/motion_art.c qa/v184r3/source/enemy_art.c qa/v184r3/source/sound.c -lm -o /tmp/rift-184r3-performance
/tmp/rift-184r3-performance > qa/v184r3/evidence/performance.txt
