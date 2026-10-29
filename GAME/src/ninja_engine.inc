/* Nightshade: deterministic, serializable effect stack. A choice suspends the
   actual card, including its final pile move and replay; no temporary hand
   vacancy or fake card copy is used to implement Sly. */
enum {NT_PLAY=1,NT_EFFECT,NT_FINISH,NT_DRAW,NT_DISCARD,NT_SHIVS,NT_DISCARD_ALL,NT_AFTER_DISCARD,NT_CHOOSE,NT_ENDTURN};
enum {NC_DISCARD=1,NC_HAND_TRICK,NC_NIGHTMARE,NC_RETAIN};
static int ninja_processing;
static int ninja_push(Game*g,int kind,int raw,int target,int n,int aux,int depth){
 if(g->ninja.task_count>=NINJA_TASK_MAX){msg(g,"Effect queue full. Finish current choices.");return 0;}
 NinjaTask*t=&g->ninja.tasks[g->ninja.task_count++];*t=(NinjaTask){kind,raw,target,n,aux,depth};return 1;
}
/* Per-combat instance tags let a replay update the same physical card's
   permanent cost reduction, while Nightmare copies remain independent. */
static int ninja_instance_used(const Game*g,int serial){
 for(int i=0;i<g->draw_count;i++)if(CARD_INSTANCE(g->draw[i])==serial)return 1;
 for(int i=0;i<g->hand_count;i++)if(CARD_INSTANCE(g->hand[i])==serial)return 1;
 for(int i=0;i<g->discard_count;i++)if(CARD_INSTANCE(g->discard[i])==serial)return 1;
 for(int i=0;i<g->exhaust_count;i++)if(CARD_INSTANCE(g->exhaust[i])==serial)return 1;
 for(int i=0;i<g->ninja.task_count;i++)if(CARD_INSTANCE(g->ninja.tasks[i].raw)==serial)return 1;
 return 0;
}
static int ninja_stamp(Game*g,int raw){
 if(CARD_ID(raw)<NINJA_FIRST||CARD_ID(raw)>=NINJA_LAST||CARD_INSTANCE(raw))return raw;
 for(int i=0;i<511;i++){int serial=g->ninja.next_instance%511+1;g->ninja.next_instance=serial;if(!ninja_instance_used(g,serial))return raw|(serial<<CARD_INSTANCE_SHIFT);}return raw;
}
static void ninja_reduce_instance(Game*g,int raw){
 int serial=CARD_INSTANCE(raw);if(!serial)return;
 int *piles[]={g->hand,g->draw,g->discard,g->exhaust};int count[]={g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};
 for(int p=0;p<4;p++)for(int i=0;i<count[p];i++)if(CARD_INSTANCE(piles[p][i])==serial){int c=piles[p][i],n=MIN(3,CARD_COUNTER(c)+1);piles[p][i]=(c&~CARD_COUNTER_MASK)|(n<<CARD_COUNTER_SHIFT);return;}
}
static int ninja_flags(int raw){int id=CARD_ID(raw);if(id<NINJA_FIRST||id>=NINJA_LAST)return 0;const NinjaCardDef*d=&ninja_cards[id-NINJA_FIRST];return CARD_UP(raw)?d->upflags:d->flags;}
int game_card_is_sly(const Game*g,int raw){(void)g;return (raw&(CARD_SLY|CARD_SLY_TURN))||(ninja_flags(raw)&8);}
int game_card_is_retained(const Game*g,int raw){return (regent_flags(raw)&2)||g->regent.retain_all||CARD_MOD(raw)==MOD_BOUND||(raw&CARD_RETAIN_TURN)||(ninja_flags(raw)&2)||(CARD_ID(raw)==N_SHIV&&g->ninja.v[NS_PHANTOM]);}
static int card_character(int id){
 if(id<0||id>=CARD_COUNT)return -1;
 if(id>=ASCENT_CARD_FIRST)return (id-ASCENT_CARD_FIRST)/20;
 if(id<NINJA_FIRST)return CHAR_WARDEN;if(id<NINJA_LAST)return CHAR_NIGHTSHADE;
 return id<REGENT_LAST?CHAR_REGENT:-1;
}
static unsigned custom_deck_characters(const Game*g){
 unsigned mask=0;for(int i=0;i<g->deck_count;i++){int c=card_character(CARD_ID(g->deck[i]));if(c>=0)mask|=1u<<c;}
 return mask?mask:1u<<g->character;
}
static int ninja_owned_card(const Game*g,int id){
 int c=card_character(id);if(c<0||!ascent_card_available(g,id))return 0;
 return g->revision184.custom?!!(custom_deck_characters(g)&(1u<<c)):c==g->character;
}
static int ninja_clean(int raw){return raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL|CARD_RETAIN_TURN);}
static int ninja_base_damage(const Game*g,int raw,int in_hand){
 int id=CARD_ID(raw),up=CARD_UP(raw);if(id<NINJA_FIRST||id>=NINJA_LAST)return 0;
 const NinjaCardDef*d=&ninja_cards[id-NINJA_FIRST];int n=up?d->updamage:d->damage;
 if(id==N_MEMENTO_MORI)n=9+2*up+(4+up)*MIN(1000000,g->ninja.v[NS_DISCARDS_TURN]);
 if(id==N_MURDER)n=1+MIN(1000000,g->ninja.v[NS_DRAWS_COMBAT]);
 if(id==N_PRECISE_CUT)n=MAX(0,13+3*up-2*MAX(0,g->hand_count-in_hand));
 if(id==N_SHIV)n+=g->ninja.v[NS_ACCURACY]+(g->ninja.v[NS_SHIVS_TURN]==0?g->ninja.v[NS_PHANTOM]:0);
 return n;
}
static int ninja_block_base(Game*g,int raw){int id=CARD_ID(raw),up=CARD_UP(raw);const NinjaCardDef*d=&ninja_cards[id-NINJA_FIRST];if(id==N_MIRAGE){int n=0;for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0)n+=g->enemies[e].poison;return n;}return up?d->upblock:d->block;}
static void ninja_proc(Game*g,int e,int amount,int ignore_block){
 if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0||amount<=0)return;
 int b=ignore_block?0:MIN(amount,g->enemies[e].block);g->enemies[e].block-=b;g->enemies[e].hp-=amount-b;
 record_combat_fx(GFX_PROC,-1,e,amount,b);
}
static void ninja_all_proc(Game*g,int n){for(int e=0;e<g->enemy_count;e++)ninja_proc(g,e,n,0);}
static void ninja_poison(Game*g,int e,int n){
 if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0||n<=0)return;
 n+=relic(g,NR_SKULL);g->enemies[e].poison=MIN(1000000000,g->enemies[e].poison+MIN(n,1000000000-g->enemies[e].poison));
 record_combat_fx(GFX_STATUS,-1,e,4,0);
 if(g->ninja.v[NS_OUTBREAK]&&++g->ninja.v[NS_OUTBREAK_COUNT]%3==0)ninja_all_proc(g,g->ninja.v[NS_OUTBREAK]);
}
static void ninja_poison_all(Game*g,int n){for(int e=0;e<g->enemy_count;e++)ninja_poison(g,e,n);}
static void ninja_weak(Game*g,int e,int n){if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0)return;g->enemies[e].weak=MIN(99,g->enemies[e].weak+n);record_combat_fx(GFX_STATUS,-1,e,2,0);}
static void ninja_on_draw(Game*g,int raw){
 g->ninja.v[NS_LAST_DRAW]=raw+1;
 if((g->character!=CHAR_NIGHTSHADE&&!g->revision184.custom))return;
 g->ninja.v[NS_DRAWS_COMBAT]=MIN(1000000,g->ninja.v[NS_DRAWS_COMBAT]+1);
 if(!g->ic[IC_OWN_TURN]||g->ninja.v[NS_OPENING_DRAW])return;
 if(g->ninja.v[NS_CORROSIVE])ninja_poison_all(g,g->ninja.v[NS_CORROSIVE]);
 if(g->ninja.v[NS_SPEEDSTER])ninja_all_proc(g,g->ninja.v[NS_SPEEDSTER]);
}
static void ninja_on_hit(Game*g,int e,int unblocked){
 if(unblocked>0&&r140_attack_context){if(g->ninja.v[NS_ENVENOM])ninja_poison(g,e,g->ninja.v[NS_ENVENOM]);if(r150_card_raw&CARD_INKY)ninja_weak(g,e,1);}
}
static int ninja_has_living(const Game*g){for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0)return 1;return 0;}
static void ninja_power(Game*g,int raw,int counter,int amount){
 int id=CARD_ID(raw);g->ninja.powers[id-NINJA_FIRST]=MIN(1000000,g->ninja.powers[id-NINJA_FIRST]+1);
 if(counter>=0)g->ninja.v[counter]=MIN(1000000,g->ninja.v[counter]+amount);
}
static void ninja_shivs(Game*g,int count,int up,int ink){for(int i=0;i<count;i++)add_generated(g,N_SHIV|(up?UPBIT:0)|(ink?CARD_INKY:0));}
static void ninja_discard_effects(Game*g,int raw,int depth){
 raw&=~CARD_RETAIN_TURN;g->ninja.v[NS_DISCARDS_TURN]=MIN(1000000,g->ninja.v[NS_DISCARDS_TURN]+1);
 if(relic(g,NR_TINGSHA))ninja_proc(g,random_living_foe(g),3,0);
 if(relic(g,NR_BANDAGES))relic_block(g,3);
 if(game_card_is_sly(g,raw)&&cards[CARD_ID(raw)].cost!=-2&&ninja_has_living(g)){
  ninja_push(g,NT_PLAY,raw|CARD_FREE,g->target,game_card_cost(raw)==-1?g->energy:0,0,depth>0?3:0);
 }else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=ninja_clean(raw);
}
int game_ninja_choice_eligible(const Game*g,int ix){
 if(ix<0||ix>=g->hand_count)return 0;
 if(g->ninja.choice_kind==NC_HAND_TRICK)return cards[CARD_ID(g->hand[ix])].type==1&&!game_card_is_sly(g,g->hand[ix]);
 if(g->ninja.choice_kind==NC_RETAIN)return !game_card_is_retained(g,g->hand[ix]);
 return 1;
}
const char *game_ninja_choice_title(const Game*g){switch(g->ninja.choice_kind){case NC_DISCARD:return "DISCARD / CHOOSE A CARD";case NC_HAND_TRICK:return "HAND TRICK / CHOOSE A SKILL";case NC_NIGHTMARE:return "NIGHTMARE / CHOOSE A CARD";case NC_RETAIN:return "RETAIN / CHOOSE OR FINISH";default:return "CHOOSE A CARD";}}
static void ninja_process(Game*g);
static void ninja_open_choice(Game*g,int kind,int count,int source,int target,int depth){
 g->ninja.choice_kind=kind;g->ninja.choice_remaining=count;g->ninja.choice_source=source;g->ninja.choice_target=target;g->ninja.choice_depth=depth;g->ninja.choice_count=0;
 int first=-1;for(int i=0;i<g->hand_count;i++)if(game_ninja_choice_eligible(g,i)){first=i;break;}
 if(first<0||!ninja_has_living(g)){g->ninja.choice_kind=0;return;}
 g->selection=first;g->screen=NINJA_CHOICE;g->prior_screen=COMBAT;
}
static void ninja_effect(Game*g,NinjaTask t){
 int raw=t.raw,id=CARD_ID(raw),up=CARD_UP(raw),e=t.target,x=t.n,depth=t.depth;
 int r151_before[6];r151_status_snapshot(g,r151_before);
 int dmg=ninja_base_damage(g,raw,0),block=ninja_block_base(g,raw);
 if(e<0||e>=g->enemy_count||g->enemies[e].hp<=0)e=random_living_foe(g);
#define NP(counter,amount) ninja_power(g,raw,counter,amount)
#define ND(n) ninja_push(g,NT_DRAW,raw,e,n,0,depth)
#define DISC(n) ninja_push(g,NT_DISCARD,raw,e,n,0,depth)
#define CHOICE(kind) ninja_push(g,NT_CHOOSE,raw,e,kind,1,depth)
 switch(id){
 case N_ABRASIVE:g->dexterity++;NP(NS_THORNS,4+2*up);break;
 case N_ACCELERANT:NP(NS_ACCELERANT,1+up);break;
 case N_ACCURACY:NP(NS_ACCURACY,4+2*up);break;
 case N_ACROBATICS:DISC(1);ND(3+up);break;
 case N_ADRENALINE:gain_energy(g,1+up);ND(2);break;
 case N_AFTERIMAGE:NP(NS_AFTERIMAGE,1);break;
 case N_ANTICIPATE:g->dexterity+=2+up;g->ninja.v[NS_TEMP_DEX]+=2+up;break;
 case N_ASSASSINATE:deal(g,e,dmg);if(e>=0)apply_vulnerable(g,&g->enemies[e],1+up);break;
 case N_BACKFLIP:card_block(g,block);ND(2);break;
 case N_BACKSTAB:case N_SLICE:case N_STRIKE_SILENT:deal(g,e,dmg);break;
 case N_BLADE_DANCE:ninja_shivs(g,3+up,0,0);break;
 case N_BLADE_OF_INK:ninja_shivs(g,2+up,0,1);break;
 case N_BLUR:card_block(g,block);g->ninja.v[NS_BLUR]++;break;
 case N_BOUNCING_FLASK:for(int i=0;i<3+up;i++)ninja_poison(g,random_living_foe(g),3);break;
 case N_BUBBLE_BUBBLE:if(e>=0&&g->enemies[e].poison>0)ninja_poison(g,e,9+3*up);break;
 case N_BULLET_TIME:g->reward_potion|=1;for(int i=0;i<g->hand_count;i++)g->hand[i]|=CARD_FREE;break;
 case N_BURST:g->ninja.v[NS_BURST]+=1+up;break;
 case N_CALCULATED_GAMBLE:ninja_push(g,NT_DISCARD_ALL,raw,e,1,0,depth);break;
 case N_CLOAK_AND_DAGGER:card_block(g,block);ninja_shivs(g,1+up,0,0);break;
 case N_CORROSIVE_WAVE:g->ninja.v[NS_CORROSIVE]+=2+up;break;
 case N_DAGGER_SPRAY:all_damage(g,dmg);all_damage(g,dmg);break;
 case N_DAGGER_THROW:deal(g,e,dmg);DISC(1);ND(1);break;
 case N_DASH:card_block(g,block);deal(g,e,dmg);break;
 case N_DEADLY_POISON:ninja_poison(g,e,5+2*up);break;
 case N_DEFEND_SILENT:case N_DEFLECT:case N_UNTOUCHABLE:case N_MIRAGE:card_block(g,block);break;
 case N_DODGE_AND_ROLL:card_block(g,block);g->ninja.v[NS_NEXT_BLOCK]+=block+(CARD_MOD(raw)==MOD_WARD?4:0);break;
 case N_ECHOING_SLASH:{int pending=1;for(int i=0;i<MAX_ENEMIES+1&&pending>0;i++){pending--;int before[2];for(int q=0;q<g->enemy_count;q++)before[q]=g->enemies[q].hp;all_damage(g,dmg);for(int q=0;q<g->enemy_count;q++)if(before[q]>0&&g->enemies[q].hp<=0)pending++;}break;}
 case N_ENVENOM:NP(NS_ENVENOM,1+up);break;
 case N_ESCAPE_PLAN:g->ninja.v[NS_LAST_DRAW]=0;draw_card(g);if(preview_trace)preview_trace->uncertain=1;if(g->ninja.v[NS_LAST_DRAW]&&cards[CARD_ID(g->ninja.v[NS_LAST_DRAW]-1)].type==1)card_block(g,block);break;
 case N_EXPERTISE:ND(MAX(0,6+up-g->hand_count));break;
 case N_EXPOSE:if(e>=0){g->enemies[e].block=0;apply_vulnerable(g,&g->enemies[e],2+up);record_combat_fx(GFX_STATUS,-1,e,1,0);}break;
 case N_FAN_OF_KNIVES:NP(NS_FAN,1);ninja_shivs(g,4+up,0,0);break;
 case N_FINISHER:for(int i=0;i<MIN(1000,g->ic[IC_ATTACKS_THIS_TURN]);i++)deal(g,e,dmg);break;
 case N_FLECHETTES:{int n=0;for(int i=0;i<g->hand_count;i++)n+=cards[CARD_ID(g->hand[i])].type==1;for(int i=0;i<n;i++)deal(g,e,dmg);break;}
 case N_FLICK_FLACK:case N_GRAND_FINALE:all_damage(g,dmg);break;
 case N_FOOTWORK:NP(-1,0);g->dexterity+=2+up;break;
 case N_HAND_TRICK:card_block(g,block);CHOICE(NC_HAND_TRICK);break;
 case N_HAZE:ninja_poison_all(g,4+2*up);break;
 case N_HIDDEN_DAGGERS:ninja_push(g,NT_SHIVS,raw,e,2,up,depth);DISC(2);break;
 case N_INFINITE_BLADES:NP(NS_BLADES,1);break;
 case N_KNIFE_TRAP:{int taken[MAX_PILE],n=0,kept=0;for(int i=0;i<g->exhaust_count;i++){int c=g->exhaust[i];if(CARD_ID(c)==N_SHIV&&n<NINJA_TASK_MAX-8)taken[n++]=c|(up?UPBIT:0);else g->exhaust[kept++]=c;}g->exhaust_count=kept;for(int i=n-1;i>=0;i--)ninja_push(g,NT_PLAY,taken[i]|CARD_FREE,e,0,0,depth?3:0);break;}
 case N_LEADING_STRIKE:deal(g,e,dmg);ninja_shivs(g,2,0,0);break;
 case N_LEG_SWEEP:ninja_weak(g,e,2+up);card_block(g,block);break;
 case N_MALAISE:if(e>=0){g->enemies[e].strength-=x+up;ninja_weak(g,e,x+up);}break;
 case N_MASTER_PLANNER:NP(NS_PLANNER,1);break;
 case N_MEMENTO_MORI:case N_MURDER:case N_PINPOINT:case N_PRECISE_CUT:deal(g,e,dmg);break;
 case N_NEUTRALIZE:case N_SUCKER_PUNCH:case N_SUPPRESS:deal(g,e,dmg);ninja_weak(g,e,id==N_SUPPRESS?3+2*up:1+up);break;
 case N_NIGHTMARE:CHOICE(NC_NIGHTMARE);break;
 case N_NOXIOUS_FUMES:NP(NS_FUMES,2+up);break;
 case N_OUTBREAK:NP(NS_OUTBREAK,11+4*up);break;
 case N_PHANTOM_BLADES:NP(NS_PHANTOM,9+3*up);break;
 case N_PIERCING_WAIL:for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){g->enemies[i].strength-=6+2*up;g->ic[IC_MANGLE0+i]+=6+2*up;record_combat_fx(GFX_STATUS,-1,i,8,0);}break;
 case N_POISONED_STAB:deal(g,e,dmg);ninja_poison(g,e,3+up);break;
 case N_POUNCE:deal(g,e,dmg);g->ninja.v[NS_POUNCE]=1;break;
 case N_PREDATOR:deal(g,e,dmg);g->ninja.v[NS_NEXT_DRAW]+=2;break;
 case N_PREPARED:DISC(1+up);ND(1+up);break;
 case N_REFLEX:ND(2+up);break;
 case N_RICOCHET:for(int i=0;i<4+up;i++){int foe=preview_trace&&preview_choice>=0?preview_choice:random_living_foe(g);if(foe<0)break;if(g->enemies[foe].hp<=0)foe=random_living_foe(g);deal(g,foe,dmg);}break;
 case N_SCARE:for(int i=0;i<g->enemy_count;i++)ninja_weak(g,i,1);break;
 case N_SERPENT_FORM:NP(NS_SERPENT,4+2*up);break;
 case N_SHADOW_STEP:g->ninja.v[NS_SHADOW_NEXT]++;ninja_push(g,NT_DISCARD_ALL,raw,e,0,0,depth);break;
 case N_SHADOWMELD:g->ninja.v[NS_SHADOWMELD]++;break;
 case N_SHIV:if(g->ninja.v[NS_FAN])all_damage(g,dmg);else deal(g,e,dmg);break;
 case N_SKEWER:for(int i=0;i<x;i++)deal(g,e,dmg);break;
 case N_SNAKEBITE:ninja_poison(g,e,7+3*up);break;
 case N_SPEEDSTER:NP(NS_SPEEDSTER,2);break;
 case N_STORM_OF_STEEL:ninja_push(g,NT_DISCARD_ALL,raw,e,2,up,depth);break;
 case N_STRANGLE:deal(g,e,dmg);if(e>=0&&g->enemies[e].hp>0)g->ninja.v[NS_STRANGLE0+e]+=2+up;break;
 case N_SURVIVOR:card_block(g,block);DISC(1);break;
 case N_TACTICIAN:gain_energy(g,1+up);break;
 case N_THE_HUNT:{int before=e>=0?g->enemies[e].hp:0;deal(g,e,dmg);if(before>0&&g->enemies[e].hp<=0)g->ninja.v[NS_BONUS_REWARDS]++;break;}
 case N_TOOLS_OF_THE_TRADE:NP(NS_TOOLS,1);break;
 case N_TRACKING:NP(NS_TRACKING,1);break;
 case N_UP_MY_SLEEVE:ninja_shivs(g,3+up,0,0);break;
 case N_WELL_LAID_PLANS:NP(NS_PLANS,1+up);break;
 case N_WRAITH_FORM:NP(NS_WRAITH,1);g->ninja.v[NS_INTANGIBLE]+=2+up;break;
 }
 r151_after_effect(g,r151_before);
#undef NP
#undef ND
#undef DISC
#undef CHOICE
}
static void ninja_finish(Game*g,NinjaTask t){
 int raw=t.raw,id=CARD_ID(raw),type=cards[id].type;
 if(type==2)record_combat_fx(GFX_POWER,-1,-1,0,0);
 if(CARD_MOD(raw)==MOD_KINDLED&&g->screen==COMBAT)draw_card(g);
 if(id==N_UP_MY_SLEEVE){if(raw&CARD_EPHEMERAL)ninja_reduce_instance(g,raw);else{int count=MIN(3,CARD_COUNTER(raw)+1);raw=(raw&~CARD_COUNTER_MASK)|(count<<CARD_COUNTER_SHIFT);}}
 if(type==1&&g->ninja.v[NS_PLANNER])raw|=CARD_SLY;
 if(!(raw&CARD_EPHEMERAL)){
  if((ninja_flags(raw)&1)||(raw&CARD_FORCE_EXHAUST)||CARD_MOD(raw)==MOD_FLEETING||CARD_MOD(raw)==MOD_KINDLED||(g->ic[IC_CORRUPTION]&&type==1))exhaust_card(g,ninja_clean(raw));
  else if(type!=2&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=ninja_clean(raw);
 }
 g->turn_played++;
 if(type==0){g->ic[IC_ATTACKS_THIS_TURN]++;g->shop_relic++;if(g->ic[IC_ATTACKS_THIS_TURN]%3==0){if(relic(g,5))g->strength++;if(relic(g,24))heal(g,3);}int rage=(g->reward_potion>>8)&255;if(rage)gain_block(g,rage);}
 if(type==1){g->ninja.v[NS_SKILLS_TURN]++;if(++g->ic[36]%3==0&&relic(g,22))g->dexterity++;}
 if(id==N_SHIV){g->ninja.v[NS_SHIVS_TURN]++;if(relic(g,NR_DART)){g->dexterity++;g->ninja.v[NS_TEMP_DEX]++;}}
 if(relic(g,43)&&(type==0||type==1)){g->ic[IC_DUELIST]|=type==0?1:2;if(g->ic[IC_DUELIST]==3){g->ic[IC_DUELIST]|=4;gain_energy(g,1);}}
 r151_played(g,type);regent_foreign_finish(g,raw);
 /* Only powers that existed before this card fire for this play. */
 int after=t.n,serpent=t.aux;
 if(after)relic_block(g,after);
 if(serpent)ninja_proc(g,random_living_foe(g),serpent,0);
 for(int e=0;e<g->enemy_count;e++)if(g->ninja.v[NS_STRANGLE0+e]){int amount=g->ninja.v[NS_STRANGLE0+e]-(id==N_STRANGLE&&e==t.target?2+CARD_UP(raw):0);ninja_proc(g,e,amount,1);}
 g->selection=CLAMP(g->selection,0,MAX(0,g->hand_count-1));
}
static int ninja_can_play(const Game*g,int raw){int id=CARD_ID(raw);return id!=N_FLANKING&&id!=N_SNEAKY&&(id!=N_GRAND_FINALE||g->draw_count==0);}
static void ninja_process(Game*g){
 if(ninja_processing)return;ninja_processing=1;
 int saved_attack=r140_attack_context,saved_raw=r150_card_raw,saved_card=combat_fx_card,saved_rust=r140_rust_targets;
 int saved_phase=preview_phase,saved_depth=preview_depth;
 for(int steps=0;steps<4096&&g->screen==COMBAT&&g->ninja.task_count>0;steps++){
  NinjaTask t=g->ninja.tasks[--g->ninja.task_count];int id=CARD_ID(t.raw);
  r150_card_raw=t.raw;r140_attack_context=(id<CARD_COUNT&&cards[id].type==0);combat_fx_card=t.raw;
  if(preview_trace){preview_phase=t.depth>0&&t.depth<=2;preview_depth=t.depth;}
  switch(t.kind){
  case NT_PLAY:{
   if(id>=ASCENT_CARD_FIRST){resolve_card(g,-1,t.target,t.raw|CARD_FREE);break;}
   if(!ninja_has_living(g)||!ninja_can_play(g,t.raw)){
    if(!(t.raw&CARD_EPHEMERAL)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=ninja_clean(t.raw);
    break;
   }
   if(t.target<0||t.target>=g->enemy_count||g->enemies[t.target].hp<=0)t.target=random_living_foe(g);
   if(cards[id].type==1)g->ninja.v[NS_POUNCE]=0;
   if(cards[id].type==0)g->ic[IC_UNRELENTING]=0;
   r140_rust_targets=0;regent_foreign_begin(g,t.raw);
   int repeat=0;
   if(!(t.raw&CARD_EPHEMERAL)){
    if((g->character==CHAR_REGENT||g->revision184.custom))repeat+=g->regent.instance_replay[CARD_INSTANCE(t.raw)];
    if(cards[id].type==0){repeat+=CARD_MOD(t.raw)==MOD_ECHO;if(g->ic[IC_NEXT_ATTACK_REPEAT]>0){repeat++;g->ic[IC_NEXT_ATTACK_REPEAT]--;}}
    if(cards[id].type==1&&g->ninja.v[NS_BURST]>0){repeat++;g->ninja.v[NS_BURST]--;}
   }
   if(preview_trace&&t.depth==2)preview_trace->repeated++;
   for(int i=0;i<repeat;i++)ninja_push(g,NT_PLAY,t.raw|CARD_FREE|CARD_EPHEMERAL,t.target,t.n,0,t.depth?2:0);
   ninja_push(g,NT_FINISH,t.raw,t.target,g->ninja.v[NS_AFTERIMAGE],g->ninja.v[NS_SERPENT],t.depth);
   ninja_push(g,NT_EFFECT,t.raw,t.target,t.n,0,t.depth);
   break;}
  case NT_EFFECT:ninja_effect(g,t);break;
  case NT_FINISH:if(preview_trace)preview_phase=0;ninja_finish(g,t);break;
  case NT_DRAW:for(int i=0;i<t.n&&g->screen==COMBAT;i++)draw_card(g);break;
  case NT_DISCARD:ninja_open_choice(g,NC_DISCARD,MIN(t.n,g->hand_count),t.raw,t.target,t.depth);break;
  case NT_SHIVS:ninja_shivs(g,t.n,t.aux,0);break;
  case NT_DISCARD_ALL:{
   int batch[MAX_HAND],n=g->hand_count;memcpy(batch,g->hand,(size_t)n*sizeof(int));g->hand_count=0;
   if(t.n==1)ninja_push(g,NT_DRAW,t.raw,t.target,n,0,t.depth);
   else if(t.n==2)ninja_push(g,NT_SHIVS,t.raw,t.target,n,t.aux,t.depth);
   /* Effects execute in hand order after the whole discard leaves the hand. */
   for(int i=n-1;i>=0;i--)ninja_push(g,NT_AFTER_DISCARD,batch[i],t.target,0,0,t.depth);
   break;}
  case NT_AFTER_DISCARD:ninja_discard_effects(g,t.raw,t.depth);break;
  case NT_CHOOSE:ninja_open_choice(g,t.n,t.aux,t.raw,t.target,t.depth);break;
  case NT_ENDTURN:g->ninja.v[NS_RETAIN_READY]=1;game_end_turn(g);break;
  }
 }
 r140_attack_context=saved_attack;r150_card_raw=saved_raw;combat_fx_card=saved_card;r140_rust_targets=saved_rust;preview_phase=saved_phase;preview_depth=saved_depth;ninja_processing=0;
 if(g->screen==COMBAT&&g->ninja.task_count==0){if(g->hp<=0)player_death(g);if(g->screen==COMBAT)check_win(g);}
}
static void ninja_play_from_hand(Game*g,int index,int target){
 if(g->screen!=COMBAT||index<0||index>=g->hand_count||g->ninja.task_count)return;
 int raw=ninja_stamp(g,g->hand[index]),id=CARD_ID(raw),cost=game_card_cost_now(g,raw);
 if(!ninja_can_play(g,raw)){msg(g,id==N_GRAND_FINALE?"Your draw pile must be empty.":"Requires a multiplayer ally.");return;}
 if(cost==-1)cost=g->energy;
 if(cost<0||cost>g->energy){msg(g,"Not enough Energy.");return;}
 int x=game_card_cost(raw)==-1?g->energy:0;g->energy-=cost;
 for(int i=index;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];g->hand_count--;
 if(cards[id].type==1&&g->ninja.v[NS_POUNCE])g->ninja.v[NS_POUNCE]=0;
 if(cards[id].type==0&&g->ic[IC_UNRELENTING])g->ic[IC_UNRELENTING]=0;
 ninja_push(g,NT_PLAY,raw,target,x,0,preview_trace?1:0);ninja_process(g);
}
static void ninja_choose(Game*g,int index,int finish){
 if(g->screen!=NINJA_CHOICE)return;
 if(finish&&g->ninja.choice_kind!=NC_RETAIN){msg(g,"Choose a card to finish the effect.");return;}
 if(!finish&&!game_ninja_choice_eligible(g,index)){msg(g,"This card is not eligible.");return;}
 int kind=g->ninja.choice_kind;
 if(!finish){
  int raw=g->hand[index];
  if(kind==NC_DISCARD){g->ninja.choice_raws[g->ninja.choice_count++]=raw;for(int i=index;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];g->hand_count--;}
  else if(kind==NC_HAND_TRICK)g->hand[index]|=CARD_SLY_TURN;
  else if(kind==NC_NIGHTMARE){int n=g->ninja.nightmare_count;if(n<MAX_HAND){g->ninja.nightmare_raw[n]=ninja_clean(raw)&~(CARD_INSTANCE_MASK|CARD_SLY_TURN);g->ninja.nightmare_n[n]=3;g->ninja.nightmare_count++;}}
  else if(kind==NC_RETAIN)g->hand[index]|=CARD_RETAIN_TURN;
  g->ninja.choice_remaining--;
 }
 int first=-1;for(int i=0;i<g->hand_count;i++)if(game_ninja_choice_eligible(g,i)){first=i;break;}
 if(!finish&&g->ninja.choice_remaining>0&&first>=0){g->selection=MIN(index,g->hand_count-1);if(!game_ninja_choice_eligible(g,g->selection))g->selection=first;return;}
 int depth=g->ninja.choice_depth;
 if(kind==NC_DISCARD)for(int i=g->ninja.choice_count-1;i>=0;i--)ninja_push(g,NT_AFTER_DISCARD,g->ninja.choice_raws[i],g->ninja.choice_target,0,0,depth);
 g->ninja.choice_kind=g->ninja.choice_count=g->ninja.choice_remaining=0;g->screen=COMBAT;g->selection=0;ninja_process(g);
}
static int ninja_prepare_retain(Game*g){
 if(!g->ninja.v[NS_PLANS]||g->ninja.v[NS_RETAIN_READY]||g->ninja.task_count)return 0;
 int n=0;for(int i=0;i<g->hand_count;i++)n+=!game_card_is_retained(g,g->hand[i]);
 if(!n)return 0;
 ninja_push(g,NT_ENDTURN,0,0,0,0,0);ninja_open_choice(g,NC_RETAIN,MIN(n,g->ninja.v[NS_PLANS]),0,0,0);return g->screen==NINJA_CHOICE;
}
static void ninja_start_turn_before(Game*g){
 g->ninja.v[NS_RETAIN_READY]=0;g->ninja.v[NS_OPENING_DRAW]=1;
 g->ninja.v[NS_SHADOW_NOW]=MAX(0,g->ninja.v[NS_SHADOW_NOW]-1)+g->ninja.v[NS_SHADOW_NEXT];g->ninja.v[NS_SHADOW_NEXT]=0;
 g->ninja.v[NS_DISCARDS_TURN]=g->ninja.v[NS_SKILLS_TURN]=g->ninja.v[NS_SHIVS_TURN]=0;
 if(g->ninja.v[NS_WRAITH])g->dexterity-=g->ninja.v[NS_WRAITH];
 if(g->ninja.v[NS_NEXT_BLOCK]){gain_block(g,g->ninja.v[NS_NEXT_BLOCK]);g->ninja.v[NS_NEXT_BLOCK]=0;}
}
static void ninja_start_turn_after(Game*g){
 g->ninja.v[NS_OPENING_DRAW]=0;
 if(g->ninja.v[NS_FUMES])ninja_poison_all(g,g->ninja.v[NS_FUMES]);
 if(g->turn==1){if(relic(g,NR_FUNNEL))ninja_poison_all(g,4);if(relic(g,NR_SCROLL))ninja_shivs(g,3,0,0);}
 ninja_shivs(g,g->ninja.v[NS_BLADES],0,0);
 for(int i=0;i<g->ninja.nightmare_count;i++)for(int j=0;j<g->ninja.nightmare_n[i];j++)add_generated(g,g->ninja.nightmare_raw[i]&~CARD_INSTANCE_MASK);g->ninja.nightmare_count=0;
 if(g->ninja.v[NS_TOOLS]){ninja_push(g,NT_DISCARD,0,0,g->ninja.v[NS_TOOLS],0,0);ninja_push(g,NT_DRAW,0,0,g->ninja.v[NS_TOOLS],0,0);ninja_process(g);}
}
