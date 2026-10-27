from pathlib import Path
q=Path(__file__).parent/'source'
def edit(name,changes):
 p=q/name;s=p.read_text()
 for old,new in changes:
  assert old in s,(name,old[:100]);s=s.replace(old,new)
 p.write_text(s)
edit('game.c',[
 ('{"Violet Orb","Potions become: Healing 23 HP; Strength +3; Guard 25 Block; Fire 25 base damage; Energy +3; Insight draws 4."}', '{"Violet Orb","Improves every potion you use: +5 healing, +5 Block or +5 base Fire damage; +1 Strength, +1 Energy or +1 card drawn."}'),
 ('{"Seer\'s Oath","Draw 2 additional cards on the first turn of every combat."}', '{"Old Compass","Draw 2 extra cards on the first turn of each combat."} /* legacy Seer\'s Oath ID; never offered */'),
 ('static void add_relic(Game*g,int id){', 'static void add_relic(Game*g,int id){if(id==30)id=7;'),
 ('id<37;id++)if(!relic(g,id))', 'id<37;id++)if(id!=30&&!relic(g,id))'),
 ('2*(relic(g,7)+relic(g,30))', '2*relic(g,7)'),
 ('e->weak=MAX(e->weak,2);', 'e->weak+=2;'),
 ('if(!game_validate(&temp))return 0;*g=temp;return 1;', '''if(!game_validate(&temp))return 0;
 /* Retire the duplicate blessing without renumbering any saved relic IDs. */
 int count=0,compass=0;
 for(int i=0;i<temp.relic_count;i++){
  int id=temp.relics[i]==30?7:temp.relics[i];
  if(id==7&&compass)continue;
  if(id==7)compass=1;
  temp.relics[count++]=id;
 }
 temp.relic_count=count;
 /* A saved, unclaimed Seer's Oath becomes a repeatable Strength blessing;
    it remains a legal blessing slot even when Compass is already owned. */
 if(temp.screen==BLESSING)for(int i=0;i<3;i++)if(temp.reward[i]==30){
  int replacement=37;
  for(;replacement<40;replacement++){int used=0;for(int j=0;j<3;j++)used|=temp.reward[j]==replacement;if(!used)break;}
  temp.reward[i]=replacement<40?replacement:37;
 }
 *g=temp;return 1;'''),
 ('const char *game_potion_description(int id){return id>=0&&id<6?potion_details[id]:"Empty slot";}', '''const char *game_potion_description(int id){return id>=0&&id<6?potion_details[id]:"Empty slot";}
const char *game_potion_description_now(const Game*g,int id){
 static const char*enhanced[]={"Heal 23 HP (Violet Orb).","Gain 3 Strength this combat (Violet Orb).","Gain 25 Block (Violet Orb).","Deal 25 base damage to all enemies (Violet Orb).","Gain 3 Energy immediately (Violet Orb).","Draw 4 cards (Violet Orb)."};
 return g&&relic(g,17)&&id>=0&&id<6?enhanced[id]:game_potion_description(id);
}''')])
edit('game.h',[('const char *game_potion_description(int id);','const char *game_potion_description(int id);\nconst char *game_potion_description_now(const Game *g,int id);')])
edit('psp_start.c',[('before_input.hand_count!=game.hand_count||fx || events ||','before_input.hand_count!=game.hand_count||before_input.potion_count!=game.potion_count||fx || events ||')])
edit('combat_fx.inc',[
 ('static int r137_last_screen,r1311_finishing,r1311_kill_at;', 'static int r137_last_screen,r1311_finishing,r1311_kill_at,r140_chime_done;'),
 ('r137_player_action=0;', 'r137_player_action=0;r140_chime_done=0;'),
 ('e->kind==GFX_POWER?30:', 'e->kind==GFX_POWER?16:'),
 (' int age=r137_age(tick);\n for(int i=0;i<r137_count;', ''' int age=r137_age(tick);
 if(r1311_finishing&&!r140_chime_done&&r137_raw_age(tick)>=r1311_banner_at()){
  r140_chime_done=1;rift_sound_effect(10);
 }
 for(int i=0;i<r137_count;'''),
 ('static void r137_scene(const Game*g){', '''/* Three readable casting poses, then immediately return to the idle stance. */
static int r140_power_pose(int age){return age<0||age>=16?-1:age<4?1:age<12?3:5;}
static int r140_idle_breath(int art,unsigned tick){return (wave(tick/4u+(unsigned)art*17u,96+art%5*12,2)+2)/2;}
static void r137_scene(const Game*g){'''),
 ('}else if(pp>=0){int a=age-r137_at[pp];dy=0;pose=3;pose_frame=MIN(5,a/5);lean=0;}', '}else if(pp>=0){int f=r140_power_pose(age-r137_at[pp]);if(f>=0){dy=0;pose=3;pose_frame=f;lean=0;}}'),
 ('pose=r137_prepose;pose_frame=pose==2?2:pre<10?0:1;', 'pose=r137_prepose;pose_frame=pose==2?2:pose==3?1:pre<10?0:1;'),
 ('r137_blit(41,58,150,113,r138_death', 'r137_blit(41,61,150,113,r138_death'),
 ('r137_blit(51+dx,58+dy,163,113', 'r137_blit(51+dx,61+dy,163,113'),
 ('r1311_power_y[pose_frame],150,113', 'r1311_power_y[pose_frame]+3,150,113'),
 ('r137_blit(53+dx,58+dy,150,113', 'r137_blit(53+dx,61+dy,150,113'),
 ('r137_actor(41+dx,87+dy+wave(animation_tick/4,110,1),pw,ph', 'r137_actor(41+dx,90+dy,pw,ph'),
 ('int atk=r137_latest(e,GFX_ATTACK,age),guard=r137_latest(e,GFX_BLOCK,age);', '''int atk=r137_latest(e,GFX_ATTACK,age),guard=r137_latest(e,GFX_BLOCK,age);
  if(atk<0&&guard<0&&hit<0&&!enemy_windup){
   int breath=r140_idle_breath(art,animation_tick);h-=breath;y+=breath;
  }'''),
 ('e->kind==GFX_POWER&&a<30', 'e->kind==GFX_POWER&&a<16'),
 ('int phase=MIN(5,a/5),size=65+(a<15?a:30-a);', 'int phase=a<4?1:a<12?3:5,size=65+(a<8?a:16-a);'),
 ('a>23?(30-a)*255/7:240', 'a>11?(16-a)*255/5:240')])
edit('sound.h',[('unsigned power_seen,power_age,power_phase;', 'unsigned power_seen,power_age,power_phase;\n unsigned victory_seen,victory_age,victory_phase[3];')])
edit('sound.c',[
 ('impact_serial=0,power_serial=0;', 'impact_serial=0,power_serial=0,victory_serial=0;'),
 ('if(kind<=0||kind>9)return;', 'if(kind<=0||kind>10)return;\n if(kind==10){victory_serial++;return;}'),
 ('s->power_seen=power_serial;', 's->victory_seen=victory_serial;s->victory_age=40000;for(int i=0;i<3;i++)s->victory_phase[i]=0;\n s->power_seen=power_serial;'),
 ('  out[i*2]=limit(sfx);', '''  /* Short three-note bell flourish, separate from attacks and cursor SFX. */
  if(s->victory_seen!=victory_serial){s->victory_seen=victory_serial;s->victory_age=0;for(int n=0;n<3;n++)s->victory_phase[n]=0;}
  if(s->victory_age<28665u){
   static const unsigned step[3]={50960076u,64188931u,76352335u};
   for(int n=0;n<3;n++){
    int a=(int)s->victory_age-n*3528,duration=21609;
    if(a>=0&&a<duration){
     int env=(duration-a)*4096/duration;if(a<160)env=env*a/160;
     int bell=triangle(s->victory_phase[n])/2+triangle(s->victory_phase[n]*2u)/4+triangle(s->victory_phase[n]*3u)/8;
     sfx+=(bell*env/4096)*4000/32768;s->victory_phase[n]+=step[n];
    }
   }
   s->victory_age++;
  }
  out[i*2]=limit(sfx);''')])
# Render changes use generated artwork in every replaced selection / victory asset.
edit('render.c',[
 ('#include "rift1311_art.h"', '#include "rift1311_art.h"\n#include "rift140_art.h"'),
 ('static char r1310_potion_notice[72];', 'static int r140_potion_gained=-1;static unsigned r140_potion_tick;\nstatic char r1310_potion_notice[72];'),
 (' r137_feedback(before,after,action,tick);', ''' r137_feedback(before,after,action,tick);
 if(after->potion_count>before->potion_count&&before->screen==SHOP){r140_potion_gained=after->potions[after->potion_count-1];r140_potion_tick=tick;}
 else if(before->screen!=after->screen)r140_potion_gained=-1;'''),
 ('pix[(177+yy)*sw+17+xx]=r134_mix(pix[(177+yy)*sw+17+xx]', 'pix[(186+yy)*sw+39+xx]=r134_mix(pix[(186+yy)*sw+39+xx]'),
 ('r134_blit(419,176,36,28,r1311_arrow_glow', 'r134_blit(392,186,36,28,r1311_arrow_glow'),
 ('static void reward_screen(const Game*g){', '''static void r140_potion_card(const Game*g,int id,int x,int y,int w){
 r134_blit(x,y,w,43,r140_selection,208,28,255);
 r134_blit(x+15,y+8,27,27,r134_utility_icons[0],40,40,255);
 r134_text(x+49,y+7,game_potion_name(id),GOLD,&r134_f9b);
 r134_wrap(x+49,y+20,w-70,2,10,game_potion_description_now(g,id),WHITE,&r134_f8,0);
}
static void reward_screen(const Game*g){'''),
 ('if(g->ic[37]>0&&g->ic[37]<=6){char notice[90];snprintf(notice,sizeof(notice),"POTION GAINED: %s",game_potion_name(g->ic[37]-1));r134_center(240,219,notice,GREEN,&r134_f9b);}\n button(198,234,84,20,"SKIP",g->selection==3);', '''if(g->ic[37]>0&&g->ic[37]<=6){
  r140_potion_card(g,g->ic[37]-1,20,211,366);
  button(394,228,68,23,"SKIP",g->selection==3);
 }else button(198,234,84,20,"SKIP",g->selection==3);'''),
 ('  button(x+12,194,166,35,i?"FORGE - UPGRADE":"REST - HEAL 30%",g->selection==i);', '''  int chosen=g->selection==i;
  if(!chosen)dark_rect(x+6,53,178,187,125);
  r134_blit(x+7,191,176,43,r140_selection,208,28,chosen?255:100);
  r134_center(x+95,208,i?"UPGRADE A CARD":"HEAL 30% HP",chosen?WHITE:MUTED,&r134_f10);
  if(chosen)r134_blit(x+74,42,42,28,r138_choice,42,28,255);'''),
 ('if(!memcmp(g->message,"Potion gained:",14))r134_text(34,241,g->message,GREEN,&r134_f9b);', 'if(r140_potion_gained>=0&&animation_tick-r140_potion_tick<720)r140_potion_card(g,r140_potion_gained,28,209,424);'),
 ('int y=53+i*61;gilded_panel(27,y,426,58,i==g->selection);', 'int y=53+i*61;if(i==g->selection)r134_blit(27,y,426,58,r140_selection,208,28,255);'),
 ('r134_blit(23,y-4,208,26,r139_select,208,26,255)', 'r134_blit(23,y-4,208,28,r140_selection,208,28,255)'),
 ('r134_text(chosen?59:36,y+5', 'r134_text(46,y+5'),
 ('game_potion_description(g->potions[i])', 'game_potion_description_now(g,g->potions[i])'),
 ('if(selected)gilded_panel(30,y-5,420,42,1);', 'if(selected)r134_blit(30,y-5,420,43,r140_selection,208,28,255);'),
 ('text(42,y,b,selected?GOLD:WHITE,1);', 'r134_text(66,y+1,b,selected?GOLD:WHITE,&r134_f9b);\n       r134_blit(42,y+4,18,25,r134_utility_icons[0],40,40,255);'),
 ('wrapped_text(52,y+14,game_potion_description_now(g,g->potions[i]),50,2,MUTED);', 'r134_wrap(66,y+15,354,2,10,game_potion_description_now(g,g->potions[i]),WHITE,&r134_f8,0);'),
 ('r134_blit(31,y-5,200,26,r139_select,208,26,255)', 'r134_blit(31,y-5,200,28,r140_selection,208,28,255)'),
 ('r134_text(i==pick?65:42,y+3', 'r134_text(55,y+3'),
 ('pix=pixels;sw=stride;dark_rect(0,28,W,146,150);\n     r134_center(240,113,"VICTORY",GOLD,&r134_f18b);', 'pix=pixels;sw=stride;\n     r134_blit(78,71,324,100,r140_victory,324,100,255);')])
print('Applied 1.4.0 presentation and relic fixes')
