#ifndef ASCENT_PROGRESS_H
#define ASCENT_PROGRESS_H
#define ASCENT_CARD_FIRST 334
#define ASCENT_CARD_LAST 394
#define ASCENT_SLOTS 4
#define ASCENT_ALL_CARDS 0xfffffu
typedef struct {
 unsigned id,slot,cards,potions,relics,secret,won,ascended;
 int unlocked_cards[2],unlocked_potions[2],unlocked_relic;
} AscentRun;
typedef struct {
 unsigned run,won,ascended;
 int cards[2],potions[2],relic;
} AscentReceipt;
typedef struct {
 unsigned magic,version,generation,next_run,cards[3],potions,relics;
 unsigned wins[3],best_act[3],ascended,debug;
 AscentReceipt receipts[ASCENT_SLOTS];
} AscentAccount;
typedef struct {int present,character,act,floor,hp,maxhp,screen;unsigned generation;int custom;} AscentSlot;
#endif
