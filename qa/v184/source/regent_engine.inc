/* Regent 1.6. All resource payments occur exactly once at the hand boundary;
 * replay/autoplay tasks are free, retain their original X, and serialize choices. */
enum {RT_PLAY=1,RT_EFFECT,RT_FINISH,RT_DRAW,RT_CHOOSE,RT_ENDTURN,RT_AUTOPLAY,RT_RETURN,RT_SEEK_THREE};
enum {RC_TOP=1,RC_DISCARD_TOP,RC_TRANSFORM_HAND,RC_TRANSFORM_DRAW,RC_GUARDS,RC_SKILL_TRIPLE,RC_COPY_COLORLESS,RC_GENERATE,RC_SEEK,RC_EXHAUST,RC_ENTROPY,RC_SEEK_SKILL,RC_SEEK_ATTACK,RC_STRATAGEM,RC_SEEK_THREE};
#define RP(g,id) ((g)->regent.powers[(id)-REGENT_FIRST])
static int regent_flags(int raw){int id=CARD_ID(raw);if(id<REGENT_FIRST||id>=EXTRA_LAST)return 0;const RegentCardDef*d=&regent_cards[id-REGENT_FIRST];return CARD_UP(raw)?d->upflags:d->flags;}
static int regent_stamp(Game*g,int raw){
 if(((g->character!=CHAR_REGENT&&!g->revision184.custom)&&CARD_ID(raw)<REGENT_FIRST)||CARD_INSTANCE(raw))return raw;
 for(int i=0;i<511;i++){int k=g->ninja.next_instance%511+1;g->ninja.next_instance=k;if(ninja_instance_used(g,k))continue;int used=0;for(int t=0;t<g->regent.task_count;t++)if(CARD_INSTANCE(g->regent.tasks[t].raw)==k)used=1;for(int t=0;t<g->regent.return_count;t++)if(CARD_INSTANCE(g->regent.return_cards[t])==k)used=1;if(used)continue;g->regent.instance_cost[k]=g->regent.instance_damage[k]=g->regent.instance_replay[k]=0;return raw|(k<<CARD_INSTANCE_SHIFT);}return raw;
}
static int regent_push(Game*g,int kind,int raw,int target,int n,int aux,int depth){if(g->regent.task_count>=REGENT_TASK_MAX){msg(g,"Effect queue full.");return 0;}g->regent.tasks[g->regent.task_count++]=(RegentTask){kind,raw,target,n,aux,depth};return 1;}
static int regent_clean(int raw){return raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL|CARD_RETAIN_TURN);}
static int regent_cost_now(const Game*g,int raw){
 if(RP(g,R_VOID_FORM)&&g->regent.void_used<2*RP(g,R_VOID_FORM))return 0;
 int n=game_card_cost(raw);if(CARD_ID(raw)==R_KINGLY_KICK)n=MAX(0,n-g->regent.instance_cost[CARD_INSTANCE(raw)]);return n;
}
int game_card_star_cost(const Game*g,int raw){int id=CARD_ID(raw);if(id<REGENT_FIRST||id>=EXTRA_LAST)return 0;if((raw&CARD_FREE)||(g&&RP(g,R_VOID_FORM)&&g->regent.void_used<2*RP(g,R_VOID_FORM)))return 0;const RegentCardDef*d=&regent_cards[id-REGENT_FIRST];return CARD_UP(raw)?d->upstars:d->stars;}
static int card_grants_stars(int raw){
 switch(CARD_ID(raw)){
 case R_BIG_BANG:case R_CONVERGENCE:case R_GATHER_LIGHT:case R_GENESIS:
 case R_GLOW:case R_HIDDEN_CACHE:case R_KNOCKOUT_BLOW:case R_ROYAL_GAMBLE:
 case R_SHINING_STRIKE:case R_SOLAR_STRIKE:case R_THE_SEALED_THRONE:case R_VENERATE:
 case 374:case 375:case 377:case 379:case 381:case 384:case 385:case 386:case 391:case 393:return 1;
 default:return 0;
 }
}
int game_shows_stars(const Game*g){
 if(!g)return 0;if(g->character==CHAR_REGENT)return 1;
 /* A generator still belongs to the combat after drawing, discarding or
    exhausting it. Active/delayed generator cards also keep their resource visible. */
 if(RP(g,R_GENESIS)||RP(g,R_THE_SEALED_THRONE)||g->regent.next_stars)return 1;
 const int*piles[]={g->deck,g->hand,g->draw,g->discard,g->exhaust};
 const int counts[]={g->deck_count,g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};
 for(int p=0;p<5;p++)for(int i=0;i<counts[p];i++)if(card_grants_stars(piles[p][i]))return 1;
 return 0;
}
static void regent_gain_stars(Game*g,int n){if(n<=0)return;n=MIN(n,999-g->regent.stars);if(!n)return;g->regent.stars+=n;record_combat_fx(GFX_STARS,-1,-1,n,0);g->regent.stars_gained=MIN(1000000,g->regent.stars_gained+n);if(RP(g,R_BLACK_HOLE))ninja_all_proc(g,RP(g,R_BLACK_HOLE));}
static void regent_created(Game*g){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom))return;g->regent.created=MIN(1000000,g->regent.created+1);
 g->strength=MIN(1000000,g->strength+RP(g,R_ARSENAL));if(RP(g,R_PILLAR_OF_CREATION))relic_block(g,RP(g,R_PILLAR_OF_CREATION));
}
static void regent_on_draw(Game*g,int raw){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom))return;int id=CARD_ID(raw),k=CARD_INSTANCE(raw);
 if(id==R_KINGLY_KICK&&k)g->regent.instance_cost[k]=MIN(99,g->regent.instance_cost[k]+1);
 if(id==R_KINGLY_PUNCH&&k)g->regent.instance_damage[k]=MIN(1000000,g->regent.instance_damage[k]+4+2*CARD_UP(raw));
 g->regent.draws=(g->regent.draws+1)%10;gain_energy(g,g->regent.automation_phase[g->regent.draws]);
}
static void regent_on_hit(Game*g,int e,int amount){
 (void)amount;if(!r140_attack_context||(g->character!=CHAR_REGENT&&!g->revision184.custom))return;g->regent.hit_count[e]=MIN(1000000,g->regent.hit_count[e]+1);
 int n=RP(g,R_MONARCHS_GAZE);if(n&&g->enemies[e].hp>0){g->enemies[e].strength-=n;g->ic[IC_MANGLE0+e]+=n;record_combat_fx(GFX_STATUS,-1,e,1,0);}
}
static void regent_temp_weak(Game*g,int e,int n){if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0)return;g->enemies[e].strength-=n;g->ic[IC_MANGLE0+e]+=n;record_combat_fx(GFX_STATUS,-1,e,1,0);}
static int regent_base_damage(const Game*g,int raw){
 int id=CARD_ID(raw),up=CARD_UP(raw);if(id<REGENT_FIRST||id>=EXTRA_LAST)return 0;const RegentCardDef*d=&regent_cards[id-REGENT_FIRST];int n=up?d->updamage:d->damage;
 if(id==R_SOVEREIGN_BLADE)n=10+g->regent.forge;
 if(id==R_KINGLY_PUNCH)n+=g->regent.instance_damage[CARD_INSTANCE(raw)];
 if(id==R_LUNAR_BLAST)n=(4+up)*g->regent.skills;
 if(id==R_RADIATE)n=(3+up)*g->regent.stars_gained;
 if(id==R_SUPERMASSIVE)n=5+(3+up)*g->regent.created;
 if(id==R_CRESCENT_SPEAR){int *piles[4]={(int*)g->hand,(int*)g->draw,(int*)g->discard,(int*)g->exhaust};int counts[4]={g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};int count=0;for(int p=0;p<4;p++)for(int i=0;i<counts[p];i++){int c=CARD_ID(piles[p][i]);if(c>=REGENT_FIRST&&c<EXTRA_LAST&&(CARD_UP(piles[p][i])?regent_cards[c-REGENT_FIRST].upstars:regent_cards[c-REGENT_FIRST].stars)!=0)count++;}if(r140_attack_context&&CARD_ID(r150_card_raw)==id&&!(r150_card_raw&CARD_EPHEMERAL))count++;n=8+(2+up)*count;}
 if(id==C_GOLD_AXE)n=g->regent.plays_combat;
 if(id==C_MIND_BLAST)n=g->draw_count;
 return MIN(100000000,n);
}
static int regent_colorless(Game*g,int up){int id=COLORLESS_FIRST+rnd(g,COLORLESS_COUNT);return id|(up?UPBIT:0);}
static void regent_create(Game*g,int raw){add_generated(g,raw&~CARD_INSTANCE_MASK);}
static void regent_forge(Game*g,int n){
 if(n<=0)return;int factor=1+MIN(20,RP(g,R_HAMMER_TIME));g->regent.forge=MIN(1000000,g->regent.forge+n*factor);
 if(!g->regent.blade_created){g->regent.blade_created=1;regent_create(g,R_SOVEREIGN_BLADE);}
}
static int regent_take(int*p,int*n,int ix){int raw=p[ix];for(int i=ix;i+1<*n;i++)p[i]=p[i+1];--*n;return raw;}
static void regent_to_hand(Game*g,int raw){if(g->hand_count<MAX_HAND)g->hand[g->hand_count++]=raw;else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=regent_clean(raw);}
static void regent_recall(Game*g,int id){
 int *piles[]={g->draw,g->discard,g->exhaust};int*count[]={&g->draw_count,&g->discard_count,&g->exhaust_count};
 for(int p=0;p<3;p++)for(int i=*count[p]-1;i>=0;i--)if(CARD_ID(piles[p][i])==id){regent_to_hand(g,regent_take(piles[p],count[p],i));return;}
}
static int regent_choice_pile(const Game*g){switch(g->regent.choice_kind){case RC_DISCARD_TOP:return 2;case RC_TRANSFORM_DRAW:case RC_SEEK:case RC_SEEK_SKILL:case RC_SEEK_ATTACK:case RC_STRATAGEM:return 1;case RC_GENERATE:case RC_SEEK_THREE:return 3;default:return 0;}}
int game_regent_choice_count(const Game*g){int p=regent_choice_pile(g);return p==0?g->hand_count:p==1?g->draw_count:p==2?g->discard_count:g->regent.option_count;}
int game_regent_choice_raw(const Game*g,int i){if(i<0||i>=game_regent_choice_count(g))return -1;int p=regent_choice_pile(g);return p==0?g->hand[i]:p==1?g->draw[i]:p==2?g->discard[i]:g->regent.options[i];}
int game_regent_choice_eligible(const Game*g,int i){int raw=game_regent_choice_raw(g,i);if(raw<0)return 0;int id=CARD_ID(raw),k=g->regent.choice_kind;
 if(k==RC_SKILL_TRIPLE||k==RC_SEEK_SKILL)return cards[id].type==1;
 if(k==RC_SEEK_ATTACK)return cards[id].type==0;
 if(k==RC_COPY_COLORLESS)return (id>=COLORLESS_FIRST&&id<EXTRA_LAST)||id==R_MINION_STRIKE||id==R_MINION_DIVE_BOMB||id==R_MINION_SACRIFICE||id==R_DEBRIS;
 if(k==RC_GUARDS)return id!=R_MINION_SACRIFICE;
 if(k==RC_TRANSFORM_DRAW)return id!=R_MINION_DIVE_BOMB;
 return 1;
}
const char*game_regent_choice_title(const Game*g){switch(g->regent.choice_kind){case RC_TOP:return "CHOOSE / PUT ON TOP OF DRAW PILE";case RC_DISCARD_TOP:return "DISCARD / RETURN TO DRAW PILE";case RC_TRANSFORM_HAND:return "BEGONE! / CHOOSE A MINION";case RC_TRANSFORM_DRAW:return "CHARGE!! / TRANSFORM DRAW CARDS";case RC_GUARDS:return "GUARDS!!! / TRANSFORM OR FINISH";case RC_SKILL_TRIPLE:return "DECISIONS / PLAY A SKILL THREE TIMES";case RC_COPY_COLORLESS:return "HEIRLOOM / COPY A COLORLESS CARD";case RC_GENERATE:return "CHOOSE A CREATED CARD";case RC_SEEK_THREE:return "SEEKER STRIKE / CHOOSE A DRAW CARD";case RC_EXHAUST:return "EXHAUST / CHOOSE A CARD";case RC_ENTROPY:return "ENTROPY / TRANSFORM A CARD";default:return "DRAW PILE / CHOOSE A CARD";}}
static void regent_open_choice(Game*g,RegentTask t){
 if(!ninja_has_living(g))return;
 if(g->hand_count>=MAX_HAND&&(t.n==RC_SEEK||t.n==RC_SEEK_SKILL||t.n==RC_SEEK_ATTACK||t.n==RC_STRATAGEM||t.n==RC_SEEK_THREE))return;
 g->regent.choice_kind=t.n;g->regent.choice_remaining=t.aux;g->regent.choice_raw=t.raw;g->regent.choice_target=t.target;g->regent.choice_depth=t.depth;g->regent.choice_aux=0;
 int first=-1;for(int i=0;i<game_regent_choice_count(g);i++)if(game_regent_choice_eligible(g,i)){first=i;break;}
 if(first<0){g->regent.choice_kind=0;return;}g->selection=first;g->screen=REGENT_CHOICE;g->prior_screen=COMBAT;
}
static void regent_offer(Game*g,int raw,int e,int up,int mode,int depth){
 g->regent.option_count=0;int pool[512],n=0;
 for(int id=3;id<CARD_COUNT;id++){
  int valid=mode==0?(id>=COLORLESS_FIRST&&id<EXTRA_LAST):mode==1?(id>=REGENT_FIRST&&id<REGENT_LAST&&solo_reward_eligible(id)&&id!=R_ROYALTIES):(id<NINJA_LAST&&id!=63&&id!=85&&id!=N_FLANKING&&id!=N_SNEAKY&&cards[id].type==0&&solo_reward_eligible(id));
  if(id>=ASCENT_CARD_FIRST)valid=ascent_card_available(g,id)&&(mode==1||(mode==2&&cards[id].type==0));
  if(valid)pool[n++]=id;
 }
 for(int i=0;i<3&&n;i++){int k=rnd(g,n);g->regent.options[g->regent.option_count++]=pool[k]|(up?UPBIT:0)|(mode?CARD_FREE:0);pool[k]=pool[--n];}
 regent_push(g,RT_CHOOSE,raw,e,RC_GENERATE,1,depth);
}
static void regent_process(Game*g);
static void regent_after_shuffle(Game*g){if((g->character==CHAR_REGENT||g->revision184.custom)&&RP(g,C_STRATAGEM))g->regent.shuffle_pending=1;}
static void regent_seek_three(Game*g,int raw,int e,int depth){
 shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);
 regent_push(g,RT_SEEK_THREE,raw,e,0,0,depth);
}
static int regent_delayed_block(const Game*g,int raw,int base){
 if(g->regent.panic)return 0;
 int n=MAX(0,base+g->dexterity+(relic(g,15)?2:0)+(CARD_MOD(raw)==MOD_WARD?4:0));
 /* Both halves of the same card share Unmovable's card multiplier. */
 if(g->ic[IC_UNMOVABLE]&&g->ic[IC_UNMOVABLE_USED]==1)n*=2;
 for(int i=0;i<MIN(20,g->ninja.v[NS_SHADOWMELD]);i++)n=(int)MIN(999LL,(long long)n*2);
 return MIN(999,n);
}
#include "regent_effects.inc"
static void regent_finish(Game*g,RegentTask t){
 int raw=t.raw,id=CARD_ID(raw),type=cards[id].type;
 if(type==2)record_combat_fx(GFX_POWER,-1,-1,0,0);
 if(CARD_MOD(raw)==MOD_KINDLED)draw_card(g);
 if(!(raw&CARD_EPHEMERAL)){
  int clean=regent_clean(raw);
  if((regent_flags(raw)&1)||(raw&CARD_FORCE_EXHAUST)||CARD_MOD(raw)==MOD_FLEETING||CARD_MOD(raw)==MOD_KINDLED||(g->ic[IC_CORRUPTION]&&type==1))exhaust_card(g,clean);
  else if(type!=2){
   if(id==R_PARTICLE_WALL)regent_to_hand(g,clean);
   else if((id==R_SHINING_STRIKE||(RP(g,C_NOSTALGIA)&&!g->regent.nostalgia_used&&(type==0||type==1)))&&g->draw_count<MAX_PILE){g->draw[g->draw_count++]=clean;if(RP(g,C_NOSTALGIA))g->regent.nostalgia_used=1;}
   else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=clean;
  }
 }
 g->turn_played++;g->regent.plays_combat=MIN(1000000,g->regent.plays_combat+1);
 if(type==0){g->ic[IC_ATTACKS_THIS_TURN]++;g->shop_relic++;if(g->ic[IC_ATTACKS_THIS_TURN]%3==0){if(relic(g,5))g->strength++;if(relic(g,24))heal(g,3);}int rage=(g->reward_potion>>8)&255;if(rage)gain_block(g,rage);
  if(RP(g,C_CALAMITY)){int pool[256],n=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(solo_reward_eligible(c)&&cards[c].type==0)pool[n++]=c;for(int i=0;i<MIN(MAX_HAND,RP(g,C_CALAMITY))&&n;i++)regent_create(g,pool[rnd(g,n)]);}
 }
 if(type==1){g->regent.skills++;if(++g->ic[36]%3==0&&relic(g,22))g->dexterity++;if(g->regent.skills%3==0){int*p[]={g->draw,g->discard,g->exhaust};int*n[]={&g->draw_count,&g->discard_count,&g->exhaust_count};for(int z=0;z<3;z++)for(int i=*n[z]-1;i>=0&&g->hand_count<MAX_HAND;i--)if(CARD_ID(p[z][i])==R_MAKE_IT_SO)regent_to_hand(g,regent_take(p[z],n[z],i));}}
 if(relic(g,43)&&(type==0||type==1)){g->ic[IC_DUELIST]|=type==0?1:2;if(g->ic[IC_DUELIST]==3){g->ic[IC_DUELIST]|=4;gain_energy(g,1);}}
 r151_played(g,type);
 if(t.n){g->strength+=t.n;g->ic[IC_TEMP_STRENGTH]+=t.n;}
 if(t.aux)regent_gain_stars(g,t.aux);
 if(g->regent.panache_phase[g->turn_played%5])ninja_all_proc(g,g->regent.panache_phase[g->turn_played%5]);
 if(id==C_PANACHE)g->regent.panache_phase[g->turn_played%5]+=10+4*CARD_UP(raw);
 g->regent.attack_vigor=0;g->selection=CLAMP(g->selection,0,MAX(0,g->hand_count-1));
}
static void regent_foreign_begin(Game*g,int raw){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom))return;
 if(cards[CARD_ID(raw)].type==0){g->regent.attack_vigor=g->regent.vigor;g->regent.vigor=0;}
}
static void regent_foreign_finish(Game*g,int raw){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom))return;int type=cards[CARD_ID(raw)].type;
 g->regent.plays_combat=MIN(1000000,g->regent.plays_combat+1);
 if(type==1){g->regent.skills++;if(g->regent.skills%3==0){int*p[]={g->draw,g->discard,g->exhaust};int*n[]={&g->draw_count,&g->discard_count,&g->exhaust_count};for(int z=0;z<3;z++)for(int i=*n[z]-1;i>=0&&g->hand_count<MAX_HAND;i--)if(CARD_ID(p[z][i])==R_MAKE_IT_SO)regent_to_hand(g,regent_take(p[z],n[z],i));}}
 if(type==0&&RP(g,C_CALAMITY)){int pool[256],n=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(solo_reward_eligible(c)&&cards[c].type==0)pool[n++]=c;for(int i=0;i<MIN(MAX_HAND,RP(g,C_CALAMITY))&&n;i++)regent_create(g,pool[rnd(g,n)]);}
 if(!(raw&CARD_EPHEMERAL)&&RP(g,C_NOSTALGIA)&&!g->regent.nostalgia_used&&(type==0||type==1)&&g->draw_count<MAX_PILE){for(int i=g->discard_count-1;i>=0;i--)if(CARD_INSTANCE(g->discard[i])==CARD_INSTANCE(raw)){g->draw[g->draw_count++]=regent_take(g->discard,&g->discard_count,i);g->regent.nostalgia_used=1;break;}}
 if(g->regent.monologue){g->strength+=g->regent.monologue;g->ic[IC_TEMP_STRENGTH]+=g->regent.monologue;}
 regent_gain_stars(g,RP(g,R_THE_SEALED_THRONE));
 if(g->regent.panache_phase[g->turn_played%5])ninja_all_proc(g,g->regent.panache_phase[g->turn_played%5]);
 g->regent.attack_vigor=0;
}
static void regent_process(Game*g){
 if(regent_processing||g->ninja.task_count)return;regent_processing=1;
 int saved_attack=r140_attack_context,saved_raw=r150_card_raw,saved_card=combat_fx_card,saved_rust=r140_rust_targets,saved_phase=preview_phase,saved_depth=preview_depth;
 for(int steps=0;steps<4096&&g->screen==COMBAT&&!g->ninja.task_count&&(g->regent.task_count||g->regent.shuffle_pending);steps++){
  if(g->regent.shuffle_pending){int n=g->regent.shuffle_draws;g->regent.shuffle_pending=g->regent.shuffle_draws=0;regent_push(g,RT_DRAW,C_STRATAGEM,g->target,n,0,0);regent_push(g,RT_CHOOSE,C_STRATAGEM,g->target,RC_STRATAGEM,RP(g,C_STRATAGEM),0);}
  RegentTask t=g->regent.tasks[--g->regent.task_count];int id=CARD_ID(t.raw);
  r150_card_raw=t.raw;r140_attack_context=id<CARD_COUNT&&cards[id].type==0;combat_fx_card=t.raw;
  if(preview_trace){preview_phase=t.depth>0&&t.depth<=2;preview_depth=t.depth;}
  switch(t.kind){
  case RT_PLAY:{
   if(id>=ASCENT_CARD_FIRST){resolve_card(g,-1,t.target,t.raw|CARD_FREE);break;}
   if(id<REGENT_FIRST){
    /* Colorless Splash can create Warden/Nightshade cards. Their own tested
       resolver remains responsible for their choices and physical pile move. */
    if(id>=NINJA_FIRST){ninja_push(g,NT_PLAY,t.raw|CARD_FREE,t.target,t.n,0,t.depth);ninja_process(g);}
    else resolve_card(g,-1,t.target,t.raw|CARD_FREE);
    break;
   }
   if(!ninja_has_living(g)){if(!(t.raw&CARD_EPHEMERAL)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=regent_clean(t.raw);break;}
   if(t.target<0||t.target>=g->enemy_count||g->enemies[t.target].hp<=0)t.target=random_living_foe(g);
   r140_rust_targets=0;int repeat=0;
   if(!(t.raw&CARD_EPHEMERAL)){
    repeat=g->regent.instance_replay[CARD_INSTANCE(t.raw)];
    if(cards[id].type==0){repeat+=CARD_MOD(t.raw)==MOD_ECHO;if(g->ic[IC_NEXT_ATTACK_REPEAT]>0){repeat++;g->ic[IC_NEXT_ATTACK_REPEAT]--;}}
    if(id==R_SOVEREIGN_BLADE)repeat+=RP(g,R_SWORD_SAGE);
   }
   if(cards[id].type==0){g->regent.attack_vigor=g->regent.vigor;g->regent.vigor=0;g->ic[IC_UNRELENTING]=0;}
   if(preview_trace&&t.depth==2)preview_trace->repeated++;
   for(int i=0;i<MIN(20,repeat);i++)regent_push(g,RT_PLAY,t.raw|CARD_FREE|CARD_EPHEMERAL,t.target,t.n,t.aux,t.depth?2:0);
   regent_push(g,RT_FINISH,t.raw,t.target,g->regent.monologue,RP(g,R_THE_SEALED_THRONE),t.depth);
   regent_push(g,RT_EFFECT,t.raw,t.target,t.n,t.aux,t.depth);break;}
  case RT_EFFECT:regent_effect(g,t);break;
  case RT_FINISH:if(preview_trace)preview_phase=0;regent_finish(g,t);break;
  case RT_DRAW:for(int i=0;i<t.n&&g->hand_count<MAX_HAND;i++)draw_card(g);break;
  case RT_CHOOSE:regent_open_choice(g,t);break;
  case RT_ENDTURN:
   /* End-turn is postponed until the played Power and its finish task settle. */
   if(g->regent.task_count){for(int i=g->regent.task_count;i>0;i--)g->regent.tasks[i]=g->regent.tasks[i-1];g->regent.tasks[0]=t;g->regent.task_count++;}
   else{regent_processing=0;game_end_turn(g);regent_processing=1;}
   break;
  case RT_AUTOPLAY:{
   int*p=t.aux==2?g->discard:g->draw,*n=t.aux==2?&g->discard_count:&g->draw_count;
   int options[MAX_PILE],count=0;for(int i=0;i<*n;i++)if(t.aux!=2||cards[CARD_ID(p[i])].type==0)options[count++]=i;
   if(t.n>0&&count){int k=t.aux==3?*n-1:options[rnd(g,count)],raw=regent_take(p,n,k);if(t.n>1)regent_push(g,RT_AUTOPLAY,t.raw,t.target,t.n-1,t.aux,0);regent_push(g,RT_PLAY,raw|CARD_FREE,t.target,game_card_cost(raw)==-1?g->energy:0,(CARD_ID(raw)==R_STARDUST)?g->regent.stars:0,0);}break;}
  case RT_SEEK_THREE:
   g->regent.option_count=MIN(3,g->draw_count);for(int i=0;i<g->regent.option_count;i++)g->regent.options[i]=g->draw[g->draw_count-1-i];regent_push(g,RT_CHOOSE,t.raw,t.target,RC_SEEK_THREE,1,t.depth);break;
  case RT_RETURN:
   if(t.n>0){if(RP(g,R_CHILD_OF_THE_STARS))relic_block(g,t.n*RP(g,R_CHILD_OF_THE_STARS));if(RP(g,R_BLACK_HOLE))ninja_all_proc(g,RP(g,R_BLACK_HOLE));}break;
  }
 }
 r140_attack_context=saved_attack;r150_card_raw=saved_raw;combat_fx_card=saved_card;r140_rust_targets=saved_rust;preview_phase=saved_phase;preview_depth=saved_depth;regent_processing=0;
 if(g->screen==COMBAT&&!g->regent.task_count&&!g->ninja.task_count){player_death(g);if(g->screen==COMBAT)check_win(g);}
}
static void regent_play(Game*g,int index,int target){
 if(g->screen!=COMBAT||index<0||index>=g->hand_count||(g->regent.task_count&&!(g->hand[index]&CARD_FREE)))return;
 int raw=regent_stamp(g,g->hand[index]),cost=game_card_cost_now(g,raw),scost=game_card_star_cost(g,raw);
 int id=CARD_ID(raw);if(!game_card_is_playable(raw)||(id>=NINJA_FIRST&&id<NINJA_LAST&&!ninja_can_play(g,raw)))return;
 int x=game_card_cost(raw)==-1?g->energy:0,sx=id==R_STARDUST?g->regent.stars:0;
 if(cost==-1)cost=g->energy;if(scost==-1)scost=g->regent.stars;
 if(cost<0||cost>g->energy){msg(g,"Not enough Energy.");return;}
 if(scost>g->regent.stars){snprintf(g->message,sizeof(g->message),"Need %d more Stars.",scost-g->regent.stars);return;}
 regent_take(g->hand,&g->hand_count,index);if(!g->ic[IC_CHAIN_DEPTH]&&!regent_processing&&!ninja_processing)g->regent.void_used++;g->energy-=cost;g->regent.stars-=scost;
 if(RP(g,R_ORBIT)&&cost>0){g->regent.orbit_spent+=cost;while(g->regent.orbit_spent>=4){g->regent.orbit_spent-=4;gain_energy(g,RP(g,R_ORBIT));}}
 if(scost>0)regent_push(g,RT_RETURN,raw,target,scost,0,0);
 regent_push(g,RT_PLAY,raw,target,x,sx,preview_trace?1:0);regent_process(g);
}
static void regent_choose(Game*g,int index,int finish){
 if(g->screen!=REGENT_CHOICE)return;int kind=g->regent.choice_kind,source=g->regent.choice_raw,up=CARD_UP(source),e=g->regent.choice_target,depth=g->regent.choice_depth;
 int optional=kind==RC_GUARDS||(kind==RC_EXHAUST&&CARD_ID(source)==C_PURITY);
 if(finish&&!optional){msg(g,"Choose a card to finish the effect.");return;}
 if(!finish&&!game_regent_choice_eligible(g,index)){msg(g,"This card is not eligible.");return;}
 if(!finish){int raw=game_regent_choice_raw(g,index);int*p=g->hand,*n=&g->hand_count;int pile=regent_choice_pile(g);if(pile==1){p=g->draw;n=&g->draw_count;}else if(pile==2){p=g->discard;n=&g->discard_count;}
  switch(kind){
  case RC_TOP:case RC_DISCARD_TOP:if(g->draw_count<MAX_PILE){regent_take(p,n,index);g->draw[g->draw_count++]=raw;}break;
  case RC_TRANSFORM_HAND:case RC_TRANSFORM_DRAW:case RC_GUARDS:{int token=kind==RC_TRANSFORM_HAND?R_MINION_STRIKE:kind==RC_TRANSFORM_DRAW?R_MINION_DIVE_BOMB:R_MINION_SACRIFICE;p[index]=regent_stamp(g,token|(up?UPBIT:0));regent_created(g);break;}
  case RC_ENTROPY:{int pool[128],count=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(c!=CARD_ID(raw)&&c!=R_ROYALTIES&&solo_reward_eligible(c))pool[count++]=c;if(count){p[index]=regent_stamp(g,pool[rnd(g,count)]);regent_created(g);}break;}
  case RC_SKILL_TRIPLE:regent_take(p,n,index);regent_push(g,RT_PLAY,raw|CARD_FREE|CARD_EPHEMERAL,e,0,0,0);regent_push(g,RT_PLAY,raw|CARD_FREE|CARD_EPHEMERAL,e,0,0,0);regent_push(g,RT_PLAY,raw|CARD_FREE,e,0,0,0);break;
  case RC_COPY_COLORLESS:regent_create(g,regent_clean(raw)&~CARD_INSTANCE_MASK);break;
  case RC_GENERATE:regent_create(g,raw);break;
  case RC_SEEK_THREE:for(int j=0;j<g->draw_count;j++)if(g->draw[j]==raw){regent_to_hand(g,regent_take(g->draw,&g->draw_count,j));break;}break;
  case RC_EXHAUST:regent_take(p,n,index);{int screen=g->screen;g->screen=COMBAT;exhaust_card(g,raw);g->screen=screen;}break;
  default:if(g->hand_count<MAX_HAND){regent_take(p,n,index);regent_to_hand(g,raw);}break;
  }
  g->regent.choice_remaining--;
 }
 int first=-1;for(int i=0;i<game_regent_choice_count(g);i++)if(game_regent_choice_eligible(g,i)){first=i;break;}
 if(!finish&&g->regent.choice_remaining>0&&first>=0&&((kind!=RC_SEEK&&kind!=RC_SEEK_SKILL&&kind!=RC_SEEK_ATTACK&&kind!=RC_STRATAGEM)||g->hand_count<MAX_HAND)){g->selection=first;return;}
 g->regent.choice_kind=0;g->regent.option_count=0;g->screen=COMBAT;g->selection=0;regent_process(g);
}
static void regent_before_turn(Game*g){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom))return;g->regent.stars_gained=g->regent.skills=g->regent.nostalgia_used=g->regent.void_used=0;
 memset(g->regent.panache_phase,0,sizeof(g->regent.panache_phase));g->regent.panache_phase[0]=RP(g,C_PANACHE);
 memset(g->regent.hit_count,0,sizeof(g->regent.hit_count));memset(g->regent.conqueror,0,sizeof(g->regent.conqueror));g->regent.reflect=0;if(g->regent.panic)g->regent.panic--;
}
static void regent_start_turn(Game*g){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom)||g->screen!=COMBAT)return;
 if(g->turn==1&&(g->character==CHAR_REGENT||relic(g,63)))regent_gain_stars(g,3);
 gain_energy(g,g->regent.next_energy);g->regent.next_energy=0;
 regent_gain_stars(g,g->regent.next_stars+RP(g,R_GENESIS));g->regent.next_stars=0;
 if(g->regent.next_block){relic_block(g,g->regent.next_block);g->regent.next_block=0;}
 g->regent.vigor+=RP(g,C_PREP_TIME);
 if(RP(g,R_FURNACE))regent_forge(g,RP(g,R_FURNACE));
 for(int i=0;i<MIN(MAX_HAND,RP(g,R_SPECTRUM_SHIFT));i++)regent_create(g,regent_colorless(g,0));
 if(RP(g,C_ROLLING_BOULDER)){ninja_all_proc(g,RP(g,C_ROLLING_BOULDER));RP(g,C_ROLLING_BOULDER)+=5*g->regent.played[C_ROLLING_BOULDER-REGENT_FIRST];}
 for(int i=0;i<g->regent.return_count;i++){int raw=g->regent.return_cards[i],k=CARD_INSTANCE(raw),*p[]={g->draw,g->discard,g->exhaust},*n[]={&g->draw_count,&g->discard_count,&g->exhaust_count};for(int z=0;z<3;z++)for(int j=*n[z]-1;j>=0;j--)if((k&&CARD_INSTANCE(p[z][j])==k)||(!k&&p[z][j]==raw)){regent_to_hand(g,regent_take(p[z],n[z],j));goto returned; }returned:; }g->regent.return_count=0;
 if(RP(g,C_ENTROPY))regent_push(g,RT_CHOOSE,C_ENTROPY,g->target,RC_ENTROPY,RP(g,C_ENTROPY),0);
 if(RP(g,R_TYRANNY)){regent_push(g,RT_CHOOSE,R_TYRANNY,g->target,RC_EXHAUST,RP(g,R_TYRANNY),0);regent_push(g,RT_DRAW,R_TYRANNY,g->target,RP(g,R_TYRANNY),0,0);}
 if(g->regent.next_seek){regent_push(g,RT_CHOOSE,R_FOREGONE_CONCLUSION,g->target,RC_SEEK,g->regent.next_seek,0);g->regent.next_seek=0;}
 if(g->regent.next_draw){regent_push(g,RT_DRAW,R_GLOW,g->target,g->regent.next_draw,0,0);g->regent.next_draw=0;}
 if(RP(g,C_MAYHEM))regent_push(g,RT_AUTOPLAY,C_MAYHEM,g->target,RP(g,C_MAYHEM),3,0);
 for(int i=g->exhaust_count-1;i>=0;i--)if(CARD_ID(g->exhaust[i])==R_BOMBARDMENT){int raw=regent_take(g->exhaust,&g->exhaust_count,i);regent_push(g,RT_PLAY,raw|CARD_FREE,g->target,0,0,0);}
 regent_process(g);
}
static void regent_end_turn(Game*g){
 if((g->character!=CHAR_REGENT&&!g->revision184.custom)||g->ic[38])return;
 if(g->turn_played>=5)g->regent.next_draw+=RP(g,R_PALE_BLUE_DOT);
 for(int i=0;i<16;i++)if(g->regent.bomb_ticks[i]&&!--g->regent.bomb_ticks[i])ninja_all_proc(g,g->regent.bomb_damage[i]);
 if(g->draw_count&&CARD_ID(g->draw[g->draw_count-1])==R_IAM_INVINCIBLE){int raw=g->draw[--g->draw_count];regent_push(g,RT_PLAY,raw|CARD_FREE,g->target,0,0,0);regent_process(g);}
}
#undef RP
