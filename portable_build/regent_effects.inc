/* Only direct card effects contribute to the live description probe. */
static void regent_effect(Game*g,RegentTask t){
 int raw=t.raw,id=CARD_ID(raw),up=CARD_UP(raw),e=t.target,x=t.n,depth=t.depth;
 const RegentCardDef*d=&regent_cards[id-REGENT_FIRST];int damage=regent_base_damage(g,raw),block=up?d->upblock:d->block;
 int before[6];r151_status_snapshot(g,before);
#define DRAW(n) regent_push(g,RT_DRAW,raw,e,n,0,depth)
#define CHOOSE(k,n) regent_push(g,RT_CHOOSE,raw,e,k,n,depth)
#define POWER(id,n) do{RP(g,id)=MIN(1000000,RP(g,id)+(n));}while(0)
 switch(id){
 case R_ALIGNMENT:gain_energy(g,2+up);break;
 case R_ARSENAL:POWER(id,1);break;
 case R_ASTRAL_PULSE:all_damage(g,damage);all_damage(g,damage);break;
 case R_BEAT_INTO_SHAPE:{int hits=e>=0?g->regent.hit_count[e]:0;deal(g,e,damage);regent_forge(g,(5+2*up)*(1+hits));break;}
 case R_BEGONE:CHOOSE(RC_TRANSFORM_HAND,1);break;
 case R_BIG_BANG:DRAW(1);gain_energy(g,1);regent_gain_stars(g,1);regent_forge(g,5);break;
 case R_BLACK_HOLE:POWER(id,3+up);break;
 case R_BOMBARDMENT:case R_DEVASTATE:case R_KINGLY_KICK:case R_KINGLY_PUNCH:case R_LUNAR_BLAST:case R_MAKE_IT_SO:case R_MINION_DIVE_BOMB:case R_STRIKE_REGENT:case R_SUPERMASSIVE:case R_CRESCENT_SPEAR:deal(g,e,damage);break;
 case R_BULWARK:card_block(g,block);regent_forge(g,10+3*up);break;
 case R_BUNDLE_OF_JOY:for(int i=0;i<3+up;i++)regent_create(g,regent_colorless(g,0));break;
 case R_CELESTIAL_MIGHT:for(int i=0;i<3+up;i++)deal(g,e,damage);break;
 case R_CHARGE:CHOOSE(RC_TRANSFORM_DRAW,2);break;
 case R_CHILD_OF_THE_STARS:POWER(id,2+up);break;
 case R_CLOAK_OF_STARS:case R_DEFEND_REGENT:case R_MINION_SACRIFICE:case R_IAM_INVINCIBLE:card_block(g,block+(id==R_DEFEND_REGENT?RP(g,C_FASTEN):0));break;
 case R_COLLISION_COURSE:deal(g,e,damage);regent_create(g,R_DEBRIS);break;
 case R_COMET:case R_FALLING_STAR:case R_GAMMA_BLAST:{int n=id==R_COMET?3:id==R_GAMMA_BLAST?2:1;deal(g,e,damage);ninja_weak(g,e,n);if(e>=0)apply_vulnerable(g,&g->enemies[e],n);break;}
 case R_CONQUEROR:regent_forge(g,3+2*up);if(e>=0)g->regent.conqueror[e]=1;break;
 case R_CONSTELLATION:DRAW(1);gain_energy(g,1);card_block(g,9+3*up);break;
 case R_CONVERGENCE:g->regent.next_energy++;g->regent.next_stars+=1+up;g->regent.retain_all=1;break;
 case R_COSMIC_INDIFFERENCE:card_block(g,block);CHOOSE(RC_DISCARD_TOP,1);break;
 case R_CRASH_LANDING:all_damage(g,damage);while(g->hand_count<MAX_HAND)regent_create(g,R_DEBRIS);break;
 case R_CRUSH_UNDER:case R_DYING_STAR:all_damage(g,damage);for(int z=0;z<g->enemy_count;z++)regent_temp_weak(g,z,id==R_DYING_STAR?9+2*up:1+up);break;
 case R_DEBRIS:break;
 case R_DECISIONS_DECISIONS:CHOOSE(RC_SKILL_TRIPLE,1);DRAW(3+2*up);break;
 case R_FOREGONE_CONCLUSION:g->regent.next_seek+=2+up;break;
 case R_FURNACE:POWER(id,5+2*up);break;
 case R_GATHER_LIGHT:card_block(g,block);regent_gain_stars(g,1);break;
 case R_GENESIS:POWER(id,2+up);break;
 case R_GLIMMER:CHOOSE(RC_TOP,1);DRAW(3+up);break;
 case R_GLITTERSTREAM:{int used=g->ic[IC_UNMOVABLE_USED];card_block(g,block);int saved=g->ic[IC_UNMOVABLE_USED];g->ic[IC_UNMOVABLE_USED]=!used;g->regent.next_block+=regent_delayed_block(g,raw,5+2*up);g->ic[IC_UNMOVABLE_USED]=saved;break;}
 case R_GLOW:regent_gain_stars(g,1+up);DRAW(1);g->regent.next_draw++;break;
 case R_GUARDS:CHOOSE(RC_GUARDS,MAX_HAND);break;
 case R_GUIDING_STAR:deal(g,e,damage);DRAW(2+up);break;
 case R_HAMMER_TIME:POWER(id,1);break;
 case R_HEAVENLY_DRILL:{int n=x>=4?x*2:x;for(int i=0;i<MIN(198,n);i++)deal(g,e,damage);break;}
 case R_HEGEMONY:deal(g,e,damage);g->regent.next_energy+=2+up;break;
 case R_HEIRLOOM_HAMMER:deal(g,e,damage);CHOOSE(RC_COPY_COLORLESS,1);break;
 case R_HIDDEN_CACHE:regent_gain_stars(g,1);g->regent.next_stars+=3+up;break;
 case R_KNOCKOUT_BLOW:{int hp=e>=0?g->enemies[e].hp:0;deal(g,e,damage);if(hp>0&&g->enemies[e].hp<=0)regent_gain_stars(g,5);break;}
 case R_KNOW_THY_PLACE:ninja_weak(g,e,1);if(e>=0)apply_vulnerable(g,&g->enemies[e],1);break;
 case R_LARGESSE:regent_create(g,regent_colorless(g,up));break;
 case R_MANIFEST_AUTHORITY:card_block(g,block);regent_create(g,regent_colorless(g,up));break;
 case R_METEOR_SHOWER:all_damage(g,damage);for(int z=0;z<g->enemy_count;z++){ninja_weak(g,z,2);apply_vulnerable(g,&g->enemies[z],2);}break;
 case R_MINION_STRIKE:deal(g,e,damage);DRAW(1);break;
 case R_MONARCHS_GAZE:POWER(id,1);break;
 case R_MONOLOGUE:g->regent.monologue++;break;
 case R_NEUTRON_AEGIS:g->ic[IC_PLATING]+=8+3*up;POWER(id,1);break;
 case R_ORBIT:break;
 case R_PALE_BLUE_DOT:POWER(id,1+up);break;
 case R_PARRY:POWER(id,10+4*up);break;
 case R_PARTICLE_WALL:card_block(g,block);break;
 case R_PATTER:card_block(g,block);g->regent.vigor+=2+up;break;
 case R_PHOTON_CUT:deal(g,e,damage);CHOOSE(RC_TOP,1);DRAW(1+up);break;
 case R_PILLAR_OF_CREATION:POWER(id,3+up);break;
 case R_PLOT:g->regent.next_draw+=2+up;break;
 case R_PROPHESIZE:DRAW(6+3*up);break;
 case R_QUASAR:regent_offer(g,raw,e,up,0,depth);break;
 case R_RADIATE:all_damage(g,damage);break;
 case R_REFINE_BLADE:regent_forge(g,9+4*up);g->regent.next_energy++;break;
 case R_REFLECT:card_block(g,block);g->regent.reflect=1;break;
 case R_RESONANCE:g->strength+=1+up;for(int z=0;z<g->enemy_count;z++){g->enemies[z].strength--;record_combat_fx(GFX_STATUS,-1,z,1,0);}break;
 case R_ROYAL_GAMBLE:regent_gain_stars(g,9);break;
 case R_ROYALTIES:POWER(id,30+10*up);break;
 case R_SEEKING_EDGE:POWER(id,1);regent_forge(g,7+4*up);break;
 case R_SEVEN_STARS:for(int i=0;i<7;i++)all_damage(g,damage);break;
 case R_SHINING_STRIKE:deal(g,e,damage);regent_gain_stars(g,2);break;
 case R_SOLAR_STRIKE:deal(g,e,damage);regent_gain_stars(g,1+up);break;
 case R_SOVEREIGN_BLADE:if(RP(g,R_PARRY))card_block(g,RP(g,R_PARRY));if(RP(g,R_SEEKING_EDGE))all_damage(g,damage);else deal(g,e,damage);break;
 case R_SPECTRUM_SHIFT:POWER(id,1);break;
 case R_SPOILS_OF_BATTLE:regent_forge(g,5+3*up);DRAW(2);break;
 case R_STARDUST:for(int i=0;i<MIN(999,t.aux)&&ninja_has_living(g);i++){int foe=preview_trace&&preview_choice>=0?preview_living_foe(g,preview_choice):random_living_foe(g);deal(g,foe,damage);}break;
 case R_SUMMON_FORTH:regent_recall(g,R_SOVEREIGN_BLADE);regent_forge(g,8+3*up);break;
 case R_SWORD_SAGE:POWER(id,1);break;
 case R_TERRAFORMING:g->regent.vigor+=6+2*up;break;
 case R_THE_SEALED_THRONE:POWER(id,1);break;
 case R_THE_SMITH:regent_forge(g,30+10*up);break;
 case R_TUTOR:CHOOSE(RC_SEEK,1);break;
 case R_TYRANNY:POWER(id,1);break;
 case R_VENERATE:regent_gain_stars(g,2+up);break;
 case R_VOID_FORM:POWER(id,1);regent_push(g,RT_ENDTURN,raw,e,0,0,0);break;
 case R_WROUGHT_IN_WAR:deal(g,e,damage);regent_forge(g,7+2*up);break;
 /* Full solo Colorless generation pool, using the same choice and replay machinery. */
 case C_ALCHEMIZE:if(g->potion_count<MAX_POTIONS)g->potions[g->potion_count++]=rnd(g,6);else msg(g,"Potion belt full.");break;
 case C_ANOINTED:for(int i=g->draw_count-1;i>=0&&g->hand_count<MAX_HAND;i--)if(cards[CARD_ID(g->draw[i])].rarity==2)regent_to_hand(g,regent_take(g->draw,&g->draw_count,i));break;
 case C_AUTOMATION:POWER(id,1);g->regent.automation_phase[g->regent.draws%10]++;break;
 case C_CALAMITY:case C_ENTROPY:case C_MAYHEM:case C_NOSTALGIA:case C_STRATAGEM:POWER(id,1);break;
 case C_BEAT_DOWN:regent_push(g,RT_AUTOPLAY,raw,e,3+up,2,0);break;
 case C_BOLAS:case C_THRUMMING_HATCHET:deal(g,e,damage);if(g->regent.return_count<MAX_PILE)g->regent.return_cards[g->regent.return_count++]=regent_clean(raw);break;
 case C_CATASTROPHE:regent_push(g,RT_AUTOPLAY,raw,e,2+up,1,0);break;
 case C_DARK_SHACKLES:regent_temp_weak(g,e,9+6*up);break;
 case C_DISCOVERY:regent_offer(g,raw,e,0,1,depth);break;
 case C_DRAMATIC_ENTRANCE:all_damage(g,damage);break;
 case C_EQUILIBRIUM:card_block(g,block);g->regent.retain_all=1;break;
 case C_ETERNAL_ARMOR:g->ic[IC_PLATING]+=9+3*up;POWER(id,1);break;
 case C_FASTEN:POWER(id,4+2*up);break;
 case C_FINESSE:card_block(g,block);DRAW(1);break;
 case C_FISTICUFFS:{int dealt=e>=0?attack_amount(g,e,damage):0;deal(g,e,damage);card_block(g,dealt);break;}
 case C_FLASH_OF_STEEL:deal(g,e,damage);DRAW(1);break;
 case C_GOLD_AXE:case C_MIND_BLAST:case C_ULTIMATE_STRIKE:deal(g,e,damage);break;
 case C_HAND_OF_GREED:{int hp=e>=0?g->enemies[e].hp:0;deal(g,e,damage);if(hp>0&&g->enemies[e].hp<=0)g->gold+=20+5*up;break;}
 case C_HIDDEN_GEM:{int pool[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!g->regent.instance_replay[CARD_INSTANCE(g->draw[i])]&&CARD_MOD(g->draw[i])!=MOD_ECHO)pool[n++]=i;if(n){int k=pool[rnd(g,n)];g->draw[k]=regent_stamp(g,g->draw[k]);g->regent.instance_replay[CARD_INSTANCE(g->draw[k])]=2+up;}break;}
 case C_IMPATIENCE:{int n=0;for(int i=0;i<g->hand_count;i++)if(cards[CARD_ID(g->hand[i])].type==0)n++;if(!n)DRAW(2+up);break;}
 case C_JACK_OF_ALL_TRADES:for(int i=0;i<1+up;i++)regent_create(g,regent_colorless(g,0));break;
 case C_JACKPOT:{deal(g,e,damage);int pool[512],n=0;for(int c=REGENT_FIRST;c<REGENT_LAST;c++)if(solo_reward_eligible(c)&&c!=R_ROYALTIES&&game_card_cost(c)==0)pool[n++]=c;for(int i=0;i<3&&n;i++)regent_create(g,pool[rnd(g,n)]|(up?UPBIT:0));break;}
 case C_MASTER_OF_STRATEGY:DRAW(3+up);break;
 case C_OMNISLICE:{int hp=e>=0?g->enemies[e].hp:0;deal(g,e,damage);int dealt=e>=0?MAX(0,hp-g->enemies[e].hp):0;for(int z=0;z<g->enemy_count;z++)if(z!=e)ninja_proc(g,z,dealt,0);break;}
 case C_PANACHE:POWER(id,10+4*up);break;
 case C_PANIC_BUTTON:card_block(g,block);g->regent.panic=2;break;
 case C_PREP_TIME:POWER(id,4+2*up);break;
 case C_PRODUCTION:gain_energy(g,2+up);break;
 case C_PROLONG:g->regent.next_block+=g->block;break;
 case C_PROWESS:g->strength+=1+up;g->dexterity+=1+up;POWER(id,1);break;
 case C_PURITY:CHOOSE(RC_EXHAUST,3+2*up);break;
 case C_REND:{int n=e>=0?(g->enemies[e].weak>0)+(g->enemies[e].vulnerable>0)+(g->enemies[e].poison>0)+(g->enemies[e].strength<0):0;deal(g,e,damage+(5+3*up)*n);break;}
 case C_RESTLESSNESS:if(!g->hand_count){DRAW(2+up);gain_energy(g,2+up);}break;
 case C_ROLLING_BOULDER:POWER(id,5+5*up);break;
 case C_SALVO:deal(g,e,damage);g->regent.retain_all=1;break;
 case C_SCRAWL:DRAW(MAX_HAND-g->hand_count);break;
 case C_SECRET_TECHNIQUE:CHOOSE(RC_SEEK_SKILL,1);break;
 case C_SECRET_WEAPON:CHOOSE(RC_SEEK_ATTACK,1);break;
 case C_SEEKER_STRIKE:deal(g,e,damage);regent_seek_three(g,raw,e,depth);break;
 case C_SHOCKWAVE:for(int z=0;z<g->enemy_count;z++){ninja_weak(g,z,3+2*up);apply_vulnerable(g,&g->enemies[z],3+2*up);}break;
 case C_SPLASH:regent_offer(g,raw,e,up,2,depth);break;
 case C_THE_BOMB:{int slot=-1;for(int i=0;i<16;i++)if(g->regent.bomb_ticks[i]==3){slot=i;break;}if(slot<0)for(int i=0;i<16;i++)if(!g->regent.bomb_ticks[i]){slot=i;g->regent.bomb_damage[i]=0;break;}if(slot>=0){g->regent.bomb_ticks[slot]=3;g->regent.bomb_damage[slot]+=40+10*up;}break;}
 case C_THE_GAMBIT:card_block(g,block);g->regent.gambit=1;break;
 case C_THINKING_AHEAD:CHOOSE(RC_TOP,1);DRAW(2);break;
 case C_ULTIMATE_DEFEND:card_block(g,block+RP(g,C_FASTEN));break;
 case C_VOLLEY:for(int i=0;i<MIN(99,x)&&ninja_has_living(g);i++){int foe=preview_trace&&preview_choice>=0?preview_living_foe(g,preview_choice):random_living_foe(g);deal(g,foe,damage);}break;
 }
 if(id==R_ORBIT)POWER(id,1);
 if(cards[id].type==2)g->regent.played[id-REGENT_FIRST]++;
 r151_after_effect(g,before);
#undef DRAW
#undef CHOOSE
#undef POWER
}
