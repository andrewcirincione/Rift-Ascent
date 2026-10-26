from pathlib import Path
q=Path(__file__).parent/'source';p=q/'psp_start.c';s=p.read_text().replace('frame*3u','frame*4u').replace('pending_card_ticks=7;','pending_card_ticks=5;').replace('enemy_turn_delay=8;','enemy_turn_delay=6;').replace('enemy_turn_delay*3','enemy_turn_delay*4').replace('action_cooldown=7;','action_cooldown=4;').replace('sceAudioOutputBlocking(channel,0x4000','sceAudioOutputBlocking(channel,0x8000').replace('before_input.hand_count!=game.hand_count||fx || events ||','before_input.hand_count!=game.hand_count||fx || events ||\n       (oldscreen==COMBAT&&key==BTN_OK&&before_input.turn_played!=game.turn_played)||');p.write_text(s)
p=q/'sound.c';s=p.read_text().replace('tier==3?10000:tier==2?8000:6200','tier==3?15000:tier==2?12000:10000').replace('int gain=256;','int gain=160;').replace('int g=150+(int)s->impact_age[i]*106/4410','int g=80+(int)s->impact_age[i]*80/4410');p.write_text(s)
p=q/'combat_fx.inc';s=p.read_text();marker=' /* Direct legacy callers'
pos=s.index(marker);s=s[:pos]+''' /* Debuff-only cards need visible feedback even without a damage trace. */
 for(int j=0;j<after->enemy_count&&r137_count<R137_MAX_EVENTS;j++){
  const Enemy*a=&after->enemies[j],*b=&before->enemies[j];
  int mask=(a->vulnerable>b->vulnerable?1:0)|(a->weak>b->weak?2:0)|(a->poison>b->poison?4:0)|(a->strength>b->strength?8:0);
  if(mask&&a->hp>0){GameCombatFx e={GFX_STATUS,-1,j,mask,0,-1};r137_events[r137_count]=e;r137_audio_done[r137_count++]=0;}
 }
'''+s[pos:]
s=s.replace('e->kind==GFX_BLOCK?24:22+r137_hold(tier)','e->kind==GFX_BLOCK?14:e->kind==GFX_STATUS?20:18+r137_hold(tier)')
s=s.replace('r138_death_start+54','r138_death_start+72').replace(' r137_last_screen=after->screen;',' if((after->screen==REWARD||after->screen==RELIC_RESULT)&&r137_duration<72)r137_duration=72;\n r137_last_screen=after->screen;')
s=s.replace('else if(e->kind==GFX_POWER)rift_sound_effect(9);','else if(e->kind==GFX_POWER||e->kind==GFX_STATUS)rift_sound_effect(9);')
s=s.replace('dx=(10+tier*3)*(18-a)/18;lean=(7+tier*2)*(18-a)/18;pw+=8*(18-a)/18;dy=2*(18-a)/18;ph-=dy;','dx=0;lean=0;dy=0;')
s=s.replace('if(a<20){dy=4;ph-=4;pw+=4;lean=-5;pose=2;pose_frame=MIN(5,a/4);}','if(a<12){pose=2;pose_frame=2;}')
s=s.replace('dx=c==0?-a/2:0;dy=c==0?a/3:c==1?2:0;ph-=dy;lean=c==0?-a:0;','dx=0;dy=0;lean=0;').replace('pose_frame=pre<10?0:1;','pose_frame=pose==2?2:pre<10?0:1;')
s=s.replace('(age-r138_death_start)/7','(age-r138_death_start)/9').replace('animation_tick/3','animation_tick/4')
s=s.replace('r137_blit(36+dx,51+dy,160,120,r137_hero_frames[(pose-1)*6+pose_frame],192,144,255,0);','if(pose==1)r137_blit(29+dx,51+dy,173,120,r139_attack[pose_frame],208,144,255,0);else r137_blit(36+dx,51+dy,160,120,r137_hero_frames[(pose-1)*6+pose_frame],192,144,255,0);')
s=s.replace('  r137_actor(ex-w/2,y,w,h,rift_enemy_sprites[art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip,tilt,opacity,flash);','  if(art==9)r137_blit(ex-w/2,y,w,h,r139_hound,132,104,opacity,0);\n  else r137_actor(ex-w/2,y,w,h,rift_enemy_sprites[art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip,tilt,opacity,flash);')
s=s.replace('  if(e==g->target&&foe->hp>0)','  int living=0;for(int k=0;k<g->enemy_count;k++)living+=g->enemies[k].hp>0;\n  if(living>1&&e==g->target&&foe->hp>0)')
s=s.replace('}else if(e->kind==GFX_BLOCK&&a<24){\n   int phase=MIN(5,a/4),size=54+(a<8?a:8)/2;','}else if(e->kind==GFX_BLOCK&&a<14){\n   int phase=3,size=56;').replace('r137_shield_frames[phase],96,96,255,0);','r137_shield_frames[phase],96,96,a<8?240:(14-a)*40,0);')
s=s.replace('}else if(e->kind==GFX_POWER&&a<30){','}else if(e->kind==GFX_STATUS&&a<20){\n   int size=60+a; r137_blit(cx-size/2,119-size/2,size,size,r139_status,88,88,(20-a)*12,0);\n   const uint32_t*icon=(e->amount&1)?r135_vulnerable:(e->amount&2)?r135_weak:(e->amount&4)?r135_poison:r135_strength;\n   r134_blit(cx-12,91-a/2,24,24,icon,22,22,(20-a)*12);\n  }else if(e->kind==GFX_POWER&&a<30){')
p.write_text(s)
