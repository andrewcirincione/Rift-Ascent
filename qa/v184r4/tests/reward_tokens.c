#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
int main(void){char root[]="/tmp/rift-r4-rewards-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;int offers=0,ancients=0;
 const int tokens[]={R_DEBRIS,R_MINION_STRIKE,R_MINION_DIVE_BOMB,R_MINION_SACRIFICE,R_SOVEREIGN_BLADE,N_SHIV,CARD_GIANT_ROCK};
 for(unsigned i=0;i<sizeof(tokens)/sizeof(*tokens);i++)assert(!combat_reward_eligible(tokens[i]));
 for(int c=0;c<3;c++)for(int unlocked=0;unlocked<2;unlocked++){
  game_new(&g,18440+c);game_choose_character(&g,c);g.ascent.cards=unlocked?ASCENT_ALL_CARDS:0;
  for(int mode=0;mode<3;mode++){g.battle_type=mode==0?NODE_FIGHT:mode==1?NODE_ELITE:NODE_BOSS;
   for(int n=0;n<1000;n++){fill_rewards(&g);for(int i=0;i<3;i++){assert(combat_reward_eligible(CARD_ID(g.reward[i])));ancients+=cards[CARD_ID(g.reward[i])].rarity==3;offers++;}}
  }
  for(int n=0;n<1000;n++){int card=reward_of_rarity(&g,3,-1,-1);assert(card!=R_DEBRIS&&combat_reward_eligible(card));int shop=card_reward_except(&g,-1,-1);assert(shop!=R_DEBRIS);}
 }
 assert(ancients>0);printf("PASS %d normal/elite/boss card offers across all characters and locked/unlocked accounts; no generated tokens. Real unlocked Ancient rewards remain possible (%d seen); story/event rarity picks and shop cards exclude Debris.\n",offers,ancients);
 assert(ascent_begin(&g,0,2,18441));g.screen=REWARD;g.battle_type=NODE_BOSS;g.reward[0]=R_DEBRIS;g.reward[1]=R_COMET;g.reward[2]=R_BIG_BANG;g.v151.card_done=0;
 assert(game_validate(&g)&&ascent_checkpoint(&g)&&ascent_resume(&h,0));assert(h.reward[0]!=R_DEBRIS&&combat_reward_eligible(CARD_ID(h.reward[0])));assert(h.reward[1]==g.reward[1]&&h.reward[2]==g.reward[2]&&h.deck_count==g.deck_count);
 char path[320];snprintf(path,sizeof(path),"%s/legacy.dat",root);assert(game_save(&g,path)&&game_load(&h,path)&&h.reward[0]!=R_DEBRIS);
 add_card(&g,R_DEBRIS);g.v151.card_done=1;assert(game_save(&g,path)&&game_load(&h,path));assert(h.reward[0]==R_DEBRIS&&h.deck[h.deck_count-1]==R_DEBRIS);
 puts("PASS old slot/legacy saves repair only unclaimed Debris offers, preserving other offers and already-owned cards.");
 g.v151.card_done=0;g.reward[0]=R_DEBRIS|UPBIT;assert(game_save(&g,path)&&game_load(&h,path));assert(CARD_UP(h.reward[0])&&CARD_ID(h.reward[0])!=R_DEBRIS);
 puts("PASS replacing an old upgraded Debris offer preserves its upgrade.");
 game_new(&g,18442);game_choose_character(&g,2);game_start_battle(&g,NODE_FIGHT);g.enemy_count=1;g.enemies[0].hp=g.enemies[0].maxhp=1000;g.energy=10;g.hand_count=1;g.hand[0]=R_COLLISION_COURSE;game_play_card(&g,0,0);assert(g.hand_count==1&&CARD_ID(g.hand[0])==R_DEBRIS);game_play_card(&g,0,0);assert(!g.hand_count&&g.exhaust_count==1&&CARD_ID(g.exhaust[0])==R_DEBRIS);
 puts("PASS Collision Course still creates playable Debris, and Debris still exhausts normally.");return 0;}
