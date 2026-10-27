/* Independent card-by-card arithmetic and state-transition audit. */
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
static int r1310_oracle(int base,int strength,int weak,int vulnerable,int cruelty,int glass){
 long n=base+strength;if(n<0)n=0;
 if(weak)n=n*75/100;
 if(vulnerable){n=n*150/100;n=n*(100+cruelty)/100;}
 return (int)n+(glass?5:0);
}
static void audit_fixture(Game*g,int raw){
 math_battle(g,raw);g->energy=4;g->hand_count=4;g->hand[1]=0;g->hand[2]=1;g->hand[3]=1;
 g->draw_count=g->discard_count=0;g->deck_count=3;g->deck[0]=0;g->deck[1]=80;g->deck[2]=49;
 for(int i=0;i<2;i++)g->enemies[i].hp=g->enemies[i].maxhp=30000;
}
static int audit_base(const Game*g,int id,int up,int target,int*hits,int*all){
 *hits=1;*all=0;
 switch(id){
 case 0:return 6+3*up;case 2:return 8+3*up;
 case 3:*all=1;return 12+3*up;case 7:return 32+6*up;
 case 10:*all=1;*hits=g->energy;return 5+3*up;
 case 11:return 9+up;case 12:return 5+3*up;
 case 13:return 6+(3+up)*g->exhaust_count;
 case 14:*all=1;return 9+4*up;case 15:return 18+6*up;
 case 19:*all=1;return 4+3*up;case 21:return g->block;
 case 22:*hits=3+up;return 3;
 case 25:return 9+3*up;case 26:return 6+3*up;
 case 27:*hits=2;return 5+3*up;case 29:return 15+5*up;
 case 32:*hits=g->enemies[target].vulnerable?2:1;return 8+2*up;
 case 35:return 4+(2+up)*g->enemies[target].vulnerable;
 case 36:*all=1;*hits=4+up;return 2;
 case 37:*all=1;*hits=g->exhaust_count>=3;return 18+6*up;
 case 39:return 13+3*up;case 40:return 10+4*up;
 case 42:*hits=3;return 7+3*up;case 44:return 6+3*up;
 case 47:return 10+2*up;case 49:return 6+(2+up)*3;
 case 56:return 20+10*up;case 66:*hits=2;return 5+up;
 case 70:*all=1;return 16+5*up;case 74:return 15+5*up;
 case 79:return 9+g->ic[IC_RAMPAGE];case 80:return 7+2*up;
 case 81:*hits=g->ic[IC_HPLOSS_TURN]?2+up:1;return 5;
 case 86:*hits=1+g->ic[IC_HPLOSS_COMBAT];return 5+2*up;
 case 87:*hits=2;return 4+2*up+6; /* Only the ordinary Strike can be exhausted. */
 case 89:return 14+6*up;case 91:return 16+4*up;
 case 92:return 14+5*up;case 94:return 8+3*up;
 default:fprintf(stderr,"Missing independent Attack oracle: %d\n",id);abort();
 }
}
static void audit1310_tests(void){
 Game g,before,loaded;CardPreview preview;char desc[320],intent[128];int count,tests=0,catalog=0;
 int strengths[]={-12,-1,0,1,3,7,20};
 for(int id=0;id<96;id++)if(cards[id].type==0){catalog++;
  for(int up=0;up<2;up++)for(int si=0;si<7;si++)for(int weak=0;weak<2;weak++)for(int vul=0;vul<3;vul++)for(int branch=0;branch<2;branch++)for(int target=0;target<2;target++)for(int block=0;block<2;block++)for(int glass=0;glass<2;glass++){
   int raw=id|(up?UPBIT:0);audit_fixture(&g,raw);g.target=target;g.strength=strengths[si];g.weak=weak;g.block=17;
   g.enemies[0].vulnerable=vul==2?4:vul;g.enemies[1].vulnerable=(vul+1)%3;g.enemies[0].block=g.enemies[1].block=block?17:0;
   g.ic[IC_CRUELTY]=branch?25:0;g.ic[IC_HPLOSS_TURN]=branch;g.ic[IC_HPLOSS_COMBAT]=branch?4:0;g.ic[IC_RAMPAGE]=branch?11:0;
   g.exhaust_count=branch?3:0;for(int i=0;i<g.exhaust_count;i++)g.exhaust[i]=1;
   g.shop_cards[0]=branch?2:0;if(glass)g.relics[g.relic_count++]=3;
   before=g;int hits,all,base=audit_base(&g,id,up,target,&hits,&all);
   int str=g.strength+((id==14||id==29)?g.shop_cards[0]:0);
   game_card_preview(&g,raw,target,&preview);game_card_description_now(&g,raw,target,desc,sizeof(desc));assert(!memcmp(&g,&before,sizeof(g)));
   game_combat_fx_begin();game_play_card(&g,0,target);game_combat_fx_end();const GameCombatFx*fx=game_combat_fx_events(&count);
   int seen=0,total[2]={0},absorbed[2]={0},perfoe[2]={0};
   for(int i=0;i<count;i++)if(fx[i].kind==GFX_ATTACK&&fx[i].source==-1){
    int foe=fx[i].target;int expected=r1310_oracle(base,str,weak,before.enemies[foe].vulnerable,before.ic[IC_CRUELTY],glass);
    if(fx[i].amount!=expected){fprintf(stderr,"Attack mismatch id=%d up=%d str=%d weak=%d vul=%d expected=%d got=%d\n",id,up,str,weak,before.enemies[foe].vulnerable,expected,fx[i].amount);abort();}
    seen++;perfoe[foe]++;total[foe]+=expected;absorbed[foe]+=fx[i].absorbed;
   }
   assert(seen==hits*(all?2:1));
   for(int e=0;e<2;e++){
    assert(before.enemies[e].hp-g.enemies[e].hp==total[e]-absorbed[e]);
    assert(g.enemies[e].block==MAX(0,before.enemies[e].block-total[e]-(id==92&&e==target?6+3*up:0)));
    if(id!=22){assert(perfoe[e]==(all||e==target?hits:0));assert(preview.total[e]==total[e]);}
   }
   tests++;
  }
 }
 printf("PASS independent arithmetic for all %d Attack cards, base/upgraded: %d cases across Strength, Weak, both enemies' Vulnerable, Cruelty, Glass Emblem, Block and conditional branches\n",catalog,tests);
 int whirlwind=0;
 for(int up=0;up<2;up++)for(int energy=0;energy<=6;energy++)for(int str=-8;str<=20;str+=2)for(int weak=0;weak<2;weak++)for(int vul=0;vul<3;vul++)for(int free=0;free<3;free++)for(int repeat=0;repeat<2;repeat++)for(int glass=0;glass<2;glass++)for(int block=0;block<2;block++){
  int raw=10|(up?UPBIT:0)|(free==1?CARD_FREE:0);audit_fixture(&g,raw);g.energy=energy;g.strength=str;g.weak=weak;g.ic[IC_CRUELTY]=25;
  g.enemies[0].vulnerable=vul;g.enemies[1].vulnerable=(vul+1)%3;g.enemies[0].block=g.enemies[1].block=block?23:0;
  g.ic[IC_NEXT_ATTACK_REPEAT]=repeat;g.ic[IC_UNRELENTING]=free==2;if(glass)g.relics[g.relic_count++]=3;
  before=g;game_card_preview(&g,raw,0,&preview);assert(!memcmp(&before,&g,sizeof(g)));
  game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();const GameCombatFx*fx=game_combat_fx_events(&count);int hits[2]={0},total[2]={0};
  for(int i=0;i<count;i++)if(fx[i].kind==GFX_ATTACK){int e=fx[i].target;int expected=r1310_oracle(5+3*up,str,weak,before.enemies[e].vulnerable,25,glass&&hits[e]<energy);assert(fx[i].amount==expected);hits[e]++;total[e]+=expected;}
  for(int e=0;e<2;e++){assert(hits[e]==energy*(1+repeat));assert(preview.total[e]==total[e]);assert(preview.hits[e]==hits[e]);assert(g.enemies[e].hp==30000-MAX(0,total[e]-before.enemies[e].block));}
  assert(g.energy==(free?energy:0));whirlwind++;
 }
 printf("PASS Whirlwind: %d independent cases, X=0..6, base/upgraded, negative/positive Strength, Weak, separate Vulnerable targets, Cruelty, Block, free cards, Unrelenting and One-Two Punch\n",whirlwind);
 /* A temporary Strength bonus must not erase permanent potion Strength. */
 int potions=0;
 for(int slot=0;slot<3;slot++)for(int bonus=0;bonus<2;bonus++)for(int str=-8;str<=20;str++)for(int temp=0;temp<2;temp++){
  audit_fixture(&g,0);g.strength=str+(temp?3:0);g.ic[IC_TEMP_STRENGTH]=temp?3:0;g.potion_count=3;g.potions[0]=g.potions[1]=g.potions[2]=0;g.potions[slot]=1;
  if(bonus)g.relics[g.relic_count++]=17;int old=g.strength;game_use_potion(&g,slot);assert(g.strength==old+2+bonus&&g.potion_count==2);assert(strstr(g.message,"Strength +"));
  game_card_preview(&g,0,0,&preview);assert(preview.low[0]==MAX(0,6+g.strength));
  for(int e=0;e<2;e++)g.enemies[e].intent=1;game_end_turn(&g);assert(g.strength==str+2+bonus);potions++;
 }
 printf("PASS Strength potion: %d slot/relic/negative-Strength/temporary-buff cases; immediate preview update and persistence into next turn\n",potions);
 /* Every written damage intent agrees with actual resolution, including rounding. */
 int intents=0;
 for(int move=0;move<14;move++)for(int val=1;val<=19;val+=3)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++)for(int colossus=0;colossus<2;colossus++)for(int tank=0;tank<2;tank++){
  audit_fixture(&g,0);g.enemy_count=1;g.block=0;g.vulnerable=vul;g.ic[IC_COLOSSUS]=colossus;g.ic[IC_TANK]=tank;g.enemies[0].vulnerable=1;g.enemies[0].intent=move;g.enemies[0].intent_value=val;g.enemies[0].weak=weak;g.enemies[0].strength=3;
  int expected=game_enemy_intent_damage(&g,0);game_enemy_intent_text(&g,0,intent,sizeof(intent));assert(intent[0]);before=g;
  game_combat_fx_begin();game_end_turn(&g);game_combat_fx_end();const GameCombatFx*fx=game_combat_fx_events(&count);int dealt=0;for(int i=0;i<count;i++)if(fx[i].kind==GFX_ATTACK&&fx[i].source==0)dealt+=fx[i].amount;
  assert(expected==dealt);intents++;
 }
 printf("PASS all 14 written intents against resolution in %d modifier combinations\n",intents);
 game_new(&g,1310);game_begin_event(&g,2);int old=g.deck_count;game_action(&g,BTN_OK);assert(g.screen==EVENT_RESULT&&g.deck_count==old+1&&CARD_UP(g.reward[0]));assert(g.deck[old]==g.reward[0]);assert(game_save(&g,"qa/v150b/evidence/legacy-event-reveal-save.dat"));assert(game_load(&loaded,"qa/v150b/evidence/legacy-event-reveal-save.dat"));game_action(&loaded,BTN_OK);assert(loaded.screen==MAP&&loaded.deck_count==old+1);
 int found=0;for(int seed=1;seed<=200;seed++){game_new(&g,seed);game_start_battle(&g,NODE_FIGHT);g.potion_count=0;for(int e=0;e<g.enemy_count;e++)g.enemies[e].hp=0;game_end_turn(&g);if(g.potion_count){assert(g.ic[37]==g.potions[0]+1);found++;before=g;game_action(&g,BTN_ALT);game_action(&g,BTN_BACK);assert(g.ic[37]==before.ic[37]);}}assert(found>20);
 puts("PASS upgraded event reveal, save/resume without duplicate card, and persistent potion reward notices");
 /* Exercise renderer cache invalidation, not only the arithmetic API. */
 for(int starting=-2;starting<=10;starting++){
  audit_fixture(&g,0);g.strength=starting;card_combat_context=&g;
  char old[320],expected[320];snprintf(old,sizeof(old),"%s",r136_rules(0));
  g.potion_count=1;g.potions[0]=1;game_use_potion(&g,0);
  game_card_description_now(&g,0,0,expected,sizeof(expected));assert(!strcmp(r136_rules(0),expected));assert(strcmp(old,expected));
 }
 card_combat_context=0;
 for(int art=0;art<ENEMY_COUNT;art++)for(int move=0;move<INTENT_COUNT;move++){
  audit_fixture(&g,0);g.enemies[0].art=art;g.enemies[0].intent=move;g.enemies[0].intent_value=999;
  game_enemy_intent_text(&g,0,intent,sizeof(intent));snprintf(desc,sizeof(desc),"%s: %s",game_enemy_name(art),intent);assert(r134_width(desc,&r134_f8)<=464);
 }
 puts("PASS potion invalidates rendered card descriptions; all 792 enemy-name/intent combinations fit the top rows");
 /* Current release screenshots and emulator fixtures. */
 game_new(&g,1310);game_begin_event(&g,2);shot(&g,"ancient-lesson-before");game_action(&g,BTN_OK);shot(&g,"ancient-lesson-reveal");
 battle(&g);g.enemy_count=1;g.enemies[0].art=9;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(9));g.enemies[0].intent=7;g.enemies[0].intent_value=10;g.selection=g.hand_count;shot(&g,"gothic-hound");
 for(int i=0;i<36;i++){battle(&g);g.enemy_count=1;g.enemies[0].art=i;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(i));g.selection=g.hand_count;char n[64];snprintf(n,sizeof(n),"sized-enemy-%02d",i);shot(&g,n);}
 battle(&g);g.hand_count=10;for(int i=0;i<10;i++)g.hand[i]=i==1?80:i%2;g.selection=1;shot(&g,"hand-arrow-right");g.selection=7;shot(&g,"hand-arrow-left");
 audit_fixture(&g,10);g.hp=g.maxhp=80;g.energy=3;g.strength=4;g.weak=1;g.enemies[0].hp=g.enemies[0].maxhp=180;g.enemies[1].hp=g.enemies[1].maxhp=180;g.enemies[0].vulnerable=2;g.enemies[0].intent=10;g.enemies[1].intent=5;g.block=123;g.enemies[0].block=12;g.enemies[1].block=99;shot(&g,"whirlwind-modifiers");
 g.ic[IC_NEXT_ATTACK_REPEAT]=1;g.relics[g.relic_count++]=3;shot(&g,"whirlwind-repeat");
 battle(&g);g.strength=0;g.ic[IC_TEMP_STRENGTH]=0;g.potion_count=3;g.potions[0]=0;g.potions[1]=1;g.potions[2]=2;shot(&g,"strength-potion");
 game_new(&g,1310);g.screen=REWARD;g.reward[0]=5;g.reward[1]=80;g.reward[2]=31;g.ic[37]=2;g.selection=1;shot(&g,"potion-reward");
 battle(&g);g.energy=3;g.strength=2;g.hand_count=7;int release_hand[]={80,1,10,25,30,5,27};memcpy(g.hand,release_hand,sizeof(release_hand));g.selection=0;
 g.enemies[0].art=9;g.enemies[0].hp=g.enemies[0].maxhp=70;g.enemies[0].block=0;g.enemies[0].intent=7;g.enemies[0].intent_value=10;
 g.enemies[1].art=5;g.enemies[1].hp=g.enemies[1].maxhp=98;g.enemies[1].block=8;g.enemies[1].intent=5;g.enemies[1].intent_value=12;
 for(int e=0;e<2;e++)snprintf(g.enemies[e].name,24,"%s",game_enemy_name(g.enemies[e].art));shot(&g,"release-combat");
 g.selection=g.hand_count;shot(&g,"release-combat-end");
 r1310_potion_notice[0]=0;
}
