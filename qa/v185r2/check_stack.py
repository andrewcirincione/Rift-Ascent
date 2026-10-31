"""Check PSP compiler frames on the startup and save paths that overflowed in 1.8.5.

This is a bounded audit of these named call chains, not a whole-program proof.
Eight KiB is reserved for their small I/O/validation callees, plus a separate
32 KiB safety margin inside the unchanged 192 KiB game-thread allocation.
"""
from pathlib import Path
import argparse

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--baseline', action='store_true')
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
source = root / 'qa/v185/source'
frames = {}
files = ([root / 'qa/v185r2/evidence/baseline-game.su',
          root / 'qa/v185r2/evidence/baseline-psp_start.su'] if a.baseline
         else [source / 'game.su', source / 'psp_start.su'])
for path in files:
    for line in path.read_text().splitlines():
        symbol, count, kind = line.split('\t')
        assert kind == 'static', line
        frames[symbol.rsplit(':', 1)[-1]] = int(count)

def frame(name):
    matches = [value for symbol, value in frames.items()
               if symbol == name or symbol.startswith(name + '.')]
    assert matches, f'Missing compiler stack report for {name}; rebuild cleanly'
    return max(matches)

assert '#define RIFT_GAME_STACK_BYTES (192*1024)' in (source / 'psp_start.c').read_text()
stack = 192 * 1024
reserve = 32 * 1024
tail = 8 * 1024
paths = {
    'empty/current slot scan': ['run_game', 'ascent_open', 'ascent_resume', 'ascent_read_run'],
    'old stable slot migration': ['run_game', 'ascent_open', 'ascent_resume', 'ascent_read_run', 'migrate_layout184'],
    'legacy SAVE.DAT migration': ['run_game', 'ascent_open', 'game_load', 'migrate_layout184'],
    'in-game old slot resume': ['run_game', 'step_action', 'game_action', 'ascent_resume', 'ascent_read_run', 'migrate_layout184'],
}
if not a.baseline:
    paths['beta-only migration'] = ['run_game', 'ascent_open', 'ascent_import_beta', 'ascent_read_run', 'migrate_layout184']
    paths['save and quit'] = ['run_game', 'save_and_quit', 'ascent_checkpoint']

failed = False
for name, chain in paths.items():
    used = sum(frame(fn) for fn in chain)
    safe = used + tail <= stack - reserve
    failed |= not safe
    print(f'{"PASS" if safe else "FAIL"} {name}: {used} frame bytes + {tail} tail reserve; '
          f'{stack - used - tail} bytes remaining of {stack}')
    print('  ' + ' + '.join(f'{fn}({frame(fn)})' for fn in chain))
if frame('run_game') > 4096:
    print('FAIL run_game must not retain large save snapshots for its entire lifetime')
    failed = True
if a.baseline:
    assert failed, 'Original 1.8.5 must fail the regression check'
    print('PASS negative control: exact original 1.8.5 compiler frames rejected')
else:
    assert not failed, 'PSP thread stack budget exceeded'
    print('PASS startup/save stack budget; 192 KiB allocation unchanged')
