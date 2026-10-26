#include "../../GAME/src/game.c"
#include <assert.h>
static void owned(Game*g,int id){game_new(g,139);g->relic_count=0;add_relic(g,id);}
static void fight(Game*g,int id){owned(g,id);game_start_battle(g,NODE_FIGHT);g->enemy_count=1;g->enemies[0].hp=g->enemies[0].maxhp=999;g->enemies[0].block=g->enemies[0].weak=g->enemies[0].vulnerable=0;g->energy=99;}
static void play(Game*g,int card){g->screen=COMBAT;g->hand[0]=card;g->hand_count=1;g->selection=0;game_play_card(g,0,0);}
int main(void){Game g;int hp,gold,block;
 fight(&g,0);g.hp=40;g.enemies[0].hp=0;check_win(&g);assert(g.hp==46);
 fight(&g,1);assert(g.energy==99);g.energy=0;g.turn=0;game_start_turn(&g);assert(g.energy==4);game_start_turn(&g);assert(g.energy==3);
 fight(&g,2);g.block=100;g.enemies[0].intent=0;g.enemies[0].intent_value=10;game_end_turn(&g);assert(g.hp==80&&g.enemies[0].hp==995);
 fight(&g,3);play(&g,27);assert(g.enemies[0].hp==979);play(&g,0);assert(g.enemies[0].hp==973);g.turn=1;game_start_turn(&g);play(&g,0);assert(g.enemies[0].hp==962);
 owned(&g,4);assert(g.maxhp==94&&g.hp==94);
 fight(&g,5);for(int i=0;i<3;i++)play(&g,0);assert(g.strength==1);game_start_turn(&g);for(int i=0;i<3;i++)play(&g,0);assert(g.strength==2);
 owned(&g,6);g.screen=SHOP;g.shop_cards[0]=5;g.gold=200;int price=50+25*cards[5].rarity-30;shop_buy(&g,0);assert(g.gold==200-price);g.shop_relic=4;shop_buy(&g,3);assert(g.gold==200-price-125);
 fight(&g,7);assert(g.hand_count==7);
 owned(&g,8);g.hp=10;g.screen=REST;rest_pick(&g,0);assert(g.hp==34&&g.screen==DECK_VIEW&&g.prior_screen==REST);game_action(&g,BTN_OK);assert(CARD_UP(g.deck[0])&&g.screen==MAP);
 fight(&g,9);play(&g,0);assert(g.enemies[0].weak==2);g.enemies[0].weak=0;play(&g,0);assert(g.enemies[0].weak==0);
 fight(&g,10);assert(g.block==12);
 owned(&g,11);assert(g.maxhp==68&&g.max_energy==4);
 fight(&g,12);g.battle_type=NODE_ELITE;g.enemies[0].hp=0;check_win(&g);assert(g.permanent_strength==1);
 fight(&g,13);fill_rewards(&g);for(int i=0;i<3;i++)assert(CARD_UP(g.reward[i]));
 fight(&g,14);g.hp=40;game_start_turn(&g);assert(g.strength==3);game_start_turn(&g);assert(g.strength==3);
 fight(&g,15);play(&g,1);assert(g.block==7);
 fight(&g,16);gold=g.gold;int reward=g.reward_gold;g.enemies[0].hp=0;check_win(&g);assert(g.gold==gold+reward+20);
 fight(&g,17);g.hp=20;g.potion_count=1;g.potions[0]=0;game_use_potion(&g,0);assert(g.hp==43);g.potion_count=1;g.potions[0]=1;game_use_potion(&g,0);assert(g.strength==3);
 fight(&g,18);assert(g.hand_count==6);
 owned(&g,19);game_start_battle(&g,NODE_FIGHT);for(int i=0;i<g.enemy_count;i++)assert(g.enemies[i].vulnerable==2);
 fight(&g,20);g.hp=40;game_start_turn(&g);assert(g.hp==40);game_start_turn(&g);assert(g.hp==44);
 fight(&g,21);g.potion_count=1;g.potions[0]=1;game_use_potion(&g,0);assert(g.block==12);
 fight(&g,22);for(int i=0;i<3;i++)play(&g,1);assert(g.dexterity==1);game_start_turn(&g);for(int i=0;i<3;i++)play(&g,1);assert(g.dexterity==2);
 fight(&g,23);g.hp=1;lose_hp(&g,8);assert(g.hp==20&&g.screen==COMBAT);lose_hp(&g,25);assert(g.screen==DEFEAT);
 fight(&g,24);g.hp=40;for(int i=0;i<3;i++)play(&g,27);assert(g.hp==43);
 owned(&g,25);game_start_battle(&g,NODE_FIGHT);assert(g.energy==4&&g.hand_count==4);
 fight(&g,26);g.hp=40;play(&g,0);assert(g.enemies[0].hp==990);g.hp=41;play(&g,0);assert(g.enemies[0].hp==984);
 owned(&g,27);g.screen=REWARD;g.hp=40;g.selection=3;game_action(&g,BTN_OK);assert(g.hp==46);
 fight(&g,28);assert(g.strength==1);fight(&g,29);assert(g.block==10);fight(&g,30);assert(g.hand_count==7);
 owned(&g,31);assert(g.hp==92&&g.maxhp==92);owned(&g,32);assert(g.gold==195);
 owned(&g,33);game_start_battle(&g,NODE_FIGHT);assert(g.energy==4);game_start_turn(&g);assert(g.energy==3);
 fight(&g,34);game_start_turn(&g);assert(!g.strength);game_start_turn(&g);assert(g.strength==1);
 fight(&g,35);assert(g.dexterity==1);fight(&g,36);g.hp=40;g.enemies[0].hp=0;check_win(&g);assert(g.hp==43);
 puts("PASS all 28 relic effects and all 9 entrance blessings; every-turn/first-turn/first-card/hit/act trigger boundaries");return 0;
}
