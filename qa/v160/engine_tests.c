#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "source/game.c"
static Game arena(int ch){Game g;game_new(&g,151);game_choose_character(&g,ch);game_start_battle(&g,NODE_FIGHT);g.relic_count=0;g.enemy_count=2;for(int e=0;e<2;e++){g.enemies[e].hp=g.enemies[e].maxhp=10000;g.enemies[e].block=g.enemies[e].weak=g.enemies[e].vulnerable=g.enemies[e].poison=0;g.enemies[e].intent=1;g.enemies[e].intent_value=0;}g.hand_count=g.draw_count=g.discard_count=g.exhaust_count=0;g.hp=40;g.energy=50;g.block=g.strength=g.dexterity=0;g.ic[IC_OWN_TURN]=1;return g;}
static void play(Game*g,int raw){g->hand[g->hand_count++]=raw;game_play_card(g,g->hand_count-1,0);}
static void stock(Game*g,int n){for(int i=0;i<n;i++)g->draw[g->draw_count++]=g->character?N_DEFEND_SILENT:1;}
static void reward(Game*g,int potion){*g=arena(0);g->screen=REWARD;g->selection=0;g->v151.card_done=0;g->v151.potion_offer=potion;g->reward[0]=3;g->reward[1]=4;g->reward[2]=5;g->potion_count=0;}
int main(void){Game g,b;int total=0;
 assert(RELIC_COUNT==64);
 for(int ch=0;ch<2;ch++){
  g=arena(ch);add_relic(&g,53);lose_hp(&g,2);assert(g.hp==38&&g.energy==51&&g.block==4);lose_hp(&g,2);assert(g.energy==51&&g.block==4);g.ic[IC_OWN_TURN]=0;g.v151.relic[0]=0;lose_hp(&g,2);assert(g.energy==51);total++;
  g=arena(ch);add_relic(&g,54);game_end_turn(&g);assert(g.energy==4);play(&g,ch?N_STRIKE_SILENT:0);game_end_turn(&g);assert(g.energy==3);total++;
  g=arena(ch);add_relic(&g,55);for(int i=0;i<3;i++)play(&g,ch?N_STRIKE_SILENT:0);assert(g.enemies[0].poison==1&&g.enemies[1].poison==1);for(int i=0;i<3;i++)play(&g,ch?N_STRIKE_SILENT:0);assert(g.enemies[0].poison==2);total++;
  g=arena(ch);add_relic(&g,56);stock(&g,8);for(int i=0;i<3;i++)exhaust_card(&g,1);assert(g.energy==51&&g.hand_count==2);for(int i=0;i<3;i++)exhaust_card(&g,1);assert(g.energy==51&&g.hand_count==2);total++;
  g=arena(ch);add_relic(&g,57);for(int i=0;i<4;i++)play(&g,ch?N_ACCURACY:8);for(int i=0;i<2;i++)g.enemies[i].hp=0;check_win(&g);assert(g.hp==46);total++;
  g=arena(ch);add_relic(&g,58);stock(&g,8);play(&g,ch?N_NEUTRALIZE:2);assert(g.hand_count==1);play(&g,ch?N_NEUTRALIZE:2);assert(g.hand_count==1);total++;
  g=arena(ch);add_relic(&g,59);g.block=19;game_prepare_end_turn(&g);assert(g.enemies[0].poison==0);g.ic[38]=0;g.block=20;game_prepare_end_turn(&g);assert(g.enemies[0].poison==2&&g.enemies[1].poison==2);game_prepare_end_turn(&g);assert(g.enemies[0].poison==2);total++;
  g=arena(ch);add_relic(&g,60);int gold=g.gold;for(int i=0;i<3;i++)r151_skip_reward(&g);int ups=0;for(int i=0;i<g.deck_count;i++)ups+=CARD_UP(g.deck[i]);assert(g.gold==gold+24&&ups==1&&g.v151.relic[7]==0);total++;
  g=arena(ch);add_relic(&g,61);add_card(&g,50);add_card(&g,53);game_start_battle(&g,NODE_FIGHT);assert(g.strength==2);total++;
  g=arena(ch);add_relic(&g,62);stock(&g,8);play(&g,ch?N_STRIKE_SILENT:0);play(&g,ch?N_DEFEND_SILENT:1);int energy=g.energy;play(&g,ch?N_ACCURACY:8);assert(g.energy==energy-game_card_cost(ch?N_ACCURACY:8)+2&&g.hand_count==2&&g.v151.relic[9]==15);play(&g,ch?N_ACCURACY:8);assert(g.hand_count==2);total++;
 }
 // Card-choice completion cannot drop or duplicate a new relic trigger.
 g=arena(0);add_relic(&g,55);play(&g,0);play(&g,0);g.discard[g.discard_count++]=1;play(&g,25);assert(g.screen==CARD_CHOICE&&g.enemies[0].poison==0);game_action(&g,BTN_OK);assert(g.enemies[0].poison==1&&g.v151.relic[10]==0);
 g=arena(0);add_relic(&g,56);stock(&g,8);exhaust_card(&g,1);exhaust_card(&g,1);g.hand[g.hand_count++]=1;play(&g,17|UPBIT);assert(g.screen==CARD_CHOICE);int energy=g.energy;game_action(&g,BTN_OK);assert(g.energy==energy+1&&g.hand_count==2&&g.v151.relic[3]==3);
 // Old and new effects stack: self-loss Block + Juggernaut, poison + Skull,
 // Chalice + Cinder Heart, and Prism + Duelist energy.
 g=arena(0);add_relic(&g,53);g.ic[IC_JUGGERNAUT]=5;lose_hp(&g,1);assert(g.enemies[0].hp+g.enemies[1].hp==19995&&g.block==4);
 g=arena(1);add_relic(&g,55);add_relic(&g,NR_SKULL);for(int i=0;i<3;i++)play(&g,N_STRIKE_SILENT);assert(g.enemies[0].poison==2&&g.enemies[1].poison==2);
 g=arena(0);add_relic(&g,57);add_relic(&g,0);play(&g,8);for(int i=0;i<2;i++)g.enemies[i].hp=0;check_win(&g);assert(g.hp==48);
 g=arena(0);add_relic(&g,62);add_relic(&g,43);stock(&g,8);play(&g,0);play(&g,1);play(&g,8);assert(g.energy==50-1-1-1+1+2);
 // Every independent take/leave combination and order, no duplicate grant.
 for(int card=0;card<2;card++)for(int pot=0;pot<2;pot++)for(int order=0;order<2;order++){
  reward(&g,1);int n=g.deck_count;for(int step=0;step<2;step++){int which=(step+order)%2;g.selection=which?(pot?4:5):(card?0:3);game_action(&g,BTN_OK);if(step==0){assert(g.screen==REWARD);assert(game_save(&g,"qa/v160/evidence/pending-reward.dat"));assert(game_load(&b,"qa/v160/evidence/pending-reward.dat"));assert(!memcmp(&g,&b,sizeof(g)));}}
  assert(g.screen==MAP&&g.deck_count==n+card&&g.potion_count==pot);total++;
 }
 reward(&g,1);g.potion_count=3;g.potions[0]=0;g.potions[1]=2;g.potions[2]=5;g.selection=0;game_action(&g,BTN_OK);int n=g.deck_count;game_action(&g,BTN_OK);assert(g.screen==REWARD&&g.potion_count==3&&g.v151.potion_offer==1&&g.deck_count==n);
 int open=0,tab=0,scroll=0,potion=0;game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion);assert(open&&tab==3);game_inventory_action(&g,BTN_POTION,&open,&tab,&scroll,&potion);assert(g.potion_count==2&&g.potions[0]==2);game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion);game_action(&g,BTN_OK);assert(g.screen==MAP&&g.potion_count==3&&g.potions[2]==1&&g.deck_count==n);
 // Each requested menu can open/browse/close without changing any game byte.
 int screens[]={COMBAT,MAP,SHOP,REST,EVENT,EVENT_CARD,EVENT_RESULT,REWARD,REWARD_PREVIEW,RELIC_RESULT,DECK_VIEW};
 for(unsigned i=0;i<sizeof(screens)/sizeof(*screens);i++){
  g=arena(0);g.screen=screens[i];g.selection=1;g.scroll=2;g.shop_selection=1;g.next_screen=MOD_KEEN;b=g;open=0;
  game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion);assert(open);
  for(int j=0;j<6;j++)game_inventory_action(&g,BTN_RIGHT,&open,&tab,&scroll,&potion);
  game_inventory_action(&g,BTN_ALT,&open,&tab,&scroll,&potion);assert(tab==5);game_inventory_action(&g,BTN_DOWN,&open,&tab,&scroll,&potion);
  game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion);assert(!open&&!memcmp(&g,&b,sizeof(g)));total++;
 }
 for(int screen=CARD_CHOICE;screen<=NINJA_CHOICE;screen+=NINJA_CHOICE-CARD_CHOICE){g=arena(0);g.screen=screen;b=g;open=0;for(int key=BTN_ALT;key<=BTN_START;key*=2)assert(!game_inventory_action(&g,key,&open,&tab,&scroll,&potion));assert(!open&&!memcmp(&g,&b,sizeof(g)));}
 // Turn-end relics include automatic attacks and Plating before evaluation.
 g=arena(0);add_relic(&g,54);g.ic[IC_STAMPEDE]=1;g.hand[0]=0;g.hand_count=1;game_end_turn(&g);assert(g.energy==3);
 g=arena(0);add_relic(&g,59);g.block=15;g.ic[IC_PLATING]=5;game_prepare_end_turn(&g);assert(g.block==20&&g.enemies[0].poison==2);
 g=arena(0);add_relic(&g,60);r151_skip_reward(&g);game_start_battle(&g,NODE_FIGHT);assert(g.v151.relic[7]==1);
 // The Hunt extra card rewards do not grant the potion again.
 reward(&g,2);g.ninja.v[NS_BONUS_REWARDS]=2;g.selection=4;game_action(&g,BTN_OK);assert(g.potion_count==1);for(int i=0;i<3;i++){g.selection=3;game_action(&g,BTN_OK);assert(g.potion_count==1);}assert(g.screen==MAP);
 // A killing Fire potion closes combat inventory and preserves the reward offer.
 g=arena(0);for(int e=0;e<2;e++)g.enemies[e].hp=1;g.potion_count=1;g.potions[0]=3;open=1;tab=3;potion=0;
 assert(game_inventory_action(&g,BTN_OK,&open,&tab,&scroll,&potion)==2);assert(!open&&g.screen==REWARD&&g.potion_count==0);
 // All valid upgrade/modification combinations use the actual payment cost.
 int combos=0,changes=0;
 for(int id=0;id<CARD_COUNT;id++)for(int mod=0;mod<MOD_COUNT;mod++){
  if(!game_card_is_upgradeable(id)||(mod&&!game_can_modify(id,mod)))continue;
  int raw=id|(mod<<CARD_MOD_SHIFT),up=raw|UPBIT;g=arena(id>=NINJA_FIRST);g.hand[0]=up;g.hand_count=1;
  assert(game_card_cost(up)==game_card_cost_now(&g,up));if(game_card_cost(raw)!=game_card_cost(up))changes++;combos++;
 }
 assert(game_card_cost(21)==1&&game_card_cost(21|UPBIT)==0);assert(game_card_cost(21|UPBIT|(MOD_ECHO<<CARD_MOD_SHIFT))==1);assert(game_card_cost(N_BULLET_TIME)==3&&game_card_cost(N_BULLET_TIME|UPBIT)==2);
 // Migration of the previous release and current pending rewards.
 assert(game_load(&g,"qa/v150b/evidence/state-combat.dat"));assert(g.version==23&&g.character==CHAR_NIGHTSHADE&&g.v151.potion_offer==-1);
 printf("PASS %d relic/reward/menu scenarios; %d upgrade/mod cost combinations (%d cost changes); v21 migration\n",total,combos,changes);return 0;
}
