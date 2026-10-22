#!/usr/bin/env python3
"""Rift Ascent v17 feature upgrade. Keep Game struct binary-compatible with old saves."""
from pathlib import Path
import sys
base=Path(sys.argv[1])
h=base/"game.h"; g=base/"game.c"
hs=h.read_text(); s=g.read_text()
def put(a,b):
 global s
 if a not in s: raise RuntimeError("game.c anchor not found: "+a[:85])
 s=s.replace(a,b,1)
def head(a,b):
 global hs
 if a not in hs: raise RuntimeError("game.h anchor not found: "+a)
 hs=hs.replace(a,b,1)
head("void game_choose_node(Game *g,int lane);", """void game_choose_node(Game *g,int lane);
unsigned game_route_mask(const Game *g,int row,int lane);
int game_route_available(const Game *g,int lane);
const char *game_intent_description(int intent);
const char *game_intent_title(int intent);
const char *game_potion_name(int id);
const char *game_potion_description(int id);
int game_effective_damage(const Game *g,int raw,int target);
void game_use_potion(Game *g,int index);
""")
anchor="static void map_generate(Game*g){"
put(anchor, """/* Deterministic branching edges: no save struct layout changes. Edges are
   seeded by the run and act, so maps persist correctly across reloads. */
unsigned game_route_mask(const Game *g,int row,int lane) {
 if(row<0||row>=MAP_ROWS-1||lane<0||lane>=MAP_LANES)return 0;
 uint32_t z=g->seed ^ (uint32_t)(g->act*0x9E3779B9u)
   ^ (uint32_t)((row+1)*0x85EBCA6Bu) ^ (uint32_t)((lane+3)*0xC2B2AE35u);
 z^=z>>16;z*=0x7feb352du;z^=z>>15;z*=0x846ca68bu;z^=z>>16;
 int to=lane+(int)(z%3u)-1;
 if(to<0)to=0;if(to>=MAP_LANES)to=MAP_LANES-1;
 unsigned mask=(1u<<to);
 /* Most nodes have one road; selected junctions fork into two choices. */
 if((z>>6)%100u<38u) {
   int other=lane+((z>>11)&1u?1:-1);
   if(other>=0&&other<MAP_LANES) mask|=(1u<<other);
 }
 return mask;
}
int game_route_available(const Game *g,int lane) {
 if(!g||g->floor>=MAP_ROWS||lane<0||lane>=MAP_LANES)return 0;
 if(g->floor==0)return 1;
 return (game_route_mask(g,g->floor-1,g->lane)&(1u<<lane))!=0;
}
static void map_select_reachable(Game*g) {
 if(g->floor>=MAP_ROWS)return;
 if(game_route_available(g,g->selection))return;
 for(int i=0;i<MAP_LANES;i++)
  if(game_route_available(g,i)){g->selection=i;return;}
}
""" + anchor)
put("static void enter_map(Game *g)", "static void enter_map(Game *g)") if False else None
put("static void enter_map(Game *g)", "static void enter_map(Game *g)") if False else None
put("static void enter_map(Game *g)", "static void enter_map(Game *g)") if False else None
put("static void enter_map(Game*g){g->screen=MAP;g->selection=CLAMP(g->lane,0,2);}",
"""static void enter_map(Game*g){
 g->screen=MAP;g->selection=CLAMP(g->lane,0,2);
 if(g->floor<MAP_ROWS&&!game_route_available(g,g->selection))
   for(int k=0;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}
}""")
put(" if(g->floor>0&&abs(lane-g->lane)>1)return;",
" if(!game_route_available(g,lane))return;")
put(" if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);\n  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);\n  else if(b==BTN_OK){if(g->floor==MAP_ROWS)",
""" if(b==BTN_LEFT){for(int k=g->selection-1;k>=0;k--)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_RIGHT){for(int k=g->selection+1;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_OK){if(g->floor==MAP_ROWS)""")
# Add risk and variety to path contents without changing run structure
put("else if(r==5)n=(c==1?NODE_SHOP:NODE_EVENT);",
    "else if(r==5)n=(c==1?NODE_SHOP:(c==0?NODE_REST:NODE_EVENT));")
put("else if(r==8)n=(c==1?NODE_ELITE:NODE_TREASURE);",
    "else if(r==8)n=(c==1?NODE_ELITE:(c==0?NODE_TREASURE:NODE_REST));")
put("else n=pick<55?NODE_FIGHT:(pick<70?NODE_EVENT:(pick<82?NODE_ELITE:(pick<93?NODE_SHOP:NODE_REST)));",
"""else {
    int bias=(r<=2?10:(r>=9?-10:0));
    n=pick<49+bias?NODE_FIGHT:
      (pick<70?NODE_EVENT:(pick<82?NODE_ELITE:
        (pick<90?NODE_TREASURE:(pick<96?NODE_SHOP:NODE_REST))));
  }""")
# intent descriptions, including numeric multipliers
put("static void init_enemy(Game*g,Enemy*e,int id){",
"""static const char *intent_titles[14]={
 "STRIKE","GUARD","WEAKEN","EXPOSE","EMPOWER","HEX","VENOM",
 "DOUBLE","DRAIN","MEND","CRUSH","DREAD","LEECH","FORTIFY"
};
static const char *intent_details[14]={
 "One direct hit. Block reduces its damage.",
 "Raises enemy block. Enemy block expires next turn.",
 "Small hit and 2 Weak: your attacks deal 25% less.",
 "Full hit and 2 Vulnerable: you take 50% more.",
 "Raises enemy Strength by 2 and gains 8 Block.",
 "Half-damage hit, then adds a Burn to discard.",
 "Half-damage hit and adds 2 Poison to you.",
 "Two separate attacks, each around half the shown value.",
 "Half-damage hit; lose 1 energy next turn.",
 "Heals enemy health and grants some Block.",
 "Heavy attack and adds an unplayable Daze.",
 "Adds a Dread curse and gains Block.",
 "Half-damage hit and heals for HP actually lost.",
 "Raises enemy Block and Strength by 1."
};
const char *game_intent_title(int n){return n>=0&&n<14?intent_titles[n]:"UNKNOWN";}
const char *game_intent_description(int n){return n>=0&&n<14?intent_details[n]:"No information";}
static const char *potion_titles[6]={
 "Healing","Might","Guard","Fire","Energy","Insight"
};
static const char *potion_details[6]={
 "Recover 18 HP; cannot exceed maximum health.",
 "Gain 2 Strength this combat. Buffs each attack hit.",
 "Gain 20 Block, before any relic bonuses.",
 "Deal 20 damage to all living enemies.",
 "Gain 2 Energy immediately.",
 "Draw 3 cards from your draw/discard pile."
};
const char *game_potion_name(int id){return id>=0&&id<6?potion_titles[id]:"None";}
const char *game_potion_description(int id){return id>=0&&id<6?potion_details[id]:"Empty slot";}
int game_effective_damage(const Game *g,int raw,int target) {
 int id=CARD_ID(raw),up=CARD_UP(raw),base=0;
 if(id<0||id>=CARD_COUNT)return 0;
 switch(id){
 case 0:base=6+3*up;break;case 2:base=8+3*up;break;
 case 3:base=8+3*up;break;case 7:base=15+6*up;break;
 case 10:base=(4+2*up)*g->energy;break;
 case 11:base=7+3*up;break;case 12:base=7+3*up;break;
 case 13:base=5+3*up;break;case 14:base=4+3*up;break;
 case 15:base=22+6*up;break;case 19:base=9+3*up;break;
 case 21:base=g->block+5*up;break;
 case 22:base=3;break;case 25:base=7+3*up;break;
 case 27:base=5+3*up;break;case 29:base=17+6*up;break;
 case 32:base=13+3*up;break;case 35:base=9+3*up;break;
 case 36:base=12+6*up;break;case 39:base=12+3*up;break;
 case 41:base=6+3*up;break;case 42:base=28+9*up;break;
 case 44:base=5+3*up;break;case 47:base=11+3*up;break;
 case 49:base=6+3*up;break;
 default:return 0;
 }
 int dmg=MAX(0,base+g->strength);
 if(g->weak)dmg=dmg*3/4;
 if(target>=0&&target<g->enemy_count&&g->enemies[target].vulnerable)dmg=dmg*3/2;
 if(g->turn_played==0&&relic(g,3))dmg+=5;
 if(relic(g,26)&&g->hp==1)dmg+=2;
 if(id==27||id==22)dmg*= (id==27?2:4+up);
 if(id==10)dmg=MAX(0,base+g->strength)*(g->weak?3:4)/4*(g->energy?1:0);
 return dmg;
}
static void init_enemy(Game*g,Enemy*e,int id){""")
# potion selection (previously hard-coded last)
put("static void use_potion(Game*g){\n if(g->screen!=COMBAT||g->potion_count<=0)return;\n int id=g->potions[--g->potion_count];int bonus=relic(g,17)?5:0;",
"""void game_use_potion(Game*g,int index){
 if(g->screen!=COMBAT||g->potion_count<=0)return;
 if(index<0||index>=g->potion_count)return;
 int id=g->potions[index];int bonus=relic(g,17)?5:0;
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];
 g->potion_count--;""")
put("else if(b==BTN_POTION)use_potion(g);",
    "else if(b==BTN_POTION)game_use_potion(g,g->potion_count-1);")
# Main controls: end turn is selectable beyond last hand card
put("else if(b==BTN_RIGHT)g->selection=MIN(MAX(0,g->hand_count-1),g->selection+1);",
    "else if(b==BTN_RIGHT)g->selection=MIN(g->hand_count,g->selection+1);")
put("else if(b==BTN_OK)game_play_card(g,g->selection,g->target);\n  else if(b==BTN_BACK)game_end_turn(g);",
    "else if(b==BTN_OK){if(g->selection>=g->hand_count)game_end_turn(g);else game_play_card(g,g->selection,g->target);}\n  else if(b==BTN_BACK)game_end_turn(g);")
# D-pad title navigation (selection 0 play, 1 help)
put('if(b==BTN_OK){g->screen=MAP;msg(g,"Choose your first encounter.");}else if(b==BTN_ALT){g->screen=HELP;g->prior_screen=TITLE;}break;',
"""if(b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;
  else if(b==BTN_OK){
   if(g->selection==0){g->screen=MAP;msg(g,"Choose your first encounter.");enter_map(g);}
   else {g->screen=HELP;g->prior_screen=TITLE;}
  }else if(b==BTN_ALT){g->screen=HELP;g->prior_screen=TITLE;}break;""")
# ensure same save layout; avoid modifying struct and GAME_VERSION
h.write_text(hs);g.write_text(s)
print("Applied engine map, intent, potion, preview, and control enhancements.")
