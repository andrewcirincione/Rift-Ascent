#ifndef RIFT_A18_TYPES_H
#define RIFT_A18_TYPES_H
#define A18_CARD_FIRST 334
#define A18_CARD_COUNT 60
#define A18_CARD_LAST 394
#define A18_POTION_FIRST 6
#define A18_POTION_COUNT 20
#define A18_RELIC_FIRST 64
#define A18_RELIC_COUNT 10
#define A18_SLOT_COUNT 4
#define A18_CARD_MASK 0x000fffffu
#define A18_RELIC_MASK 0x000003ffu
/* The 1.71-r2 Game prefix remains byte-identical. Only this tail is new. */
typedef struct {
 unsigned serial,available_cards,available_potions,available_relics;
 int slot,menu_mode,act3_claimed,secret_route,true_claimed,completed_acts;
 int card_awards[2],potion_awards[2],relic_award,award_win_number;
 int collection_tab,collection_character;
 int powers[20],values[4],turn_flags,next_energy,next_draw;
 int act4_claimed; unsigned act4_clear_number;
} A18Run;
typedef struct {
 unsigned serial; int cards[2],potions[2],relic;
 unsigned true_serial,act4_serial;
} A18Receipt;
typedef struct {
 unsigned magic,version,next_serial;
 unsigned wins[3],best_acts[3],cards[3],potions,relics,true_endings;
 unsigned runs,losses;
 unsigned act4_clears[3];
 A18Receipt receipts[A18_SLOT_COUNT];
} A18Profile;
typedef struct {
 int occupied,corrupt,character,act,floor,hp,maxhp,screen;
 unsigned serial;
} A18SlotInfo;
#endif
