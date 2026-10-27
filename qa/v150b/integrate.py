from pathlib import Path
s=Path(__file__).parent/'source';p=s/'game.c';t=p.read_text()
protos='''
static int ninja_flags(int raw);
static int ninja_owned_card(const Game*g,int id);
static int ninja_base_damage(const Game*g,int raw,int in_hand);
static void ninja_on_draw(Game*g,int raw);
static void ninja_on_hit(Game*g,int enemy,int unblocked);
static void ninja_poison(Game*g,int enemy,int n);
static void ninja_process(Game*g);
static void ninja_start_turn_before(Game*g);
static void ninja_start_turn_after(Game*g);
static int ninja_prepare_retain(Game*g);
static void ninja_choose(Game*g,int index,int finish);
'''
t=t.replace('#include "card_mods.inc"','#include "card_mods.inc"\n'+protos)
needle='const int RELIC_COUNT';end=t.index(needle);pos=t.rfind('};',0,end)
t=t[:pos]+''' {"Serpent Ring","At combat start, draw 2 extra cards."},
 {"Venom Skull","Each Poison application adds 1 extra Poison."},
 {"Mourning Bells","Each card discarded on your turn deals 3 damage to a random enemy."},
 {"Grave Bindings","Each card discarded on your turn grants 3 Block."},
 {"Plague Funnel","At combat start, apply 4 Poison to all enemies."},
 {"Hidden Scroll","At combat start, add 3 Shivs to your hand."},
 {"Paper Raven","Weak enemies deal 40% less attack damage to you."},
 {"Spiral Needle","Playing a Shiv grants 1 Dexterity for this turn."},
 {"Drake Ring","Draw 2 extra cards on each of your first 3 turns. Replaces Serpent Ring."},
'''+t[pos:]
# Verify a trailing comma already separates the original last relic.
t=t.replace('int pool[128],weight[128]','int pool[256],weight[256]')
t=t.replace('id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&cards[id].rarity<=2','id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&id!=N_FLANKING&&id!=N_SNEAKY&&cards[id].rarity<=2')
t=t.replace('if(solo_reward_eligible(id)&&cards[id].rarity','if(solo_reward_eligible(id)&&ninja_owned_card(g,id)&&cards[id].rarity')
t=t.replace(' if(!n)return 5;',' if(!n)return g->character==CHAR_NIGHTSHADE?N_BACKFLIP:5;',1)
t=t.replace('int pool[40],n=0;','int pool[64],n=0;',1)
t=t.replace('if((id<=27||id>=40)&&id!=excluded&&!relic(g,id))','if((id<=27||(id>=40&&id<44)||(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51))&&id!=excluded&&!relic(g,id))')
t=t.replace('static void add_relic(Game*g,int id){','static void add_relic(Game*g,int id){if(id==NR_DRAKE){for(int i=0;i<g->relic_count;i++)if(g->relics[i]==NR_SNAKE){g->relics[i]=NR_DRAKE;return;}}')
t=t.replace(' g->hand[g->hand_count++]=drawn;',' g->hand[g->hand_count++]=drawn;\n ninja_on_draw(g,drawn);',1)
# Build the opening draw with Innate cards before ordinary cards.
t=t.replace('g->discard_count=g->exhaust_count=g->hand_count=0;}','g->discard_count=g->exhaust_count=g->hand_count=0;\n if(g->character==CHAR_NIGHTSHADE){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(ninja_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(ninja_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}\n}',1)
t=t.replace(' if(g->vulnerable) n=n*3/2;',' if(g->vulnerable) n=n*3/2;\n if(g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);',1)
t=t.replace(' player_death(g);\n}\nstatic int attack_amount',' if(g->ninja.v[NS_THORNS]&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int d=g->ninja.v[NS_THORNS],b=MIN(e->block,d);e->block-=b;e->hp-=d-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,d,b);}\n player_death(g);\n}\nstatic int attack_amount',1)
t=t.replace(' return amt;\n}\nstatic void damage_enemy',' if(r140_attack_context&&g->ninja.v[NS_TRACKING]&&e->weak)amt=(int)MIN(1000000000LL,(long long)amt*2);\n if(r140_attack_context&&g->ninja.v[NS_SHADOW_NOW])amt=(int)MIN(1000000000LL,(long long)amt*2);\n return amt;\n}\nstatic void damage_enemy',1)
t=t.replace(' e->hp-=amt;\n if(r140_attack_context',' e->hp-=amt;\n ninja_on_hit(g,idx,amt);\n if(r140_attack_context',1)
t=t.replace(' int gained=MIN(amount,999-g->block);',' if(g->ninja.v[NS_SHADOWMELD])amount=(int)MIN(1000000000LL,(long long)amount*2);\n int gained=MIN(amount,999-g->block);',1)
t=t.replace(' int amount=MIN(n,999-g->block);',' if(g->ninja.v[NS_SHADOWMELD])n=(int)MIN(1000000000LL,(long long)n*2);\n int amount=MIN(n,999-g->block);',1)
t=t.replace('static void check_win(Game*g){\n','static void check_win(Game*g){\n if(g->screen!=COMBAT||g->ninja.task_count>0)return;\n',1)
t=t.replace(' if(id<0||id>=CARD_COUNT)return 0;\n switch(id){',' if(id<0||id>=CARD_COUNT)return 0;\n if(id>=NINJA_FIRST)return ninja_base_damage(g,raw,1);\n switch(id){',1)
t=t.replace(' memset(g->ic,0,sizeof(g->ic));',' memset(g->ic,0,sizeof(g->ic));\n memset(&g->ninja,0,sizeof(g->ninja));',1)
t=t.replace(' g->turn++;',' g->turn++;\n ninja_start_turn_before(g);',1)
t=t.replace('int n=5+(relic(g,18)?1:0)+(g->turn==1?2*relic(g,7):0)-(relic(g,25)?1:0);','int n=5+(relic(g,18)?1:0)+(g->turn==1?2*relic(g,7)+2*relic(g,NR_SNAKE):0)+(g->turn<=3?2*relic(g,NR_DRAKE):0)-(relic(g,25)?1:0)+g->ninja.v[NS_NEXT_DRAW];g->ninja.v[NS_NEXT_DRAW]=0;')
t=t.replace(' g->selection=0;\n}\nstatic int enemy_raw_damage',' g->selection=0;\n ninja_start_turn_after(g);\n}\nstatic int enemy_raw_damage',1)
t=t.replace(' if(g->ic[IC_TANK])raw*=2;',' if(g->ic[IC_TANK])raw*=2;\n if(e->weak&&relic(g,NR_KRANE)){int full=enemy_raw_damage(e);raw=full*3/5;if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;if(g->ic[IC_TANK])raw*=2;}',1)
t=t.replace(' if(g->screen!=COMBAT||g->ic[38])return;',' if(g->screen!=COMBAT||g->ic[38]||g->ninja.task_count)return;\n if(ninja_prepare_retain(g))return;',1)
t=t.replace(' if(g->screen!=COMBAT)return;\n game_prepare_end_turn(g);',' if(g->screen!=COMBAT||g->ninja.task_count)return;\n game_prepare_end_turn(g);',1)
t=t.replace(' g->ic[IC_OWN_TURN]=0;',' g->dexterity-=g->ninja.v[NS_TEMP_DEX];g->ninja.v[NS_TEMP_DEX]=0;\n g->ninja.v[NS_CORROSIVE]=g->ninja.v[NS_SHADOWMELD]=g->ninja.v[NS_BURST]=0;g->ninja.v[NS_STRANGLE0]=g->ninja.v[NS_STRANGLE1]=0;\n g->ic[IC_OWN_TURN]=0;',1)
t=t.replace('  if(CARD_MOD(g->hand[i])==MOD_BOUND)g->hand[retained++]=g->hand[i];','  if(game_card_is_retained(g,g->hand[i]))g->hand[retained++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);')
t=t.replace('g->discard[g->discard_count++]=g->hand[i];','g->discard[g->discard_count++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);',1)
t=t.replace('  if(e->poison){e->hp-=e->poison;e->poison=MAX(0,e->poison-1);}','  for(int p=0;p<=MIN(1000,g->ninja.v[NS_ACCELERANT])&&e->hp>0&&e->poison>0;p++){record_combat_fx(GFX_PROC,-1,k,e->poison,0);e->hp-=e->poison;e->poison--;}')
t=t.replace(' if(!g->ic[IC_BARRICADE])g->block=0;',' if(!g->ic[IC_BARRICADE]&&!g->ninja.v[NS_BLUR])g->block=0;\n if(g->ninja.v[NS_BLUR])g->ninja.v[NS_BLUR]--;\n if(g->ninja.v[NS_INTANGIBLE])g->ninja.v[NS_INTANGIBLE]--;')
t=t.replace(' if(CARD_ID(raw)==3&&g){',' if(g&&id>=NINJA_FIRST){if(cards[id].type==1&&g->ninja.v[NS_POUNCE])return 0;if(id==N_PINPOINT)return MAX(0,game_card_cost(raw)-g->ninja.v[NS_SKILLS_TURN]);if(id==N_UP_MY_SLEEVE)return MAX(0,game_card_cost(raw)-CARD_COUNTER(raw));}\n if(CARD_ID(raw)==3&&g){',1)
t=t.replace('static void autoplay_card(Game*g,int raw,int force_exhaust){','#include "ninja_engine.inc"\nstatic void autoplay_card(Game*g,int raw,int force_exhaust){',1)
t=t.replace(' if(index<0)return;resolve_card(g,index,enemy,-1);',' if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=NINJA_FIRST){ninja_play_from_hand(g,index,enemy);return;}resolve_card(g,index,enemy,-1);')
# Keep random card generation in the chosen character pool.
t=t.replace('if(card<CARD_COUNT&&cards[card].type==0&&','if(card<CARD_COUNT&&ninja_owned_card(g,card)&&cards[card].type==0&&')
t=t.replace('if(solo_reward_eligible(id))','if(solo_reward_eligible(id)&&ninja_owned_card(g,id))')
t=t.replace('void game_update(Game*g){if(g->screen==ACT_INTRO','void game_update(Game*g){if(g->screen==COMBAT&&g->ninja.task_count)ninja_process(g);if(g->screen==ACT_INTRO')
t=t.replace('if(screen==TITLE||screen==HELP)return 0;','if(screen==TITLE||screen==HELP||screen==CHARACTER_SELECT)return 0;')
t=t.replace('if(screen==COMBAT||screen==CARD_CHOICE||','if(screen==COMBAT||screen==CARD_CHOICE||screen==NINJA_CHOICE||')
t=t.replace('g->endless=g->selection==1;begin_act(g);','g->endless=g->selection==1;g->screen=CHARACTER_SELECT;g->selection=g->character;')
t=t.replace(' case HELP:if(b==BTN_BACK',' case CHARACTER_SELECT:\n  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;\n  else if(b==BTN_OK){game_choose_character(g,g->selection);begin_act(g);}\n  else if(b==BTN_BACK){g->screen=TITLE;g->selection=g->endless?1:0;}break;\n case NINJA_CHOICE:{int n=g->hand_count;\n  if(b==BTN_UP&&g->selection>=3)g->selection-=3;\n  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);\n  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;\n  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;\n  else if(b==BTN_OK)ninja_choose(g,g->selection,0);\n  else if(b==BTN_BACK)ninja_choose(g,0,1);break;}\n case HELP:if(b==BTN_BACK',1)
t=t.replace(' case COMBAT:\n  if(b==BTN_LEFT)',' case COMBAT:\n  if(g->ninja.task_count){ninja_process(g);break;}\n  if(b==BTN_LEFT)',1)
t=t.replace('    if(g->win_screen){','    if(g->ninja.v[NS_BONUS_REWARDS]>0){g->ninja.v[NS_BONUS_REWARDS]--;fill_rewards(g);g->selection=0;msg(g,"The Hunt: choose an extra card reward.");break;}\n    if(g->win_screen){',1)
t=t.replace('msg(g,"Onward, Warden.")','msg(g,g->character==CHAR_NIGHTSHADE?"Onward, Nightshade.":"Onward, Warden.")')
t=t.replace('raw&~(0x7ff|CARD_MOD_MASK)','raw&~CARD_RAW_MASK')
t=t.replace('g->screen>EVENT_CARD','g->screen>NINJA_CHOICE')
t=t.replace(' if(!g||g->endless',' if(!g||g->character<0||g->character>1||g->ninja.task_count<0||g->ninja.task_count>NINJA_TASK_MAX||g->endless',1)
t=t.replace(' if(g->screen==MAP&&(g->selection',' if(g->screen==CHARACTER_SELECT&&(g->selection<0||g->selection>1))return 0;\n if(g->screen==NINJA_CHOICE&&(g->ninja.choice_kind<1||g->ninja.choice_kind>4||g->ninja.choice_remaining<1||g->ninja.choice_count<0||g->ninja.choice_count>MAX_HAND||g->selection<0||g->selection>=g->hand_count))return 0;\n if(g->screen==MAP&&(g->selection',1)
t=t.replace(' if(g->ic[id>=93?47:33+id/31]',' if(id>=NINJA_FIRST)return g->ninja.powers[id-NINJA_FIRST];\n if(g->ic[id>=93?47:33+id/31]',1)
t=t.replace('static void power_line(const Game*g,int id,char*out,int cap){','static void power_line(const Game*g,int id,char*out,int cap){\n if(id>=NINJA_FIRST){snprintf(out,cap,"%s x%d",cards[id].name,g->ninja.powers[id-NINJA_FIRST]);return;}')
p.write_text(t)
# Character initializer, called only when a new-run selection is confirmed.
t=p.read_text();at=t.index('void game_update(Game*g)')
t=t[:at]+'''const char *game_character_name(const Game*g){return g&&g->character==CHAR_NIGHTSHADE?"Nightshade":"Warden";}
void game_choose_character(Game*g,int character){
 if(!g||character<0||character>1)return;
 g->character=character;g->deck_count=g->relic_count=0;g->potion_count=0;
 g->max_energy=3;g->maxhp=g->hp=character==CHAR_NIGHTSHADE?70:80;g->gold=character==CHAR_NIGHTSHADE?99:95;
 for(int i=0;i<5;i++)add_card(g,character==CHAR_NIGHTSHADE?N_STRIKE_SILENT:0);
 for(int i=0;i<(character==CHAR_NIGHTSHADE?5:4);i++)add_card(g,character==CHAR_NIGHTSHADE?N_DEFEND_SILENT:1);
 if(character==CHAR_NIGHTSHADE){add_card(g,N_NEUTRALIZE);add_card(g,N_SURVIVOR);add_relic(g,NR_SNAKE);}
 else{add_card(g,2);add_relic(g,0);g->potion_count=2;g->potions[0]=0;g->potions[1]=1;}
 msg(g,character==CHAR_NIGHTSHADE?"Nightshade enters the ruined tower.":"A lone Warden climbs the ruined tower.");
}
'''+t[at:];p.write_text(t)
# Modification eligibility: all new block effects and interactive attacks.
p=s/'card_mods.inc';t=p.read_text().replace('case MOD_ECHO:return type==0&&card_cost_base(raw)>=0&&id!=25;','case MOD_ECHO:return type==0&&card_cost_base(raw)>=0&&id!=25;')
t=t.replace('case MOD_WARD:switch(id){','case MOD_WARD:if(id>=NINJA_FIRST)return ninja_cards[id-NINJA_FIRST].block>0||id==N_MIRAGE;switch(id){')
p.write_text(t)
