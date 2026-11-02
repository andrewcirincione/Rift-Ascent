"""Bounded PSP startup/save/preview audit; not a whole-program stack proof.
Compiler-inlined functions are already included in their reported caller frame.
"""
from pathlib import Path
R=Path(__file__).resolve().parents[2];S=R/'qa/v186/source';frames={}
for file in ['game.su','psp_start.su','render.su']:
 for line in (S/file).read_text().splitlines():
  symbol,n,kind=line.split('\t');assert kind=='static',line
  symbol=symbol.rsplit(':',1)[-1].split('.')[0];frames[symbol]=max(frames.get(symbol,0),int(n))
def f(n):assert n in frames,n;return frames[n]
assert '#define RIFT_GAME_STACK_BYTES (192*1024)' in (S/'psp_start.c').read_text()
paths={
 'startup slot scan':['run_game','ascent_open','ascent_resume'],
 'in-game slot resume':['run_game','step_action','game_action','ascent_resume'],
 'save and quit':['run_game','save_and_quit','ascent_checkpoint'],
 'standalone load':['run_game','game_load'],
 'rendered card preview':['run_game','game_render_frame','game_card_preview','preview_once','game_play_card','resolve_card'],
 'card action':['run_game','step_action','game_action','game_play_card','resolve_card'],
}
for name,chain in paths.items():
 used=sum(f(n) for n in chain);remaining=192*1024-used-8192
 assert remaining>=32768,(name,used,remaining)
 print(f'PASS {name}: {used} reported frame bytes + 8192 tail reserve; {remaining} bytes remain (minimum 32768)')
 print('  '+' + '.join(f'{n}({f(n)})' for n in chain))
assert f('run_game')<=4096
print('PASS 192 KiB thread unchanged; bounded startup/save/preview paths retain at least 32 KiB margin')
