static int r134_playing(void){return (rift120_played_card>=0||rift120_auto_card>=0)&&animation_tick>=rift120_play_tick&&animation_tick-rift120_play_tick<38;}
static int r134_enemy_x(const Game*g,int i){return g->enemy_count==1?356:(i?409:283);}
static void combat(const Game*g){
 card_combat_context=g;unsigned age=animation_tick>=fx_tick?animation_tick-fx_tick:999;
 int recoil=fx_player_damage>0&&age<14?(14-(int)age)*((age&1)?-1:1)/2:0;
 int lunge=fx_attack&&age<14?(int)(14-age):0;
 sprite_scaled(36+recoil+lunge,81+wave(animation_tick,110,1),160,91,rift_hero,RIFT_HERO_W,RIFT_HERO_H);
 for(int i=0;i<g->enemy_count;i++){
  const Enemy*e=&g->enemies[i];if(e->hp<=0)continue;int ex=r134_enemy_x(g,i),bob=wave(animation_tick+i*24u,96,1);
  if(fx_enemy_damage[i]>0&&age<12)ex+=(12-(int)age)*((age&1)?1:-1)/2;
  if(enemy_windup>0)ex-=((24-enemy_windup)>6&&(24-enemy_windup)<19)?8:0;
  int flip=(e->art==1||e->art==4||e->art==9||e->art==10||e->art==13||e->art==14||e->art==15||e->art==16||e->art==19||e->art==20||e->art==26||e->art==29||e->art==32||e->art==2||e->art==7||e->art==11||e->art==18||e->art==21||e->art==23||e->art==27||e->art==30||e->art==35);
  int dw=g->enemy_count==1?112:94,dh=88;
  sprite_scaled_facing(ex-dw/2,81+bob,dw,dh,rift_enemy_sprites[e->art],RIFT_ENEMY_W,RIFT_ENEMY_H,flip);
  if(i==g->target)r134_blit(ex-44,149,88,25,r134_target,88,25,255);
 }
 /* Painted steel, impact and embers replace every line-based attack effect. */
 if(effect_kind&&animation_tick>=effect_start&&animation_tick-effect_start<20){
  unsigned a=animation_tick-effect_start;int phase=MIN(5,(int)a/3);
  if(effect_kind==1||effect_kind==4){
   int ex=effect_kind==1?r134_enemy_x(g,CLAMP(fx_target,0,g->enemy_count-1)):110;
   r134_blit(ex-48,77,96,96,r134_attack_fx[phase],112,112,255);
  }else if(effect_kind==2){
   r134_blit(92,97,34,40,r134_map_icons[7],36,36,255-(int)a*8);
  }else if(effect_kind==3)r134_blit(91,96-(int)a,36,36,r134_utility_icons[0],40,40,255-(int)a*8);
 }
 if(age<28){char value[24];
  if(fx_player_damage>0){snprintf(value,sizeof(value),"-%d",fx_player_damage);r134_center(108,113-(int)age,value,RED,&r134_f18b);}
  else if(fx_heal>0){snprintf(value,sizeof(value),"+%d",fx_heal);r134_center(108,113-(int)age,value,GREEN,&r134_f18b);}
  else if(fx_block>0){snprintf(value,sizeof(value),"+%d",fx_block);r134_center(108,113-(int)age,value,BLUE,&r134_f18b);}
  for(int i=0;i<g->enemy_count;i++)if(fx_enemy_damage[i]>0){snprintf(value,sizeof(value),"-%d",fx_enemy_damage[i]);r134_center(r134_enemy_x(g,i),111-(int)age,value,age<8?WHITE:RED,&r134_f18b);}
 }
 header(g,"");
 int lx=9;for(int i=2;i>=0;i--)if(rift120_log[i][0]&&lx<450){
  char entry[74];snprintf(entry,sizeof(entry),"%s%s",lx>9?" | ":"",rift120_log[i]);
  while(r134_width(entry,&r134_f9)>470-lx){size_t n=strlen(entry);if(!n)break;entry[n-1]=0;}
  r134_text(lx,19,entry,i==2?WHITE:MUTED,&r134_f9);lx+=r134_width(entry,&r134_f9);
 }
 rift120_hp_meter(52,32,112,g->hp,g->maxhp,g->block);
 char status[96];snprintf(status,sizeof(status),"STR %d  DEX %d",g->strength,g->dexterity);r134_text(57,58,status,MUTED,&r134_f8);
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){
  const Enemy*e=&g->enemies[i];int ex=r134_enemy_x(g,i),w=g->enemy_count==1?112:88;
  rift120_hp_meter(ex-w/2,32,w,e->hp,e->maxhp,e->block);rift116_draw_enemy_intent(g,i,ex);
 }
 /* Opaque native-resolution surface covers every pixel below the battlefield. */
 r134_blit(0,174,480,98,r134_hand_hud,480,98,255);
 r134_blit(4,199,46,46,r134_energy,46,46,255);
 if(g->energy==0)for(int yy=199;yy<245;yy++)for(int xx=4;xx<50;xx++){
  if((r134_energy[(yy-199)*46+xx-4]>>24)<32)continue;uint32_t c=pix[yy*sw+xx];int v=((c&255)*30+((c>>8)&255)*59+((c>>16)&255)*11)/100;pix[yy*sw+xx]=RGB(v,v,v);
 }
 char b[16];snprintf(b,sizeof(b),"%d",g->energy);r134_center(27,212,b,WHITE,&r134_f18b);
 int start=rift133_hand_start(g),playing=r134_playing();
 for(int i=start;i<g->hand_count&&i<start+5;i++){
  int selected=i==g->selection,cy=174,cost=game_card_cost_now(g,g->hand[i]);
  if(hand_exchange&&animation_tick>=hand_exchange_tick){unsigned a=animation_tick-hand_exchange_tick;if(a<10)cy=280;else if(a<23)cy+=(23-(int)a)*6;else hand_exchange=0;}
  if(!selected||playing)card_render(56+(i-start)*71,cy,70,84,g->hand[i],0,cost!=-2&&(cost==-1||cost<=g->energy));
 }
 if(!playing&&g->selection>=start&&g->selection<g->hand_count&&g->selection<start+5){
  int cost=game_card_cost_now(g,g->hand[g->selection]);
  card_render(CLAMP(56+(g->selection-start)*71-28,35,289),88,126,166,g->hand[g->selection],1,cost!=-2&&(cost==-1||cost<=g->energy));
 }
 if(hand_exchange&&animation_tick>=hand_exchange_tick){unsigned a=animation_tick-hand_exchange_tick;if(a<15)for(int i=0;i<old_hand_count&&i<5;i++)card_render(56+i*71,174+(int)a*7,70,84,old_hand[i],0,1);}
 if(start>0)r134_text(40,184,"<",GOLD,&r134_f10);
 if(start+5<g->hand_count)r134_text(418,184,">",GOLD,&r134_f10);
 button(416,215,62,29,"END",g->selection>=g->hand_count);
 card_combat_context=0;footer("X PLAY   O END   SQ DECK   START INVENTORY");
}
