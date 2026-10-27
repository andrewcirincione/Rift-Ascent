#include <math.h>
static void fx_write_frame(Game*g,int tick,const char*name){
 static uint32_t pixels[W*H];char path[256];game_render_frame(g,pixels,W,(unsigned)tick);
 snprintf(path,sizeof(path),"qa/v150/evidence/%s.ppm",name);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n480 272\n255\n");
 for(int i=0;i<W*H;i++){fputc(pixels[i]&255,f);fputc((pixels[i]>>8)&255,f);fputc((pixels[i]>>16)&255,f);}fclose(f);
}
static void fx_fixture(Game*g,int raw,int strength){
 math_battle(g,raw);g->hp=g->maxhp=80;g->energy=9;g->strength=strength;
 g->hand_count=5;g->hand[0]=raw;g->hand[1]=1;g->hand[2]=8;g->hand[3]=7;g->hand[4]=25;
 g->enemies[0].hp=g->enemies[0].maxhp=180;g->enemies[1].hp=g->enemies[1].maxhp=180;
 g->enemies[0].art=16;g->enemies[1].art=19;
 g->enemies[0].intent=0;g->enemies[0].intent_value=16;g->enemies[1].intent=1;g->enemies[1].intent_value=12;
 r138_death_active=0;r137_count=0;hand_exchange=0;fx_transition=0;victory_replay=0;effect_kind=0;rift120_played_card=rift120_auto_card=-1;overlay_kind=0;
}
static void animation_tests(void){
 Game g,a,b;int n;const GameCombatFx*e;char name[100];
 assert(game_impact_tier(0)==0&&game_impact_tier(1)==1&&game_impact_tier(10)==1&&game_impact_tier(11)==2&&game_impact_tier(20)==2&&game_impact_tier(21)==3&&game_impact_tier(999)==3);
 assert(r137_fire_count(-3)==0&&r137_fire_count(0)==0&&r137_fire_count(1)==2&&r137_fire_count(20)==40&&r137_fire_count(99)==40);
 for(int i=1;i<=20;i++)assert(r137_fire_count(i)>r137_fire_count(i-1));
 /* Event recording must not change any game field or live RNG. */
 int states=0;
 for(int up=0;up<2;up++)for(int id=0;id<CARD_COUNT;id++)for(int scenario=0;scenario<6;scenario++){
  math_battle(&g,id|(up?UPBIT:0));g.strength=scenario*4;g.weak=scenario%2;g.dexterity=scenario-2;g.ic[IC_NEXT_ATTACK_REPEAT]=scenario%2;g.ic[IC_DARK_EMBRACE]=1;g.ic[IC_HELLRAISER]=1;
  a=b=g;game_combat_fx_end();game_play_card(&a,0,0);
  game_combat_fx_begin();game_play_card(&b,0,0);game_combat_fx_end();assert(!memcmp(&a,&b,sizeof(a)));
  e=game_combat_fx_events(&n);assert(n<=MAX_COMBAT_FX);for(int i=0;i<n;i++)assert(e[i].target>=-1&&e[i].target<MAX_ENEMIES&&e[i].kind>=1&&e[i].kind<=5);
  GameCombatFx saved[MAX_COMBAT_FX];memcpy(saved,e,sizeof(saved));int count=n;char rules[320];game_card_description_now(&g,g.hand[0],0,rules,sizeof(rules));e=game_combat_fx_events(&n);assert(count==n&&!memcmp(saved,e,sizeof(saved)));states++;
 }
 fx_fixture(&g,27,5);game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();e=game_combat_fx_events(&n);assert(n==2&&e[0].amount==10&&e[1].amount==10&&e[0].source==-1);
 fx_fixture(&g,0,20);g.enemies[0].block=100;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();e=game_combat_fx_events(&n);assert(n==1&&e[0].amount==26&&e[0].absorbed==26&&game_impact_tier(e[0].amount)==3);
 fx_fixture(&g,1,0);game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();e=game_combat_fx_events(&n);assert(n==1&&e[0].kind==GFX_BLOCK&&e[0].target==-1&&e[0].amount==5);
 fx_fixture(&g,8,0);game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();e=game_combat_fx_events(&n);assert(n==1&&e[0].kind==GFX_POWER&&g.strength==2);
 fx_fixture(&g,0,0);g.block=40;game_combat_fx_begin();game_end_turn(&g);game_combat_fx_end();e=game_combat_fx_events(&n);assert(n>=2);int enemy=0,guard=0;for(int i=0;i<n;i++){enemy+=e[i].kind==GFX_ATTACK&&e[i].source==0&&e[i].absorbed==16;guard+=e[i].kind==GFX_BLOCK&&e[i].target==1;}assert(enemy&&guard);
 /* The framebuffer hold is exact, bounded, and does not alter HUD/hand. */
 static uint32_t px[W*H],held[W*98];fx_fixture(&g,0,20);a=g;game_render_preplay(&a,982);game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&a,&g,BTN_OK,1000);
 assert(game_render_fx_busy(1000)>0);pix=px;sw=W;animation_tick=1000;memset(px,0x14,sizeof(px));r137_scene(&g);memcpy(held,px+76*W,sizeof(held));
 animation_tick=1005;memset(px,0x27,sizeof(px));r137_scene(&g);assert(!memcmp(held,px+76*W,sizeof(held)));assert(px[0]==0x27272727&&px[180*W]==0x27272727);assert(!game_render_fx_busy(1100));
 /* Final blows retain real HP and finish their clip before reward/defeat. */
 for(int mode=0;mode<3;mode++){
  fx_fixture(&g,7,20);g.enemy_count=1;g.enemies[0].hp=10;
  if(mode==1)g.battle_type=NODE_BOSS;
  if(mode==2){g.hp=1;g.block=0;g.enemies[0].hp=180;}
  snprintf(name,sizeof(name),"qa/v150/evidence/state-%s.dat",mode==0?"finisher":mode==1?"boss-finisher":"defeat");assert(game_save(&g,name));a=g;
  game_render_preplay(&g,1982);game_combat_fx_begin();if(mode==2)game_end_turn(&g);else game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&a,&g,mode==2?BTN_BACK:BTN_OK,2000);
  assert(g.screen==(mode==0?REWARD:mode==1?RELIC_RESULT:DEFEAT));assert(victory_replay);
  assert(victory_scene.hp==g.hp&&victory_scene.enemies[0].hp==g.enemies[0].hp);
  int finish=r1311_finishing?r1311_finish_end():(r137_duration>30?r137_duration:30)+10;assert(game_render_fx_busy(2000+finish-1)==1);assert(!game_render_fx_busy(2000+finish));
  for(int frame=0;frame<finish+2;frame++){snprintf(name,sizeof(name),"%s-%02d",mode==0?"finisher":mode==1?"boss-finisher":"defeat",frame);fx_write_frame(&g,2000+frame,name);}
 }
 puts("PASS normal/boss finishing blows and lethal enemy hit preserve final HP and complete before screen transitions");
 /* Identical effect-only fire pixels above the cap; no particles at zero. */
 r137_count=0;pix=px;sw=W;g.strength=20;memset(px,0,sizeof(px));r137_fire(&g,108,400,0);r137_fire(&g,108,400,1);memcpy(held,px+76*W,sizeof(held));
 g.strength=75;memset(px,0,sizeof(px));r137_fire(&g,108,400,0);r137_fire(&g,108,400,1);assert(!memcmp(held,px+76*W,sizeof(held)));
 g.strength=0;memset(px,0,sizeof(px));r137_fire(&g,108,400,0);r137_fire(&g,108,400,1);for(int i=0;i<W*H;i++)assert(px[i]==0);
 /* Actual new PCM has increasing energy and longer bass tails by tier. */
 double previous=0;static int16_t pcm[20000*2];
 for(int tier=1;tier<=3;tier++){
  RiftSound sound;rift_sound_reset(&sound);rift_sound_impact(tier==1?10:tier==2?20:21,0);rift_audio_generate(&sound,pcm,20000,0,0);
  double energy=0;int last=0,peak=0;for(int i=0;i<20000;i++){int v=abs(pcm[i*2]);energy+=(double)v*v;if(v)last=i;if(v>peak)peak=v;assert(pcm[i*2]==pcm[i*2+1]);}
  assert(energy>previous&&peak<30000);previous=energy;assert(last>(tier==1?5000:tier==2?8000:12000));
  snprintf(name,sizeof(name),"qa/v150/evidence/impact-%d.s16le",tier);FILE*f=fopen(name,"wb");assert(f);fwrite(pcm,sizeof(pcm),1,f);fclose(f);
  printf("PASS impact tier %d: energy %.0f, peak %d, tail sample %d\n",tier,energy,peak,last);
 }
 RiftSound sound;rift_sound_reset(&sound);rift_sound_impact(26,1);rift_sound_effect(9);rift_sound_effect(6);rift_audio_generate(&sound,pcm,12000,0,0);assert(sound.impact_age[0]==12000&&sound.power_age==12000&&sound.block_age==6500);
 printf("PASS: %d observer-invariance cases; tier boundaries, blocked/multi hits, player Power, enemy guard, exact hitstop, HUD isolation, Strength cap, overlapping independent audio\n",states);
 /* Real render sequences and emulator fixtures. */
 int cards_to_test[]={0,0,7,1,8,27};int strengths[]={0,10,20,0,20,5};const char*labels[]={"low","medium","heavy","block","power","multi"};
 for(int c=0;c<6;c++){
  fx_fixture(&g,cards_to_test[c],strengths[c]);snprintf(name,sizeof(name),"qa/v150/evidence/state-%s.dat",labels[c]);assert(game_save(&g,name));a=g;
  game_render_preplay(&g,1000);
  for(int frame=0;frame<18;frame++){snprintf(name,sizeof(name),"%s-%02d",labels[c],frame);fx_write_frame(&g,1000+frame,name);}
  game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&a,&g,BTN_OK,1018);
  for(int frame=18;frame<62;frame++){game_render_update_fx(1000+frame);snprintf(name,sizeof(name),"%s-%02d",labels[c],frame);fx_write_frame(&g,1000+frame,name);}
 }
 fx_fixture(&g,0,20);g.selection=g.hand_count;assert(game_save(&g,"qa/v150/evidence/state-fire20.dat"));
 for(int strength=0;strength<=20;strength+=5){g.strength=strength;for(int frame=0;frame<40;frame++){snprintf(name,sizeof(name),"fire%d-%02d",strength,frame);fx_write_frame(&g,1500+frame,name);}}
 fx_fixture(&g,0,5);g.block=22;assert(game_save(&g,"qa/v150/evidence/state-enemy-turn.dat"));a=g;game_combat_fx_begin();game_end_turn(&g);game_combat_fx_end();game_render_feedback(&a,&g,BTN_BACK,1800);
 for(int frame=0;frame<50;frame++){snprintf(name,sizeof(name),"enemy-%02d",frame);fx_write_frame(&g,1800+frame,name);}
 game_combat_fx_begin();game_combat_fx_end();r137_count=0;
}

/* Regressions specific to the faster 1.3.8 presentation. */
static void patch138_tests(void){
 Game g,before;RiftSound sound;int16_t pcm[1024];int checked=0;
 for(int id=0;id<CARD_COUNT;id++)if(cards[id].type==0){
  fx_fixture(&g,id,4);before=g;rift_sound_reset(&sound);unsigned serial=sound.impact_seen;
  game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&before,&g,BTN_OK,3000);
  int expected=0;for(int i=0;i<r137_count;i++)expected+=(r137_events[i].kind==GFX_ATTACK||r137_events[i].kind==GFX_PROC||r137_events[i].kind==GFX_SELF)&&r137_events[i].amount>0;
  /* Simulate missing multiple render frames: contact sound must not disappear. */
  game_render_update_fx(3080);rift_audio_generate(&sound,pcm,512,0,0);assert(sound.impact_seen-serial==(unsigned)expected);
  unsigned voice=sound.impact_voice;game_render_update_fx(3083);rift_audio_generate(&sound,pcm,512,0,0);assert(sound.impact_voice==voice);
  if(expected){assert(rift_sound_music_gain(&sound)<256);checked++;}
 }
 for(int type=0;type<3;type++){
  fx_fixture(&g,type==2?8:type,0);before=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&before,&g,BTN_OK,6000);
  int frames=0;while(game_render_fx_busy(6000+frames*4))frames++;assert(frames==(r137_duration+3)/4&&frames<=10);
 }
 /* A lethal state without an attack trace still holds the complete collapse. */
 fx_fixture(&g,0,0);before=g;g.hp=0;g.screen=DEFEAT;game_combat_fx_begin();game_combat_fx_end();game_render_feedback(&before,&g,BTN_BACK,7000);
 assert(r138_death_active&&r137_duration==72&&victory_replay);
 assert(game_render_fx_busy(7081)==1&&!game_render_fx_busy(7082));
 /* The raised sword occupies the newly available space, never the HP strip. */
 static uint32_t pixels[W*H];pix=pixels;sw=W;memset(pixels,0,sizeof(pixels));r138_hero_clip=1;
 r137_blit(36,51,160,120,r139_attack[1],208,144,255,0);r138_hero_clip=0;
 int blade=0;for(int y=0;y<54;y++)for(int x=0;x<W;x++)assert(!pixels[y*W+x]);
 for(int y=54;y<76;y++)for(int x=22;x<196;x++)blade+=pixels[y*W+x]!=0;assert(blade>0);
 r138_death_active=r137_count=victory_replay=0;
 printf("PASS 1.3.8: %d attack cards dispatch all contact audio once after skipped frames; music ducking; 4x combat timing; no-trace death hold; complete raised sword below HP\n",checked);
}
