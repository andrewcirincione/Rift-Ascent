#!/usr/bin/env python3
"""v1.1.9: Three original recordings only: Checkers, RiftMenu, RiftFight.
The user's files are appended privately to the final EBOOT, NEVER to
the publicly accessible GitHub repository. Music synthesized by game: none.
"""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def replace(old,new,why):
 global s
 n=s.count(old)
 if n!=1:raise RuntimeError(f"{why}: expected 1 anchor; found {n}")
 s=s.replace(old,new,1)
replace('#define RIFT_TRACK_COUNT 10','#define RIFT_TRACK_COUNT 3',"track count")
replace('#define RIFT_MUSIC_FOOTER 96','#define RIFT_MUSIC_FOOTER 40',"index footer length")
replace('static unsigned soundtrack_offset[10],soundtrack_samples[10];',
        'static unsigned soundtrack_offset[RIFT_TRACK_COUNT],soundtrack_samples[RIFT_TRACK_COUNT];',"track memory")
replace("const char magic[8]={'R','I','F','T','M','1','0','3'};",
        "const char magic[8]={'R','I','F','T','M','1','1','9'};","footer signature")
replace('soundtrack_total=read_u32(footer+88);',
        'soundtrack_total=read_u32(footer+32);',"payload length field")
replace('for(int i=0;i<10;i++){','for(int i=0;i<RIFT_TRACK_COUNT;i++){',"index validation")
replace('if(!soundtrack_active||track<0||track>=10)return 0;',
        'if(!soundtrack_active||track<0||track>=RIFT_TRACK_COUNT)return 0;',"selection validation")
replace('boot_trace("Rift 1.0.3 embedded music loaded",10);',
        'boot_trace("Rift 1.1.9 original tracks loaded",RIFT_TRACK_COUNT);',"boot logging")
replace('?2+(int)((game.seed+(unsigned)(game.floor*3+game.act))%8u)',
        '?(game.battle_type==NODE_ELITE||game.battle_type==NODE_BOSS?0:2)',
        "elite and act bosses use Checkers; regular battles use RiftFight")
# This is audio-only; no generated music regardless of whether EBOOT index loads.
assert 'rift_audio_generate(&music,audio_buffer,512,track,1);' in s
assert '2+(int)((game.seed' not in s
assert 'game.battle_type==NODE_ELITE||game.battle_type==NODE_BOSS?0:2' in s
assert '#define RIFT_TRACK_COUNT 3' in s
assert 'footer+88' not in s
assert 'track>=10' not in s
p.write_text(s)
print("PASS v1.1.9: title=Checkers, other menus=RiftMenu, regular battles=RiftFight, elite/boss fights=Checkers, only 3 indexed tracks")
