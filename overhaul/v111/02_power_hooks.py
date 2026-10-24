#!/usr/bin/env python3
"""87-card expansion: reusable power hooks, draw/energy, damage, turn lifecycle."""
from pathlib import Path
import sys
root=Path(sys.argv[1]);p=root/"game.c";h=root/"game.h";s=p.read_text();hs=h.read_text()
def fix(a,b):
 global s
 if a not in s:raise RuntimeError("IC87 power anchor missing: "+a[:110])
 s=s.replace(a,b,1)
def head(a,b):
 global hs
 if a not in hs:raise RuntimeError("IC87 power header missing: "+a[:110])
 hs=hs.replace(a,b,1)
head('enum {NODE_FIGHT,NODE_ELITE,NODE_EVENT,NODE_SHOP,NODE_REST,NODE_TREASURE,NODE_BOSS};',
'''enum {NODE_FIGHT,NODE_ELITE,NODE_EVENT,NODE_SHOP,NODE_REST,NODE_TREASURE,NODE_BOSS};
enum {
 IC_AGGRESSION,IC_BARRICADE,IC_COLOSSUS,IC_CORRUPTION,
 IC_CRIMSON,IC_CRIMSON_COPIES,IC_CRUELTY,IC_DARK_EMBRACE,
 IC_ENERGY_LOCK,IC_HELLRAISER,IC_JUGGERNAUT,IC_JUGGLING,
 IC_MANGLE0,IC_MANGLE1,IC_PYRE,IC_RAMPAGE,IC_TEMP_STRENGTH,
 IC_HPLOSS_TURN,IC_HPLOSS_COMBAT,IC_STAMPEDE,IC_PLATING,
 IC_TANK,IC_UNMOVABLE,IC_UNMOVABLE_USED,IC_UNRELENTING,
 IC_VICIOUS,IC_NEXT_ATTACK_REPEAT,IC_CHAIN_DEPTH,IC_PREVENT_EXHAUST,
 IC_ATTACKS_THIS_TURN,IC_OWN_TURN,IC_GUARD,IC_CASCADE_QUEUE
};''')
# New card flags occupy bits above the existing 7-bit ID and upgrade bit.
fix('static void draw_card(Game*g){',
'''static int has_strike_tag(const char *name);
static void apply_vulnerable(Game*g,Enemy*e,int n);
static void draw_card(Game*g);
static int random_living_foe(Game*g){
 int count=0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)count++;
 if(!count)return -1;
 int which=rnd(g,count);
 for(int i=0;i<g->enemy_count;i++)
  if(g->enemies[i].hp>0&&which--==0)return i;
 return -1;
}
static void add_generated(Game*g,int raw){
 if(g->hand_count<MAX_HAND)g->hand[g->hand_count++]=raw;
 else if(g->discard_count<MAX_PILE)
   g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);
}
static void gain_energy(Game*g,int n){
 if(n>0&&g->screen==COMBAT&&!g->ic[IC_ENERGY_LOCK])
   g->energy=MIN(99,g->energy+n);
}
static void draw_card(Game*g){''')
fix('g->hand[g->hand_count++]=g->draw[--g->draw_count];',
'''int drawn=g->draw[--g->draw_count];
 g->hand[g->hand_count++]=drawn;
 if(g->ic[IC_HELLRAISER]>0&&g->ic[IC_CHAIN_DEPTH]<20&&
   CARD_ID(drawn)<CARD_COUNT&&cards[CARD_ID(drawn)].type==0&&
   has_strike_tag(cards[CARD_ID(drawn)].name)){
   int chosen=random_living_foe(g);
   if(chosen>=0){
    g->ic[IC_CHAIN_DEPTH]++;
    g->hand[g->hand_count-1]|=CARD_FREE;
    game_play_card(g,g->hand_count-1,chosen);
    g->ic[IC_CHAIN_DEPTH]--;
   }
 }''')
# Player-only damage triggers for Spite / Tear Asunder, independent of Inferno.
fix('n=MAX(n,0);g->hp-=n;',
'''n=MAX(n,0);g->hp-=n;
 if(n>0&&g->screen==COMBAT){
  g->ic[IC_HPLOSS_COMBAT]++;
  if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
 }''')
fix('if(n>0){g->hp-=n;if(relic(g,2))',
'''if(n>0){g->hp-=n;
  if(g->screen==COMBAT){
   g->ic[IC_HPLOSS_COMBAT]++;
   if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
  }
  if(relic(g,2))''')
fix('if(g->weak)amt=amt*3/4;',
    'if(g->weak)amt=amt*3/4;')
fix('if(e->vulnerable)amt=amt*3/2;',
'''if(e->vulnerable){
  amt=amt*3/2;
  if(g->ic[IC_CRUELTY])amt=amt*(100+g->ic[IC_CRUELTY])/100;
 }''')
# Centralize block events; first *card* Block per turn can double.
fix('static void gain_block(Game*g,int n){g->block+=MAX(0,n+g->dexterity+(relic(g,15)?2:0));g->block=MIN(g->block,999);}',
'''static void proc_juggernaut(Game*g){
 if(g->ic[IC_JUGGERNAUT]<=0||g->screen!=COMBAT||g->ic[IC_GUARD])return;
 int foe=random_living_foe(g);
 if(foe<0)return;
 g->ic[IC_GUARD]=1;
 Enemy *e=&g->enemies[foe];
 int d=g->ic[IC_JUGGERNAUT],blocked=MIN(e->block,d);
 e->block-=blocked;e->hp-=d-blocked;
 g->ic[IC_GUARD]=0;
}
static void gain_block(Game*g,int n){
 int amount=MAX(0,n+g->dexterity+(relic(g,15)?2:0));
 if(amount<=0)return;
 if(g->ic[IC_UNMOVABLE]&&!g->ic[IC_UNMOVABLE_USED]){
  amount*=2;g->ic[IC_UNMOVABLE_USED]=1;
 }
 g->block=MIN(999,g->block+amount);
 proc_juggernaut(g);
}''')
# Vicious: apply vulnerable is a card-trigger, unlike a simple assignment.
fix('static void deal(Game*g,int idx,int n){damage_enemy(g,idx,n,0);}',
'''static void deal(Game*g,int idx,int n){damage_enemy(g,idx,n,0);}
static void apply_vulnerable(Game*g,Enemy*e,int n){
 if(!e||n<=0||e->hp<=0)return;
 e->vulnerable=MIN(99,e->vulnerable+n);
 if(g->ic[IC_VICIOUS]>0){
  int count=MIN(6,g->ic[IC_VICIOUS]);
  for(int i=0;i<count;i++)draw_card(g);
 }
}''')
# Vicious trigger should also fire for the 50 existing cards. All multiple
# applications call once per target in an AoE, not once for every stack.
for old,new in [
 ('g->enemies[enemy].vulnerable+=2+up;',
  'apply_vulnerable(g,&g->enemies[enemy],2+up);'),
 ('g->enemies[z].vulnerable+=1+up;',
  'apply_vulnerable(g,&g->enemies[z],1+up);'),
 ('g->enemies[enemy].vulnerable+=3+up;',
  'apply_vulnerable(g,&g->enemies[enemy],3+up);'),
 ('g->enemies[enemy].vulnerable+=1+up;',
  'apply_vulnerable(g,&g->enemies[enemy],1+up);')
]:
 s=s.replace(old,new)
# Existing Set-Vulnerable doubling (Molten Fist) is not an "Apply" action.
# Intrinsic cost handling: temporary generated card; Corruption Skills;
# Unrelenting's next attack is free, consumed only when that attack is used.
fix('int game_card_cost_now(const Game*g,int raw){',
'''int game_card_cost_now(const Game*g,int raw){
 int id=CARD_ID(raw);
 if(raw&CARD_FREE)return 0;
 if(g&&id<CARD_COUNT&&g->ic[IC_CORRUPTION]&&cards[id].type==1)return 0;
 if(g&&id<CARD_COUNT&&cards[id].type==0&&g->ic[IC_UNRELENTING])return 0;''')
# New starting point for 17-save power fields.
fix('g->reward_potion=0;g->shop_relic=0;g->shop_cards[0]=',
    'memset(g->ic,0,sizeof(g->ic));\n g->reward_potion=0;g->shop_relic=0;g->shop_cards[0]=')
fix('g->turn++;\n if(g->heal_after_battle>0)',
'''g->turn++;
 g->ic[IC_OWN_TURN]=1;
 g->ic[IC_HPLOSS_TURN]=0;
 g->ic[IC_ATTACKS_THIS_TURN]=0;
 g->ic[IC_UNMOVABLE_USED]=0;
 g->ic[IC_ENERGY_LOCK]=0;
 if(g->ic[IC_PLATING]>0)g->ic[IC_PLATING]--;
 if(g->heal_after_battle>0)''')
fix('if(g->screen!=COMBAT)return;\n g->energy=MAX(0,g->max_energy+',
'''if(g->screen!=COMBAT)return;
 if(g->ic[IC_CRIMSON_COPIES]>0){
  int copies=g->ic[IC_CRIMSON_COPIES];
  for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
  if(g->screen==COMBAT&&g->ic[IC_CRIMSON]>0){
   g->block=MIN(999,g->block+g->ic[IC_CRIMSON]);
   proc_juggernaut(g);
  }
 }
 if(g->screen!=COMBAT)return;
 g->energy=MAX(0,g->max_energy+''')
fix('g->energy_penalty);g->energy_penalty=0;',
    'g->energy_penalty+g->ic[IC_PYRE]);g->energy_penalty=0;')
fix('for(int i=0;i<n;i++)draw_card(g);\n g->selection=0;',
'''for(int i=0;i<n;i++)draw_card(g);
 if(g->ic[IC_AGGRESSION]>0){
  for(int j=0;j<g->ic[IC_AGGRESSION];j++){
   int count=0;
   for(int i=0;i<g->discard_count;i++)
    if(CARD_ID(g->discard[i])<CARD_COUNT&&cards[CARD_ID(g->discard[i])].type==0)count++;
   if(!count||g->hand_count>=MAX_HAND)break;
   int which=rnd(g,count);
   for(int i=0;i<g->discard_count;i++){
    int c=g->discard[i];
    if(cards[CARD_ID(c)].type==0&&which--==0){
     for(int k=i;k+1<g->discard_count;k++)g->discard[k]=g->discard[k+1];
     g->discard_count--;add_generated(g,c|UPBIT);break;
    }
   }
  }
 }
 /* Innate only the upgraded Aggression/Juggling copies, and only once. */
 if(g->turn==1)for(int i=g->draw_count-1;i>=0&&g->hand_count<MAX_HAND;i--){
  int c=g->draw[i],id=CARD_ID(c);
  if(CARD_UP(c)&&(id==54||id==73)){
    for(int j=i;j+1<g->draw_count;j++)g->draw[j]=g->draw[j+1];
    g->draw_count--;add_generated(g,c);
  }
 }
 g->selection=0;''')
# Colossus and Tank modify expected damage and the real enemy's hit.
fix('if(g->vulnerable)n=n*3/2;\n return n*hits;',
'''if(g->vulnerable)n=n*3/2;
 if(g->ic[IC_COLOSSUS]&&g->enemies[index].vulnerable)n=(n+1)/2;
 if(g->ic[IC_TANK])n*=2;
 return n*hits;''')
fix('damage_player(g,raw);\n if(g->shop_cards[1]>0)',
'''if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;
 if(g->ic[IC_TANK])raw*=2;
 damage_player(g,raw);
 if(g->shop_cards[1]>0)''')
# Replay Howl from Beyond from Exhaust, Stampede before normal Hand discard,
# track future attack effects as player turn (not enemy turn).
fix('void game_end_turn(Game*g){\n if(g->screen!=COMBAT)return;',
'''void game_end_turn(Game*g){
 if(g->screen!=COMBAT)return;
 if(g->ic[IC_STAMPEDE]>0&&g->hand_count>0){
  int copies=MIN(4,g->ic[IC_STAMPEDE]);
  for(int j=0;j<copies&&g->screen==COMBAT;j++){
   int count=0;
   for(int i=0;i<g->hand_count;i++)
    if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)count++;
   int foe=random_living_foe(g);
   if(!count||foe<0)break;
   int which=rnd(g,count);
   for(int i=0;i<g->hand_count;i++)
    if(cards[CARD_ID(g->hand[i])].type==0&&which--==0){
     g->hand[i]|=CARD_FREE;
     game_play_card(g,i,foe);break;
    }
  }
 }
 if(g->screen!=COMBAT)return;
 for(int i=0;i<g->exhaust_count;i++)if(CARD_ID(g->exhaust[i])==70)
   all_damage(g,16+(CARD_UP(g->exhaust[i])?5:0));
 if(g->ic[IC_PLATING]>0){
  g->block=MIN(999,g->block+g->ic[IC_PLATING]);
  proc_juggernaut(g);
 }
 if(g->ic[IC_TEMP_STRENGTH]>0){
  g->strength-=g->ic[IC_TEMP_STRENGTH];g->ic[IC_TEMP_STRENGTH]=0;
 }
 g->ic[IC_OWN_TURN]=0;''')
fix('g->block=0;\n for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)enemy_intent',
'''if(!g->ic[IC_BARRICADE])g->block=0;
 g->ic[IC_COLOSSUS]=0;
 for(int i=0;i<g->enemy_count;i++){
  if(i<2&&g->ic[IC_MANGLE0+i]!=0){
   g->enemies[i].strength+=g->ic[IC_MANGLE0+i];
   g->ic[IC_MANGLE0+i]=0;
  }
 }
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)enemy_intent''')
# Exhaust triggers Dark Embrace and Drum of Battle.
fix('g->reward_potion|=4;\n  if(g->shop_cards[2]>0)',
'''g->reward_potion|=4;
  if(CARD_ID(raw)==64&&g->ic[IC_OWN_TURN])gain_energy(g,2+CARD_UP(raw));
  if(g->ic[IC_DARK_EMBRACE]&&g->ic[IC_CHAIN_DEPTH]<20){
    g->ic[IC_CHAIN_DEPTH]++;
    for(int j=0;j<g->ic[IC_DARK_EMBRACE];j++)draw_card(g);
    g->ic[IC_CHAIN_DEPTH]--;
  }
  if(g->shop_cards[2]>0)''')
# Continue using the central card-sourced Block API for Rage (Juggernaut needs it).
fix('if(rage)g->block=MIN(999,g->block+rage);',
    'if(rage)gain_block(g,rage);')
# Card upgrades/rarities are handled in the catalog module; effect switch next.
p.write_text(s);h.write_text(hs)
print("PASS IC87 hooks: draw, power stacks, defense, vulnerability, auto-play and turn lifecycle.")
