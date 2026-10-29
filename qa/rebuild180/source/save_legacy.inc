/* Frozen 1.4.0 layout: new map rows must never reinterpret old offsets. */
typedef struct {
 uint32_t magic,version,seed,rng;
 int screen,prior_screen,act,floor,lane,selection,target,scroll;
 int hp,maxhp,block,energy,max_energy,gold,turn,energy_penalty,strength,dexterity,weak,vulnerable,poison;
 int potion_count,potions[MAX_POTIONS],relic_count,relics[40];
 int deck_count,deck[MAX_DECK],master_index;
 int draw_count,draw[MAX_PILE],discard_count,discard[MAX_PILE],exhaust_count,exhaust[MAX_PILE],hand_count,hand[MAX_HAND];
 int map[14][MAP_LANES],map_selected[14],enemy_count,reward[3],reward_gold;
 Enemy enemies[MAX_ENEMIES];
 int next_screen,battle_type,kill_count,defense_event,shop_cards[3],shop_relic;
 int reward_potion,win_screen,effect_turn,turn_played,used_potion,heal_after_battle;
 int permanent_strength,low_hp_triggered;
 char message[72], subtitle[72];
 int ic[48];
 int relic_pending,relic_resume,deck_readonly,shop_selection;
 int endless,combat_music;
} LegacyGame19;
typedef char legacy_size_must_be_3196[(sizeof(LegacyGame19)==3196)?1:-1];
static int load_legacy_run(Game*out,const unsigned char*raw,size_t n,uint32_t version){
 LegacyGame19 old;memset(&old,0,sizeof(old));
 if(version==19){if(n!=sizeof(old))return 0;memcpy(&old,raw,n);}
 else if(version==18||version==17||version==16){
  unsigned char v18[3144];memset(v18,0,sizeof(v18));
  if(version==18){if(n!=3144)return 0;memcpy(v18,raw,n);}
  else{
   size_t prefix=offsetof(LegacyGame19,map),tail=offsetof(LegacyGame19,enemy_count)-32;
   size_t size=3096;if(n!=(version==17?size:size-sizeof(old.ic)))return 0;
   memcpy(v18,raw,prefix);memcpy(v18+offsetof(LegacyGame19,enemy_count),raw+tail,n-tail);
  }
  size_t enemy_offset=offsetof(LegacyGame19,enemies);memcpy(&old,v18,enemy_offset);
  for(int i=0;i<MAX_ENEMIES;i++){
   const unsigned char*e=v18+enemy_offset+i*46;memcpy(old.enemies[i].name,e,24);
   int*values=&old.enemies[i].hp;for(int j=0;j<11;j++){short v;memcpy(&v,e+24+j*2,2);values[j]=v;}
  }
  memcpy((unsigned char*)&old+offsetof(LegacyGame19,next_screen),v18+2728,416);
  old.endless=0;old.combat_music=2;
 }else return 0;
 /* Retain every old room, visited marker, card, item and in-flight combat.
    The four appended rows use the new deterministic map. A boss already in
    progress completes normally; a map save can continue through the new rows. */
 memset(out,0,sizeof(*out));
 memcpy(&out->magic,&old.magic,sizeof(old.magic));
 memcpy(&out->version,&old.version,sizeof(old.version));
 memcpy(&out->seed,&old.seed,sizeof(old.seed));
 memcpy(&out->rng,&old.rng,sizeof(old.rng));
 memcpy(&out->screen,&old.screen,sizeof(old.screen));
 memcpy(&out->prior_screen,&old.prior_screen,sizeof(old.prior_screen));
 memcpy(&out->act,&old.act,sizeof(old.act));
 memcpy(&out->floor,&old.floor,sizeof(old.floor));
 memcpy(&out->lane,&old.lane,sizeof(old.lane));
 memcpy(&out->selection,&old.selection,sizeof(old.selection));
 memcpy(&out->target,&old.target,sizeof(old.target));
 memcpy(&out->scroll,&old.scroll,sizeof(old.scroll));
 memcpy(&out->hp,&old.hp,sizeof(old.hp));
 memcpy(&out->maxhp,&old.maxhp,sizeof(old.maxhp));
 memcpy(&out->block,&old.block,sizeof(old.block));
 memcpy(&out->energy,&old.energy,sizeof(old.energy));
 memcpy(&out->max_energy,&old.max_energy,sizeof(old.max_energy));
 memcpy(&out->gold,&old.gold,sizeof(old.gold));
 memcpy(&out->turn,&old.turn,sizeof(old.turn));
 memcpy(&out->energy_penalty,&old.energy_penalty,sizeof(old.energy_penalty));
 memcpy(&out->strength,&old.strength,sizeof(old.strength));
 memcpy(&out->dexterity,&old.dexterity,sizeof(old.dexterity));
 memcpy(&out->weak,&old.weak,sizeof(old.weak));
 memcpy(&out->vulnerable,&old.vulnerable,sizeof(old.vulnerable));
 memcpy(&out->poison,&old.poison,sizeof(old.poison));
 memcpy(&out->potion_count,&old.potion_count,sizeof(old.potion_count));
 memcpy(&out->potions,&old.potions,sizeof(old.potions));
 memcpy(&out->relic_count,&old.relic_count,sizeof(old.relic_count));
 memcpy(&out->relics,&old.relics,sizeof(old.relics));
 memcpy(&out->deck_count,&old.deck_count,sizeof(old.deck_count));
 memcpy(&out->deck,&old.deck,sizeof(old.deck));
 memcpy(&out->master_index,&old.master_index,sizeof(old.master_index));
 memcpy(&out->draw_count,&old.draw_count,sizeof(old.draw_count));
 memcpy(&out->draw,&old.draw,sizeof(old.draw));
 memcpy(&out->discard_count,&old.discard_count,sizeof(old.discard_count));
 memcpy(&out->discard,&old.discard,sizeof(old.discard));
 memcpy(&out->exhaust_count,&old.exhaust_count,sizeof(old.exhaust_count));
 memcpy(&out->exhaust,&old.exhaust,sizeof(old.exhaust));
 memcpy(&out->hand_count,&old.hand_count,sizeof(old.hand_count));
 memcpy(&out->hand,&old.hand,sizeof(old.hand));
 memcpy(&out->map,&old.map,sizeof(old.map));
 memcpy(&out->map_selected,&old.map_selected,sizeof(old.map_selected));
 memcpy(&out->enemy_count,&old.enemy_count,sizeof(old.enemy_count));
 memcpy(&out->reward,&old.reward,sizeof(old.reward));
 memcpy(&out->reward_gold,&old.reward_gold,sizeof(old.reward_gold));
 memcpy(&out->enemies,&old.enemies,sizeof(old.enemies));
 memcpy(&out->next_screen,&old.next_screen,sizeof(old.next_screen));
 memcpy(&out->battle_type,&old.battle_type,sizeof(old.battle_type));
 memcpy(&out->kill_count,&old.kill_count,sizeof(old.kill_count));
 memcpy(&out->defense_event,&old.defense_event,sizeof(old.defense_event));
 memcpy(&out->shop_cards,&old.shop_cards,sizeof(old.shop_cards));
 memcpy(&out->shop_relic,&old.shop_relic,sizeof(old.shop_relic));
 memcpy(&out->reward_potion,&old.reward_potion,sizeof(old.reward_potion));
 memcpy(&out->win_screen,&old.win_screen,sizeof(old.win_screen));
 memcpy(&out->effect_turn,&old.effect_turn,sizeof(old.effect_turn));
 memcpy(&out->turn_played,&old.turn_played,sizeof(old.turn_played));
 memcpy(&out->used_potion,&old.used_potion,sizeof(old.used_potion));
 memcpy(&out->heal_after_battle,&old.heal_after_battle,sizeof(old.heal_after_battle));
 memcpy(&out->permanent_strength,&old.permanent_strength,sizeof(old.permanent_strength));
 memcpy(&out->low_hp_triggered,&old.low_hp_triggered,sizeof(old.low_hp_triggered));
 memcpy(&out->message,&old.message,sizeof(old.message));
 memcpy(&out->subtitle,&old.subtitle,sizeof(old.subtitle));
 memcpy(&out->ic,&old.ic,sizeof(old.ic));
 memcpy(&out->relic_pending,&old.relic_pending,sizeof(old.relic_pending));
 memcpy(&out->relic_resume,&old.relic_resume,sizeof(old.relic_resume));
 memcpy(&out->deck_readonly,&old.deck_readonly,sizeof(old.deck_readonly));
 memcpy(&out->shop_selection,&old.shop_selection,sizeof(old.shop_selection));
 memcpy(&out->endless,&old.endless,sizeof(old.endless));
 memcpy(&out->combat_music,&old.combat_music,sizeof(old.combat_music));
 out->version=GAME_VERSION;
 for(int i=IC_HOURGLASS;i<=IC_COVENANT;i++)out->ic[i]=0;out->ic[47]=0;
 map_generate(out);
 int rows=version<18?12:14;
 if(version<18){
  memcpy(out->map,raw+offsetof(LegacyGame19,map),(size_t)rows*3*sizeof(int));
  memcpy(out->map_selected,raw+offsetof(LegacyGame19,map)+(size_t)rows*3*sizeof(int),(size_t)rows*sizeof(int));
  out->relic_pending=0;out->relic_resume=out->win_screen?REWARD:MAP;
 }else{
  memcpy(out->map,old.map,sizeof(old.map));memcpy(out->map_selected,old.map_selected,sizeof(old.map_selected));
 }
 if(out->screen==MAP)map_select_reachable(out);
 if(out->screen==EVENT){int leave=out->selection==1;game_begin_event(out,out->effect_turn);out->selection=leave?2:0;}
 return 1;
}
