#!/usr/bin/env python3
"""Compile and exercise the EXACT music-selection expression used by PSP.

Checks title/menu, normal encounters, elite encounters, all act bosses,
card selection, deck inspection and battle pause.
"""
from pathlib import Path
import re,subprocess,tempfile,sys
src=Path(sys.argv[1]).read_text()
m=re.search(r'music_battle\s*=\s*(\(game\.screen==TITLE\?0:.*?\));',src,re.S)
assert m,'PSP soundtrack music_battle mapping not found'
expr=m.group(1)
assert 'game.battle_type==NODE_ELITE||game.battle_type==NODE_BOSS?0:2' in expr
code=r"""
#include <assert.h>
#include <stdio.h>
enum {TITLE,MAP,COMBAT,REWARD,REST,SHOP,EVENT,DECK_VIEW,
      VICTORY,DEFEAT,HELP,BOSS_REWARD,RELIC_VIEW,PAUSE_MENU,
      EVENT_RESULT,RELIC_RESULT,CARD_CHOICE};
enum {NODE_FIGHT,NODE_ELITE,NODE_EVENT,NODE_SHOP,NODE_REST,
      NODE_TREASURE,NODE_BOSS};
typedef struct {int screen,prior_screen,battle_type;} G;
static int paused=MAP;
int game_pause_source(void){return paused;}
static int pick(G game){return EXPRESSION;}
static void check(int screen,int prior,int type,int from,int want){
 paused=from;G g={screen,prior,type};
 int got=pick(g);
 if(got!=want){fprintf(stderr,"music check failed screen=%d type=%d expected=%d got=%d\n",
 screen,type,want,got);assert(got==want);}
}
int main(void){
 check(TITLE,MAP,NODE_FIGHT,MAP,0);
 check(MAP,TITLE,NODE_FIGHT,MAP,1);
 check(SHOP,MAP,NODE_FIGHT,MAP,1);
 check(REWARD,COMBAT,NODE_ELITE,MAP,1);
 check(COMBAT,MAP,NODE_FIGHT,MAP,2);
 check(COMBAT,MAP,NODE_ELITE,MAP,0);
 check(COMBAT,MAP,NODE_BOSS,MAP,0);
 check(CARD_CHOICE,COMBAT,NODE_FIGHT,COMBAT,2);
 check(CARD_CHOICE,COMBAT,NODE_BOSS,COMBAT,0);
 check(DECK_VIEW,COMBAT,NODE_FIGHT,COMBAT,2);
 check(DECK_VIEW,COMBAT,NODE_ELITE,COMBAT,0);
 check(PAUSE_MENU,MAP,NODE_FIGHT,COMBAT,2);
 check(PAUSE_MENU,MAP,NODE_ELITE,COMBAT,0);
 check(PAUSE_MENU,MAP,NODE_BOSS,CARD_CHOICE,0);
 check(PAUSE_MENU,MAP,NODE_FIGHT,MAP,1);
 puts("PASS: real PSP mapping uses Checkers for title/elite/boss, RiftMenu elsewhere, RiftFight for normal encounters");
 return 0;
}
""".replace('EXPRESSION',expr)
with tempfile.TemporaryDirectory(prefix='rift119-') as td:
 p=Path(td)
 (p/'music_test.c').write_text(code)
 subprocess.run(['gcc','-std=c11','-O2','-Wall','-Werror','-o',str(p/'music_test'),str(p/'music_test.c')],check=True)
 subprocess.run([str(p/'music_test')],check=True)
