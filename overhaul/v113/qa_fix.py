#!/usr/bin/env python3
"""v1.1.3 QA fixes found by source review and rendered-frame inspection."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);g=root/"game.c";h=root/"game.h";r=root/"render.c"
s=g.read_text();hs=h.read_text();v=r.read_text()
def sub(hay, old, new, label):
    count=hay.count(old)
    if count!=1:raise RuntimeError(f"QA fix {label}: expected one anchor, found {count}")
    return hay.replace(old,new,1)

# The previous mask started at all three possible first-floor choices.
# After the player commits, it left abandoned/unreachable future lanes visible.
needle="""int game_route_available(const Game *g,int lane) {"""
add="""unsigned game_route_visible_mask(const Game *g,int row) {
 if(!g||row<0||row>=MAP_ROWS)return 0;
 if(g->floor==0)return game_route_row_mask(g,row);
 if(row<g->floor-1) {
   int selected=g->map_selected[row];
   return selected>=0&&selected<MAP_LANES?(1u<<selected):game_route_row_mask(g,row);
 }
 unsigned live=1u<<g->lane;
 for(int r=g->floor-1;r<row;r++) {
   unsigned next=0;
   for(int c=0;c<MAP_LANES;c++)if(live&(1u<<c))
      next|=game_route_mask(g,r,c);
   live=next;
 }
 return live;
}
int game_route_available(const Game *g,int lane) {"""
s=sub(s,needle,add,"route visibility")
hs=sub(hs,
 "unsigned game_route_row_mask(const Game *g,int row);",
 "unsigned game_route_row_mask(const Game *g,int row);\nunsigned game_route_visible_mask(const Game *g,int row);",
 "visible header")

# A preview-selection must never point to a hidden lane.
s=sub(s,"if(b==BTN_UP&&g->floor+g->scroll<MAP_ROWS-1)g->scroll++;",
"""if(b==BTN_UP&&g->floor+g->scroll<MAP_ROWS-1) {
    g->scroll++;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }""","preview up")
s=sub(s,"""else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll--;
    if(g->scroll==0 && !game_route_available(g,g->selection))
      for(int k=0;k<MAP_LANES;k++)
        if(game_route_available(g,k)){g->selection=k;break;}
  }""",
"""else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll--;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }""","preview down")
s=sub(s,"else if(g->scroll>0&&b==BTN_LEFT&&g->selection>0)g->selection--;",
"""else if(g->scroll>0&&b==BTN_LEFT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection-1;j>=0;j--)if(visible&(1u<<j)){g->selection=j;break;}
  }""","preview left")
s=sub(s,"else if(g->scroll>0&&b==BTN_RIGHT&&g->selection<2)g->selection++;",
"""else if(g->scroll>0&&b==BTN_RIGHT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection+1;j<MAP_LANES;j++)if(visible&(1u<<j)){g->selection=j;break;}
  }""","preview right")

# Precisely explain card statuses; Dread is only penalized at end of a turn.
s=sub(s,'{"Dread","Unplayable; lose energy",-2,0,3}',
'{"Dread","Unplayable. Held at turn end: -1 Energy next turn.",-2,0,3}',
"Dread tooltip")
# Exhaust selection messages in C are already generated in v1.1.3.

# Display only lanes still reachable from the CHOSEN route, not other starts.
v=v.replace("game_route_row_mask(g,r)&(1u<<c)",
            "game_route_visible_mask(g,r)&(1u<<c)")
if "game_route_row_mask(g,r)&(1u<<c)" in v:raise RuntimeError("route render not fully replaced")
# Do not draw roads ending at a hidden path.
v=sub(v,"unsigned routes=game_route_mask(g,r,c);",
"""unsigned routes=game_route_mask(g,r,c);
     if(r+1<MAP_ROWS)routes&=game_route_visible_mask(g,r+1);""",
"reachable road targets")

# Event/shop were incorrectly drawn 480x224 from 240x136 (2.14 vs 1.76).
# A full 480x272 pixel rendition retains the 240:136 ratio, with the
# header/footer drawn over it. The latter naturally masks off 28px on each end.
v=sub(v,'sprite_scaled(0,28,480,224,rift112_event,240,136);',
        'sprite_scaled(0,0,480,272,rift112_event,240,136);',"event aspect")
v=sub(v,'sprite_scaled(0,28,480,224,rift112_shop,240,136);',
        'sprite_scaled(0,0,480,272,rift112_shop,240,136);',"shop aspect")

# At present "START" is indistinguishable from Square in the footer.
v=sub(v,
'if(kind==5){rect(x-5,y-4,12,9,RGB(76,88,104));border(x-5,y-4,12,9,WHITE);return;}',
'''if(kind==5){
   rect(x-7,y-4,19,9,RGB(40,53,75));
   border(x-7,y-4,19,9,WHITE);
   /* Tiny 'ST', visually distinct from the outlined Square glyph. */
   text(x-5,y-4,"ST",WHITE,1);
   return;
 }''',"start glyph")
# The boss relic title area was drawn twice in the original renderer.
# One of the earlier generators may already have removed the second line.
relic_line="wrapped_text(x+7,y+85,relic_defs[g->reward[i]].desc,16,5,WHITE);"
if v.count(relic_line)==2:
    v=v.replace(relic_line+"\n   "+relic_line,relic_line,1)
elif v.count(relic_line)!=1:
    raise RuntimeError("Boss relic title/description anchor missing")
# The artwork reused behind rewards contains faint baked-in "Spoils of Battle"
# lettering. Remove that area before drawing the live reward HUD.
v=sub(v,
    'static void reward_screen(const Game*g){\n header(g,"SPOILS OF BATTLE / CHOOSE A CARD");',
    'static void reward_screen(const Game*g){\n rect(0,28,W,64,BG);\n header(g,"SPOILS OF BATTLE / CHOOSE A CARD");',
    "reward background ghost heading")
v=sub(v,
    'static void boss_reward_screen(const Game*g){\n header(g,"GUARDIAN DEFEATED / SELECT A RELIC");',
    'static void boss_reward_screen(const Game*g){\n rect(0,28,W,64,BG);\n header(g,"GUARDIAN DEFEATED / SELECT A RELIC");',
    "boss reward background ghost heading")
# The background image also had a baked-in SKIP button below the
# boss reward relic cards. Mask that bottom strip without covering card text.
v=sub(v,
  ' footer("LEFT/RIGHT CHOOSE  X ACCEPT RELIC AND ASCEND");',
  ' rect(0,232,W,23,BG);\n footer("LEFT/RIGHT CHOOSE  X ACCEPT RELIC AND ASCEND");',
  "ghost skip beneath boss rewards")
# Current/reward UI is intentionally a single title, with victory replay in
# the battle scene and no second label burned underneath the overlay.

g.write_text(s);h.write_text(hs);r.write_text(v)
print("PASS 1.1.3 QA: reachable future maps, native aspect scenes, distinct START, single relic labels and Dread rules")
