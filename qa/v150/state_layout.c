#include "game.h"
#include <stdio.h>
int main(void){printf("{\"size\":%zu",sizeof(Game));
#define FIELD(n) printf(",\""#n"\":%zu",offsetof(Game,n))
FIELD(screen);FIELD(prior_screen);FIELD(act);FIELD(floor);FIELD(lane);FIELD(selection);FIELD(target);FIELD(scroll);FIELD(hp);FIELD(maxhp);FIELD(block);FIELD(energy);FIELD(gold);FIELD(turn);FIELD(strength);FIELD(deck_count);FIELD(deck);FIELD(draw_count);FIELD(discard_count);FIELD(exhaust_count);FIELD(hand_count);FIELD(hand);FIELD(enemies);FIELD(relic_count);FIELD(relics);FIELD(potion_count);FIELD(potions);FIELD(ic);FIELD(effect_turn);FIELD(reward);FIELD(endless);FIELD(message);
puts("}");}
