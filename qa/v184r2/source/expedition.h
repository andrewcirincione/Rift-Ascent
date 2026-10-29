#ifndef RIFT_EXPEDITION_181_H
#define RIFT_EXPEDITION_181_H
/* Append only: the entire v1.8 Game prefix remains byte-identical. */
typedef struct {
 int extra_relics[32];
 unsigned story_seen;
 int story_pending,story_id,gift_kind,gift;
 int combat[12];
} Expedition181;
enum {E181_CARDS,E181_POWER_ENERGY,E181_WEAK_ATTACK,E181_HURT_BLOCK,
 E181_EXHAUST_HEAL,E181_LAST_MERCY,E181_FIRST_ATTACK,E181_NEXT_BLOCK,E181_POTION_USED};
#endif
