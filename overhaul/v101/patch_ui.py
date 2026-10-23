#!/usr/bin/env python3
"""1.0.1 UI clarity: gameplay icons, previews, glyphs, relic ledger, cards."""
from pathlib import Path
import sys
root=Path(sys.argv[1]); p=root/"render.c"; s=p.read_text()
def sub(a,b):
 global s
 if a not in s:raise RuntimeError("missing render anchor: "+a[:90])
 s=s.replace(a,b,1)
# Compact hand cards allocate space to readable descriptions, not huge art.
sub("int tall=h>=110,arth=tall?57:21;",
    "int tall=h>=120,arth=tall?57:18;")
sub("  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?175+wave(animation_tick,60,1):181;",
    "  int x=56+(i-start)*73,selected=(i==g->selection),y=selected?132+wave(animation_tick,60,1):148;")
sub("card_render(x,y,70,selected?78:71,g->hand[i]",
    "card_render(x,y,70,selected?117:103,g->hand[i]")
# A partial card on the right makes horizontal overflow visible.
sub(" card_combat_context=0;\n button(414",''' if(start+visible<g->hand_count){
   gilded_panel(383,166,25,74,0);
   text(386,194,">",GOLD,2);
 }
 if(start>0)text(42,189,"<",GOLD,2);
 card_combat_context=0;
 button(414''')
# Replace overly cryptic enemy intent labels with actual Weak-aware damage.
start=s.index("  switch(e->intent){",s.index("static void combat("))
end=s.index("  dark_rect(ex-42,142",start)
s=s[:start]+'''  {
    int hits=game_enemy_intent_hits(g,i);
    int damage=game_enemy_intent_damage(g,i);
    dark_rect(ex-46,45,92,34,199);
    if(hits==2)snprintf(b,sizeof(b),"2x%d DMG",damage/2);
    else if(hits)snprintf(b,sizeof(b),"%d DAMAGE",damage);
    else snprintf(b,sizeof(b),"NO ATTACK");
    center(ex,47,b,hits?RED:GOLD,1);
    const char *effect="";
    switch(e->intent){
      case 1:effect="BLOCK";break;case 2:effect="WEAK 2";break;
      case 3:effect="VULN 2";break;case 4:effect="STR +2 BLK +8";break;
      case 5:effect="ADDS BURN";break;case 6:effect="POISON +2";break;
      case 8:effect="LOSE 1 ENERGY";break;case 9:effect="HEAL + BLOCK";break;
      case 10:effect="ADDS DAZE";break;case 11:effect="DREAD + BLOCK";break;
      case 12:effect="HEALS ON HIT";break;case 13:effect="BLOCK + STR";break;
    }
    if(effect[0])center(ex,64,effect,GOLD,1);
  }
'''+s[end:]
# Full tooltip: live incoming action numbers plus every condition.
sub('''      snprintf(b,sizeof(b),"%s  /  %d",game_intent_title(e->intent),e->intent_value);
      center(240,109,b,RED,1);
      wrapped_text(43,135,game_intent_description(e->intent),55,5,WHITE);''',
'''      int hits=game_enemy_intent_hits(g,g->target);
      int damage=game_enemy_intent_damage(g,g->target);
      if(hits>1)snprintf(b,sizeof(b),"%d HITS x %d DAMAGE (BEFORE BLOCK)",hits,damage/hits);
      else snprintf(b,sizeof(b),"%d INCOMING DAMAGE (BEFORE BLOCK)",damage);
      center(240,109,b,RED,1);
      wrapped_text(43,129,game_intent_description(e->intent),55,5,WHITE);
      snprintf(b,sizeof(b),"FOE: STR %+d | WEAK %d | VUL %d | PSN %d",
               e->strength,e->weak,e->vulnerable,e->poison);
      text(43,207,b,GOLD,1);''')
# Status page includes selected foe, not just player.
sub('''   wrapped_text(43,154,"Weak reduces attack by 25%. Vulnerable increases damage taken by 50%. Poison damages you at turn end.",53,5,MUTED);''',
'''   wrapped_text(43,149,"Weak: -25% outgoing. Vulnerable: +50% incoming. Poison: HP loss at end of turn.",53,3,MUTED);
   if(g->enemy_count){
     const Enemy *e=&g->enemies[g->target];
     snprintf(b,sizeof(b),"FOE: %s  HP %d  BLOCK %d",e->name,e->hp,e->block);
     text(43,191,b,GOLD,1);
     snprintf(b,sizeof(b),"STR %+d   WEAK %d   VUL %d   PSN %d",
              e->strength,e->weak,e->vulnerable,e->poison);
     text(43,207,b,WHITE,1);
   }''')
# Fifth inspector tab: collected relics.
sub('''const char *tabs[]={"CARD","FOE","STATUS","POTIONS"};''',
    '''const char *tabs[]={"CARD","FOE","STATUS","POTIONS","RELICS"};''')
sub(''' for(int i=0;i<4;i++){
    int px=30+i*106;
    rect(px,48,96,19,''',
''' for(int i=0;i<5;i++){
    int px=28+i*87;
    rect(px,48,83,19,''')
sub('''    center(px+48,52,tabs[i],overlay_page==i?GOLD:WHITE,1);''',
    '''    center(px+41,52,tabs[i],overlay_page==i?GOLD:WHITE,1);''')
sub(''' }else {
    for(int i=0;i<g->potion_count;i++){''',
    ''' }else if(overlay_page==3) {
    for(int i=0;i<g->potion_count;i++){''')
sub('''    if(g->potion_count==0)center(240,113,"NO POTIONS IN YOUR BELT",MUTED,1);
 }
 footer(''',
'''    if(g->potion_count==0)center(240,113,"NO POTIONS IN YOUR BELT",MUTED,1);
 }else {
    for(int i=0;i<g->relic_count&&i<7;i++){
       int id=g->relics[i],y=80+i*20;
       snprintf(b,sizeof(b),"%s: %s",relic_defs[id].name,relic_defs[id].desc);
       fittext(38,y,b,56,i==g->selection?GOLD:WHITE);
    }
    if(g->relic_count>7)text(38,224,"FULL LIST: MAP -> START",MUTED,1);
 }
 footer(''')
# Replace plain letter node labels with recognizable vector drawings.
anchor="static void map_screen(const Game*g){"
sub(anchor,'''static void room_icon(int n,int x,int y) {
 switch(n){
 case NODE_FIGHT: /* crossed swords */
    line(x-6,y-6,x+6,y+6,WHITE);
    line(x+6,y-6,x-6,y+6,WHITE);
    line(x-7,y+5,x-3,y+9,GOLD);line(x+7,y+5,x+3,y+9,GOLD);break;
 case NODE_ELITE:
    circle(x,y-2,6,WHITE);circle(x-3,y-3,2,BG);circle(x+3,y-3,2,BG);
    rect(x-4,y+4,8,4,WHITE);break;
 case NODE_EVENT:text(x-3,y-6,"?",WHITE,1);break;
 case NODE_SHOP: /* coin purse */
    circle(x,y+2,6,GOLD);line(x-4,y-6,x+4,y-6,WHITE);
    text(x-3,y-4,"$",BG,1);break;
 case NODE_REST:triangle(x,y-8,x-5,y+7,x+5,y+7,GOLD);break;
 case NODE_TREASURE:triangle(x,y-7,x-7,y+1,x,y+8,GOLD);
    triangle(x,y-7,x+7,y+1,x,y+8,GOLD);break;
 case NODE_BOSS:triangle(x-7,y+6,x,y-7,x+7,y+6,RED);break;
 }
}
'''+anchor)
sub('''   char s[]={node_symbol(n),0};center(x,y-5,s,BG,1);''',
    '''   room_icon(n,x,y);''')
# Show a distinct final boss room above final tier.
sub(''' text(324,63,"THE ASCENT",GOLD,1);''',
''' if(g->floor>=MAP_ROWS-5){
    int last_y=220-(MAP_ROWS-1-first)*37;
    line(168,last_y-13,168,52,GOLD);
    circle(168,58,12,RGB(68,23,31));
    room_icon(NODE_BOSS,168,58);
    text(190,53,"BOSS",RED,1);
 }
 text(324,63,"THE ASCENT",GOLD,1);''')
# Replace list-only card deck with visual grid + selected and upgrade preview.
start=s.index("static void deck_screen(const Game*g){")
end=s.index("static void title_screen(",start)
s=s[:start]+'''static void relic_screen(const Game*g){
 header(g,"RELICS / ITEMS DISCOVERED");
 char b[112];
 snprintf(b,sizeof(b),"%d RELICS",g->relic_count);
 text(351,49,b,GOLD,1);
 int first=CLAMP(g->selection-6,0,MAX(0,g->relic_count-10));
 for(int i=first;i<g->relic_count&&i<first+10;i++){
   int id=g->relics[i],y=66+(i-first)*18;
   if(i==g->selection)rect(16,y-2,448,17,RGB(75,63,44));
   snprintf(b,sizeof(b),"%s - %s",relic_defs[id].name,relic_defs[id].desc);
   fittext(24,y,b,62,i==g->selection?GOLD:WHITE);
 }
 footer("UP/DOWN BROWSE   X/CIRCLE RETURN");
}
static void deck_screen(const Game*g){
 header(g,g->prior_screen==REST||g->prior_screen==EVENT?
        "SELECT AN UPGRADE / BEFORE AND AFTER":"YOUR DECK / CARD COLLECTION");
 char b[128];
 int first=(g->selection/6)*6;
 for(int i=first;i<g->deck_count&&i<first+6;i++){
    int idx=i-first;
    int x=18+(idx%3)*108,y=66+(idx/3)*90;
    card_render(x,y,99,84,g->deck[i],i==g->selection,1);
 }
 gilded_panel(344,55,126,188,0);
 if(g->deck_count>0){
   int raw=g->deck[g->selection],id=CARD_ID(raw);
   fittext(351,66,cards[id].name,16,GOLD);
   text(351,84,"CURRENT",WHITE,1);
   wrapped_text(351,99,card_description(raw),16,4,WHITE);
   if(g->prior_screen==REST||g->prior_screen==EVENT){
     text(351,149,"AFTER UPGRADE",GOLD,1);
     if(CARD_UP(raw)||id>=50)text(351,168,"MAX UPGRADED",RED,1);
     else wrapped_text(351,166,card_description(raw|UPBIT),16,5,GREEN);
   }else{
     snprintf(b,sizeof(b),"CARD %d / %d",g->selection+1,g->deck_count);
     text(351,167,b,GOLD,1);
   }
 }
 if(first+6<g->deck_count)text(351,224,"MORE BELOW v",GOLD,1);
 footer(g->prior_screen==REST||g->prior_screen==EVENT?
        "D-PAD CHOOSE  X UPGRADE  O RETURN":"D-PAD BROWSE  O RETURN");
}
'''+s[end:]
sub(''' case HELP:help_screen(g);break;''',
    ''' case HELP:help_screen(g);break;
 case RELIC_VIEW:relic_screen(g);break;''')
p.write_text(s)
print("Version 1.0.1 UI patch applied.")
