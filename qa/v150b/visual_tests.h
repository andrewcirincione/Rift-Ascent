static void dynamic_visual_tests(void){
 Game g;char lines[24][192],name[80],desc[320];int maxheight=0,cases=0;
 for(int up=0;up<2;up++)for(int id=0;id<96;id++)for(int scenario=0;scenario<8;scenario++){
  int raw=id|(up?UPBIT:0);math_battle(&g,raw);g.strength=scenario*4-2;g.dexterity=scenario-2;g.weak=scenario%2;g.enemies[1].vulnerable=2;g.ic[IC_CRUELTY]=50;g.ic[IC_NEXT_ATTACK_REPEAT]=scenario%2;g.reward_potion=4;g.ic[IC_HPLOSS_COMBAT]=20;g.ic[IC_UNMOVABLE]=1;g.shop_cards[0]=2;
  if(scenario==7){g.strength=999;g.dexterity=999;g.hand[1]=7;g.hand[2]=42;g.hand[3]=49;}
  card_combat_context=&g;R135CardLayout l=r135_card_layout(raw,126);game_card_description_now(&g,raw,g.target,desc,sizeof(desc));
  int full=r134_lines(rift131_clean_rules(desc),92,l.font,lines);assert(l.rows==full);assert(l.body_y+l.rows*l.leading==l.height-18||l.height==100);
  if(l.height>180){fprintf(stderr,"OVERFLOW id=%d scenario=%d height=%d %s\n",raw,scenario,l.height,desc);abort();}
  if(l.height>maxheight)maxheight=l.height;cases++;
 }
 card_combat_context=0;
 printf("PASS: %d dynamic description layouts, maximum %d px; complete text retained, preview stays below HP bars\n",cases,maxheight);
 math_battle(&g,0);g.hp=63;g.maxhp=80;g.energy=3;g.block=8;g.strength=3;g.dexterity=2;g.weak=1;g.enemies[0].hp=76;g.enemies[0].maxhp=94;g.enemies[0].vulnerable=2;g.enemies[0].block=4;g.enemies[0].weak=2;g.enemies[0].poison=3;g.enemies[1].hp=104;g.enemies[1].maxhp=110;g.enemies[1].strength=2;g.enemies[1].vulnerable=1;g.ic[IC_CRUELTY]=25;g.ic[IC_UNMOVABLE]=1;g.ic[33+61/31]|=1u<<(61%31);g.ic[33+88/31]|=1u<<(88%31);g.hand_count=5;g.hand[0]=0;g.hand[1]=1;g.hand[2]=29;g.hand[3]=19;g.hand[4]=25;
 shot(&g,"combat-dynamic");game_render_set_overlay(1,2,0);game_render_set_status_scroll(0);shot(&g,"player-status");game_render_set_status_scroll(7);shot(&g,"played-powers");game_render_set_overlay(1,1,0);shot(&g,"enemy-status");game_render_set_overlay(1,0,0);shot(&g,"card-math");game_render_set_overlay(0,0,0);
 g.selection=1;shot(&g,"dex-block");g.selection=3;g.enemies[1].vulnerable=0;shot(&g,"all-targets");
 math_battle(&g,87);g.hand[1]=0;g.hand[2]=7;g.hand_count=3;g.strength=2;g.weak=1;g.enemies[0].vulnerable=1;shot(&g,"random-range");
 math_battle(&g,0);g.strength=3;g.weak=1;g.enemies[0].vulnerable=2;g.ic[IC_CRUELTY]=25;g.enemies[0].block=4;shot(&g,"math-attack");
 math_battle(&g,1);g.dexterity=2;g.ic[IC_UNMOVABLE]=1;shot(&g,"math-block");
 math_battle(&g,2);g.enemy_count=1;g.ic[IC_NEXT_ATTACK_REPEAT]=1;shot(&g,"math-repeat");
 math_battle(&g,25);g.ic[IC_NEXT_ATTACK_REPEAT]=1;g.strength=999;g.weak=1;g.enemies[0].vulnerable=2;shot(&g,"long-dynamic");
}
