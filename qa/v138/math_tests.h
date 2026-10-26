/* Independent arithmetic oracle + actual-resolution comparisons. */
static int qa_cases;
static void math_battle(Game*g,int raw){
 game_new(g,136);game_start_battle(g,NODE_FIGHT);
 g->screen=COMBAT;g->hp=g->maxhp=500;g->relic_count=0;g->strength=g->dexterity=g->weak=g->vulnerable=g->poison=0;
 memset(g->ic,0,sizeof(g->ic));g->ic[IC_OWN_TURN]=1;
 g->shop_cards[0]=g->shop_cards[1]=g->shop_cards[2]=0;g->reward_potion=0;g->heal_after_battle=g->used_potion=0;
 g->energy=10;g->block=0;g->turn_played=0;g->hand_count=4;g->hand[0]=raw;g->hand[1]=0;g->hand[2]=1;g->hand[3]=5;g->selection=g->target=0;
 g->draw_count=3;g->draw[0]=1;g->draw[1]=0;g->draw[2]=1;g->discard_count=0;g->exhaust_count=0;
 g->enemy_count=2;for(int e=0;e<2;e++){memset(&g->enemies[e],0,sizeof(Enemy));g->enemies[e].hp=g->enemies[e].maxhp=999;g->enemies[e].art=e?19:16;snprintf(g->enemies[e].name,24,"%s",game_enemy_name(g->enemies[e].art));g->enemies[e].intent_value=12;}
}
static void assert_desc(Game*g,int raw,const char*part){char desc[320];game_card_description_now(g,raw,g->target,desc,sizeof(desc));if(!strstr(desc,part)){fprintf(stderr,"Card %d expected '%s': %s\n",raw,part,desc);abort();}}
static int oracle(int base,int strength,int weak,int vul,int cruelty,int glass,int skull){
 /* Written in rational arithmetic: floor after each individual modifier. */
 long value=base+strength;if(value<0)value=0;
 if(weak)value=(value*75)/100;
 if(vul){value=(value*150)/100;value=(value*(100+cruelty))/100;}
 return (int)value+glass*5+skull*6;
}
static void math_tests(void){Game g,before,actual;CardPreview p;char text[320];
 assert(sizeof(Game)==3144);
 /* 7,040 combinations: negative/positive Strength, both Weak/Vulnerable,
    Cruelty stacks, first-Attack relic after a Skill, and varied enemy Block. */
 for(int str=-9;str<=12;str++)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++)for(int cruelty=0;cruelty<=100;cruelty+=25)for(int relics=0;relics<4;relics++)for(int block=0;block<=15;block+=5){
  math_battle(&g,0);g.strength=str;g.weak=weak;g.enemies[0].vulnerable=vul;g.ic[IC_CRUELTY]=cruelty;g.enemies[0].block=block;g.turn_played=2;
  if(relics&1)g.relics[g.relic_count++]=3;if(relics&2){g.relics[g.relic_count++]=26;g.hp=1;}
  before=g;game_card_preview(&g,0,0,&p);assert(!memcmp(&g,&before,sizeof(g)));
  int damage=oracle(6,str,weak,vul,cruelty,relics&1,(relics>>1)&1);
  assert(p.low[0]==damage&&p.high[0]==damage&&p.total[0]==damage);
  game_play_card(&g,0,0);assert(g.enemies[0].hp==999-(damage>block?damage-block:0));assert(g.enemies[0].block==(block>damage?block-damage:0));qa_cases++;
 }
 for(int dex=-10;dex<=12;dex++)for(int fort=0;fort<2;fort++)for(int unmovable=0;unmovable<3;unmovable++)for(int initial=0;initial<=999;initial+=333){
  math_battle(&g,1);g.dexterity=dex;g.block=initial;if(fort)g.relics[g.relic_count++]=15;g.ic[IC_UNMOVABLE]=unmovable>0;g.ic[IC_UNMOVABLE_USED]=unmovable==2;
  int expected=5+dex+2*fort;if(expected<0)expected=0;if(unmovable==1)expected*=2;if(expected+initial>999)expected=999-initial;
  game_card_preview(&g,1,0,&p);assert(p.block_total==expected);game_play_card(&g,0,0);assert(g.block==initial+expected);qa_cases++;
 }
 /* Every base/upgraded catalog entry, target, varied statuses, and energy.
    Compare isolated prediction to independently resolved live-copy outcomes. */
 for(int up=0;up<2;up++)for(int id=0;id<CARD_COUNT;id++)for(int target=0;target<2;target++)for(int variant=0;variant<12;variant++){
  int raw=id|(up?UPBIT:0);math_battle(&g,raw);g.target=target;g.strength=variant-4;g.dexterity=variant%5-2;g.weak=variant%2;g.enemies[0].vulnerable=variant%3;g.enemies[1].vulnerable=(variant+1)%3;g.ic[IC_CRUELTY]=(variant%3)*25;g.block=variant*3;
  g.exhaust_count=variant%4;for(int i=0;i<g.exhaust_count;i++)g.exhaust[i]=1;
  g.energy=variant%4;g.ic[IC_HPLOSS_TURN]=variant%2;g.ic[IC_HPLOSS_COMBAT]=variant;g.reward_potion=(variant%2)?4:0;
  before=g;game_card_preview(&g,raw,target,&p);game_card_description_now(&g,raw,target,text,sizeof(text));assert(!memcmp(&g,&before,sizeof(g)));assert(strlen(text)>0&&strlen(text)<319);
  actual=g;int cost=game_card_cost_now(&actual,raw);if(cost>actual.energy)actual.energy=cost;
  game_play_card(&actual,0,target);
  if(!p.random&&!p.uncertain&&id!=57&&id!=68){
   if(cards[id].type==0)for(int e=0;e<2;e++)assert(p.total[e]==g.enemies[e].hp-actual.enemies[e].hp);
   if(p.block_count)assert(p.block_total==actual.block-g.block);
  }
  qa_cases++;
 }
 /* Known troublesome order / rounding cases with literal expected values. */
 math_battle(&g,0);g.strength=3;g.weak=1;g.enemies[0].vulnerable=2;g.ic[IC_CRUELTY]=25;assert_desc(&g,0,"Deal 11 damage");
 math_battle(&g,29);g.shop_cards[0]=2;g.strength=3;g.weak=1;g.enemies[0].vulnerable=2;game_card_preview(&g,29,0,&p);assert(p.low[0]==22);assert_desc(&g,29,"Deal 22 damage");
 math_battle(&g,42);g.strength=2;g.weak=1;g.enemies[0].vulnerable=2;game_card_preview(&g,42,0,&p);assert(p.low[0]==9&&p.hits[0]==3&&p.total[0]==27);assert_desc(&g,42,"9 damage 3 times");
 math_battle(&g,16);g.reward_potion=4;g.dexterity=2;g.ic[IC_UNMOVABLE]=1;game_card_preview(&g,16,0,&p);assert(p.block_first==20&&p.block_second==10&&p.block_total==30);assert_desc(&g,16,"20 Block");assert_desc(&g,16,"10 more Block");
 math_battle(&g,45);g.dexterity=2;g.ic[IC_UNMOVABLE]=1;game_card_preview(&g,45,0,&p);assert(p.block_total==21);assert_desc(&g,45,"7-14 Block each (21 now)");
 math_battle(&g,27);g.hp=1;g.relic_count=2;g.relics[0]=26;g.relics[1]=24;game_card_preview(&g,27,0,&p);assert(p.low[0]==5&&p.high[0]==11&&p.total[0]==16);assert_desc(&g,27,"5-11 damage twice");
 math_battle(&g,2);g.enemy_count=1;g.ic[IC_NEXT_ATTACK_REPEAT]=1;game_card_preview(&g,2,0,&p);assert(p.low[0]==8&&p.high[0]==12&&p.total[0]==20&&p.repeated);assert_desc(&g,2,"8-12 damage");assert_desc(&g,2,"Plays twice");
 math_battle(&g,80);g.enemy_count=1;g.ic[IC_NEXT_ATTACK_REPEAT]=1;game_card_preview(&g,80,0,&p);assert(p.low[0]==7&&p.high[0]==9&&p.total[0]==16);
 math_battle(&g,87);g.hand_count=3;g.hand[1]=0;g.hand[2]=7;g.strength=2;g.weak=1;g.enemies[0].vulnerable=1;game_card_preview(&g,87,0,&p);assert(p.low[0]==13&&p.high[0]==42);assert_desc(&g,87,"13-42 twice");
 math_battle(&g,19);g.strength=2;g.enemies[1].vulnerable=1;game_card_preview(&g,19,0,&p);assert(p.low[0]==6&&p.low[1]==9);assert_desc(&g,19,"6-9");
 math_battle(&g,10);g.energy=0;game_card_preview(&g,10,0,&p);assert(p.total[0]==0);assert_desc(&g,10,"0 times (X)");
 math_battle(&g,0);g.enemies[0].hp=3;g.enemies[0].block=2;game_card_preview(&g,0,0,&p);assert(p.low[0]==6);game_play_card(&g,0,0);assert(g.enemies[0].hp==-1);
 math_battle(&g,29);g.hp=2;game_card_preview(&g,29,0,&p);assert(p.total[0]==0);game_play_card(&g,0,0);assert(g.screen==DEFEAT&&g.enemies[0].hp==999);
 math_battle(&g,0);g.energy=0;assert_desc(&g,0,"Deal 6 damage");assert(g.energy==0);
 /* Randomness stays intact: previewing produces precisely the same next play. */
 for(int seed=1;seed<=300;seed++)for(int id=0;id<CARD_COUNT;id++){
  math_battle(&g,id);g.rng=seed;g.ic[IC_DARK_EMBRACE]=1;g.ic[IC_HELLRAISER]=1;g.ic[IC_NEXT_ATTACK_REPEAT]=1;g.shop_cards[0]=2;g.shop_cards[2]=3;before=g;actual=g;
  game_card_description_now(&g,id,0,text,sizeof(text));assert(!memcmp(&g,&before,sizeof(g)));
  game_play_card(&g,0,0);game_play_card(&actual,0,0);assert(!memcmp(&g,&actual,sizeof(g)));qa_cases++;
 }
 /* Every playable Power records its name and persists without changing save ABI. */
 for(int id=0;id<CARD_COUNT;id++)if(cards[id].type==2&&id!=85){
  math_battle(&g,id);game_play_card(&g,0,0);int found=0;
  for(int i=0;i<game_status_count(&g);i++){game_status_line(&g,i,text,sizeof(text));if(!strncmp(text,cards[id].name,strlen(cards[id].name))&&text[strlen(cards[id].name)]==':')found++;}assert(found==1);
 }
 math_battle(&g,8);game_play_card(&g,0,0);assert(game_save(&g,"../evidence138/math-save.dat"));assert(game_load(&actual,"../evidence138/math-save.dat"));assert(!memcmp(&g,&actual,sizeof(g)));
 math_battle(&g,0);g.enemies[0].strength=-2;g.enemies[0].weak=2;g.enemies[0].vulnerable=1;g.enemies[0].poison=4;g.ic[IC_MANGLE0]=10;assert(game_enemy_status_count(&g,0)==5);game_enemy_status_line(&g,0,4,text,sizeof(text));assert(strstr(text,"Mangle"));
 assert(game_load(&actual,"qa/v136/fixtures/legacy-1.3.5.dat"));
 printf("PASS: %d arithmetic/resolution/RNG cases; all 184 card variants; all played Powers; enemy statuses; save ABI; modifier order, caps, death, random ranges, repeated hits and self-damage\n",qa_cases);
}
