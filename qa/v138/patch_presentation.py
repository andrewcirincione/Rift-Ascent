from pathlib import Path
q=Path(__file__).parent/'source'
p=q/'psp_start.c';s=p.read_text()
# Presentation clock advances three timeline ticks per rendered frame.
for fn in ['game_render_update_fx','game_render_fx_busy']:
 s=s.replace(fn+'(frame)',fn+'(frame*3u)')
s=s.replace('game_render_preplay(&game,frame);','game_render_preplay(&game,frame*3u);').replace('pending_card_ticks=18;','pending_card_ticks=6;')
s=s.replace('step_action(BTN_OK,frame);','step_action(BTN_OK,frame*3u);').replace('step_action(BTN_BACK,frame);','step_action(BTN_BACK,frame*3u);').replace('step_action(key,frame);','step_action(key,frame*3u);')
s=s.replace('game_render_feedback(&before_input,&game,BTN_POTION,frame);','game_render_feedback(&before_input,&game,BTN_POTION,frame*3u);').replace('game_effect_start(3,frame);','game_effect_start(3,frame*3u);')
s=s.replace('enemy_turn_delay=23;','enemy_turn_delay=8;').replace('game_render_set_enemy_windup(enemy_turn_delay);','game_render_set_enemy_windup(enemy_turn_delay*3);')
for n,v in [(21,7),(20,7),(44,15),(14,5),(10,4)]:s=s.replace(f'action_cooldown={n};',f'action_cooldown={v};')
s=s.replace('game_render_frame(&game,next_frame,STRIDE,frame);','game_render_frame(&game,next_frame,STRIDE,frame*3u);')
s=s.replace('/* Pre-resolution card presentation: game state is deliberately\n           untouched for 18 rendered frames', '/* Pre-resolution card presentation: game state is deliberately\n           untouched for six rendered frames')
s=s.replace('int l=soundtrack_active?audio_last+(int)audio_buffer[i*2]*2/3:', 'int gain=rift_sound_music_gain(&music);\n           int l=soundtrack_active?audio_last*gain/256+(int)audio_buffer[i*2]:')
s=s.replace('int r=soundtrack_active?audio_last+(int)audio_buffer[i*2+1]*2/3:', 'int r=soundtrack_active?audio_last*gain/256+(int)audio_buffer[i*2+1]:')
p.write_text(s)
p=q/'combat_fx.inc';s=p.read_text().replace('  if(age-r137_at[i]>5)continue; /* Do not replay stale sounds after a pause. */','  /* Every recorded contact dispatches once, including a skipped visual frame. */')
s=s.replace('dx-=((14-t)*(2+tier))/4;lean=-7;', 'dx-=((14-t)*(1+tier))/14;lean=-2;')
# Hero pose reaches y=56 at its highest blade tip; effects keep their lower clip.
s=s.replace('static void r137_blit(int x', 'static int r138_hero_clip;\nstatic void r137_blit(int x')
s=s.replace('if(dy<R137_FX_Y||dy>=174)continue;', 'if(dy<(r138_hero_clip?54:R137_FX_Y)||dy>=174)continue;',1)
s=s.replace('if(pose){r137_blit(36+dx,51+dy,160,120,r137_hero_frames[(pose-1)*6+pose_frame],192,144,255,0);}', 'if(pose){r138_hero_clip=1;r137_blit(36+dx,51+dy,160,120,r137_hero_frames[(pose-1)*6+pose_frame],192,144,255,0);r138_hero_clip=0;}')
# Preserve calm idle and fire speed while action timing is tripled.
s=s.replace('r137_fire(g,104+dx,animation_tick,0)','r137_fire(g,104+dx,animation_tick/3,0)').replace('r137_fire(g,104+dx,animation_tick,1)','r137_fire(g,104+dx,animation_tick/3,1)').replace('wave(animation_tick,110,1)','wave(animation_tick/3,110,1)').replace('wave(animation_tick+e*24u,96,1)','wave(animation_tick/3+e*24u,96,1)')
p.write_text(s)
p=q/'sound.c';s=p.read_text().replace('static volatile int impact_requests[8];','static volatile int impact_requests[32];').replace('impact_serial&7u','impact_serial&31u').replace('>8u)s->impact_seen=impact_serial-8u','>32u)s->impact_seen=impact_serial-32u').replace('s->impact_seen++&7u','s->impact_seen++&31u')
s=s.replace('int amp=tier==3?5600:tier==2?3900:2300;', 'int amp=tier==3?10000:tier==2?8000:6200;')
s=s.replace('   sfx+=hit;\n   unsigned base=', '''   /* Dry, bit-crushed crack above the bass body: audible on PSP speakers. */
   if(a<1800){int grit=(noise/4096)*4096;hit+=(grit*((1800-a)*env/1800)/4096)*(amp/2)/32768;}
   sfx+=hit;
   unsigned base=''')
s+='''\nint rift_sound_music_gain(const RiftSound*s){
 int gain=256;
 for(int i=0;i<4;i++)if(s->impact_tier[i]&&s->impact_age[i]<4410u){int g=150+(int)s->impact_age[i]*106/4410;if(g<gain)gain=g;}
 return gain;
}
'''
p.write_text(s)
p=q/'sound.h';s=p.read_text().replace('void rift_sound_reset', 'int rift_sound_music_gain(const RiftSound *s);\nvoid rift_sound_reset');p.write_text(s)
