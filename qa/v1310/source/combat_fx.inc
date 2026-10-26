/* Combat presentation timeline: observer-only, bounded memory, integer math. */
#define R137_FX_Y 76
#define R137_FX_H 98
#define R137_MAX_EVENTS 64
static GameCombatFx r137_events[R137_MAX_EVENTS];
static int r137_at[R137_MAX_EVENTS],r137_count,r137_duration,r137_audio_done[R137_MAX_EVENTS];
static unsigned r137_start;
static int r137_player_action;
static int r137_last_screen;
static int r138_death_active,r138_death_start;
static Game r137_before;
static uint32_t r137_held[W*R137_FX_H];
static int r137_held_event=-1,r137_held_valid;
static int r137_hold(int tier){return tier==3?6:tier==2?3:tier==1?1:0;}
static int r137_fire_count(int strength){return CLAMP(strength,0,20)*2;}
static int r137_age(unsigned tick){return tick>=r137_start?(int)(tick-r137_start):999;}
int game_render_fx_busy(unsigned tick){
 int a=r137_age(tick),end=r137_duration;
 if(r137_last_screen==REWARD||r137_last_screen==RELIC_RESULT||r137_last_screen==DEFEAT)end=(end>30?end:30)+10;
 return (r137_count||r138_death_active)&&a<end?end-a:0;
}
static int r137_all(int raw){int c=CARD_ID(raw);return c==3||c==10||c==14||c==19||c==36||c==37||c==70;}
static void r137_feedback(const Game*before,const Game*after,int action,unsigned tick){
 int n=0;const GameCombatFx*events=game_combat_fx_events(&n);
 if(before->screen!=COMBAT){r137_count=0;r137_held_valid=0;r138_death_active=0;return;}
 r138_death_active=after->screen==DEFEAT;r138_death_start=0;
 r137_before=*before;r137_count=MIN(n,R137_MAX_EVENTS);r137_start=tick;r137_duration=0;r137_held_valid=0;r137_held_event=-1;
 r137_player_action=0;
 if(action==BTN_OK&&before->selection>=0&&before->selection<before->hand_count){
  int raw=before->hand[before->selection];r137_player_action=cards[CARD_ID(raw)].type==0?1:cards[CARD_ID(raw)].type==2?3:0;
 }
 for(int i=0;i<r137_count;i++){r137_events[i]=events[i];r137_audio_done[i]=0;}
 /* Debuff-only cards need visible feedback even without a damage trace. */
 for(int j=0;j<after->enemy_count&&r137_count<R137_MAX_EVENTS;j++){
  const Enemy*a=&after->enemies[j],*b=&before->enemies[j];
  int mask=(a->vulnerable>b->vulnerable?1:0)|(a->weak>b->weak?2:0)|(a->poison>b->poison?4:0)|(a->strength>b->strength?8:0);
  if(mask&&a->hp>0){GameCombatFx e={GFX_STATUS,-1,j,mask,0,-1};r137_events[r137_count]=e;r137_audio_done[r137_count++]=0;}
 }
 /* Direct legacy callers can still display a guard / power without a trace. */
 if(!r137_count&&after->block>before->block){GameCombatFx e={GFX_BLOCK,-1,-1,after->block-before->block,0,-1};r137_events[r137_count++]=e;r137_audio_done[0]=0;}
 int attack_count=0;for(int i=0;i<r137_count;i++)if(r137_events[i].kind==GFX_ATTACK)attack_count++;
 int spacing=attack_count>8?2:attack_count>4?4:8;
 int beat=0,seen=0,last_card=-1,last_source=-9,last_kind=0;
 for(int i=0;i<r137_count;i++){
  GameCombatFx*e=&r137_events[i];int tier=game_impact_tier(e->amount);
  if(e->kind==GFX_ATTACK){
   int mask=e->target<0?4:1<<e->target;
   int together=i>0&&last_kind==GFX_ATTACK&&r137_all(e->card)&&e->card==last_card&&e->source==last_source&&!(seen&mask);
   if(i>0&&!together){beat+=spacing;seen=0;}
   seen|=mask;last_card=e->card;last_source=e->source;
  }else if(e->kind==GFX_POWER)beat=0;
  r137_at[i]=MIN(44,beat);
  int end=r137_at[i]+(e->kind==GFX_POWER?30:e->kind==GFX_BLOCK?14:e->kind==GFX_STATUS?20:18+r137_hold(tier));
  if(end>r137_duration)r137_duration=end;
  last_kind=e->kind;
 }
 if(r138_death_active){
  for(int i=0;i<r137_count;i++)if(r137_events[i].target==-1&&r137_events[i].kind!=GFX_BLOCK&&r137_at[i]>r138_death_start)r138_death_start=r137_at[i];
  if(r137_duration<r138_death_start+72)r137_duration=r138_death_start+72;
 }
 if((after->screen==REWARD||after->screen==RELIC_RESULT)&&r137_duration<72)r137_duration=72;
 r137_last_screen=after->screen;
}
void game_render_update_fx(unsigned tick){
 int age=r137_age(tick);
 for(int i=0;i<r137_count;i++)if(!r137_audio_done[i]&&age>=r137_at[i]){
  const GameCombatFx*e=&r137_events[i];r137_audio_done[i]=1;
  /* Every recorded contact dispatches once, including a skipped visual frame. */
  if(e->kind==GFX_ATTACK||e->kind==GFX_PROC)rift_sound_impact(e->amount,e->absorbed>0);
  else if(e->kind==GFX_BLOCK)rift_sound_effect(8);
  else if(e->kind==GFX_POWER||e->kind==GFX_STATUS)rift_sound_effect(9);
  else if(e->kind==GFX_SELF)rift_sound_impact(MIN(10,e->amount),0);
 }
}
/* FX are clipped to the battlefield, preserving all top HUD and hand pixels. */
static int r138_hero_clip;
static void r137_blit(int x,int y,int w,int h,const uint32_t*src,int iw,int ih,int opacity,int flip){
 if(w<=0||h<=0||opacity<=0)return;
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if(dy<(r138_hero_clip?54:R137_FX_Y)||dy>=174)continue;
  for(int xx=0;xx<w;xx++){
   int dx=x+xx;if(dx<22||dx>=W-10)continue;
   int sx=xx*iw/w;if(flip)sx=iw-1-sx;
   uint32_t c=src[(yy*ih/h)*iw+sx];int a=(c>>24)*opacity/255;
   if(a)pix[dy*sw+dx]=r134_mix(pix[dy*sw+dx],c,a);
  }
 }
}
static void r137_fire(const Game*g,int x,unsigned tick,int front){
 int level=CLAMP(g->strength,0,20),count=r137_fire_count(level);
 for(int i=0;i<count;i++){
  unsigned seed=particle_hash((unsigned)i*72817u+137u);
  if((i%3==0)!=front)continue;
  int life=36+(int)(seed%24),age=(int)((tick+(seed>>8))%(unsigned)life);
  int side=(seed&1)?1:-1;
  int px=x+side*(34+(int)((seed>>12)%25))+wave(tick+i*7u,28,4);
  int py=169-age*(35+level*2)/life;
  int size=8+level/2+(int)((seed>>19)%5);
  if(front){py=169-age*(35+level)/life;px=x+side*(25+(int)((seed>>16)%32));size=7+level/2;}
  int opacity=(front?185:235)*(life-age)/life;
  if(age<5)opacity=opacity*age/5;
  int frame=(age/3+i)%6;
  r137_blit(px-size/2,py-size,size,size+size/2,r137_fire_frames[frame],32,32,opacity,seed&2);
 }
}
/* Pose fallback for all 36 enemy silhouettes: anticipatory crouch, lean,
   contact compression and recoil. No new geometry is drawn over the actors. */
static void r137_actor(int x,int y,int w,int h,const uint16_t*src,int iw,int ih,int flip,int lean,int opacity,int flash){
 if(w<1||h<1)return;
 int left=0,top=0,cw=iw,ch=ih;
 for(int i=0;i<36;i++)if(src==rift_enemy_sprites[i]){left=r1310_enemy_bounds[i][0];top=r1310_enemy_bounds[i][1];cw=r1310_enemy_bounds[i][2];ch=r1310_enemy_bounds[i][3];break;}
 for(int yy=0;yy<h;yy++){
  int dy=y+yy;if(dy<R137_FX_Y||dy>=174)continue;
  int shift=lean*(h-yy)/h;
  for(int xx=0;xx<w;xx++){
   int dx=x+xx+shift;if(dx<22||dx>=W-10)continue;
   int sx=xx*cw/w;if(flip)sx=cw-1-sx;
   uint16_t c=src[(top+yy*ch/h)*iw+left+sx];if(c==RIFT_TRANSPARENT_565)continue;
   unsigned r=(c>>11)&31,g=(c>>5)&63,b=c&31;
   uint32_t color=RGB((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
   if(flash)color=r134_mix(color,RGB(255,237,199),flash);
   pix[dy*sw+dx]=opacity==255?color:r134_mix(pix[dy*sw+dx],color,opacity);
  }
 }
}
static int r137_local_time(int age,int tier){int hold=r137_hold(tier);return age<hold?0:age-hold;}
static int r137_latest(int actor,int kind,int age){
 int chosen=-1;
 for(int i=0;i<r137_count;i++){
  const GameCombatFx*e=&r137_events[i];int a=age-r137_at[i];
  if(a<0||a>=30)continue;
  if(kind==GFX_ATTACK?(e->kind==GFX_ATTACK&&e->source==actor):(e->kind==kind&&e->target==actor))chosen=i;
 }
 return chosen;
}
static void r137_scene(const Game*g){
 int age=r137_age(animation_tick),pa=r137_latest(-1,GFX_ATTACK,age),pb=r137_latest(-1,GFX_BLOCK,age),pp=r137_latest(-1,GFX_POWER,age);
 int dx=0,dy=0,lean=0,pw=150,ph=85,pose=0,pose_frame=5;
 int pre=(rift120_played_card>=0&&animation_tick>=rift120_play_tick)?(int)(animation_tick-rift120_play_tick):99;
 if(pa>=0){int a=r137_local_time(age-r137_at[pa],game_impact_tier(r137_events[pa].amount)),tier=game_impact_tier(r137_events[pa].amount);
  if(a<18){dx=0;lean=0;dy=0;pose=1;pose_frame=a<3?2:a<8?3:a<14?4:5;}
 }else if(pp>=0){int a=age-r137_at[pp];dy=-(a<15?a:30-a)/3;pose=3;pose_frame=MIN(5,a/5);lean=-2;}
 else if(pb>=0){int a=age-r137_at[pb];if(a<12){pose=2;pose_frame=2;}}
 else if(pre<18&&r136_play_state_valid){int c=cards[CARD_ID(rift120_played_card)].type;
  int a=pre<9?pre:17-pre;dx=0;dy=0;lean=0;
  pose=r137_prepose;pose_frame=pose==2?2:pre<10?0:1;
 }
 int player_flash=0;
 for(int i=0;i<r137_count;i++)if(r137_events[i].target==-1&&(r137_events[i].kind==GFX_ATTACK||r137_events[i].kind==GFX_SELF)){
  int a=age-r137_at[i],tier=game_impact_tier(r137_events[i].amount);if(a>=0&&a<14+r137_hold(tier)){
   int t=r137_local_time(a,tier);dx-=((14-t)*(1+tier))/14;lean=-2;player_flash=t<2?90:0;
   if(r137_events[i].absorbed>0&&t<12){pose=2;pose_frame=2;dy=3;}
  }
 }
 if(g->hp<=0&&r138_death_active){
  int frame=CLAMP((age-r138_death_start)/9,0,5);
  r138_hero_clip=1;r137_blit(41,58,150,113,r138_death[frame],192,144,255,0);r138_hero_clip=0;
 }else{
  r137_fire(g,118+dx,animation_tick/4,0);
  if(pose){r138_hero_clip=1;if(pose==1)r137_blit(51+dx,58+dy,163,113,r139_attack[pose_frame],208,144,255,0);else r137_blit(53+dx,58+dy,150,113,r137_hero_frames[(pose-1)*6+pose_frame],192,144,255,0);r138_hero_clip=0;}
  else r137_actor(41+dx,87+dy+wave(animation_tick/4,110,1),pw,ph,rift_hero,RIFT_HERO_W,RIFT_HERO_H,0,lean,255,player_flash);
  r137_fire(g,118+dx,animation_tick/4,1);
 }
 for(int e=0;e<g->enemy_count;e++){
  const Enemy*foe=&g->enemies[e];int hit=-1;
  for(int i=0;i<r137_count;i++)if(r137_events[i].target==e&&(r137_events[i].kind==GFX_ATTACK||r137_events[i].kind==GFX_PROC)&&age>=r137_at[i]&&age-r137_at[i]<28)hit=i;
  if(foe->hp<=0&&hit<0)continue;
  int art=foe->art,maxw=g->enemy_count==1?144:112,h=92;
  int w=art==9?144:r1310_enemy_bounds[art][2]*h/r1310_enemy_bounds[art][3];
  if(w>maxw){h=h*maxw/w;w=maxw;}
  int ex=r134_enemy_x(g,e),y=174-h+wave(animation_tick/4+e*24u,96,1),tilt=0,flash=0,opacity=255;
  int atk=r137_latest(e,GFX_ATTACK,age),guard=r137_latest(e,GFX_BLOCK,age);
  if(enemy_windup>0&&game_enemy_intent_hits(g,e)>0){int t=23-enemy_windup;ex+=t/4;y+=t/7;h-=t/7;tilt=3;}
  if(atk>=0){int tier=game_impact_tier(r137_events[atk].amount),t=r137_local_time(age-r137_at[atk],tier);if(t<18){ex-=(20+4*tier)*(18-t)/18;tilt=-(9+tier)*(18-t)/18;w+=6;y+=2;h-=2;}}
  if(guard>=0&&age-r137_at[guard]<18){y+=4;h-=4;w+=4;tilt=3;}
  if(hit>=0){int a=age-r137_at[hit],tier=game_impact_tier(r137_events[hit].amount),t=r137_local_time(a,tier);if(t<18){ex+=(3+tier*3)*(18-t)/18;tilt=5+tier;flash=t<2?90+tier*20:0;}if(foe->hp<=0){opacity=CLAMP(255-a*11,0,255);y+=a/2;}}
  int flip=(art==5||art==24||art==1||art==4||art==9||art==10||art==13||art==14||art==15||art==16||art==19||art==20||art==26||art==29||art==32||art==2||art==7||art==11||art==18||art==21||art==27||art==30||art==35);
  if(art!=9&&atk>=0&&age-r137_at[atk]<9)r137_actor(ex-w/2+9,y,w,h,rift_enemy_sprites[art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip,tilt,45,0);
  if(art==9)r137_blit(ex-w/2,y,w,h,r1310_hound,144,96,opacity,0);
  else r137_actor(ex-w/2,y,w,h,rift_enemy_sprites[art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip,tilt,opacity,flash);
  int living=0;for(int k=0;k<g->enemy_count;k++)living+=g->enemies[k].hp>0;
  if(living>1&&e==g->target&&foe->hp>0)r134_blit(ex-44,149,88,25,r134_target,88,25,255);
 }
 int holding=-1,hold_tier=0;
 for(int i=0;i<r137_count;i++){
  const GameCombatFx*e=&r137_events[i];int a=age-r137_at[i],tier=game_impact_tier(e->amount);
  if(a<0||a>=30)continue;
  /* At most the newest effect of each kind on an actor is drawn. */
  int newer=0;for(int j=i+1;j<r137_count;j++)if(r137_events[j].target==e->target&&r137_events[j].kind==e->kind&&age>=r137_at[j]&&age-r137_at[j]<30)newer=1;
  if(newer)continue;
  int cx=e->target<0?118:r134_enemy_x(g,e->target);
  if(e->kind==GFX_ATTACK||e->kind==GFX_PROC){
   int t=r137_local_time(a,tier),size=54+tier*16,phase=MIN(5,t/3);
   if(t<18){
    if(e->kind==GFX_ATTACK)r137_blit(cx-size/2,124-size/2,size,size,r134_attack_fx[phase],112,112,220,e->target<0);
    r137_blit(cx-size/2+4,124-size/2,size,size,r137_impact_frames[phase],96,96,(18-t)*255/18,e->target<0);
    if(e->absorbed>0)r137_blit(cx-27,98,54,62,r137_shield_frames[MIN(5,3+t/5)],96,96,(18-t)*235/18,0);
   }
   if(a<r137_hold(tier)&&tier>=hold_tier){holding=i;hold_tier=tier;}
   if(t<22){char number[24];int hp=e->amount-e->absorbed;
    snprintf(number,sizeof(number),hp>0?"-%d":"%d",hp>0?hp:e->absorbed);
    r135_shadow_text(cx-r134_width(number,&r134_f18b)/2,105-t/2,number,hp>0?WHITE:BLUE,&r134_f18b);
   }
  }else if(e->kind==GFX_BLOCK&&a<14){
   int phase=3,size=56;
   r137_blit(cx-size/2,112-size/2,size,size+14,r137_shield_frames[phase],96,96,a<8?240:(14-a)*40,0);
   char number[24];snprintf(number,sizeof(number),"+%d",e->amount);r135_shadow_text(cx-r134_width(number,&r134_f18b)/2,104-a/2,number,BLUE,&r134_f18b);
  }else if(e->kind==GFX_STATUS&&a<20){
   int size=60+a; r137_blit(cx-size/2,119-size/2,size,size,r139_status,88,88,(20-a)*12,0);
   const uint32_t*icon=(e->amount&1)?r135_vulnerable:(e->amount&2)?r135_weak:(e->amount&4)?r135_poison:r135_strength;
   r134_blit(cx-12,91-a/2,24,24,icon,22,22,(20-a)*12);
  }else if(e->kind==GFX_POWER&&a<30){
   int phase=MIN(5,a/5),size=65+(a<15?a:30-a);
   r137_blit(118-size/2,120-size/2,size,size,r137_power_frames[phase],96,96,a>23?(30-a)*255/7:240,0);
  }else if(e->kind==GFX_SELF&&a<22){char number[24];snprintf(number,sizeof(number),"-%d",e->amount);r135_shadow_text(98,107-a/2,number,RED,&r134_f18b);}
 }
 /* Brief hold of the actual battlefield framebuffer at contact. HUD, hand,
    audio and input polling are never frozen. Each tier has a finite hold. */
 if(holding>=0){
  if(!r137_held_valid||r137_held_event!=holding){
   for(int y=0;y<R137_FX_H;y++)memcpy(r137_held+y*W,pix+(R137_FX_Y+y)*sw,W*sizeof(uint32_t));
   r137_held_valid=1;r137_held_event=holding;
  }else for(int y=0;y<R137_FX_H;y++)memcpy(pix+(R137_FX_Y+y)*sw,r137_held+y*W,W*sizeof(uint32_t));
 }else r137_held_valid=0;
}
