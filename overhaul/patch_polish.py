#!/usr/bin/env python3
"""Finishing passes: explicit enemy windup, effective damage badges, controls."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c"
s=p.read_text()
def edit(a,b):
 global s
 if a not in s:raise RuntimeError("polish anchor missing: "+a[:100])
 s=s.replace(a,b,1)
edit("static int overlay_kind,overlay_page,overlay_potion;",
"""static int overlay_kind,overlay_page,overlay_potion;
static int enemy_windup;
static const Game *card_combat_context;
void game_render_set_enemy_windup(int ticks){enemy_windup=ticks;}
""")
edit(""" wrapped_text(x+5,y+21+arth,card_description(raw),(w-12)/7,maxrows,
                           selected?WHITE:MUTED);""",
""" wrapped_text(x+5,y+21+arth,card_description(raw),(w-12)/7,maxrows,
                           selected?WHITE:MUTED);
 if(!tall&&card_combat_context&&d->type==0){
   int preview=game_effective_damage(card_combat_context,raw,card_combat_context->target);
   if(preview>0){
     char num[12];snprintf(num,sizeof(num),"%d",preview);
     rect(x+w-22,y+arth+14,21,13,RGB(47,20,23));
     border(x+w-22,y+arth+14,21,13,RED);
     center(x+w-11,y+arth+15,num,GOLD,1);
   }
 }""")
edit("""static void combat(const Game*g){
 /* Independent""",
"""static void combat(const Game*g){
 card_combat_context=g;
 /* Independent""")
edit(""" int recoil=fx_player_damage>0&&age<14?(int)((14-age)*((age&1)?-1:1)/2):0;""",
""" int recoil=fx_player_damage>0&&age<14?(int)((14-age)*((age&1)?-1:1)/2):0;
 if(enemy_windup>0)recoil+=(enemy_windup&3)==0?1:0;""")
edit("""  if(fx_player_damage>0&&age<12)ex-=(int)((12-age)*2/3);""",
"""  if(fx_player_damage>0&&age<12)ex-=(int)((12-age)*2/3);
  if(enemy_windup>0)ex-=((24-enemy_windup)>6 && (24-enemy_windup)<19)?8:0;""")
edit(""" header(g,"BATTLE / X PLAY   START EXPLAIN");""",
""" if(enemy_windup>0) {
    dark_rect(175,67,158,32,184);
    border(175,67,158,32,RED);
    center(254,74,"ENEMY TURN",GOLD,1);
    rect(181,90,(enemy_windup*146)/24,3,RED);
 }
 header(g,"BATTLE / X PLAY   START EXPLAIN");""")
edit("""  card_render(x,y,70,selected?78:71,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));
 }
 button(414""",
"""  card_render(x,y,70,selected?78:71,g->hand[i],selected,cards[id].cost!=-2&&(cards[id].cost<=g->energy||cards[id].cost==-1));
 }
 card_combat_context=0;
 button(414""")
edit('text(29,107,"CIRCLE End turn / cancel",WHITE,1);',
     'text(29,107,"RIGHT  Move to END, CROSS ends turn",WHITE,1);')
edit('text(29,127,"SQUARE View your deck",WHITE,1);',
     'text(29,127,"START  Inspect card, enemy, or status",WHITE,1);')
edit('text(29,147,"TRIANGLE Use next potion in battle",WHITE,1);',
     'text(29,147,"TRIANGLE  Open potion belt / use any",WHITE,1);')
edit('text(29,187,"Gold buys cards, relics and rest. Heal wisely.",MUTED,1);',
     'text(29,187,"Weak -25% dealt; Vulnerable +50% taken.",MUTED,1);')
# Always explain that maps have actual roads and reward circles.
edit('footer("ROADS BRANCH / CHOOSE LIT NODE    X ENTER   SQUARE DECK");',
     'footer("FOLLOW CONNECTED ROADS   X ENTER   SQUARE DECK");')
p.write_text(s)
print("Added enemy turn cue, combat damage badge, and coherent in-game controls.")
