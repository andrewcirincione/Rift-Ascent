from pathlib import Path
p=Path(__file__).parent/'source/game.c';s=p.read_text();a=s.index('/* List only active effects;');b=s.index('int game_status_count',a)
s=s[:a]+'''/* Temporary status and persistent Powers are separate; each Power appears once. */
static int power_present(const Game*g,int id){
 if(g->ic[33+id/31]&(int)(1u<<(id%31)))return 1;
 switch(id){case 20:return g->shop_cards[2];case 28:return g->used_potion;case 41:return g->shop_cards[0];case 48:return g->heal_after_battle;
 case 54:return g->ic[IC_AGGRESSION];case 55:return g->ic[IC_BARRICADE];case 59:return g->ic[IC_CORRUPTION];case 60:return g->ic[IC_CRIMSON_COPIES];case 61:return g->ic[IC_CRUELTY];case 62:return g->ic[IC_DARK_EMBRACE];case 69:return g->ic[IC_HELLRAISER];case 72:return g->ic[IC_JUGGERNAUT];case 73:return g->ic[IC_JUGGLING];case 78:return g->ic[IC_PYRE];case 82:return g->ic[IC_STAMPEDE];case 84:return g->ic[IC_PLATING];case 85:return g->ic[IC_TANK];case 88:return g->ic[IC_UNMOVABLE];default:return 0;}
}
static void power_line(const Game*g,int id,char*out,int cap){
 const char*fmt=0;int a=0,b=0;
 switch(id){
 case 8:fmt="Inflame: Strength granted this combat";break;
 case 20:fmt="Feel No Pain: +%d Block per exhausted card";a=g->shop_cards[2];break;
 case 28:fmt="Inferno: -%d HP/turn; self HP loss hits all for %d";a=(unsigned)g->used_potion>>16;b=g->used_potion&65535;break;
 case 41:fmt="Rupture: +%d Strength on self-inflicted HP loss";a=g->shop_cards[0];break;
 case 48:fmt="Demon Form: +%d Strength each turn";a=g->heal_after_battle;break;
 case 54:fmt="Aggression: return and upgrade %d Attack(s)/turn";a=g->ic[IC_AGGRESSION];break;
 case 55:fmt="Barricade: keep Block between turns";break;
 case 59:fmt="Corruption: Skills cost 0 and Exhaust";break;
 case 60:fmt="Crimson Mantle: +%d Block, -%d HP each turn";a=g->ic[IC_CRIMSON];b=g->ic[IC_CRIMSON_COPIES];break;
 case 61:fmt="Cruelty: +%d%% damage to Vulnerable foes";a=g->ic[IC_CRUELTY];break;
 case 62:fmt="Dark Embrace: draw %d per exhausted card";a=g->ic[IC_DARK_EMBRACE];break;
 case 69:fmt="Hellraiser: autoplay Strikes when drawn";break;
 case 72:fmt="Juggernaut: gain Block, deal %d damage";a=g->ic[IC_JUGGERNAUT];break;
 case 73:fmt="Juggling: copy the third Attack each turn";break;
 case 78:fmt="Pyre: +%d Energy each turn";a=g->ic[IC_PYRE];break;
 case 82:fmt="Stampede: autoplay %d Attack(s) at turn end";a=g->ic[IC_STAMPEDE];break;
 case 84:fmt="Stone Armor: %d Plating remaining";a=g->ic[IC_PLATING];break;
 case 85:fmt="Tank: ally protection active";break;
 case 88:fmt=g->ic[IC_UNMOVABLE_USED]?"Unmovable: already used this turn":"Unmovable: next card Block gain doubled";break;
 default:snprintf(out,(size_t)cap,"%s: played this combat",cards[id].name);return;
 }
 snprintf(out,(size_t)cap,fmt,a,b);
}
static int status_entries(const Game*g,int wanted,char*out,int cap){
 int n=0;
#define STATUS_FLAG(value,label) do {if(value){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,"%s",label);n++;}}while(0)
#define STATUS(value,format) do {int v=(value);if(v){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,format,v);n++;}}while(0)
 STATUS(g->strength,"Strength: %d damage per hit");
 STATUS(g->dexterity,"Dexterity: %d Block from cards");
 STATUS(g->weak,"Weak: %d turns; attacks deal 25%% less");
 STATUS(g->vulnerable,"Vulnerable: %d turns; take 50%% more damage");
 STATUS(g->poison,"Poison: lose %d HP at turn end");
 STATUS(g->energy_penalty,"Energy drain: -%d Energy next turn");
 STATUS(g->shop_cards[1],"Flame Barrier: reflect %d per hit this turn");
 STATUS(g->reward_potion>>8,"Rage: +%d Block per Attack this turn");
 STATUS_FLAG(g->ic[IC_COLOSSUS],"Colossus: halve damage from Vulnerable foes");
 STATUS_FLAG(g->reward_potion&1,"Draw locked for this turn");
 STATUS(g->ic[IC_RAMPAGE],"Rampage: +%d damage this combat");
 STATUS_FLAG(g->ic[IC_ENERGY_LOCK],"Energy locked for this turn");
 STATUS(g->ic[IC_TEMP_STRENGTH],"Temporary Strength: %d expires this turn");
 STATUS_FLAG(g->ic[IC_UNRELENTING],"Unrelenting: next Attack costs 0");
 STATUS(g->ic[IC_VICIOUS],"Vicious: draw %d when applying Vulnerable");
 STATUS(g->ic[IC_NEXT_ATTACK_REPEAT],"One-Two Punch: repeat next %d Attack(s)");
 int count=0;for(int id=0;id<CARD_COUNT;id++)if(cards[id].type==2&&power_present(g,id))count++;
 if(count){
  if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,"POWERS");n++;
  for(int id=0;id<CARD_COUNT;id++)if(cards[id].type==2&&power_present(g,id)){
   if(n==wanted&&out&&cap>0)power_line(g,id,out,cap);n++;
  }
 }
#undef STATUS
#undef STATUS_FLAG
 return n;
}
''' +s[b:]
p.write_text(s)
