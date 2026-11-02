/* Rift's independent implementation of the researched relic rules. See the
   versioned coverage manifest for the explicitly described solo adaptations. */
static int sr_is_strike(int raw){return has_strike_tag(cards[CARD_ID(raw)].name);}
static int sr_is_defend(int raw){int id=CARD_ID(raw);return id==1||id==N_DEFEND_SILENT||id==R_DEFEND_REGENT;}
static int sr_is_starter(int raw){int id=CARD_ID(raw);return id==0||id==N_STRIKE_SILENT||id==R_STRIKE_REGENT||sr_is_defend(raw);}
int game_relic_is_ancient(int id){for(unsigned i=0;i<sizeof(sr_catalog)/sizeof(*sr_catalog);i++)if(sr_catalog[i].id==id)return sr_catalog[i].rarity==5||id==SR_BLACK_BLOOD||id==SR_DIVINE_DESTINY||id==SR_RING_OF_THE_DRAKE;return 0;}
static int sr_allowed(const Game*g,int id){
 if(!ascent_card_available(g,id))return 0;
 if(SU(g,SU_FOREIGN))return card_character(id)>=0&&card_character(id)!=g->character;
 if(relic(g,SR_PRISMATIC_GEM))return 1;
 if(relic(g,SR_DINGY_RUG)&&id>=COLORLESS_FIRST&&id<EXTRA_LAST)return 1;
 return ninja_owned_card(g,id);
}
static int sr_eligible_relic(const Game*g,int id,int ancient){
 for(unsigned i=0;i<sizeof(sr_catalog)/sizeof(*sr_catalog);i++)if(sr_catalog[i].id==id){
  if(sr_catalog[i].character>=0&&sr_catalog[i].character!=g->character&&!g->revision184.custom)return 0;
  if(ancient){if(id==SR_BLACK_BLOOD&&g->character!=0)return 0;if(id==SR_RING_OF_THE_DRAKE&&g->character!=1)return 0;if(id==SR_DIVINE_DESTINY&&g->character!=2)return 0;return game_relic_is_ancient(id);}
  return sr_catalog[i].rarity>=1&&sr_catalog[i].rarity<=3;
 }
 return !ancient&&((id>=1&&id<=27)||(id>=40&&id<44)||(id>=53&&id<63)||(id>=64&&id<74&&(g->ascent.relics&(1u<<(id-64))))||(id>=74&&id<94));
}
static int sr_counter(Game*g,int id,int period){SC(g,id)=(SC(g,id)+1)%period;return SC(g,id)==0;}
static void sr_draw(Game*g,int n){for(int i=0;i<MIN(MAX_HAND,n);i++)draw_card(g);}
static void sr_proc(Game*g,int target,int n){ninja_proc(g,target,n,0);}
static void sr_all(Game*g,int n){for(int i=0;i<g->enemy_count;i++)sr_proc(g,i,n);}
static void sr_maxhp(Game*g,int n){g->maxhp=MAX(1,g->maxhp+n);g->hp=CLAMP(g->hp+MAX(0,n),0,g->maxhp);}
static void sr_strength(Game*g,int n){int old=g->strength;g->strength=MIN(1000000,g->strength+n);SB(g,SB_STR_BASE)=old;sr_after_strength(g);}
static void sr_after_strength(Game*g){
 int delta=g->strength-SB(g,SB_STR_BASE);
 if(delta>0&&relic(g,SR_RUINED_HELMET)&&!SB(g,SB_RUINED)){SB(g,SB_RUINED)=1;g->strength=MIN(1000000,g->strength+delta*relic(g,SR_RUINED_HELMET));}
 SB(g,SB_STR_BASE)=g->strength;
}
int game_potion_id(const Game*g,int i){return i<0||i>=11?-1:i<3?g->potions[i]:g->revision186.extra_potions[i-3];}
static int*sr_potion_slot(Game*g,int i){return i<3?&g->potions[i]:&g->revision186.extra_potions[i-3];}
int game_potion_capacity(const Game*g){return MIN(11,3+2*relic(g,SR_POTION_BELT)+4*relic(g,SR_ALCHEMICAL_COFFER)+relic(g,SR_PHIAL_HOLSTER));}
static void sr_brew(Game*g,int n,int id){if(relic(g,SR_SOZU))return;for(int i=0;i<n&&g->potion_count<game_potion_capacity(g);i++)*sr_potion_slot(g,g->potion_count++)=id<0?ascent_random_potion(g):id;}
static int sr_cost(const Game*g,int raw,int cost){
 if(!g||cost<0||!game_card_is_playable(raw))return cost;
 int id=CARD_ID(raw);
 if((raw&CARD_FREE)||(relic(g,SR_NUTRITIOUS_SOUP)&&sr_is_strike(raw)))return 0;
 if(relic(g,SR_BRILLIANT_SCARF)&&SB(g,SB_HAND_PLAYS)%5==4)return 0;
 if(SU(g,SU_IMBUED)==id+1&&g->turn==1)return 0;
 int k=CARD_INSTANCE(raw);if(k&&g->revision186.card_cost[k])cost=g->revision186.card_cost[k]-1;
 if(relic(g,SR_PAELS_CLAW)&&sr_is_defend(raw))cost=MAX(0,cost-relic(g,SR_PAELS_CLAW));
 if(cards[id].type==2)cost+=relic(g,SR_SPIKED_GAUNTLETS);
 return MIN(99,cost);
}
static int sr_can_play(const Game*g){return !relic(g,SR_VELVET_CHOKER)||SB(g,SB_PLAYS)<6;}
static int sr_x(const Game*g){return 2*relic(g,SR_CHEMICAL_X);}
static int sr_repeat(Game*g,int raw){return !(raw&CARD_EPHEMERAL)&&!SB(g,SB_PLAYS)?relic(g,SR_THROWING_AXE):0;}
static void sr_paid(Game*g,int raw,int stars){
 if(!g->ic[IC_CHAIN_DEPTH]&&!regent_processing&&!ninja_processing)SB(g,SB_HAND_PLAYS)++;
 int k=CARD_INSTANCE(raw);if(k)g->revision186.card_cost[k]=0;
 if(stars>0){
  if(relic(g,SR_MINI_REGENT)&&!SB(g,SB_IMBUED)){SB(g,SB_IMBUED)=1;sr_strength(g,relic(g,SR_MINI_REGENT));}
  if(relic(g,SR_GALACTIC_DUST)){int sum=SC(g,SR_GALACTIC_DUST)+stars;SC(g,SR_GALACTIC_DUST)=sum%10;relic_block(g,(sum/10)*10*relic(g,SR_GALACTIC_DUST));}
 }
}
static void sr_play_begin(Game*g,int raw){
 SB(g,SB_PLAYS)++;
 SB(g,SB_STR_BASE)=g->strength;
 if(cards[CARD_ID(raw)].type==0){SB(g,SB_ATTACKS)++;SB(g,SB_PEN)=relic(g,SR_PEN_NIB)&&sr_counter(g,SR_PEN_NIB,10);}
 if(cards[CARD_ID(raw)].type==1)SB(g,SB_SKILLS)++;
}
static void sr_create_ethereal(Game*g,int raw){
 int h=g->hand_count,d=g->discard_count;add_generated(g,raw|CARD_FORCE_EXHAUST);
 int made=g->hand_count>h?g->hand[g->hand_count-1]:g->discard_count>d?g->discard[g->discard_count-1]:-1;
 if(made>=0){int k=CARD_INSTANCE(made);if(k)g->revision186.ethereal[k/8]|=1u<<(k%8);}
}
static void sr_play_finish(Game*g,int raw){
 int id=CARD_ID(raw),type=cards[id].type,cost=game_card_cost(raw),firstpower=type==2&&!SB(g,SB_FIRST_POWER);
 sr_after_strength(g);
 if(type==0){
  if(relic(g,SR_DAUGHTER_OF_THE_WIND))relic_block(g,relic(g,SR_DAUGHTER_OF_THE_WIND));
  if(relic(g,SR_NUNCHAKU)&&sr_counter(g,SR_NUNCHAKU,10))gain_energy(g,relic(g,SR_NUNCHAKU));
  if(SB(g,SB_ATTACKS)%3==0){g->dexterity+=relic(g,SR_KUNAI);relic_block(g,4*relic(g,SR_ORNAMENTAL_FAN));if(relic(g,SR_KUSARIGAMA))sr_proc(g,random_living_foe(g),6*relic(g,SR_KUSARIGAMA));}
  if(SB(g,SB_ATTACKS)==1&&relic(g,SR_MUSIC_BOX))for(int n=0;n<relic(g,SR_MUSIC_BOX);n++)sr_create_ethereal(g,raw&~CARD_INSTANCE_MASK);
  if(relic(g,SR_BONE_FLUTE)&&CARD_INSTANCE(raw)&&(g->revision186.generated[CARD_INSTANCE(raw)/8]&(1u<<(CARD_INSTANCE(raw)%8))))relic_block(g,2*relic(g,SR_BONE_FLUTE));
 }
 if(type==1){if(relic(g,SR_TUNING_FORK)&&sr_counter(g,SR_TUNING_FORK,10))relic_block(g,7*relic(g,SR_TUNING_FORK));if(SB(g,SB_SKILLS)%3==0)sr_all(g,5*relic(g,SR_LETTER_OPENER));}
 if(type==2){
  SB(g,SB_FIRST_POWER)=1;
  if(firstpower)relic_block(g,7*relic(g,SR_PERMAFROST));
  sr_draw(g,relic(g,SR_GAME_PIECE));sr_all(g,8*relic(g,SR_LOST_WISP));
  for(int n=0;n<relic(g,SR_MUMMIFIED_HAND);n++){int eligible[MAX_HAND],count=0;for(int i=0;i<g->hand_count;i++)if(game_card_cost_now(g,g->hand[i])>0)eligible[count++]=i;if(count)g->hand[eligible[rnd(g,count)]]|=CARD_FREE;}
 }
 if(type<=2){int before=SB(g,SB_TYPES);SB(g,SB_TYPES)|=1<<type;if(before!=7&&SB(g,SB_TYPES)==7){sr_strength(g,relic(g,SR_RAINBOW_RING));g->dexterity+=relic(g,SR_RAINBOW_RING);}}
 if(relic(g,SR_IRON_CLUB)&&sr_counter(g,SR_IRON_CLUB,4))sr_draw(g,relic(g,SR_IRON_CLUB));
 if(cost>=2)relic_block(g,4*relic(g,SR_INTIMIDATING_HELMET));if(cost>=3)gain_energy(g,relic(g,SR_IVORY_TILE));
 if(type<2&&!(raw&CARD_EPHEMERAL))SB(g,SB_LAST)=(raw&~(CARD_FREE|CARD_EPHEMERAL|CARD_FORCE_EXHAUST))+1;
 if(relic(g,SR_RAZOR_TOOTH)&&type<2){int k=CARD_INSTANCE(raw),*p[]={g->hand,g->draw,g->discard,g->exhaust};int ns[]={g->hand_count,g->draw_count,g->discard_count,g->exhaust_count};for(int z=0;z<4;z++)for(int i=0;i<ns[z];i++)if((k&&CARD_INSTANCE(p[z][i])==k)||(!k&&p[z][i]==raw)){p[z][i]|=UPBIT;goto upgraded;}}
 upgraded:sr_kills(g);
 if(relic(g,SR_UNCEASING_TOP)&&!g->hand_count&&g->screen==COMBAT&&g->ic[IC_OWN_TURN]&&!SB(g,SB_TOP_GUARD)){SB(g,SB_TOP_GUARD)=1;sr_draw(g,relic(g,SR_UNCEASING_TOP));SB(g,SB_TOP_GUARD)=0;}
}
static void sr_drawn(Game*g,int*raw){
 int k=CARD_INSTANCE(*raw);
 if(k&&(relic(g,SR_SNECKO_EYE)||relic(g,SR_FAKE_SNECKO_EYE))&&game_card_cost(*raw)>=0)g->revision186.card_cost[k]=1+rnd(g,4);
 if(g->turn==1&&SB(g,SB_OPENING)&&(relic(g,SR_BELLOWS)||SC(g,SR_BONE_TEA))) *raw|=UPBIT;
}
static void sr_created(Game*g,int raw){
 int k=CARD_INSTANCE(raw);if(k)g->revision186.generated[k/8]|=1u<<(k%8);
 if(relic(g,SR_REGALITE))relic_block(g,2*relic(g,SR_REGALITE));
 if(++SB(g,SB_CREATED)==7)sr_all(g,30*relic(g,SR_METRONOME));
}
static void sr_shuffle(Game*g){
 if(SB(g,SB_RESHUFFLE))return;SB(g,SB_RESHUFFLE)=1;
 relic_block(g,6*relic(g,SR_THE_ABACUS));
 for(int n=0;n<relic(g,SR_BIIIG_HUG)&&g->draw_count<MAX_PILE;n++)g->draw[g->draw_count++]=52;
 SB(g,SB_RESHUFFLE)=0;
}
static void sr_exhaust(Game*g,int raw){
 if(relic(g,SR_CHARONS_ASHES))sr_all(g,3*relic(g,SR_CHARONS_ASHES));
 if(relic(g,SR_FORGOTTEN_SOUL))sr_proc(g,random_living_foe(g),relic(g,SR_FORGOTTEN_SOUL));
 if(relic(g,SR_JOSS_PAPER)&&sr_counter(g,SR_JOSS_PAPER,5))sr_draw(g,relic(g,SR_JOSS_PAPER));
 if(cards[CARD_ID(raw)].type==1&&relic(g,SR_BURNING_STICKS)&&!SB(g,SB_STICKS)){SB(g,SB_STICKS)=1;for(int i=0;i<relic(g,SR_BURNING_STICKS);i++)add_generated(g,raw&~CARD_INSTANCE_MASK);}
}
static int sr_hp_loss(Game*g,int n){
 n=MAX(0,n-relic(g,SR_TUNGSTEN_ROD));
 if(relic(g,SR_BEATING_REMNANT)){int limit=MAX(2,22-2*relic(g,SR_BEATING_REMNANT));n=MIN(n,MAX(0,limit-SB(g,SB_HP_TURN)));}
 return n;
}
static void sr_lost_hp(Game*g,int n){
 if(n<=0)return;SB(g,SB_HP_TURN)+=n;SB(g,SB_HP_COMBAT)+=n;SB(g,SB_NEXT_BLOCK)+=3*relic(g,SR_SELF_FORMING_CLAY);
 if(relic(g,SR_DEMON_TONGUE)&&g->ic[IC_OWN_TURN]&&!SB(g,SB_DEMON)){SB(g,SB_DEMON)=1;heal(g,n*relic(g,SR_DEMON_TONGUE));}
 if(relic(g,SR_CENTENNIAL_PUZZLE)&&!SB(g,SB_PUZZLE)){SB(g,SB_PUZZLE)=1;if(g->hp>0)sr_draw(g,3*relic(g,SR_CENTENNIAL_PUZZLE));}
}
static int sr_attack_bonus(const Game*g,int raw){
 int n=0,id=CARD_ID(raw);
 if(sr_is_strike(raw))n+=3*relic(g,SR_STRIKE_DUMMY)+relic(g,SR_FAKE_STRIKE_DUMMY);
 if(CARD_UP(raw))n+=3*relic(g,SR_MINIATURE_CANNON);
 if(CARD_MOD(raw))n+=9*relic(g,SR_MYSTIC_LIGHTER);
 if(g->hp*2<=g->maxhp)n+=3*relic(g,SR_RED_SKULL);
 if(SB(g,SB_ATTACKS)==1)n+=8*relic(g,SR_AKABEKO);
 (void)id;return n;
}
static int sr_block_amount(Game*g,int n){
 if(!g->potion_count)n+=2*relic(g,SR_BELT_BUCKLE);
 if(relic(g,SR_VAMBRACE)&&!SB(g,SB_VAMBRACE)){SB(g,SB_VAMBRACE)=1;n*=1+relic(g,SR_VAMBRACE);}
 if(relic(g,SR_PAELS_LEGION)&&g->turn>=SB(g,SB_LEGION)){SB(g,SB_LEGION)=g->turn+3;n*=1+relic(g,SR_PAELS_LEGION);}
 if(strstr(cards[CARD_ID(r150_card_raw)].name,"Minion")&&relic(g,SR_VITRUVIAN_MINION))n*=1+relic(g,SR_VITRUVIAN_MINION);
 return MIN(1000000,n);
}
static void sr_kills(Game*g){
 if(SB(g,SB_KILL_GUARD))return;SB(g,SB_KILL_GUARD)=1;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp<=0&&!(SB(g,SB_DEAD)&(1<<i))){SB(g,SB_DEAD)|=1<<i;gain_energy(g,relic(g,SR_GREMLIN_HORN));sr_draw(g,relic(g,SR_GREMLIN_HORN));}
 SB(g,SB_KILL_GUARD)=0;
}
static void sr_upgrade(Game*g,int n,int type){
 for(int j=0;j<n;j++){int ix[MAX_DECK],count=0;for(int i=0;i<g->deck_count;i++)if(game_card_is_upgradeable(g->deck[i])&&(type<0||cards[CARD_ID(g->deck[i])].type==type))ix[count++]=i;if(!count)break;g->deck[ix[rnd(g,count)]]|=UPBIT;}
}
static int sr_new_card(const Game*g,int raw){
 int type=cards[CARD_ID(raw)].type;
 if((type==0&&relic(g,SR_MOLTEN_EGG))||(type==1&&relic(g,SR_TOXIC_EGG))||(type==2&&relic(g,SR_FROZEN_EGG)))raw|=UPBIT;
 if(relic(g,SR_FRESNEL_LENS)&&!CARD_MOD(raw)&&game_can_modify(raw,MOD_WARD)&&strstr(card_description(raw),"Block"))raw|=MOD_WARD<<CARD_MOD_SHIFT;
 return raw;
}
static void sr_added(Game*g,int raw){
 int count=1+relic(g,SR_BING_BONG);count=MIN(count,MAX_DECK-g->deck_count+1);
 for(int i=1;i<count;i++)g->deck[g->deck_count++]=raw;
 for(int i=0;i<count;i++){
  sr_gain_gold(g,15*relic(g,SR_LUCKY_FYSH));
  if(relic(g,SR_BOOK_OF_FIVE_RINGS)&&sr_counter(g,SR_BOOK_OF_FIVE_RINGS,5))heal(g,20*relic(g,SR_BOOK_OF_FIVE_RINGS));
  if(CARD_ID(raw)>=50&&CARD_ID(raw)<=53)sr_maxhp(g,6*relic(g,SR_DARKSTONE_PERIAPT));
 }
}
static void sr_core(Game*g){
 int disk=relic(g,SR_DATA_DISK),copies=relic(g,SR_CRACKED_CORE),infused=relic(g,SR_INFUSED_CORE),virus=relic(g,SR_SYMBIOTIC_VIRUS);
 int extra=(copies||infused||virus)?3*relic(g,SR_RUNIC_CAPACITOR)+relic(g,SR_GOLD_PLATED_CABLES):0;
 for(int i=0;i<MIN(96,copies+3*infused+(!virus?extra:0));i++)sr_proc(g,random_living_foe(g),(infused?4:3)+disk);
 for(int i=0;i<MIN(96,virus+(virus?extra:0));i++)sr_proc(g,random_living_foe(g),6*MAX(1,g->turn)+disk);
}
static void sr_battle(Game*g){
 memset(g->revision186.battle,0,sizeof(g->revision186.battle));memset(g->revision186.card_cost,0,sizeof(g->revision186.card_cost));memset(g->revision186.generated,0,sizeof(g->revision186.generated));memset(g->revision186.ethereal,0,sizeof(g->revision186.ethereal));
 g->energy=0;
 sr_strength(g,relic(g,SR_VAJRA)+3*relic(g,SR_SWORD_OF_JADE)+(g->battle_type==NODE_ELITE?2*relic(g,SR_SLING_OF_COURAGE):0));
 g->dexterity+=relic(g,SR_ODDLY_SMOOTH_STONE);
 relic_block(g,10*relic(g,SR_ANCHOR)+4*relic(g,SR_FAKE_ANCHOR)+5*relic(g,SR_PHYLACTERY_UNBOUND));
 heal(g,2*relic(g,SR_BLOOD_VIAL)+relic(g,SR_FAKE_BLOOD_VIAL)+(g->battle_type==NODE_BOSS?25*relic(g,SR_PANTOGRAPH):0));
 g->ic[IC_PLATING]+=4*relic(g,SR_GORGET);
 for(int i=0;i<g->enemy_count;i++){g->enemies[i].weak+=relic(g,SR_RED_MASK);g->enemies[i].vulnerable+=relic(g,SR_BAG_OF_MARBLES);g->enemies[i].strength+=relic(g,SR_PHILOSOPHERS_STONE);}
 if(relic(g,SR_ROYAL_POISON))lose_hp(g,4*relic(g,SR_ROYAL_POISON));
 if(relic(g,SR_FESTIVE_POPPER))sr_all(g,9*relic(g,SR_FESTIVE_POPPER));
 if(relic(g,SR_DELICATE_FROND))sr_brew(g,game_potion_capacity(g),-1);
 sr_brew(g,relic(g,SR_PETRIFIED_TOAD),2);
 if(SC(g,SR_EMBER_TEA)){SC(g,SR_EMBER_TEA)--;sr_strength(g,2*relic(g,SR_EMBER_TEA));}
 int dazed=3*relic(g,SR_BLESSED_ANTLER)+(SC(g,SR_TEA_OF_DISCOURTESY)?2:0);SC(g,SR_TEA_OF_DISCOURTESY)=0;
 for(int i=0;i<dazed&&g->draw_count<MAX_PILE;i++)g->draw[g->draw_count++]=52;
 for(int i=0;i<3*relic(g,SR_FUNERARY_MASK)&&g->draw_count<MAX_PILE;i++)g->draw[g->draw_count++]=regent_stamp(g,C_FINESSE|CARD_INKY|CARD_FORCE_EXHAUST);
 if(dazed||relic(g,SR_FUNERARY_MASK))shuffle(g,g->draw,g->draw_count);
 for(int j=0;j<2*relic(g,SR_STONE_CRACKER);j++){int ix[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(game_card_is_upgradeable(g->draw[i]))ix[n++]=i;if(n)g->draw[ix[rnd(g,n)]]|=UPBIT;}
 if(relic(g,SR_FUR_COAT)&&SC(g,SR_FUR_COAT)&&g->battle_type==NODE_FIGHT){SC(g,SR_FUR_COAT)--;for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp=1;}
 if(SU(g,SU_IMBUED)){for(int i=0;i<g->draw_count;i++)if(CARD_ID(g->draw[i])+1==SU(g,SU_IMBUED)){int raw=g->draw[i];g->draw[i]=g->draw[g->draw_count-1];g->draw[g->draw_count-1]=raw;break;}}
 SB(g,SB_TEA_ENERGY)=SC(g,SR_VENERABLE_TEA_SET)+SC(g,SR_FAKE_VENERABLE_TEA_SET);SC(g,SR_VENERABLE_TEA_SET)=SC(g,SR_FAKE_VENERABLE_TEA_SET)=0;
}
static void sr_turn_before(Game*g){
 SB(g,SB_HP_PREV)=SB(g,SB_HP_TURN);SB(g,SB_HP_TURN)=0;
 SB(g,SB_PLAYS)=SB(g,SB_ATTACKS)=SB(g,SB_SKILLS)=SB(g,SB_TYPES)=SB(g,SB_HAND_PLAYS)=SB(g,SB_DEMON)=SB(g,SB_IMBUED)=0;
 SB(g,SB_OPENING)=1;SB(g,SB_STR_BASE)=g->strength;
 if(relic(g,SR_HAPPY_FLOWER))SC(g,SR_HAPPY_FLOWER)=(SC(g,SR_HAPPY_FLOWER)+1)%3;
 if(relic(g,SR_PENDULUM))SC(g,SR_PENDULUM)=(SC(g,SR_PENDULUM)+1)%3;
 if(relic(g,SR_FAKE_HAPPY_FLOWER))SC(g,SR_FAKE_HAPPY_FLOWER)=(SC(g,SR_FAKE_HAPPY_FLOWER)+1)%5;
 if(relic(g,SR_POLLINOUS_CORE))SC(g,SR_POLLINOUS_CORE)=(SC(g,SR_POLLINOUS_CORE)+1)%4;
 if(relic(g,SR_BRIMSTONE)){sr_strength(g,2*relic(g,SR_BRIMSTONE));for(int i=0;i<g->enemy_count;i++)g->enemies[i].strength+=relic(g,SR_BRIMSTONE);}
}
static int sr_turn_energy(Game*g){
 int n=relic(g,SR_BLESSED_ANTLER)+relic(g,SR_BLOOD_SOAKED_ROSE)+relic(g,SR_ECTOPLASM)+relic(g,SR_PRISMATIC_GEM)+relic(g,SR_PHILOSOPHERS_STONE)+relic(g,SR_SOZU)+relic(g,SR_SPIKED_GAUNTLETS)+relic(g,SR_VELVET_CHOKER)+relic(g,SR_WHISPERING_EARRING);
 if(SC(g,SR_PUMPKIN_CANDLE))n+=relic(g,SR_PUMPKIN_CANDLE);
 n+=(g->turn==1?-1:1)*relic(g,SR_BREAD);
 if(g->turn>=3)n+=relic(g,SR_PAELS_FLESH);
 if(!SC(g,SR_HAPPY_FLOWER))n+=relic(g,SR_HAPPY_FLOWER);
 if(!SC(g,SR_FAKE_HAPPY_FLOWER))n+=relic(g,SR_FAKE_HAPPY_FLOWER);
 if(g->turn==1)n+=4*relic(g,SR_VERY_HOT_COCOA)+SB(g,SB_TEA_ENERGY)+(g->battle_type==NODE_ELITE?relic(g,SR_BOOMING_CONCH):0);
 if(g->turn==2)n+=2*relic(g,SR_CANDELABRA);if(g->turn==3)n+=3*relic(g,SR_CHANDELIER);
 int buy=MIN(relic(g,SR_SEAL_OF_GOLD),g->gold/5);g->gold-=5*buy;n+=buy;
 n+=SB(g,SB_NEXT_ENERGY);SB(g,SB_NEXT_ENERGY)=0;
 if(relic(g,SR_ICE_CREAM)){n+=MAX(0,SB(g,SB_CARRY)-g->ic[IC_HOURGLASS]);SB(g,SB_CARRY)=0;}
 return n;
}
static int sr_turn_draw(Game*g){
 int n=relic(g,SR_PAELS_BLOOD)+2*relic(g,SR_FIDDLE)+2*relic(g,SR_SNECKO_EYE);
 if(!SC(g,SR_PENDULUM))n+=relic(g,SR_PENDULUM);
 if(!SC(g,SR_POLLINOUS_CORE))n+=2*relic(g,SR_POLLINOUS_CORE);
 if(g->turn==1)n-=2*relic(g,SR_BIG_MUSHROOM);
 if(g->turn==1&&g->battle_type==NODE_ELITE)n+=2*relic(g,SR_BOOMING_CONCH);
 n+=SB(g,SB_NEXT_DRAW);SB(g,SB_NEXT_DRAW)=0;return n;
}
static int sr_random_card(Game*g,int type,int colorless){
 int pool[512],n=0;for(int id=3;id<CARD_COUNT;id++)if(combat_reward_eligible(id)&&ascent_card_available(g,id)&&(type<0||cards[id].type==type)&&(colorless?(id>=COLORLESS_FIRST&&id<EXTRA_LAST):sr_allowed(g,id)))pool[n++]=id;
 return n?pool[rnd(g,n)]:C_FINESSE;
}
static void sr_pending(Game*g,int kind,int n,int arg){
 /* Only one pickup can request an interactive selection at a time; ordinary
    random relics never request one. Counts are serialized through each pick. */
 SU(g,SU_PICK)=kind;SU(g,SU_REMAIN)=n;SU(g,SU_ARG)=arg;
}
static void sr_turn_after(Game*g){
 relic_block(g,7*relic(g,SR_SAI)+SB(g,SB_NEXT_BLOCK));SB(g,SB_NEXT_BLOCK)=0;
 if(g->turn==2)relic_block(g,14*relic(g,SR_HORN_CLEAT));if(g->turn==3){relic_block(g,18*relic(g,SR_CAPTAINS_WHEEL));sr_strength(g,relic(g,SR_SPARKLING_ROUGE));g->dexterity+=relic(g,SR_SPARKLING_ROUGE);}
 int phyl=relic(g,SR_BOUND_PHYLACTERY)+2*relic(g,SR_PHYLACTERY_UNBOUND);relic_block(g,phyl);g->regent.vigor+=phyl;
 sr_all(g,3*relic(g,SR_MERCURY_HOURGLASS)+g->turn*relic(g,SR_MR_STRUGGLES));
 if(SB(g,SB_HP_PREV)&&relic(g,SR_EMOTION_CHIP))for(int i=0;i<relic(g,SR_EMOTION_CHIP);i++)sr_core(g);
 for(int i=0;i<relic(g,SR_TOASTY_MITTENS)&&g->draw_count;i++){int raw=g->draw[--g->draw_count];exhaust_card(g,raw);sr_strength(g,1);}
 for(int i=0;i<relic(g,SR_CROSSBOW);i++)add_generated(g,sr_random_card(g,0,0)|CARD_FREE);
 if(g->turn==1){
  SC(g,SR_BONE_TEA)=0;
  if(relic(g,SR_DIVINE_DESTINY))regent_gain_stars(g,6*relic(g,SR_DIVINE_DESTINY));
  if(relic(g,SR_FENCING_MANUAL))regent_forge(g,10*relic(g,SR_FENCING_MANUAL));
  for(int i=0;i<2*relic(g,SR_ORANGE_DOUGH);i++)add_generated(g,sr_random_card(g,-1,1));
  for(int i=0;i<2*relic(g,SR_BIG_HAT);i++)sr_create_ethereal(g,sr_random_card(g,-1,1));
  for(int i=0;i<relic(g,SR_RADIANT_PEARL);i++)sr_create_ethereal(g,C_PREP_TIME|CARD_FREE);
  for(int i=0;i<relic(g,SR_VEXING_PUZZLEBOX);i++)add_generated(g,sr_random_card(g,-1,0)|CARD_FREE);
  for(int pass=0;pass<2;pass++){int limit=pass?2*relic(g,SR_POWER_CELL):relic(g,SR_JEWELED_MASK);for(int j=0;j<limit&&g->hand_count<MAX_HAND;j++){
   int ix[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(pass?game_card_cost(g->draw[i])==0:cards[CARD_ID(g->draw[i])].type==2)ix[n++]=i;
   if(n){int raw=regent_take(g->draw,&g->draw_count,ix[rnd(g,n)]);if(!pass)g->revision186.card_cost[CARD_INSTANCE(raw)]=1;regent_to_hand(g,raw);}
  }}
  if(relic(g,SR_GAMBLING_CHIP)){int count=0;for(int i=g->hand_count-1;i>=0;i--)if(!game_card_is_playable(g->hand[i])||(sr_is_starter(g->hand[i])&&!CARD_UP(g->hand[i]))){int raw=regent_take(g->hand,&g->hand_count,i);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw;count++;ninja_discard_effects(g,raw,0);}sr_draw(g,count);}
  if(relic(g,SR_TOOLBOX)||relic(g,SR_CHOICES_PARADOX))sr_pending(g,SP_OPENING,relic(g,SR_TOOLBOX)+relic(g,SR_CHOICES_PARADOX),!!relic(g,SR_CHOICES_PARADOX));
 }else if(relic(g,SR_HISTORY_COURSE)&&SB(g,SB_LAST)){
  int raw=SB(g,SB_LAST)-1;for(int i=0;i<relic(g,SR_HISTORY_COURSE)&&sr_can_play(g);i++)autoplay_card(g,raw|CARD_EPHEMERAL,0);
 }
 SB(g,SB_OPENING)=0;sr_after_strength(g);sr_kills(g);
 if(g->turn==1&&relic(g,SR_WHISPERING_EARRING)){SB(g,SB_EARRING)=1;}
}
static int sr_end_turn(Game*g){
 /* Inspect the hand before the normal discard step. Status/Ethereal cleanup
    is still owned by game_end_turn, including when Pael grants an extra turn. */
 int extra=relic(g,SR_PAELS_EYE)&&!SB(g,SB_PLAYS)&&!SB(g,SB_EYE);
 if(extra){SB(g,SB_EYE)=1;for(int i=g->hand_count-1;i>=0;i--){int raw=regent_take(g->hand,&g->hand_count,i);exhaust_card(g,raw);}}
 if(relic(g,SR_CLOAK_CLASP))relic_block(g,g->hand_count*relic(g,SR_CLOAK_CLASP));
 if(!SB(g,SB_ATTACKS))relic_block(g,4*relic(g,SR_RIPPLE_BASIN));
 if(!g->block)relic_block(g,6*relic(g,SR_ORICHALCUM)+3*relic(g,SR_FAKE_ORICHALCUM));
 if(g->block>=10&&relic(g,SR_PARRYING_SHIELD))sr_proc(g,random_living_foe(g),6*relic(g,SR_PARRYING_SHIELD));
 if(!g->hand_count)sr_all(g,20*relic(g,SR_SCREAMING_FLAGON));
 if(g->turn==7)sr_all(g,52*relic(g,SR_STONE_CALENDAR));
 regent_gain_stars(g,relic(g,SR_LUNAR_PASTRY));sr_core(g);
 if(SB(g,SB_PLAYS)<=3)SB(g,SB_NEXT_DRAW)+=3*relic(g,SR_POCKETWATCH);
 if(g->energy>0)SB(g,SB_NEXT_ENERGY)+=relic(g,SR_PAELS_TEARS);
 for(int i=0;i<relic(g,SR_BOOKMARK);i++){int ix[MAX_HAND],n=0;for(int k=0;k<g->hand_count;k++)if(game_card_is_retained(g,g->hand[k])&&game_card_cost_now(g,g->hand[k])>0)ix[n++]=k;if(n){int k=CARD_INSTANCE(g->hand[ix[rnd(g,n)]]);if(k)g->revision186.card_cost[k]=1;}}
 SB(g,SB_CARRY)=CLAMP(g->energy,0,99);sr_kills(g);return extra;
}
static int sr_special_relic(Game*g,int rarity,int excluded){
 int pool[384],n=0;
 if(g->relic_count>=MAX_RELICS)return -1;
 for(unsigned i=0;i<sizeof(sr_catalog)/sizeof(*sr_catalog);i++){
  int id=sr_catalog[i].id;
  if(sr_catalog[i].rarity!=rarity||id==excluded||relic(g,id))continue;
  if(sr_catalog[i].character>=0&&sr_catalog[i].character!=g->character)continue;
  pool[n++]=id;
 }
 return n?pool[rnd(g,n)]:random_relic_except(g,excluded);
}
static void sr_random_relics(Game*g,int n){for(int i=0;i<MIN(n,MAX_RELICS);i++){int id=random_relic(g);if(id>=0)add_relic(g,id);}}
static void sr_replace_starter(Game*g,int old,int newer){int found=0;for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==old){*relic_slot(g,i)=newer;found=1;break;}if(!found)add_relic(g,newer);}
static void sr_pickup(Game*g,int id){
 switch(id){
 case SR_ALCHEMICAL_COFFER:sr_brew(g,4,-1);break;
 case SR_POTION_BELT:break;case SR_PHIAL_HOLSTER:sr_brew(g,2,-1);break;
 case SR_ARCANE_SCROLL:sr_pending(g,SP_REWARD,1,2);break;
 case SR_ARCHAIC_TOOTH:sr_pending(g,SP_ANCIENT_STARTER,1,0);break;
 case SR_ASTROLABE:sr_pending(g,SP_TRANSFORM,3,1);break;
 case SR_BEAUTIFUL_BRACELET:case SR_KIFUDA:sr_pending(g,SP_FORGE,3,MOD_BOUND);break;
 case SR_BIG_MUSHROOM:sr_maxhp(g,20);break;
 case SR_BIIIG_HUG:sr_pending(g,SP_REMOVE,4,0);break;
 case SR_BLACK_BLOOD:for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==SR_BURNING_BLOOD){for(int j=i;j+1<g->relic_count;j++)*relic_slot(g,j)=game_relic_id(g,j+1);g->relic_count--;break;}break;
 case SR_BLOOD_SOAKED_ROSE:add_card(g,51);break;
 case SR_BONE_TEA:SC(g,id)++;break;
 case SR_BYRDPIP:add_card(g,R_MINION_STRIKE|UPBIT);break;
 case SR_CALLING_BELL:sr_random_relics(g,3);add_card(g,51);break;
 case SR_CAULDRON:sr_brew(g,5,-1);break;
 case SR_CLAWS:sr_pending(g,SP_TRANSFORM,6,2);break;
 case SR_CURSED_PEARL:sr_gain_gold(g,333);add_card(g,51);break;
 case SR_DISTINGUISHED_CAPE:sr_maxhp(g,-9);for(int i=0;i<3;i++)add_card(g,C_PANIC_BUTTON|CARD_INKY|CARD_FORCE_EXHAUST);break;
 case SR_DIVINE_DESTINY:for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==SR_DIVINE_RIGHT){for(int j=i;j+1<g->relic_count;j++)*relic_slot(g,j)=game_relic_id(g,j+1);g->relic_count--;break;}break;
 case SR_DOLLYS_MIRROR:sr_pending(g,SP_COPY,1,0);break;
 case SR_DUSTY_TOME:sr_pending(g,SP_REWARD,1,3);break;
 case SR_ELECTRIC_SHRYMP:sr_pending(g,SP_FORGE,1,64);break;
 case SR_EMBER_TEA:SC(g,id)+=5;break;
 case SR_EMPTY_CAGE:sr_pending(g,SP_REMOVE,2,0);break;
 case SR_FAKE_LEES_WAFFLE:heal(g,MAX(1,g->maxhp/10));break;
 case SR_FAKE_MANGO:sr_maxhp(g,3);break;
 case SR_FRAGRANT_MUSHROOM:lose_hp(g,15);sr_upgrade(g,2,-1);break;
 case SR_FUR_COAT:SC(g,id)+=7;break;
 case SR_GLASS_EYE:sr_pending(g,SP_REWARD,5,256);break;
 case SR_GNARLED_HAMMER:sr_pending(g,SP_FORGE,3,MOD_KEEN);break;
 case SR_GOLDEN_COMPASS:SU(g,SU_COMPASS_ACT)=g->act;for(int r=g->floor;r<MAP_ROWS;r++)for(int c=0;c<3;c++)if(g->map[r][c]==NODE_FIGHT)g->map[r][c]=NODE_TREASURE;break;
 case SR_GOLDEN_PEARL:sr_gain_gold(g,150);break;
 case SR_HEFTY_TABLET:add_card(g,51);sr_pending(g,SP_REWARD,1,2);break;
 case SR_JEWELRY_BOX:sr_upgrade(g,MAX_DECK,-1);break;
 case SR_KALEIDOSCOPE:sr_pending(g,SP_REWARD,2,32|1);break;
 case SR_LARGE_CAPSULE:sr_random_relics(g,2);add_card(g,g->character==0?0:g->character==1?N_STRIKE_SILENT:R_STRIKE_REGENT);add_card(g,g->character==0?1:g->character==1?N_DEFEND_SILENT:R_DEFEND_REGENT);break;
 case SR_LEAD_PAPERWEIGHT:sr_pending(g,SP_REWARD,1,16|1);break;
 case SR_LEAFY_POULTICE:sr_maxhp(g,-12);for(int type=0;type<2;type++)for(int i=0;i<g->deck_count;i++)if((type?sr_is_defend(g->deck[i]):sr_is_starter(g->deck[i])&&!sr_is_defend(g->deck[i]))&&!(relic(g,SR_NUTRITIOUS_SOUP)&&sr_is_strike(g->deck[i]))){g->deck[i]=reward_of_rarity(g,1,CARD_ID(g->deck[i]),-1);break;}break;
 case SR_LEES_WAFFLE:sr_maxhp(g,7);heal(g,g->maxhp);break;
 case SR_LIZARD_TAIL:SC(g,id)++;break;
 case SR_LOOMING_FRUIT:sr_maxhp(g,31);break;
 case SR_LOST_COFFER:sr_brew(g,1,-1);sr_pending(g,SP_REWARD,1,1);break;
 case SR_MASSIVE_SCROLL:sr_pending(g,SP_REWARD,1,16|64|2);break;
 case SR_NEOWS_BONES:sr_random_relics(g,2);add_card(g,51);break;
 case SR_NEOWS_TALISMAN:for(int type=0;type<2;type++)for(int i=0;i<g->deck_count;i++)if(game_card_is_upgradeable(g->deck[i])&&(type?sr_is_defend(g->deck[i]):sr_is_starter(g->deck[i])&&!sr_is_defend(g->deck[i]))){g->deck[i]|=UPBIT;break;}break;
 case SR_NEOWS_TORMENT:add_card(g,10|UPBIT);break;
 case SR_NEW_LEAF:sr_pending(g,SP_TRANSFORM,1,0);break;
 case SR_NUTRITIOUS_OYSTER:sr_maxhp(g,11);break;
 case SR_OLD_COIN:sr_gain_gold(g,300);break;
 case SR_ORRERY:sr_pending(g,SP_REWARD,5,1);break;
 case SR_PAELS_GROWTH:case SR_ROYAL_STAMP:sr_pending(g,SP_FORGE,1,MOD_ECHO);break;
 case SR_PAELS_HORN:for(int i=0;i<2;i++)add_card(g,5|UPBIT|(MOD_BOUND<<CARD_MOD_SHIFT));break;
 case SR_PAELS_TOOTH:SC(g,id)+=5;sr_pending(g,SP_REMOVE,5,0);break;
 case SR_PANDORAS_BOX:for(int i=0;i<g->deck_count;i++)if(sr_is_starter(g->deck[i])&&!(relic(g,SR_NUTRITIOUS_SOUP)&&sr_is_strike(g->deck[i])))g->deck[i]=reward_of_rarity(g,MAX(0,cards[CARD_ID(g->deck[i])].rarity),-1,-1);break;
 case SR_PEAR:sr_maxhp(g,10);break;
 case SR_POMANDER:sr_pending(g,SP_UPGRADE,1,0);break;
 case SR_PRECARIOUS_SHEARS:lose_hp(g,16);sr_pending(g,SP_REMOVE,2,0);break;
 case SR_PRECISE_SCISSORS:sr_pending(g,SP_REMOVE,1,0);break;
 case SR_PRESERVED_FOG:add_card(g,51);sr_pending(g,SP_REMOVE,3,0);break;
 case SR_PUMPKIN_CANDLE:SC(g,id)+=5;break;
 case SR_PUNCH_DAGGER:sr_pending(g,SP_FORGE,1,MOD_KEEN|256);break;
 case SR_SAND_CASTLE:sr_upgrade(g,6,-1);break;
 case SR_SCROLL_BOXES:sr_pending(g,SP_REWARD,3,1|64);break;
 case SR_SEA_GLASS:sr_pending(g,SP_REWARD,5,1|32);break;
 case SR_SERE_TALON:add_card(g,51);add_card(g,51);for(int i=0;i<3;i++)add_card(g,C_JACK_OF_ALL_TRADES|UPBIT);break;
 case SR_SIGNET_RING:sr_gain_gold(g,999);break;
 case SR_SILKEN_TRESS:g->gold=0;SC(g,id)++;break;
 case SR_SILVER_CRUCIBLE:SC(g,id)+=3;SU(g,SU_TEMP)++;break;
 case SR_SMALL_CAPSULE:sr_random_relics(g,1);break;
 case SR_STORYBOOK:add_card(g,38|UPBIT);break;
 case SR_STRAWBERRY:sr_maxhp(g,7);break;
 case SR_TANXS_WHISTLE:add_card(g,R_SUMMON_FORTH|UPBIT);break;
 case SR_TEA_OF_DISCOURTESY:SC(g,id)++;break;
 case SR_TOUCH_OF_OROBAS:sr_replace_starter(g,g->character==0?SR_BURNING_BLOOD:g->character==1?SR_RING_OF_THE_SNAKE:SR_DIVINE_RIGHT,g->character==0?SR_BLACK_BLOOD:g->character==1?SR_RING_OF_THE_DRAKE:SR_DIVINE_DESTINY);break;
 case SR_TOY_BOX:for(int i=0;i<4&&SU(g,SU_WAX_COUNT)<96;i++){int r=random_relic(g);if(r<0)break;add_relic(g,r);g->revision186.wax[SU(g,SU_WAX_COUNT)++]=r;}break;
 case SR_TRI_BOOMERANG:sr_pending(g,SP_FORGE,3,MOD_BOUND|256|512);break;
 case SR_WAR_PAINT:sr_upgrade(g,2,1);break;
 case SR_WHETSTONE:sr_upgrade(g,2,0);break;
 case SR_WINGED_BOOTS:SC(g,id)+=3;break;
 case SR_WONGOS_MYSTERY_TICKET:SC(g,id)=5;break;
 case SR_YUMMY_COOKIE:sr_pending(g,SP_UPGRADE,4,0);break;
 default:break; /* All other entries act at a named runtime hook. */
 }
}
static int sr_repeatable(int id){return id==SR_SAND_CASTLE||id==SR_SEA_GLASS||id==SR_YUMMY_COOKIE||id==SR_ASTROLABE||id==SR_ORRERY;}
static void sr_offer_blessing(Game*g){
 static const int rule[]={SR_ICE_CREAM,SR_RUNIC_PYRAMID,SR_NUTRITIOUS_SOUP,SR_PRISMATIC_GEM,SR_SNECKO_EYE,SR_FIDDLE,SR_PAELS_CLAW,SR_VELVET_CHOKER};
 static const int build[]={SR_TOUCH_OF_OROBAS,SR_CROSSBOW,SR_MUSIC_BOX,SR_THROWING_AXE,SR_PAELS_GROWTH,SR_PAELS_LEGION,SR_ELECTRIC_SHRYMP,SR_JEWELED_MASK,SR_VERY_HOT_COCOA,SR_PAELS_FLESH};
 static const int neow_risk[]={SR_HEFTY_TABLET,SR_CURSED_PEARL,SR_LARGE_CAPSULE,SR_LEAFY_POULTICE,SR_PRECARIOUS_SHEARS,SR_NEOWS_BONES,SR_SILKEN_TRESS,SR_SILVER_CRUCIBLE};
 static const int neow_build[]={SR_NEW_LEAF,SR_PRECISE_SCISSORS,SR_ARCANE_SCROLL,SR_KALEIDOSCOPE,SR_SCROLL_BOXES,SR_NEOWS_TALISMAN,SR_NEOWS_TORMENT,SR_BOOMING_CONCH,SR_PHIAL_HOLSTER,SR_WINGED_BOOTS,SR_FISHING_ROD,SR_SMALL_CAPSULE};
 if(g->act==1){g->reward[0]=neow_risk[rnd(g,sizeof(neow_risk)/sizeof(*neow_risk))];int a=rnd(g,sizeof(neow_build)/sizeof(*neow_build)),b=rnd(g,sizeof(neow_build)/sizeof(*neow_build)-1);if(b>=a)b++;g->reward[1]=neow_build[a];g->reward[2]=neow_build[b];g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose your opening boon. Greater Ancients await in later acts.");return;}
 int used[3]={-1,-1,-1};
 for(int slot=0;slot<3;slot++){
  int pool[384],n=0;
  if(slot<2){const int*ids=slot?build:rule;int size=slot?sizeof(build)/sizeof(*build):sizeof(rule)/sizeof(*rule);for(int i=0;i<size;i++){int id=ids[i];if(!relic(g,id))pool[n++]=id;}}
  if(!n||slot==2)for(unsigned i=0;i<sizeof(sr_catalog)/sizeof(*sr_catalog);i++){int id=sr_catalog[i].id;if(!sr_eligible_relic(g,id,1)||id==used[0]||id==used[1])continue;if(!relic(g,id)||sr_repeatable(id))pool[n++]=id;}
  used[slot]=n?pool[rnd(g,n)]:SR_SAND_CASTLE;g->reward[slot]=used[slot];
 }
 g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose an Ancient boon. Its rules last for this run.");
}
static void sr_prepare_offers(Game*g){
 int kind=SU(g,SU_PICK),arg=SU(g,SU_ARG),rarity=arg&3;
 if(arg&256)rarity=SU(g,SU_REMAIN)>3?0:SU(g,SU_REMAIN)>1?1:2;
 SU(g,SU_FOREIGN)=!!(arg&32);
 for(int i=0;i<3;i++){
  int id;
  if(kind==SP_OPENING||arg&16||arg&128){int pool[512],n=0;for(int c=3;c<CARD_COUNT;c++)if(combat_reward_eligible(c)&&ascent_card_available(g,c)&&(kind==SP_OPENING||(cards[c].rarity==rarity))&&((kind==SP_OPENING||arg&16)?(c>=COLORLESS_FIRST&&c<EXTRA_LAST):sr_allowed(g,c))&&(!(arg&128)||cards[c].type==2)&&(!i||c!=CARD_ID(SU(g,SU_OFFER0)))&&(i<2||c!=CARD_ID(SU(g,SU_OFFER1))))pool[n++]=c;id=n?pool[rnd(g,n)]:C_FINESSE;}
  else id=reward_of_rarity(g,rarity,i?CARD_ID(SU(g,SU_OFFER0)):-1,i>1?CARD_ID(SU(g,SU_OFFER1)):-1);
  SU(g,SU_OFFER0+i)=id|((arg&64)?UPBIT:0);if(kind==SP_REWARD)reward_mark(g,id);
 }
 SU(g,SU_FOREIGN)=0;
}
int game_ancient_reward(const Game*g){return SU(g,SU_PICK)==SP_REWARD||SU(g,SU_PICK)==SP_OPENING;}
int game_ancient_count(const Game*g){return game_ancient_reward(g)?3:g->deck_count;}
int game_ancient_card(const Game*g,int i){return i<0||i>=game_ancient_count(g)?-1:game_ancient_reward(g)?SU(g,SU_OFFER0+i):g->deck[i];}
int game_ancient_eligible(const Game*g,int i){
 int raw=game_ancient_card(g,i);if(raw<0)return 0;int kind=SU(g,SU_PICK),arg=SU(g,SU_ARG);
 if(kind==SP_REMOVE||kind==SP_TRANSFORM){if(relic(g,SR_NUTRITIOUS_SOUP)&&sr_is_strike(raw))return 0;if(kind==SP_REMOVE&&g->deck_count<=1)return 0;return 1;}
 if(kind==SP_UPGRADE)return game_card_is_upgradeable(raw);
 if(kind==SP_FORGE){if(arg==64)return cards[CARD_ID(raw)].type==1;return (!(arg&512)||cards[CARD_ID(raw)].type==0)&&game_can_modify(raw,arg&7);}
 if(kind==SP_ANCIENT_STARTER)return sr_is_starter(raw)||CARD_ID(raw)==2||CARD_ID(raw)==N_NEUTRALIZE||CARD_ID(raw)==R_FALLING_STAR;
 return g->deck_count<MAX_DECK||kind==SP_OPENING;
}
const char*game_ancient_title(const Game*g){switch(SU(g,SU_PICK)){case SP_REMOVE:return "ANCIENT BOON / REMOVE";case SP_UPGRADE:return "ANCIENT BOON / UPGRADE";case SP_TRANSFORM:return "ANCIENT BOON / TRANSFORM";case SP_COPY:return "ANCIENT BOON / COPY";case SP_FORGE:return "ANCIENT BOON / ENCHANT";case SP_REWARD:return "ANCIENT BOON / CARD REWARD";case SP_OPENING:return "ANCIENT BOON / OPENING HAND";default:return "ANCIENT BOON / STARTER";}}
static void sr_open_pending(Game*g){
 if(!SU(g,SU_PICK)||g->screen==ANCIENT_PICK||g->screen==DEFEAT||g->screen==PAUSE_MENU||g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE||g->screen==CARD_CHOICE)return;
 SU(g,SU_RESUME)=g->screen;g->screen=ANCIENT_PICK;g->selection=0;msg(g,"");
 if(game_ancient_reward(g))sr_prepare_offers(g);
 int n=game_ancient_count(g),any=0;for(int i=0;i<n;i++)any|=game_ancient_eligible(g,i);
 if(!any){g->screen=SU(g,SU_RESUME);SU(g,SU_PICK)=SU(g,SU_REMAIN)=0;}
}
static int sr_action(Game*g,int b){
 if(g->screen!=ANCIENT_PICK)return 0;
 int n=game_ancient_count(g);
 if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);if(b==BTN_RIGHT)g->selection=MIN(n-1,g->selection+1);
 if(b==BTN_UP)g->selection=MAX(0,g->selection-3);if(b==BTN_DOWN)g->selection=MIN(n-1,g->selection+3);
 if(b!=BTN_OK&&b!=BTN_BACK)return 1;
 if(b==BTN_OK&&!game_ancient_eligible(g,g->selection)){msg(g,"Choose an eligible card, or Circle to finish.");return 1;}
 if(b==BTN_OK){int raw=game_ancient_card(g,g->selection),k=SU(g,SU_PICK),a=SU(g,SU_ARG),ix=g->selection;
  if(k==SP_REMOVE){for(int i=ix;i+1<g->deck_count;i++)g->deck[i]=g->deck[i+1];g->deck_count--;}
  else if(k==SP_UPGRADE)g->deck[ix]|=UPBIT;
  else if(k==SP_TRANSFORM)g->deck[ix]=a==2?CARD_GIANT_ROCK|UPBIT:reward_of_rarity(g,1,CARD_ID(raw),-1)|(a?UPBIT:0);
  else if(k==SP_COPY||k==SP_REWARD)add_card(g,raw);
  else if(k==SP_FORGE){if(a==64)SU(g,SU_IMBUED)=CARD_ID(raw)+1;else g->deck[ix]=(raw&~CARD_MOD_MASK)|((a&7)<<CARD_MOD_SHIFT)|((a&256)?UPBIT:0);}
  else if(k==SP_ANCIENT_STARTER){int id=g->character==CHAR_REGENT?R_METEOR_SHOWER:g->character==CHAR_NIGHTSHADE?N_SUPPRESS:36;g->deck[ix]=(raw&~511)|id;}
  else if(k==SP_OPENING)add_generated(g,raw|(a?(MOD_BOUND<<CARD_MOD_SHIFT):0));
 }
 if(b==BTN_BACK&&SU(g,SU_PICK)!=SP_REWARD)SU(g,SU_REMAIN)=1;
 if(--SU(g,SU_REMAIN)<=0){g->screen=SU(g,SU_RESUME);SU(g,SU_PICK)=0;g->selection=0;if(g->screen==COMBAT)check_win(g);}
 else{g->selection=0;if(game_ancient_reward(g))sr_prepare_offers(g);int any=0;for(int i=0;i<game_ancient_count(g);i++)any|=game_ancient_eligible(g,i);if(!any){g->screen=SU(g,SU_RESUME);SU(g,SU_PICK)=SU(g,SU_REMAIN)=0;}}
 return 1;
}
static void sr_victory(Game*g){
 if(SB(g,37))return;SB(g,37)=1;sr_kills(g);
 heal(g,12*relic(g,SR_BLACK_BLOOD));
 if(g->hp*2<=g->maxhp)heal(g,12*relic(g,SR_MEAT_ON_THE_BONE));
 sr_maxhp(g,relic(g,SR_CHOSEN_CHEESE));
 sr_gain_gold(g,15*g->enemy_count*relic(g,SR_AMETHYST_AUBERGINE));
 if(relic(g,SR_WHITE_BEAST_STATUE)&&!relic(g,SR_SOZU))g->v151.potion_offer=ascent_random_potion(g);
 if(g->battle_type==NODE_ELITE){sr_random_relics(g,relic(g,SR_BLACK_STAR));sr_upgrade(g,4*relic(g,SR_WAR_HAMMER),-1);
  if(relic(g,SR_SWORD_OF_STONE)&&++SC(g,SR_SWORD_OF_STONE)>=5){for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==SR_SWORD_OF_STONE)*relic_slot(g,i)=SR_SWORD_OF_JADE;}
 }
 if(g->battle_type==NODE_BOSS&&g->act==1)sr_random_relics(g,2*relic(g,SR_LAVA_ROCK));
 if(g->battle_type==NODE_FIGHT&&relic(g,SR_FISHING_ROD)&&sr_counter(g,SR_FISHING_ROD,3))sr_upgrade(g,relic(g,SR_FISHING_ROD),-1);
 if(SC(g,SR_PUMPKIN_CANDLE))SC(g,SR_PUMPKIN_CANDLE)--;
 if(SC(g,SR_WONGOS_MYSTERY_TICKET)&&!--SC(g,SR_WONGOS_MYSTERY_TICKET))sr_random_relics(g,3*relic(g,SR_WONGOS_MYSTERY_TICKET));
 if(SU(g,SU_WAX_COUNT)&&++SU(g,SU_WAX_CLOCK)>=3){SU(g,SU_WAX_CLOCK)=0;int id=g->revision186.wax[0];for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==id){for(int j=i;j+1<g->relic_count;j++)*relic_slot(g,j)=game_relic_id(g,j+1);g->relic_count--;break;}for(int i=0;i+1<SU(g,SU_WAX_COUNT);i++)g->revision186.wax[i]=g->revision186.wax[i+1];SU(g,SU_WAX_COUNT)--;}
 SU(g,SU_REWARD_INDEX)=0;SU(g,SU_REWARD_RARE)=g->battle_type==NODE_ELITE?relic(g,SR_WHITE_STAR):0;
 SU(g,SU_REWARD_POWER)=relic(g,SR_LASTING_CANDY)&&sr_counter(g,SR_LASTING_CANDY,2)?relic(g,SR_LASTING_CANDY):0;
 SU(g,SU_REWARD_UP)=SC(g,SR_PAELS_TOOTH)?1:0;if(SC(g,SR_PAELS_TOOTH))SC(g,SR_PAELS_TOOTH)--;
 g->ninja.v[NS_BONUS_REWARDS]+=SU(g,SU_REWARD_RARE)+SU(g,SU_REWARD_POWER)+SU(g,SU_REWARD_UP)+(g->battle_type==NODE_FIGHT?relic(g,SR_PRAYER_WHEEL):0);
}
static void sr_reward(Game*g){
 SU(g,SU_REROLLED)=0;
 if(SU(g,SU_REWARD_INDEX)++>0){
  if(SU(g,SU_REWARD_RARE)){SU(g,SU_REWARD_RARE)--;for(int i=0;i<3;i++)g->reward[i]=reward_of_rarity(g,2,i?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1);}
  else if(SU(g,SU_REWARD_POWER)){SU(g,SU_REWARD_POWER)--;for(int i=0;i<3;i++)g->reward[i]=sr_random_card(g,2,0);}
  else if(SU(g,SU_REWARD_UP)){SU(g,SU_REWARD_UP)--;for(int i=0;i<3;i++)g->reward[i]|=UPBIT;}
 }
 int upgrade=SC(g,SR_SILVER_CRUCIBLE)>0||(relic(g,SR_LAVA_LAMP)&&!SB(g,SB_HP_COMBAT));
 if(SC(g,SR_SILVER_CRUCIBLE))SC(g,SR_SILVER_CRUCIBLE)--;
 int glam=relic(g,SR_GLITTER)||SC(g,SR_SILKEN_TRESS);if(SC(g,SR_SILKEN_TRESS))SC(g,SR_SILKEN_TRESS)--;
 for(int i=0;i<3;i++){int raw=g->reward[i];if(upgrade)raw|=UPBIT;if(glam&&!CARD_MOD(raw)){int m=cards[CARD_ID(raw)].type==0?MOD_KEEN:MOD_WARD;if(game_can_modify(raw,m))raw|=m<<CARD_MOD_SHIFT;}g->reward[i]=raw;}
 for(int n=0;n<relic(g,SR_WING_CHARM);n++){int ix[3],count=0;for(int i=0;i<3;i++)if(game_can_modify(g->reward[i],MOD_BOUND))ix[count++]=i;if(count)g->reward[ix[rnd(g,count)]]|=MOD_BOUND<<CARD_MOD_SHIFT;}
}
static void sr_skipped(Game*g){if(relic(g,SR_PAELS_WING)&&sr_counter(g,SR_PAELS_WING,2))sr_random_relics(g,relic(g,SR_PAELS_WING));}
int game_shop_price(const Game*g,int base){
 int n=MAX(1,base-30*relic(g,6));for(int i=0;i<relic(g,SR_MEMBERSHIP_CARD);i++)n=MAX(1,n/2);for(int i=0;i<relic(g,SR_THE_COURIER);i++)n=MAX(1,n*4/5);return n;
}
static void sr_room(Game*g,int node){
 SU(g,SU_CAMP_ACTIVE)=0;
 if(!SU(g,SU_MAW_OFF))sr_gain_gold(g,12*relic(g,SR_MAW_BANK));
 if(node==NODE_EVENT){heal(g,5*relic(g,SR_PLANISPHERE));g->revision182.next_combat[1]=MIN(99,g->revision182.next_combat[1]+8*relic(g,SR_JUZU_BRACELET));}
 if(node==NODE_REST){heal(g,(g->deck_count/5)*3*relic(g,SR_ETERNAL_FEATHER));SC(g,SR_VENERABLE_TEA_SET)=2*relic(g,SR_VENERABLE_TEA_SET);SC(g,SR_FAKE_VENERABLE_TEA_SET)=relic(g,SR_FAKE_VENERABLE_TEA_SET);SU(g,SU_CAMP_USED)=0;SU(g,SU_CAMP_ACTIVE)=1;}
 if(node==NODE_SHOP){heal(g,15*relic(g,SR_MEAL_TICKET));if(relic(g,SR_LORDS_PARASOL)){for(int i=0;i<3;i++)if(g->shop_cards[i]>=0){add_card(g,g->shop_cards[i]);g->shop_cards[i]=-1;}if(g->shop_relic>=0){add_relic(g,g->shop_relic);g->shop_relic=-1;}sr_brew(g,1,-1);}}
}
static void sr_rest(Game*g){heal(g,15*relic(g,SR_REGAL_PILLOW));sr_maxhp(g,5*relic(g,SR_STONE_HUMIDIFIER));sr_brew(g,2*relic(g,SR_TINY_MAILBOX),-1);if(relic(g,SR_DREAM_CATCHER))sr_pending(g,SP_REWARD,relic(g,SR_DREAM_CATCHER),1);}
static int sr_rest_kind(const Game*g,int index){
 if(index<2)return index;int n=2;
 if(relic(g,SR_GIRYA)&&SC(g,SR_GIRYA)<3*relic(g,SR_GIRYA)){if(index==n)return 2;n++;}
 if(relic(g,SR_SHOVEL)){if(index==n)return 3;n++;}
 if(relic(g,SR_MEAT_CLEAVER)){if(index==n)return 4;n++;}
 if(relic(g,SR_PUMPKIN_CANDLE)){if(index==n)return 5;n++;}
 return 6;
}
int game_rest_count(const Game*g){return 2+(relic(g,SR_GIRYA)&&SC(g,SR_GIRYA)<3*relic(g,SR_GIRYA))+!!relic(g,SR_SHOVEL)+!!relic(g,SR_MEAT_CLEAVER)+!!relic(g,SR_PUMPKIN_CANDLE)+!!relic(g,SR_MINIATURE_TENT);}
const char*game_rest_title(const Game*g,int i){static const char*names[]={"Rest","Upgrade","Train","Dig","Cook","Kindle","Leave"};return names[sr_rest_kind(g,i)];}
const char*game_rest_detail(const Game*g,int i){static const char*lines[]={"Recover 30% of maximum HP, plus resting relic effects.","Upgrade a card in your deck.","Gain 1 permanent Strength. Limited to 3 uses per Girya.","Uncover one ordinary relic.","Gain and heal 5 Max HP.","Renew Pumpkin Candle for 5 more combats.","Continue your ascent."};return lines[sr_rest_kind(g,i)];}
static int sr_rest_action(Game*g,int i){
 int kind=sr_rest_kind(g,i),bit=1<<kind;
 if(relic(g,SR_MINIATURE_TENT)&&(SU(g,SU_CAMP_USED)&bit)){msg(g,"You already used this option at this camp.");return 1;}
 if(kind==6){SU(g,SU_CAMP_ACTIVE)=0;enter_map(g);return 1;}
 if(kind<2)return 0;
 SU(g,SU_CAMP_USED)|=bit;
 if(kind==2){g->permanent_strength++;SC(g,SR_GIRYA)++;}
 if(kind==3)sr_random_relics(g,relic(g,SR_SHOVEL));
 if(kind==4)sr_maxhp(g,5*relic(g,SR_MEAT_CLEAVER));
 if(kind==5)SC(g,SR_PUMPKIN_CANDLE)=5*relic(g,SR_PUMPKIN_CANDLE);
 enter_map(g);return 1;
}
static void sr_potion(Game*g){int n=3*relic(g,SR_REPTILE_TRINKET);sr_strength(g,n);g->ic[IC_TEMP_STRENGTH]+=n;}
static void sr_gain_gold(Game*g,int amount){
 if(amount<=0||relic(g,SR_ECTOPLASM))return;
 long long total=(long long)amount*(100+25*relic(g,SR_BOWLER_HAT))/100;
 g->gold=(int)MIN(1000000000LL,(long long)g->gold+total);
 sr_maxhp(g,relic(g,SR_DRAGON_FRUIT));
}
void game_action(Game*g,int b){
 if(g->screen==ANCIENT_PICK&&b!=BTN_PAUSE){sr_action(g,b);}
 else if(g->screen==REWARD&&b==BTN_ALT&&relic(g,SR_DRIFTWOOD)&&!SU(g,SU_REROLLED)&&!g->v151.card_done){
  SU(g,SU_REROLLED)=1;int old[3];memcpy(old,g->reward,sizeof(old));
  for(int i=0;i<3;i++)g->reward[i]=reward_of_rarity(g,cards[CARD_ID(old[i])].rarity,i?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1)|(old[i]&UPBIT);
  for(int i=0;i<3;i++)reward_mark(g,g->reward[i]);msg(g,"Reward rerolled. Rarities preserved.");
 }else game_action_impl(g,b);
 /* The autoplay boon yields safely at genuine card choices. A later action
    resumes it; there is never a recursive game_action call. */
 for(int steps=0;steps<20&&g->screen==COMBAT&&SB(g,SB_EARRING);steps++){
  int ix=-1;for(int i=0;i<g->hand_count;i++)if(game_card_is_playable(g->hand[i])&&game_card_cost_now(g,g->hand[i])>=-1&&game_card_cost_now(g,g->hand[i])<=g->energy&&game_card_star_cost(g,g->hand[i])<=g->regent.stars){ix=i;break;}
  if(ix<0||!sr_can_play(g)){SB(g,SB_EARRING)=0;game_end_turn(g);break;}
  int count=SB(g,SB_PLAYS);game_play_card(g,ix,g->target);if(SB(g,SB_PLAYS)==count&&g->screen==COMBAT){SB(g,SB_EARRING)=0;game_end_turn(g);break;}
 }
 sr_open_pending(g);
}
