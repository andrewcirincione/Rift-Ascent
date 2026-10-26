static void patch139_tests(void){
 /* Compare generated pose foot anchors with the actual idle sprite, excluding the grounded sword tip. */
 int idle_min=999,idle_max=-1;
 for(int y=RIFT_HERO_H-12;y<RIFT_HERO_H;y++)for(int x=0;x<175;x++)if(rift_hero[y*RIFT_HERO_W+x]!=RIFT_TRANSPARENT_565){if(x<idle_min)idle_min=x;if(x>idle_max)idle_max=x;}
 double idle_x=41+(idle_min+idle_max)*75.0/RIFT_HERO_W;
 for(int frame=0;frame<6;frame++){int lo=999,hi=-1;
  for(int y=128;y<144;y++)for(int x=0;x<208;x++)if((r139_attack[frame][y*208+x]>>24)>32){if(x<lo)lo=x;if(x>hi)hi=x;}
  double center=51+(lo+hi)*81.5/208;assert(fabs(center-idle_x)<1.5);
 }
 puts("PASS all six attack stance anchors remain within 1.5 pixels of the idle stance");
 Game g,t,b;int n,cases=0;const GameCombatFx*fx;char path[120];
 /* User-controlled partial payment, missing HP, odd Gold and zero boundaries. */
 for(int gold=0;gold<=45;gold++)for(int missing=0;missing<=25;missing++)for(int wanted=0;wanted<=20;wanted++){
  game_new(&g,139);g.screen=EVENT;g.effect_turn=1;g.gold=gold;g.hp=g.maxhp-missing;g.scroll=wanted;
  int expected=MIN(wanted,MIN(missing,gold/2));assert(game_moon_heal_now(&g)==expected);game_action(&g,BTN_OK);assert(g.hp==g.maxhp-missing+expected&&g.gold==gold-2*expected);cases++;
 }
 printf("PASS Moon Spring: %d partial-payment, cap, missing-HP and zero-value cases\n",cases);
 /* All combat reward rolls use their encounter's rarity pool; choices unique. */
 int bins[3][3]={{0}};
 for(int mode=0;mode<3;mode++)for(int seed=1;seed<=3000;seed++){
  game_new(&g,seed);game_start_battle(&g,mode==0?NODE_FIGHT:mode==1?NODE_ELITE:NODE_BOSS);
  for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;game_end_turn(&g);if(g.screen==RELIC_RESULT)game_action(&g,BTN_BACK);assert(g.screen==REWARD);
  for(int i=0;i<3;i++){int id=CARD_ID(g.reward[i]),rarity=cards[id].rarity;bins[mode][rarity]++;if(!mode)assert(rarity<2&&id!=48);if(mode==2)assert(rarity==2);for(int j=0;j<i;j++)assert(id!=CARD_ID(g.reward[j]));}
 }
 assert(bins[0][1]>3000&&bins[0][1]<3700&&bins[1][2]>650&&bins[1][2]<1150);
 printf("PASS 27,000 reward cards: normal C/U/R %d/%d/%d; elite %d/%d/%d; boss %d/%d/%d\n",bins[0][0],bins[0][1],bins[0][2],bins[1][0],bins[1][1],bins[1][2],bins[2][0],bins[2][1],bins[2][2]);
 /* Blessings occur once per entrance, persist over saves, and heal between acts. */
 game_new(&g,139);game_action(&g,BTN_OK);assert(g.screen==BLESSING);
 for(int act=1;act<=3;act++){
  assert(g.screen==BLESSING&&g.act==act&&g.hp==g.maxhp);for(int i=0;i<3;i++)for(int j=0;j<i;j++)assert(g.reward[i]!=g.reward[j]);
  assert(game_save(&g,"../evidence1310/blessing-save.dat"));assert(game_load(&t,"../evidence1310/blessing-save.dat"));assert(!memcmp(&g,&t,sizeof(g)));
  int old=g.relic_count;game_action(&g,BTN_OK);assert(g.screen==MAP&&g.relic_count==old+1);
  game_start_battle(&g,NODE_BOSS);g.hp=1;for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;game_end_turn(&g);game_action(&g,BTN_BACK);g.selection=3;game_action(&g,BTN_OK);
 }
 assert(g.screen==VICTORY);
 /* Preview and deck browsing never take a reward and preserve its selection. */
 game_new(&g,139);g.screen=REWARD;g.reward[0]=5;g.reward[1]=31;g.reward[2]=48;g.selection=1;int deck=g.deck_count;
 game_action(&g,BTN_POTION);assert(g.screen==REWARD_PREVIEW);game_action(&g,BTN_RIGHT);game_action(&g,BTN_OK);assert(g.screen==REWARD&&g.selection==2&&g.deck_count==deck);
 game_action(&g,BTN_ALT);assert(g.screen==DECK_VIEW);game_action(&g,BTN_OK);assert(g.deck_count==deck);game_action(&g,BTN_BACK);assert(g.screen==REWARD&&g.selection==2);
 /* Every real Block gain emits its presentation event, not just Defend. */
 int block_cards=0;for(int up=0;up<2;up++)for(int id=0;id<CARD_COUNT;id++){
  math_battle(&g,id|(up?UPBIT:0));CardPreview preview;game_card_preview(&g,g.hand[0],0,&preview);b=g;
  game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();fx=game_combat_fx_events(&n);int blocks=0;for(int i=0;i<n;i++)blocks+=fx[i].kind==GFX_BLOCK;
  if(preview.block_count||g.block>b.block){assert(blocks);game_render_feedback(&b,&g,BTN_OK,1000);assert(r137_count);block_cards++;}
 }
 printf("PASS Block animation events for %d base/upgraded cards; all-act blessings, save/resume, full healing and reward browsing\n",block_cards);
 /* Relic trigger timing is counted per played card, not per hit. */
 math_battle(&g,0);g.hp=470;g.relics[g.relic_count++]=5;g.relics[g.relic_count++]=24;
 for(int i=0;i<3;i++){g.hand[0]=27;g.hand_count=1;game_play_card(&g,0,0);assert(g.strength==(i==2?1:0));assert(g.hp==(i==2?473:470));}
 math_battle(&g,1);g.relics[g.relic_count++]=22;for(int i=0;i<3;i++){g.hand[0]=1;g.hand_count=1;game_play_card(&g,0,0);}assert(g.dexterity==1);
 math_battle(&g,27);g.relics[g.relic_count++]=3;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();fx=game_combat_fx_events(&n);assert(n==2&&fx[0].amount==10&&fx[1].amount==10);g.hand[0]=0;g.hand_count=1;game_play_card(&g,0,0);assert(g.enemies[0].hp==973);
 math_battle(&g,0);g.relics[g.relic_count++]=2;g.enemy_count=1;g.enemies[0].intent=0;g.enemies[0].intent_value=12;g.block=20;game_end_turn(&g);assert(g.hp==500&&g.enemies[0].hp==995);
 math_battle(&g,0);g.relics[g.relic_count++]=23;g.hp=1;g.enemy_count=1;g.enemies[0].intent_value=12;game_end_turn(&g);assert(g.hp==125&&g.defense_event==1&&g.screen==COMBAT);
 puts("PASS relic card-vs-hit counters, Glass Emblem each-turn semantics, blocked retaliation and Last Ember rescue");
 /* Status-only cards receive feedback with no attack. */
 fx_fixture(&g,30,0);b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();game_render_feedback(&b,&g,BTN_OK,1200);int status=0;for(int i=0;i<r137_count;i++)status+=r137_events[i].kind==GFX_STATUS;assert(status==1);fx_write_frame(&g,1204,"status-only-effect");
 fx_fixture(&g,5,0);assert(game_save(&g,"../evidence1310/state-shrug.dat"));fx_fixture(&g,30,0);assert(game_save(&g,"../evidence1310/state-status-only.dat"));
 game_new(&g,139);game_action(&g,BTN_OK);shot(&g,"blessing");
 game_new(&g,139);g.screen=EVENT;g.effect_turn=1;g.hp=47;g.gold=35;g.scroll=9;shot(&g,"moon-partial");
 battle(&g);g.enemy_count=1;g.enemies[0].art=9;g.selection=g.hand_count;shot(&g,"hound-end");
 game_new(&g,139);g.screen=DECK_VIEW;g.prior_screen=REST;g.selection=2;shot(&g,"upgrade-selection");
 game_new(&g,139);g.screen=SHOP;g.shop_cards[0]=5;g.shop_cards[1]=31;g.shop_cards[2]=48;g.shop_relic=5;g.selection=1;shot(&g,"shop-selection");
 math_battle(&g,25);g.screen=CARD_CHOICE;g.next_screen=25;g.discard_count=3;g.discard[0]=0;g.discard[1]=1;g.discard[2]=7;g.selection=1;shot(&g,"discard-selection");
 game_new(&g,139);g.screen=REWARD_PREVIEW;g.reward[0]=65;g.reward[1]=48;g.reward[2]=31;g.selection=0;shot(&g,"reward-preview");
 game_new(&g,139);g.screen=RELIC_VIEW;g.relic_count=28;for(int i=0;i<28;i++)g.relics[i]=i;g.selection=3;shot(&g,"relic-description");
 r137_count=r138_death_active=victory_replay=0;
}
