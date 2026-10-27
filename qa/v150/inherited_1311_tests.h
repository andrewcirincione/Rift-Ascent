static void patch1311_tests(void){
 Game g,t,b;int priorhp=0,priorattack=0;
 for(int id=0;id<6;id++){
  math_battle(&g,0);g.hand_count=0;g.draw_count=4;g.draw[3]=1;g.relics[g.relic_count++]=17;g.hp=g.maxhp-30;g.strength=g.weak=g.block=0;g.enemies[0].block=g.enemies[0].vulnerable=0;g.enemies[1].block=g.enemies[1].vulnerable=0;
  g.potion_count=1;g.potions[0]=id;b=g;game_use_potion(&g,0);
  if(id==0)assert(g.hp==b.hp+23);if(id==1)assert(g.strength==3);if(id==2)assert(g.block==25);
  if(id==3)assert(b.enemies[0].hp-g.enemies[0].hp==25);if(id==4)assert(g.energy==b.energy+3);if(id==5)assert(g.hand_count==4);
 }
 battle(&g);g.endless=1;g.act=1000000;g.enemies[0].hp=g.enemies[0].maxhp=1000000000;shot(&g,"endless-large-health");

 for(int endless=0;endless<2;endless++){
  game_new(&g,1311);g.selection=endless;game_action(&g,BTN_OK);assert(g.screen==ACT_INTRO&&g.endless==endless);
  for(int i=0;i<119;i++)game_update(&g);assert(g.screen==ACT_INTRO);game_update(&g);assert(g.screen==BLESSING);
  game_action(&g,BTN_OK);assert(g.screen==MAP);
 }
 game_new(&g,1311);g.endless=1;
 for(int act=1;act<=1000;act++){
  assert(g.act==act);game_start_battle(&g,NODE_BOSS);
  if(act>=3){assert(g.enemies[0].hp>priorhp&&g.enemies[0].intent_value>priorattack);priorhp=g.enemies[0].hp;priorattack=g.enemies[0].intent_value;}
  for(int i=0;i<g.enemy_count;i++)g.enemies[i].hp=0;
  game_end_turn(&g);assert(g.screen==RELIC_RESULT);game_action(&g,BTN_OK);assert(g.screen==REWARD);
  g.selection=3;game_action(&g,BTN_OK);assert(g.screen==ACT_INTRO&&g.act==act+1&&g.hp==g.maxhp);
  while(g.screen==ACT_INTRO)game_update(&g);assert(g.screen==BLESSING);
  for(int i=0;i<3;i++)for(int j=0;j<i;j++)assert(g.reward[i]!=g.reward[j]);
  g.selection=act%3;game_action(&g,BTN_OK);assert(g.screen==MAP&&game_validate(&g));
  if(act%17==0){assert(game_save(&g,"qa/v150/evidence/endless-save.dat"));assert(game_load(&t,"qa/v150/evidence/endless-save.dat"));assert(!memcmp(&g,&t,sizeof(g)));}
 }
 assert(g.relic_count==40&&g.maxhp>80&&g.permanent_strength>0);
 int acts[]={4,10,100,10000,1000000};
 for(int a=0;a<5;a++)for(int mode=0;mode<=6;mode++)if(mode==NODE_FIGHT||mode==NODE_ELITE||mode==NODE_BOSS){
  game_new(&g,1311);g.endless=1;g.act=acts[a];game_start_battle(&g,mode);assert(game_validate(&g));game_end_turn(&g);assert(g.screen==COMBAT||g.screen==DEFEAT);
 }
 game_new(&g,1311);g.act=3;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=1;g.hand_count=1;g.hand[0]=0;g.energy=3;
 b=g;game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();assert(g.screen==VICTORY&&g.gold==b.gold&&g.relic_count==b.relic_count&&g.deck_count==b.deck_count&&g.hp==b.hp);
 game_render_feedback(&b,&g,BTN_OK,1000);assert(game_render_fx_busy(1000)&&game_render_fx_busy(1070)&&!game_render_fx_busy(1400));
 battle(&g);g.enemy_count=1;g.enemies[0].hp=1;g.enemies[0].poison=2;g.hand_count=0;b=g;
 game_combat_fx_begin();game_end_turn(&g);game_combat_fx_end();assert(g.screen==REWARD);game_render_feedback(&b,&g,BTN_BACK,2000);assert(game_render_fx_busy(2000)&&game_render_fx_busy(2070)&&!game_render_fx_busy(2400));
 for(int plating=0;plating<=20;plating++)for(int barricade=0;barricade<2;barricade++){
  battle(&g);g.block=0;g.ic[IC_PLATING]=plating;g.ic[IC_BARRICADE]=barricade;g.hand_count=0;g.enemies[0].intent=g.enemies[1].intent=0;g.enemies[0].intent_value=g.enemies[1].intent_value=5;b=g;
  game_end_turn(&b);int hp=g.hp,turn=g.turn;game_combat_fx_begin();game_prepare_end_turn(&g);game_combat_fx_end();
  assert(g.hp==hp&&g.turn==turn&&g.block==plating&&g.ic[38]==1);
  int n;const GameCombatFx*fx=game_combat_fx_events(&n);assert((plating?1:0)==n);if(n)assert(fx[0].kind==GFX_BLOCK&&fx[0].amount==plating);
  game_prepare_end_turn(&g);assert(g.block==plating);
  assert(game_save(&g,"qa/v150/evidence/plating-save.dat"));assert(game_load(&t,"qa/v150/evidence/plating-save.dat"));game_end_turn(&t);assert(!memcmp(&b,&t,sizeof(b)));
 }
 assert(game_load(&g,"../evidence1310/state-release-combat.dat"));assert(g.version==GAME_VERSION&&!g.endless&&g.combat_music==2&&g.enemy_count>0);assert(game_validate(&g));
 int tracks[5]={0};for(int seed=1;seed<=1000;seed++){game_new(&g,seed);game_start_battle(&g,NODE_FIGHT);tracks[game_music_track(&g)]++;assert(game_save(&g,"qa/v150/evidence/music-save.dat"));assert(game_load(&t,"qa/v150/evidence/music-save.dat"));assert(game_music_track(&g)==game_music_track(&t));}
 assert(tracks[2]>400&&tracks[4]>400);game_start_battle(&g,NODE_ELITE);assert(game_music_track(&g)==3);game_start_battle(&g,NODE_BOSS);assert(game_music_track(&g)==0);
 game_new(&g,1311);g.selection=1;shot(&g,"title-endless");g.screen=ACT_INTRO;g.scroll=60;g.act=4;g.endless=1;shot(&g,"act-intro");
 battle(&g);g.ic[IC_PLATING]=6;g.strength=3;g.weak=1;g.vulnerable=2;g.block=0;shot(&g,"plating-combat");g.hand_count=0;g.enemies[0].intent=g.enemies[1].intent=0;g.enemies[0].intent_value=g.enemies[1].intent_value=5;shot(&g,"plating-turn");
 battle(&g);g.screen=SHOP;g.shop_cards[0]=0;g.shop_cards[1]=54;g.shop_cards[2]=48;g.shop_relic=17;g.selection=3;shot(&g,"shop-orb");
 game_new(&g,1311);g.act=3;game_start_battle(&g,NODE_BOSS);g.enemies[0].hp=1;g.hand_count=1;g.hand[0]=0;g.energy=3;shot(&g,"final-boss");g.endless=1;shot(&g,"endless-boss");
 for(int art=0;art<36;art++){battle(&g);g.enemy_count=1;g.enemies[0].art=art;snprintf(g.enemies[0].name,24,"%s",game_enemy_name(art));char name[30];snprintf(name,sizeof(name),"ground-%02d",art);shot(&g,name);}
 game_new(&g,1311);game_start_battle(&g,NODE_FIGHT);g.combat_music=2;shot(&g,"music-normal-original");g.combat_music=4;shot(&g,"music-normal-new");game_start_battle(&g,NODE_ELITE);shot(&g,"music-elite");
 puts("PASS 1.3.11: 1000 endless acts, repeatable blessings, deep scaling, v18 migration, 42 phased Plating/save cases, final boss without rewards, finishing FX, stable music selection, 36 grounding renders");
}
