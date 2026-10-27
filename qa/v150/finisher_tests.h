/* Follow-up 1.3.11 regressions: real combat transitions and independently measured feet. */
static void followup1311_tests(void){
 Game g,b;int cases=0;
 for(int mode=0;mode<4;mode++)for(int multi=0;multi<3;multi++){
  fx_fixture(&g,multi==0?0:multi==1?27:3,0);g.enemy_count=multi==2?2:1;
  g.battle_type=mode==0?NODE_FIGHT:mode==1?NODE_ELITE:NODE_BOSS;g.act=3;g.endless=mode==3;
  for(int j=0;j<g.enemy_count;j++){g.enemies[j].block=0;g.enemies[j].hp=multi==0?1:multi==1?9:7;}
  b=g;RiftSound sound;int16_t pcm[1024];rift_sound_reset(&sound);unsigned serial=sound.impact_seen;
  game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,1000);
  assert(g.screen==(mode==0?REWARD:mode==2?VICTORY:RELIC_RESULT));assert(r1311_finishing&&victory_replay);
  int end=r1311_finish_end(),kill=r1311_kill_at;assert(kill==(multi==1?8:0));assert(end-kill==172);
  assert(!game_render_music_paused(999));
  for(int t=0;t<end+2;t++){
   assert(game_render_music_paused(1000+t)==(t>=kill&&t<end));
   assert(!!game_render_fx_busy(1000+t)==(t<end));
   if(t>=kill&&t<kill+112)assert(r137_age(1000+t)==kill+(t-kill)/4);
   game_render_update_fx(1000+t);
  }
  rift_audio_generate(&sound,pcm,512,0,0);int expected=0;
  for(int i=0;i<r137_count;i++)expected+=(r137_events[i].kind==GFX_ATTACK||r137_events[i].kind==GFX_PROC)&&r137_events[i].amount>0;
  assert(sound.impact_seen-serial==(unsigned)expected);
  char name[80];snprintf(name,sizeof(name),"followup-victory-%d-%d",mode,multi);fx_write_frame(&g,1000+r1311_banner_at()+20,name);cases++;
 }
 fx_fixture(&g,27,0);b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,2000);
 assert(!r1311_finishing);for(int t=0;t<100;t++){assert(!game_render_music_paused(2000+t));assert(r137_age(2000+t)==t);}
 for(int frame=0;frame<6;frame++){
  int lo=999,hi=-1,bottom=0;
  for(int y=134;y<144;y++)for(int x=0;x<192;x++){
   uint32_t c=r137_hero_frames[12+frame][y*192+x];int r=c&255,green=c>>8&255;
   if((c>>24)>128&&!(r>40&&r*10>green*16)){if(x<lo)lo=x;if(x>hi)hi=x;if(y>bottom)bottom=y;}
  }
  double center=r1311_power_x[frame]+(lo+hi)*75.0/192.0;
  double feet=r1311_power_y[frame]+bottom*113.0/144.0;
  assert(fabs(center-118)<1.0&&fabs(feet-171)<1.0);
 }
 fx_fixture(&g,8,0);b=g;game_render_preplay(&g,2982);game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,3000);
 for(int f=0;f<6;f++){char name[80];snprintf(name,sizeof(name),"followup-power-%d",f);fx_write_frame(&g,3000+f*5,name);}
 r1311_finishing=r137_count=victory_replay=0;
 printf("PASS 1.3.11 follow-up: %d normal/elite/final/endless single/multi/AoE finishers; exact impact mute, quarter-speed, victory hold, once-only SFX, normal-hit timing; six Power anchors within one pixel\n",cases);
}
