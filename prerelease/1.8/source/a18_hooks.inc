/* Custom card/relic effects share the existing attack, Block, Poison, Forge,
 * draw, exhaust and preview paths. No parallel damage rules or fake previews. */
#include "a18_exhaust.inc"
static void a18_exhaust_left(Game*g){
 if(g->hand_count<1)return;int raw=g->hand[0];for(int i=0;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];g->hand_count--;exhaust_card(g,raw);
}
static void a18_discard_left(Game*g){
 if(g->hand_count<1)return;int raw=g->hand[0];for(int i=0;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];g->hand_count--;ninja_discard_effects(g,raw,0);
}
static void regent_spend_stars(Game*g,int n){
 n=CLAMP(n,0,g->regent.stars);if(!n)return;g->regent.stars-=n;record_combat_fx(GFX_STARS,-1,-1,-n,0);
 /* Orbit counts Energy, not Stars. */
}
static void a18_turn_start(Game*g){
 if(g->screen!=COMBAT)return;
 if(g->a18.next_energy){gain_energy(g,g->a18.next_energy);g->a18.next_energy=0;}
 int nd=g->a18.next_draw;g->a18.next_draw=0;for(int i=0;i<MIN(10,nd);i++)draw_card(g);
 if(relic(g,64)&&g->turn==1){g->strength+=3;g->ic[IC_TEMP_STRENGTH]+=3;}
 if(relic(g,65)&&g->block==0)relic_block(g,4);
 if(relic(g,66)&&g->turn==1){draw_card(g);draw_card(g);}
 if(relic(g,68)){if(g->character==CHAR_REGENT)regent_gain_stars(g,2);else if(g->turn==1)gain_energy(g,1);}
 if(relic(g,70)&&g->turn%3==0){draw_card(g);draw_card(g);gain_energy(g,1);}
 if(relic(g,73)&&g->turn==1)for(int e=0;e<g->enemy_count;e++){ninja_weak(g,e,1);apply_vulnerable(g,&g->enemies[e],1);}
 if(g->character==CHAR_WARDEN){
  if(g->a18.powers[12]&&g->exhaust_count>=3){gain_energy(g,g->a18.powers[12]);relic_block(g,g->a18.values[0]);}
  if(g->a18.powers[18]){g->strength+=g->a18.powers[18];relic_block(g,g->a18.values[1]*MIN(5,g->exhaust_count));}
 }else if(g->character==CHAR_NIGHTSHADE){if(g->a18.powers[12])ninja_poison_all(g,g->a18.powers[12]);}
 else{
  if(g->a18.powers[10])relic_block(g,MIN(g->a18.powers[10],g->regent.stars));
  if(g->a18.powers[14]){regent_gain_stars(g,g->a18.powers[14]);if(g->regent.blade_created)g->strength+=g->a18.values[3];}
  if(g->a18.powers[18]){regent_forge(g,g->a18.powers[18]);regent_gain_stars(g,g->a18.powers[18]);}
 }
}
static void a18_self_loss(Game*g){
 if(g->screen!=COMBAT||g->hp<=0||!g->ic[IC_OWN_TURN])return;
 if(g->character==CHAR_WARDEN&&g->a18.powers[7])relic_block(g,g->a18.powers[7]);
 if(relic(g,69)&&!(g->a18.turn_flags&1)){g->a18.turn_flags|=1;g->strength+=2;}
}
static void a18_exhaust_hook(Game*g){
 if(g->screen!=COMBAT&&g->screen!=CARD_CHOICE&&g->screen!=NINJA_CHOICE&&g->screen!=REGENT_CHOICE)return;
 if(!(g->a18.turn_flags&2)){
  g->a18.turn_flags|=2;
  if(g->character==CHAR_WARDEN)g->strength+=g->a18.powers[6];
  if(relic(g,71))gain_energy(g,1);
 }
}
static void a18_play_hook(Game*g,int type){
 if(g->screen!=COMBAT)return;
 if(type==0&&!(g->a18.turn_flags&4)){
  g->a18.turn_flags|=4;if(relic(g,67))ninja_poison(g,g->target,2);
 }
 if(type==1&&!(g->a18.turn_flags&8)){
  g->a18.turn_flags|=8;
  if(g->character==CHAR_NIGHTSHADE&&g->a18.powers[10])ninja_poison_all(g,g->a18.powers[10]);
  if(relic(g,72))draw_card(g);
 }
 if(!(g->a18.turn_flags&16)){
  g->a18.turn_flags|=16;
  if(g->character==CHAR_NIGHTSHADE&&g->a18.powers[19]){
   relic_block(g,g->a18.powers[19]);for(int i=0;i<MIN(10,g->a18.values[2]);i++)draw_card(g);
  }
 }
}
static const char*a18_potion_names[20]={
 "Frostfire Flask","Nightsap","Sunder Glass","Ironwake","Rift Draught","Starlight Elixir","Glassskin","Cleansing Rain","Red Communion","Quietus","Ashwater","Thornwine","Time Salt","Echo Tonic","Fortress Dew","Void Ink","Blade Vial","Sovereign Oil","Bloodmercy","Abyssal Tincture"};
static const char*a18_potion_details[20]={
 "Deal 18 damage to ALL enemies. Apply 2 Weak to each.",
 "Apply 8 Poison to ALL enemies.",
 "Remove ALL enemy Block. Apply 3 Vulnerable to each enemy.",
 "Gain 24 Block.",
 "Gain 3 Energy. Draw 1 card.",
 "Proxy gains 8 Stars. Other characters gain 2 Energy.",
 "Gain 12 Block and 1 Dexterity for this combat.",
 "Remove Weak, Vulnerable and Poison. Heal 6 HP. Usable outside combat.",
 "Lose 4 HP. Gain 4 Strength and 4 Dexterity for this combat.",
 "Apply 4 Weak to ALL enemies.",
 "Return up to 2 cards from the top of Exhaust to your hand.",
 "Gain 6 Block and 6 Thorns for this combat.",
 "Next turn: gain 2 extra Energy and draw 2 extra cards.",
 "Your next Attack plays twice.",
 "Gain 30 Block. Keep Block at the next turn start.",
 "Exhaust all unplayable cards in hand. Draw 2 cards, plus 1 per card exhausted.",
 "Add 3 upgraded Shivs to your hand.",
 "Proxy Forges 12. Other characters gain 4 Strength.",
 "Heal 25% of maximum HP. Remove Weak. Usable outside combat.",
 "Gain 1 Intangible. Each incoming hit deals at most 1 damage this turn."};
static void a18_use_potion(Game*g,int index){
 int id=g->potions[index],combat=g->screen==COMBAT,bonus=relic(g,17)?1:0;
 if(id<6||id>=26)return;
 if(!combat&&(!game_inventory_available(g)||(id!=13&&id!=24))){msg(g,"Use this potion during combat.");return;}
 if(!combat&&g->hp==g->maxhp&&!g->weak&&!g->vulnerable&&!g->poison){msg(g,"No healing or cleansing needed. Potion kept.");return;}
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;
 switch(id){
 case 6:all_damage(g,18+5*bonus);for(int e=0;e<g->enemy_count;e++)ninja_weak(g,e,2+bonus);break;
 case 7:ninja_poison_all(g,8+2*bonus);break;
 case 8:for(int e=0;e<g->enemy_count;e++){g->enemies[e].block=0;apply_vulnerable(g,&g->enemies[e],3+bonus);}break;
 case 9:relic_block(g,24+5*bonus);break;
 case 10:gain_energy(g,3+bonus);draw_card(g);break;
 case 11:if(g->character==CHAR_REGENT)regent_gain_stars(g,8+2*bonus);else gain_energy(g,2+bonus);break;
 case 12:relic_block(g,12+5*bonus);g->dexterity+=1+bonus;break;
 case 13:g->weak=g->vulnerable=g->poison=0;heal(g,6+5*bonus);break;
 case 14:lose_hp(g,4);if(g->hp>0){g->strength+=4+bonus;g->dexterity+=4+bonus;}break;
 case 15:for(int e=0;e<g->enemy_count;e++)ninja_weak(g,e,4+bonus);break;
 case 16:for(int j=0;j<2+bonus&&g->exhaust_count&&g->hand_count<MAX_HAND;j++)g->hand[g->hand_count++]=g->exhaust[--g->exhaust_count];break;
 case 17:relic_block(g,6+5*bonus);g->ninja.v[NS_THORNS]+=6+2*bonus;break;
 case 18:g->a18.next_energy+=2+bonus;g->a18.next_draw+=2+bonus;break;
 case 19:g->ic[IC_NEXT_ATTACK_REPEAT]+=1+bonus;break;
 case 20:relic_block(g,30+5*bonus);g->ninja.v[NS_BLUR]++;break;
 case 21:{int n=0;for(int j=g->hand_count-1;j>=0;j--)if(!game_card_is_playable(g->hand[j])){int c=g->hand[j];for(int k=j;k+1<g->hand_count;k++)g->hand[k]=g->hand[k+1];g->hand_count--;exhaust_card(g,c);n++;}for(int j=0;j<MIN(10,2+bonus+n);j++)draw_card(g);break;}
 case 22:for(int j=0;j<3+bonus;j++)add_generated(g,N_SHIV|UPBIT);break;
 case 23:if(g->character==CHAR_REGENT)regent_forge(g,12+3*bonus);else g->strength+=4+bonus;break;
 case 24:g->weak=0;heal(g,MAX(1,g->maxhp/4)+5*bonus);break;
 case 25:g->ninja.v[NS_INTANGIBLE]++;if(bonus)relic_block(g,5);break;
 }
 if(combat&&relic(g,21))relic_block(g,12);
 snprintf(g->message,sizeof(g->message),"Used %s",a18_potion_names[id-6]);if(combat)check_win(g);
}
