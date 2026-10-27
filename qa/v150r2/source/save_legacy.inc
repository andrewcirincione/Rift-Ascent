/* Frozen 1.4.0 layout: new map rows must never reinterpret old offsets. */
typedef struct {
 uint32_t magic,version,seed,rng;
 int screen,prior_screen,act,floor,lane,selection,target,scroll;
 int hp,maxhp,block,energy,max_energy,gold,turn,energy_penalty,strength,dexterity,weak,vulnerable,poison;
 int potion_count,potions[MAX_POTIONS],relic_count,relics[MAX_RELICS];
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
 memset(out,0,sizeof(*out));memcpy(out,&old,offsetof(Game,map));
 size_t tail=offsetof(LegacyGame19,enemy_count);
 memcpy((unsigned char*)out+offsetof(Game,enemy_count),(unsigned char*)&old+tail,sizeof(old)-tail);
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
