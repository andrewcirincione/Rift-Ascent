#!/usr/bin/env bash
# Run from the repository root; executables and regenerated fixtures are local.
set -euo pipefail
test -f qa/v150/source/game.c
qa_bin_dir=${RIFT_QA_BIN_DIR:-/tmp/rift-ascent-150-qa}
mkdir -p "$qa_bin_dir" qa/v150/evidence
compiler=${CC:-gcc}
flags=(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
export ASAN_OPTIONS=detect_leaks=0
for suite in mechanics events expansion potion_belt navigation variety map_choice reward_endless relic bully ashen_energy scaling; do
  "$compiler" "${flags[@]}" "qa/v150/${suite}_tests.c" -lm -o "$qa_bin_dir/$suite"
  "$qa_bin_dir/$suite" > "qa/v150/evidence/$suite.txt"
  cat "qa/v150/evidence/$suite.txt"
done
render_sources=(GAME/src/game.c GAME/src/artwork.c GAME/src/motion_art.c GAME/src/enemy_art.c GAME/src/sound.c)
for suite in regression presentation mod_visual; do
  source_file="qa/v150/$suite.c"
  if [[ "$suite" != regression ]]; then source_file="qa/v150/${suite}_tests.c"; fi
  "$compiler" "${flags[@]}" -I GAME/src "$source_file" "${render_sources[@]}" -lm -o "$qa_bin_dir/$suite"
  "$qa_bin_dir/$suite" > "qa/v150/evidence/$suite.txt"
  cat "qa/v150/evidence/$suite.txt"
done
python3 qa/v150/audit_controls.py > qa/v150/evidence/controls.txt
"$compiler" -O2 -I qa/v150/source qa/v150/playtest.c qa/v150/source/game.c -lm -o "$qa_bin_dir/playtest"
"$qa_bin_dir/playtest" 300 1 0 > qa/v150/evidence/balance-final-standard.txt
"$qa_bin_dir/playtest" 300 1001 1 > qa/v150/evidence/balance-final-defensive.txt
"$qa_bin_dir/playtest" 300 2001 2 > qa/v150/evidence/balance-final-aggressive.txt
"$compiler" -I GAME/src qa/v150/state_layout.c -o "$qa_bin_dir/state-layout"
"$qa_bin_dir/state-layout" > qa/v150/evidence/state-layout.json
