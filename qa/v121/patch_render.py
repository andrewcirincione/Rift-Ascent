#!/usr/bin/env python3
"""1.2.1 small-screen UI corrections and pre-effect/kill camera polish."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/"render.c";s=p.read_text()
def one(old,new,name):
 global s
 n=s.count(old)
 if n!=1:raise RuntimeError(f"v121 renderer {name}: expected 1 anchor, got {n}")
 s=s.replace(old,new,1)
# Stable basic card images: never vary by screen position.
one('rift120_art(art_id,x,y)','rift120_cards[art_id]','one Strike/Defend image')
one('rift120_art(id,x,y)','rift120_cards[id]','one Strike/Defend image elsewhere')
# Current XMB card artwork may be quite detailed, but consistent static tiles.
# Enemy names go into a real event feed on encounter entry.
one('''   rift120_log_add("A NEW FIGHT BEGINS");''',
'''   char encounter[64]="FOES: ";
   for(int i=0;i<after->enemy_count;i++){
      int used=(int)strlen(encounter);
      if(i>0)snprintf(encounter+used,sizeof(encounter)-used,"%s"," + ");
      used=(int)strlen(encounter);
      snprintf(encounter+used,sizeof(encounter)-used,"%s",
               game_enemy_name(after->enemies[i].art));
   }
   rift120_log_add(encounter);''','enemy names appear on entry')
# Distinguish turns without ugly unformatted separators.
one(''' if(before->screen==COMBAT&&after->screen==COMBAT&&
    before->turn!=after->turn){''',
''' if(before->screen==COMBAT&&after->screen==COMBAT&&
    before->turn!=after->turn){
   char turn[64];snprintf(turn,sizeof(turn),"TURN %d",after->turn);
   rift120_log_add(turn);''','turn marker')
start=s.index('static void rift120_hp_meter(')
end=s.index('static void background(',start)
s=s[:start]+'''/* Unified compact health capsules.  Block lives outside the capsule,
   exactly where it belongs for both player and enemies. */
static void rift120_hp_meter(int x,int y,int w,int hp,int maxhp,int shield){
 char b[32];
 int bw=w+8;
 rect(x+4,y-3,bw-8,27,RGB(9,12,20));
 rect(x+2,y-1,bw-4,23,RGB(9,12,20));
 rect(x,y+2,bw,17,RGB(9,12,20));
 snprintf(b,sizeof(b),"%d/%d",hp,maxhp);
 center(x+bw/2,y,b,WHITE,1);
 rect(x+4,y+13,bw-8,6,RGB(57,48,48));
 if(maxhp>0)rect(x+4,y+13,((bw-8)*CLAMP(hp,0,maxhp))/maxhp,6,RED);
 if(shield>0){
   border(x+4,y+12,bw-8,8,BLUE);
   const int bx=x+bw+3;
   rect(bx,y+5,13,14,RGB(20,42,75));
   border(bx,y+5,13,14,BLUE);
   triangle(bx+3,y+8,bx+10,y+8,bx+6,y+16,BLUE);
   snprintf(b,sizeof(b),"%d",shield);text(bx+16,y+8,b,WHITE,1);
 }
}
'''+s[end:]
one('rift120_hp_meter(45,142,86,g->hp,g->maxhp,g->block);',
    'rift120_hp_meter(45,142,70,g->hp,g->maxhp,g->block);','player HP')
one('rift120_hp_meter(ex-43,142,86,e->hp,e->maxhp,e->block);',
    'rift120_hp_meter(ex-(g->enemy_count==1?39:29),142,g->enemy_count==1?70:53,e->hp,e->maxhp,e->block);','enemy HP overlap')
one('''    border(ex-45,138,90,33,GOLD);''',
'''    if(g->enemy_count>1)border(ex-35,138,70,30,GOLD);''','no solo target highlight')
# Paint both old and alternative routes with accessible nodes highlighted,
# preserving the complete map without silently erasing previous choices.
one('''     if(!(game_route_visible_mask(g,r)&(1u<<c)))continue;''',
    '''     if(!(game_route_row_mask(g,r)&(1u<<c)))continue;''','all branches retained edges')
one('''     if(r+1<MAP_ROWS)routes&=game_route_visible_mask(g,r+1);''',
    '''     if(r+1<MAP_ROWS)routes&=game_route_row_mask(g,r+1);''','all branches retained endpoints')
one('''   if(!(game_route_visible_mask(g,r)&(1u<<c)))continue;''',
    '''   if(!(game_route_row_mask(g,r)&(1u<<c)))continue;''','all map nodes retained')
# Show three action segments newest first. Delimit turn marker in gold.
start=s.index(' {\n   char logline[160]="";')
end=s.index(' if(!memcmp(g->message,"Random exhaust:",15))',start)
s=s[:start]+''' {
   int x=13;
   for(int i=2;i>=0;i--){
     if(!rift120_log[i][0])continue;
     if(x>13){text(x,31," | ",GOLD,1);x+=21;}
     const char *entry=rift120_log[i];
     int cols=MIN(28,(468-x)/7);
     if(cols<=0)break;
     fittext(x,31,entry,cols,(memcmp(entry,"TURN",4)==0)?GOLD:WHITE);
     x+=MIN((int)strlen(entry),cols)*7;
     if(x>454)break;
   }
 }
'''+s[end:]
one(''' "Pay 12 gold. Choose TWO eligible deck cards to upgrade; effects previewed.",''',
    ''' "Pay 12 gold to upgrade two eligible cards.",''','rusted forge unnecessary text')
one('''   else if(i==4)snprintf(label,sizeof(label),"SERVICE HEAL 18 HP");''',
    '''   else if(i==4)snprintf(label,sizeof(label),"NO HEALING SERVICE");''','shop option label')
one(''' "Restore 18 HP, up to your maximum health. One purchase per selection."''',
    ''' "Merchants sell cards, relics and potions, not healing."''','merchant tooltip')
# Avoid random attack/art-shaped particle geometry surrounding enemies.
one('''      for(int k=0;k<12;k++){
        unsigned seed=particle_hash((unsigned)(i*97+k*2003));
        int dx=((int)(seed%47)-23)*(int)age/15;
        int dy=((int)((seed>>8)%39)-20)*(int)age/15;
        put(ex+dx,110+dy,((k+age)&1)?GOLD:RED);
      }''','''      /* No unrelated decorative geometry around enemy silhouettes. */''','remove enemy random shapes')
# All enemies face the player at screen-left unless explicitly known already.
one('''   int facing_wrong=(e->art==1||e->art==4||e->art==9||e->art==10||
                     e->art==14||e->art==15||e->art==16||e->art==20||
                     e->art==26||e->art==29||e->art==32);''',
'''   int facing_wrong=(e->art==1||e->art==4||e->art==9||e->art==10||
                     e->art==14||e->art==15||e->art==16||e->art==20||
                     e->art==26||e->art==29||e->art==32||
                     e->art==2||e->art==6||e->art==7||e->art==11||
                     e->art==18||e->art==21||e->art==23||e->art==27||
                     e->art==30||e->art==35);''','flip more enemy profiles')
# A killing blow is legible for 30 frames; combat continues showing the scene
# at one-third-speed before the overlay fades into the actual reward.
one(''' animation_tick=tick;
 pix=pixels;sw=stride;background(g);''',
''' animation_tick=tick;
 if(g->screen==REWARD&&victory_replay&&tick>=victory_tick &&
    tick-victory_tick<30){
   Game shot=victory_scene;
   shot.screen=COMBAT;
   for(int i=0;i<shot.enemy_count;i++)if(shot.enemies[i].hp<=0)shot.enemies[i].hp=1;
   unsigned age=tick-victory_tick;
   game_render_frame(&shot,pixels,stride,victory_tick+age/3);
   if(age>17){
      pix=pixels;sw=stride;dark_rect(0,28,W,227,(int)(age-17)*16);
   }
   return;
 }
 pix=pixels;sw=stride;background(g);''','slow motion killing blow')
# Pre-effect card panel: render identical full-size card, never stretched,
# rise for 8 frames, pulse 4 frames, then fade 8 frames.
start=s.index('   if(tick>=rift120_play_tick && tick-rift120_play_tick<26){')
end=s.index('   inspector(g);',start)
s=s[:start]+'''   if(tick>=rift120_play_tick && tick-rift120_play_tick<24){
     unsigned age=tick-rift120_play_tick;
     int shown=age>=12&&rift120_auto_card>=0?rift120_auto_card:
       rift120_played_card;
     if(shown<0)shown=rift120_auto_card;
     if(shown>=0&&shown<CARD_COUNT*2){
       int rise=age<9?(int)age*2:18;
       int x=188,y=59-rise;
       card_combat_context=0;
       card_render(x,y,104,132,shown,1,1);
       border(x-2,y-2,108,136,age<12?GOLD:BLUE);
       if(age>=17)dark_rect(x,y,104,132,(int)(age-16)*29);
     }
   }else if(tick>=rift120_play_tick&&tick-rift120_play_tick>=24){
      rift120_played_card=-1;rift120_auto_card=-1;
   }
'''+s[end:]
p.write_text(s)
print("PASS 1.2.1: unified HUD, card animation, final blow, logs, paths, clean enemy silhouettes")
