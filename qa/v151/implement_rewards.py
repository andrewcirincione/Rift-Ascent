from pathlib import Path
p=Path(__file__).parent/'source/game.c';s=p.read_text()
start=s.index(' case REWARD:\n',s.index('void game_action('));end=s.index(' case REST:\n',start)
s=s[:start]+''' case REWARD:
  if(b==BTN_ALT){g->shop_selection=g->selection;g->prior_screen=REWARD;g->screen=DECK_VIEW;g->deck_readonly=1;g->selection=g->scroll=0;}
  else if(b==BTN_POTION&&g->selection<3&&!g->v151.card_done){g->screen=REWARD_PREVIEW;}
  else if(b==BTN_DOWN){if(g->selection<3)g->selection=3;else if(g->v151.potion_offer>=0)g->selection=4;}
  else if(b==BTN_UP){if(g->selection>=4)g->selection=g->v151.card_done?4:3;else if(g->selection==3)g->selection=1;}
  else if(b==BTN_LEFT){if(g->selection>=4)g->selection=4;else if(!g->v151.card_done)g->selection=MAX(0,g->selection-1);}
  else if(b==BTN_RIGHT){if(g->selection>=4)g->selection=5;else if(!g->v151.card_done)g->selection=MIN(3,g->selection+1);}
  else if(b==BTN_OK){
    if(g->selection<4&&!g->v151.card_done){
      if(g->selection<3){if(g->deck_count>=MAX_DECK){msg(g,"Deck full. Skip this card reward.");break;}add_card(g,g->reward[g->selection]);}
      else r151_skip_reward(g);
      g->v151.card_done=1;
    }else if(g->selection>=4&&g->v151.potion_offer>=0){
      if(g->selection==4){if(g->potion_count>=MAX_POTIONS){msg(g,"Belt full. Discard a potion in inventory.");break;}g->potions[g->potion_count++]=g->v151.potion_offer;g->ic[37]=g->v151.potion_offer+1;}
      g->v151.potion_offer=-1;
    }
    if(!g->v151.card_done){g->selection=0;break;}
    if(g->v151.potion_offer>=0){g->selection=4;break;}
    if(g->ninja.v[NS_BONUS_REWARDS]>0){g->ninja.v[NS_BONUS_REWARDS]--;fill_rewards(g);g->v151.card_done=0;g->selection=0;msg(g,"The Hunt: choose an extra card reward.");break;}
    if(g->win_screen){
      int final=g->win_screen==2;g->win_screen=0;
      if(final){g->screen=VICTORY;msg(g,"THE SPIRE HAS FALLEN");}
      else{g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;map_generate(g);begin_act(g);}
    }else{enter_map(g);msg(g,g->character==CHAR_NIGHTSHADE?"Onward, Nightshade.":"Onward, Warden.");}
  }
  break;
'''+s[end:]
p.write_text(s)
p=Path(__file__).parent/'source/render.c';s=p.read_text()
s=s.replace('r134_blit(78,71,324,100,r140_victory,324,100,255);','r134_blit(126,88,228,70,r140_victory,324,100,255);')
s=s.replace('r134_text(352,61,"CURRENT",GOLD,&r134_f9b);','snprintf(b,sizeof(b),"CURRENT: %d ENERGY",game_card_cost(raw));r134_text(352,61,b,GOLD,&r134_f8);')
s=s.replace('r134_text(352,147,"UPGRADED",GOLD,&r134_f9b);','snprintf(b,sizeof(b),"UPGRADE: %d ENERGY",game_card_cost(raw|UPBIT));r134_text(352,147,b,GOLD,&r134_f8);')
# X and unplayable costs need their actual symbols, not negative integers.
s=s.replace('snprintf(b,sizeof(b),"CURRENT: %d ENERGY",game_card_cost(raw));','int cost=game_card_cost(raw);if(cost<0)snprintf(b,sizeof(b),"CURRENT: %s ENERGY",cost==-1?"X":"-");else snprintf(b,sizeof(b),"CURRENT: %d ENERGY",cost);')
s=s.replace('snprintf(b,sizeof(b),"UPGRADE: %d ENERGY",game_card_cost(raw|UPBIT));','cost=game_card_cost(raw|UPBIT);if(cost<0)snprintf(b,sizeof(b),"UPGRADE: %s ENERGY",cost==-1?"X":"-");else snprintf(b,sizeof(b),"UPGRADE: %d ENERGY",cost);')
p.write_text(s)
