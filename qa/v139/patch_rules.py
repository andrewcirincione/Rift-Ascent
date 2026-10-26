from pathlib import Path
q=Path(__file__).parent/'source';p=q/'game.h';s=p.read_text().replace('CARD_CHOICE,POTION_VIEW};','CARD_CHOICE,POTION_VIEW,BLESSING,REWARD_PREVIEW};').replace('GFX_PROC,GFX_SELF};','GFX_PROC,GFX_SELF,GFX_STATUS};');s=s.replace('void game_new(Game *g,uint32_t seed);','void game_new(Game *g,uint32_t seed);\nint game_moon_heal_now(const Game *g);');p.write_text(s)
p=q/'game.c';s=p.read_text();a=s.index('const RelicDef relic_defs[]');b=s.index('const int RELIC_COUNT',a)
rows=[('Cinder Heart','After each won combat, heal 6 HP.'),('Copper Ring','On the first turn of each combat, gain 1 extra Energy.'),('Spiked Mail','When an enemy attacks you, deal 4 damage to that enemy, even if you Block.'),('Glass Emblem','Each turn, every hit of your first Attack card deals +5 damage.'),('Stone Flask','On pickup, gain 14 maximum HP and heal 14 HP.'),('Fury Censer','Every 3 Attack cards played in one turn, gain 1 Strength for this combat.'),('Merchant Coin','Cards and relics cost 30 less Gold in shops.'),('Old Compass','Draw 2 extra cards on the first turn of each combat.'),('Resting Idol','Resting also lets you upgrade one card, if any are eligible.'),('Rust Amulet','Each turn, your first Attack applies 2 Weak to each enemy it hits.'),('Guardian Knot','Start each combat with 12 Block.'),('Ashen Crown','On pickup, gain 1 Energy per turn; lose 12 maximum HP.'),('Medal of Valor','After each won elite fight, gain 1 permanent Strength for future combats.'),('Locket','Every card offered as a combat reward is upgraded.'),('Burning Thorn','At turn start, if at half HP or less, gain 3 Strength. Once per combat.'),('Fortress Pin','Each card effect that grants Block grants 2 extra Block.'),('Gold Tooth','After each won combat, gain 20 extra Gold.'),('Violet Orb','Potions: +5 healing, Block or damage; +1 Strength, Energy or card drawn.'),('Sage\'s Ink','Draw 1 extra card at the start of every turn.'),('Broken Clock','At combat start, apply 2 Vulnerable to all enemies.'),('Iron Seed','At the start of turns 3, 6, 9 and so on, heal 4 HP.'),('Wildflower','Whenever you use a potion, gain 12 Block.'),('Worn Chain','Every 3 Skill cards played in one turn, gain 1 Dexterity for this combat.'),('Last Ember','Once per act, lethal damage instead restores you to 25% of maximum HP.'),('Bloodstone','Every 3 Attack cards played in one turn, heal 3 HP.'),('Sun Token','Gain 1 Energy per turn. Draw 1 fewer card at each turn start.'),('Hollow Skull','While at half HP or less, each hit of your Attacks deals +3 damage.'),('Scavenger','Whenever you skip a combat card reward, heal 6 HP.'),('Vow of Steel','Start every combat with 1 additional Strength.'),('Sanctuary','Start every combat with 10 additional Block.'),('Seer\'s Oath','Draw 2 additional cards on the first turn of every combat.'),('Vital Offering','On pickup, gain 12 maximum HP and heal to full.'),('Pilgrim\'s Purse','On pickup, gain 100 Gold.'),('Ember Pact','Gain 1 additional Energy on the first turn of every combat.'),('Blood Vow','At the start of turns 3, 6, 9 and so on, gain 1 Strength.'),('Bulwark Oath','Start every combat with 1 additional Dexterity.'),('Quiet Mercy','After every won combat, heal 3 additional HP.')]
import json
s=s[:a]+'const RelicDef relic_defs[]={\n'+',\n'.join(' {'+json.dumps(n)+','+json.dumps(d)+'}' for n,d in rows)+'\n};\n'+s[b:]
a=s.index('static int card_reward(Game*g)');b=s.index('static void add_relic',a)
s=s[:a]+'''static int reward_of_rarity(Game*g,int rarity,int excluded0,int excluded1){
 int pool[92],n=0;
 for(int id=3;id<CARD_COUNT;id++)if(solo_reward_eligible(id)&&cards[id].rarity==rarity&&id!=excluded0&&id!=excluded1)pool[n++]=id;
 return n?pool[rnd(g,n)]:5;
}
/* Events and shops have their own pool; combat rarity is explicit below. */
static int card_reward(Game*g){int roll=rnd(g,100);return reward_of_rarity(g,roll<9?2:roll<46?1:0,-1,-1);}
static void fill_rewards(Game*g){
 for(int i=0;i<3;i++){
  int roll=rnd(g,100),rare=g->battle_type==NODE_BOSS?2:g->battle_type==NODE_ELITE?(roll<10?2:roll<50?1:0):(roll<37?1:0);
  int c=reward_of_rarity(g,rare,i>0?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1);
  int upgraded=relic(g,13)||(rare!=2&&rnd(g,100)<(g->act-1)*25);
  g->reward[i]=c|(upgraded?UPBIT:0);
 }
}
'''+s[b:]
s=s.replace('if(id==4){g->maxhp+=10;g->hp+=10;}','if(id==4){g->maxhp+=14;g->hp+=14;}if(id==31){g->maxhp+=12;g->hp=g->maxhp;}if(id==32)g->gold+=100;')
s=s.replace('1+rnd(g,RELIC_COUNT-1)','1+rnd(g,27)')
pos=s.index('/* Deterministic branching edges:')
s=s[:pos]+'''static void offer_blessing(Game*g){
 int pool[9],n=0;for(int id=28;id<37;id++)if(!relic(g,id))pool[n++]=id;
 for(int i=0;i<3;i++){int pick=rnd(g,n);g->reward[i]=pool[pick];pool[pick]=pool[--n];}
 g->screen=BLESSING;g->selection=g->scroll=0;
 msg(g,"Choose one blessing for your ascent.");
}
int game_moon_heal_now(const Game*g){return CLAMP(g->scroll,0,MIN(20,MIN(g->gold/2,g->maxhp-g->hp)));}
'''+s[pos:]
s=s.replace('g->defense_event=g->act;g->hp=1;','g->defense_event=g->act;g->hp=MAX(1,g->maxhp/4);')
s=s.replace('if(relic(g,26)&&g->hp==1)amt+=6;','if(relic(g,26)&&g->hp*2<=g->maxhp)amt+=3;')
s=s.replace('e->weak=MAX(e->weak,1);','e->weak=MAX(e->weak,2);').replace(' if(relic(g,24))heal(g,1);','')
s=s.replace('  if(relic(g,2)){for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp-=3;}}',' }\n if(relic(g,2)&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int absorbed=MIN(4,e->block);e->block-=absorbed;e->hp-=4-absorbed;record_combat_fx(GFX_PROC,-1,combat_fx_source,4,absorbed);}')
s=s.replace('(relic(g,16)?12:0)+(relic(g,27)?10:0)','(relic(g,16)?20:0)').replace('if(relic(g,0))heal(g,6);','if(relic(g,0))heal(g,6);if(relic(g,36))heal(g,3);')
s=s.replace('g->permanent_strength+(relic(g,5)?1:0)','g->permanent_strength+(relic(g,28)?1:0)').replace('g->dexterity=relic(g,22)?1:0;','g->dexterity=relic(g,35)?1:0;')
s=s.replace('if(relic(g,10))g->block+=8;','if(relic(g,10))g->block+=12;if(relic(g,29))g->block+=10;')
s=s.replace('g->ic[IC_ATTACKS_THIS_TURN]=0;','g->ic[IC_ATTACKS_THIS_TURN]=0;g->ic[36]=0;')
s=s.replace('(g->turn==1&&relic(g,1)?1:0)','(g->turn==1?(relic(g,1)+relic(g,33)):0)')
s=s.replace('if(relic(g,20)&&g->turn%3==0)heal(g,2);','if(relic(g,20)&&g->turn%3==0)heal(g,4);if(relic(g,34)&&g->turn%3==0)g->strength++;')
s=s.replace('{g->strength++;g->low_hp_triggered=1;}','{g->strength+=3;g->low_hp_triggered=1;}')
s=s.replace('(g->turn==1&&relic(g,7)?1:0)','(g->turn==1?2*(relic(g,7)+relic(g,30)):0)')
s=s.replace('g->ic[IC_ATTACKS_THIS_TURN]++;','g->ic[IC_ATTACKS_THIS_TURN]++;\n  if(g->ic[IC_ATTACKS_THIS_TURN]%3==0){if(relic(g,5))g->strength++;if(relic(g,24))heal(g,3);}')
s=s.replace(' if(repeat_this_attack&&',' if(cards[id].type==1&&++g->ic[36]%3==0&&relic(g,22))g->dexterity++;\n if(repeat_this_attack&&')
s=s.replace('hit<2&&g->screen==COMBAT','hit<2&&g->screen==COMBAT&&e->hp>0')
s=s.replace('int percent=g->act==1?94:g->act==2?108:128;','int percent=g->act==1?94:g->act==2?108:128;\n if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=7;')
s=s.replace('int percent=g->act==1?82:g->act==2?95:114;','int percent=g->act==1?82:g->act==2?95:114;\n if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=5;')
s=s.replace('int paid=MIN(g->gold,40);\n   int restored=MIN(20,paid/2);','int restored=game_moon_heal_now(g);')
s=s.replace('g->effect_turn=rnd(g,12);msg','g->effect_turn=rnd(g,12);g->scroll=MIN(20,MIN(g->gold/2,g->maxhp-g->hp));msg')
s=s.replace('int discount=relic(g,6)?15:0;','int discount=relic(g,6)?30:0;')
s=s.replace('if(relic(g,21)){g->block+=6;record_combat_fx(GFX_BLOCK,-1,-1,6,0);}','if(relic(g,21)){g->block+=12;record_combat_fx(GFX_BLOCK,-1,-1,12,0);}')
s=s.replace('if(option==0){heal(g,g->maxhp*3/10+(relic(g,8)?5:0));enter_map(g);msg(g,"Recovered at the campfire.");}', 'if(option==0){heal(g,g->maxhp*3/10);if(relic(g,8)&&count_upgradeable(g)){g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=g->scroll=0;msg(g,"Rested. Choose an upgrade.");}else{enter_map(g);msg(g,"Recovered at the campfire.");}}')
s=s.replace('g->screen=MAP;msg(g,"Choose your first encounter.");enter_map(g);','offer_blessing(g);')
s=s.replace('case REWARD:\n  if(b==BTN_DOWN)','case REWARD:\n  if(b==BTN_ALT){g->shop_selection=g->selection;g->prior_screen=REWARD;g->screen=DECK_VIEW;g->deck_readonly=1;g->selection=g->scroll=0;}\n  else if(b==BTN_POTION&&g->selection<3){g->screen=REWARD_PREVIEW;}\n  else if(b==BTN_DOWN)')
s=s.replace('if(g->selection<3)add_card(g,g->reward[g->selection]);','if(g->selection<3)add_card(g,g->reward[g->selection]);else if(relic(g,27))heal(g,6);')
s=s.replace('else{heal(g,g->maxhp/3);g->act++;g->floor=0;g->lane=1;\n        map_generate(g);enter_map(g);msg(g,"A new ascent begins.");}','else{g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;\n        map_generate(g);offer_blessing(g);}')
s=s.replace('case EVENT:\n  if(b==BTN_LEFT','case EVENT:\n  if(g->effect_turn==1&&b==BTN_UP)g->scroll=MIN(g->scroll+1,MIN(20,MIN(g->gold/2,g->maxhp-g->hp)));\n  else if(g->effect_turn==1&&b==BTN_DOWN)g->scroll=MAX(0,g->scroll-1);\n  else if(b==BTN_LEFT')
s=s.replace('g->prior_screen==SHOP?g->shop_selection:0','(g->prior_screen==SHOP||g->prior_screen==REWARD)?g->shop_selection:0')
s=s.replace(' case POTION_VIEW:', ''' case BLESSING:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK){add_relic(g,g->reward[g->selection]);enter_map(g);}
  break;
 case REWARD_PREVIEW:
  if(b==BTN_LEFT)g->selection=(g->selection+2)%3;
  else if(b==BTN_RIGHT)g->selection=(g->selection+1)%3;
  else if(b==BTN_BACK||b==BTN_POTION||b==BTN_OK)g->screen=REWARD;
  break;
 case POTION_VIEW:''')
s=s.replace('g->screen>POTION_VIEW','g->screen>REWARD_PREVIEW')
s=s.replace(' if(g->screen==BOSS_REWARD&&(g->selection<0||g->selection>3))return 0;',' if(g->screen==BOSS_REWARD&&(g->selection<0||g->selection>3))return 0;\n if((g->screen==BLESSING||g->screen==REWARD_PREVIEW)&&(g->selection<0||g->selection>2))return 0;\n if(g->screen==BLESSING)for(int i=0;i<3;i++)if(g->reward[i]<28||g->reward[i]>=37)return 0;\n if(g->screen==REWARD_PREVIEW)for(int i=0;i<3;i++)if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;')
s=s.replace('Gain 1 Energy per Attack in Hand. No more Energy.','Gain 1 Energy per Attack in your hand. Then you cannot gain Energy this turn.')
p.write_text(s)
