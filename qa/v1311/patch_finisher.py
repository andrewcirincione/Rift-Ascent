from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'combat_fx.inc';s=p.read_text().replace('static int r137_last_screen,r1311_finishing;','static int r137_last_screen,r1311_finishing,r1311_kill_at;\n#define R1311_SLOW_WINDOW 28\n#define R1311_SLOW_FACTOR 4\n#define R1311_VICTORY_HOLD 144\nstatic int r1311_banner_at(void){return r1311_kill_at+R1311_SLOW_WINDOW*R1311_SLOW_FACTOR;}\nstatic int r1311_finish_end(void){return r1311_banner_at()+R1311_VICTORY_HOLD+24;}')
s=s.replace('static int r137_age(unsigned tick){return tick>=r137_start?(int)(tick-r137_start):999;}','''static int r137_raw_age(unsigned tick){return tick>=r137_start?(int)(tick-r137_start):999;}
static int r137_age(unsigned tick){
 int age=r137_raw_age(tick);
 if(r1311_finishing&&age>=r1311_kill_at){
  int d=age-r1311_kill_at,window=R1311_SLOW_WINDOW*R1311_SLOW_FACTOR;
  age=r1311_kill_at+(d<window?d/R1311_SLOW_FACTOR:R1311_SLOW_WINDOW+d-window);
 }
 return age;
}
int game_render_music_paused(unsigned tick){
 int age=r137_raw_age(tick);
 return r1311_finishing&&age>=r1311_kill_at&&age<r1311_finish_end();
}''')
s=s.replace(' int a=r137_age(tick),end=r137_duration;',' int a=r137_age(tick),end=r137_duration;\n if(r1311_finishing){a=r137_raw_age(tick);end=r1311_finish_end();return a<end?end-a:0;}')
s=s.replace(' r137_last_screen=after->screen;',''' r1311_kill_at=0;
 if(r1311_finishing){
  int hp[2]={before->enemies[0].hp,before->enemies[1].hp};
  for(int i=0;i<r137_count;i++){
   const GameCombatFx*e=&r137_events[i];
   if(e->target>=0&&e->target<before->enemy_count&&(e->kind==GFX_ATTACK||e->kind==GFX_PROC)&&hp[e->target]>0){
    hp[e->target]-=MAX(0,e->amount-e->absorbed);
    if(hp[e->target]<=0&&r137_at[i]>r1311_kill_at)r1311_kill_at=r137_at[i];
   }
  }
 }
 r137_last_screen=after->screen;''');p.write_text(s)
p=q/'render.c';s=p.read_text().replace('tick-victory_tick<(unsigned)(r137_duration>30?r137_duration+10:40)','game_render_fx_busy(tick)')
s=s.replace('   int fade=r137_duration>30?r137_duration:30;','''   if(r1311_finishing){
    int banner=r1311_banner_at();
    if(age>=(unsigned)banner){
     pix=pixels;sw=stride;dark_rect(0,28,W,146,150);
     r134_center(240,113,"VICTORY",GOLD,&r134_f18b);
     int fade= (int)age-banner-R1311_VICTORY_HOLD;
     if(fade>0)dark_rect(0,28,W,227,MIN(255,fade*255/24));
    }
    return;
   }
   int fade=r137_duration>30?r137_duration:30;''');p.write_text(s)
p=q/'game.h';s=p.read_text().replace('int game_render_fx_busy(unsigned tick);','int game_render_fx_busy(unsigned tick);\nint game_render_music_paused(unsigned tick);');p.write_text(s)
p=q/'psp_start.c';s=p.read_text().replace('static volatile int music_battle=0;','static volatile int music_battle=0;\nstatic volatile int music_paused=0;').replace('        int track=music_battle;','        int track=music_battle,paused=music_paused;').replace('           if(soundtrack_active){','           if(soundtrack_active&&!paused){').replace('           }else audio_last=0;','           }else if(!soundtrack_active)audio_last=0;').replace('int l=soundtrack_active?','int l=soundtrack_active&&!paused?').replace('int r=soundtrack_active?','int r=soundtrack_active&&!paused?')
s=s.replace('        game_render_update_fx(frame*4u);','        music_paused=game_render_music_paused(frame*4u);\n        game_render_update_fx(frame*4u);')
s=s.replace('/* Keep the battle track through the last contact, death and fade. */','/* Keep its stream selected but paused after the killing contact. */');p.write_text(s)
