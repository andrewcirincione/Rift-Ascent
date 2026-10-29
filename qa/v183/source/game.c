#include "game.h"
#include "story170_data.h"
#include "expedition_stories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A private observer is enabled only while resolving a copied Game for a
   preview. No renderer, audio, save, or live RNG is touched by the probe. */
static CardPreview *preview_trace;
static int ascent_complete(Game*,int);
static int expedition_action(Game*,int);
static void heal(Game*,int);
static void relic_block(Game*,int);
static void expedition_roll(Game*);
static void expedition_prepare_gift(Game*);
static void expedition_mystery(Game*);
static void expedition_boss_spoils(Game*);
static int expedition_relic(Game*);
static void revision182_migrate_room(Game*);
static void ascent_record_act(Game*);
static int ascent_random_potion(Game*);
static int preview_depth,preview_phase,preview_choice=-1,preview_nested;
/* Transient call-stack value: a repeated X Attack retains its original X. */
static int repeated_x=-1;
static void preview_hit(int enemy,int amount){
 if(!preview_trace||!preview_phase)return;
 CardPreview*p=preview_trace;
 if(preview_nested)p->uncertain=1;
 if(!p->hits[enemy]||amount<p->low[enemy])p->low[enemy]=amount;
 if(!p->hits[enemy]||amount>p->high[enemy])p->high[enemy]=amount;
 p->hits[enemy]++;p->total[enemy]+=amount;
 if(preview_depth>1)p->repeat_hits++;
}

/* Presentation observes resolution; it never changes combat or consumes RNG. */
static GameCombatFx combat_fx[MAX_COMBAT_FX];
static int r140_attack_context,r140_rust_targets;
static int r150_card_raw;
static int combat_fx_count,combat_fx_recording,combat_fx_source=-1,combat_fx_card=-1;
void game_combat_fx_begin(void){combat_fx_count=0;combat_fx_recording=1;combat_fx_source=combat_fx_card=-1;}
void game_combat_fx_end(void){combat_fx_recording=0;}
const GameCombatFx *game_combat_fx_events(int*count){if(count)*count=combat_fx_count;return combat_fx;}
int game_impact_tier(int damage){return damage>20?3:damage>10?2:damage>0?1:0;}
static void record_combat_fx(int kind,int source,int target,int amount,int absorbed){
 if(!combat_fx_recording||preview_trace||combat_fx_count>=MAX_COMBAT_FX)return;
 GameCombatFx*e=&combat_fx[combat_fx_count++];
 e->kind=kind;e->source=source;e->target=target;e->amount=amount;e->absorbed=absorbed;e->card=combat_fx_card;
}

/* Original Rift Ascent art; card mechanics adapted from the reference catalog. */
const CardDef cards[]={
 {"Strike","Deal 6 damage",1,0,0},                         //0
 {"Defend","Gain 5 Block",1,0,1},                       //1
 {"Bash","Deal 8 damage. Apply 2 Vulnerable.",2,0,0},            //2
 {"Stomp","12 damage to all enemies. Cost -1 per Attack played this turn.",3,1,0},          //3
 {"Battle Trance","Draw 3 cards. Cannot draw more this turn.",0,1,1},                    //4
 {"Shrug It Off","Gain 8 Block. Draw 1 card.",1,0,1},                  //5
 {"Bloodletting","Lose 3 HP. Gain 2 Energy.",0,0,1},     //6
 {"Bludgeon","Deal 32 damage.",3,1,0},               //7
 {"Inflame","Gain 2 Strength.",1,1,2},                //8
 {"Impervious","Gain 30 Block. Exhaust.",2,2,1},                    //9
 {"Whirlwind","Deal 5 damage to all enemies X times.",-1,1,0},         //10
 {"Pommel Strike","Deal 9 damage. Draw 1 card.",1,0,0},                   //11
 {"Iron Wave","Gain 5 Block. Deal 5 damage.",1,0,0},          //12
 {"Ashen Strike","Deal 6 damage, plus 3 base damage for each card in your Exhaust pile.",1,1,0},                    //13
 {"Breakthrough","Lose 1 HP. Deal 9 damage to all enemies.",1,0,0},         //14
 {"Cinder","Deal 18 damage. Exhaust a RANDOM card in hand.",2,0,0},        //15
 {"Evil Eye","Gain 8 Block. If you exhausted a card this turn, gain 8 more Block.",1,1,1},      //16
 {"True Grit","Gain 7 Block. Exhaust RANDOM card.",1,0,1},          //17
 {"Brand","Lose 1 HP. Exhaust 1 card. Gain 1 Strength.",0,2,1},//18
 {"Thunderclap","Hit all enemies for 4. Apply 1 Vulnerable.",1,0,0},//19
 {"Feel No Pain","Gain 3 Block whenever a card is Exhausted.",1,1,2},              //20
 {"Body Slam","Deal damage equal to your Block.",1,0,0},  //21
 {"Sword Boomerang","3 damage to a random enemy 3 times.",1,0,0},           //22
 {"Armaments","Gain 5 Block. Upgrade 1 card in hand.",1,0,1},      //23
 {"Flame Barrier","Gain 12 Block. Reflect 4 per enemy hit this turn.",2,1,1},       //24
 {"Headbutt","Deal 9 damage. Choose a discarded card and put it on top of your draw pile.",1,0,0},             //25
 {"Pillage","Deal 6 damage. Draw cards until you draw a card that is not an Attack.",1,1,0},               //26
 {"Twin Strike","Deal 5 damage twice.",1,0,0},                //27
 {"Inferno","Each turn: lose 1 HP. HP lost on your turn: deal 6 damage to all enemies.",1,1,2},         //28
 {"Hemokinesis","Lose 2 HP. Deal 15 damage.",1,1,0},              //29
 {"Tremble","Apply 3 Vulnerable. Exhaust.",1,0,1},           //30
 {"Taunt","Gain 7 Block. Apply 1 Vulnerable.",1,1,1},      //31
 {"Dismantle","Deal 8 damage. Twice if enemy is Vulnerable.",1,1,0},     //32
 {"Rage","Each Attack this turn grants 3 Block.",0,1,1},        //33
 {"Dominate","Apply 1 Vulnerable. Gain 1 Strength per Vulnerable. Exhaust.",1,1,1},//34
 {"Bully","Deal 4 damage, plus 2 base damage for each turn of Vulnerable on the target.",0,1,0},  //35
 {"Conflagration","Deal 2 damage to ALL enemies 4 times.",1,2,0},     //36
 {"Pact's End","If at least 3 cards are exhausted, deal 18 damage to all enemies.",0,2,0},       //37
 {"Offering","Lose 6 HP. Gain 2 Energy. Draw 3 cards. Exhaust.",0,2,1},               //38
 {"Uppercut","Deal 13 damage. Apply Weak and Vulnerable.",2,1,0},           //39
 {"Molten Fist","Deal 10 damage. Double foe Vulnerable. Exhaust.",1,0,0},             //40
 {"Rupture","Whenever you lose HP on your turn, gain 1 Strength.",1,1,2},                 //41
 {"Fiend Fire","Exhaust hand. Deal 7 per exhausted card.",2,2,0},               //42
 {"Burning Pact","Exhaust 1 card. Draw 2 cards.",1,1,1},             //43
 {"Anger","Deal 6 damage. Add a copy to discard.",0,0,0},           //44
 {"Second Wind","Exhaust all non-Attacks. +5 Block each.",1,1,1},        //45
 {"Blood Wall","Lose 2 HP. Gain 16 Block.",2,0,1},         //46
 {"Feed","Deal 10 damage. On kill, gain 3 max HP.",1,2,0},       //47
 {"Demon Form","At turn start: +3 Strength.",3,2,2},       //48
 {"Perfected Strike","Deal 6, +2 per Strike you own.",2,0,0},   //49
 {"Burn","Unplayable. Lose 2 HP if held at the end of your turn.",-2,0,3},          //50
 {"Daze","Unplayable",-2,0,3},                         //51
 {"Injury","Unplayable",-2,0,3},                       //52
 {"Dread","Unplayable. Held at turn end: -1 Energy next turn.",-2,0,3},          //53
 {"Aggression","At the start of your turn, return a random Attack from your discard pile to your hand and upgrade it.",1,2,2}, // 54
 {"Barricade","Block is retained between turns.",3,2,2}, // 55
 {"Break","Deal 20 damage. Apply 5 Vulnerable.",1,3,0}, // 56
 {"Cascade","Play the top X cards of your draw pile.",-1,2,1}, // 57
 {"Colossus","Gain 5 Block. Half damage from Vulnerable foes.",1,1,1}, // 58
 {"Corruption","Skills cost 0 and Exhaust on play.",3,3,2}, // 59
 {"Crimson Mantle","Turn start: lose 1 HP; gain 8 Block.",1,2,2}, // 60
 {"Cruelty","Vulnerable foes take +25% damage.",1,2,2}, // 61
 {"Dark Embrace","When a card Exhausts, draw 1.",2,2,2}, // 62
 {"Demonic Shield","MULTIPLAYER: lose 1 HP; give ally your Block.",0,1,1}, // 63
 {"Drum of Battle","Draw 2 cards. On Exhaust: gain 2 Energy.",1,1,1}, // 64
 {"Expect a Fight","Gain 1 Energy per Attack in your hand. Then you cannot gain Energy this turn.",2,1,1}, // 65
 {"Fight Me!","Deal 5 twice. Gain 3 STR; enemy +1 STR.",2,1,0}, // 66
 {"Forgotten Ritual","If Exhausted this turn, gain 3 Energy. Exhaust.",1,1,1}, // 67
 {"Havoc","Play top Draw card; Exhaust it.",1,0,1}, // 68
 {"Hellraiser","Draw Strike: auto-play it on random enemy.",2,2,2}, // 69
 {"Howl from Beyond","Deal 16 damage to all enemies. At turn end, play this card from your exhaust pile.",3,1,0}, // 70
 {"Infernal Blade","Add random Attack to Hand; free this turn. Exhaust.",1,1,1}, // 71
 {"Juggernaut","Gain Block: deal 6 to random foe.",2,2,2}, // 72
 {"Juggling","On 3rd Attack each turn, copy it to Hand.",1,1,2}, // 73
 {"Mangle","Deal 15 damage. Enemy -10 STR this turn.",3,2,0}, // 74
 {"Not Yet","Heal 10 HP. Exhaust.",2,2,1}, // 75
 {"One-Two Punch","Next Attack this turn plays twice.",1,2,1}, // 76
 {"Primal Force","Transform hand Attacks into Giant Rocks.",0,2,1}, // 77
 {"Pyre","Start turn: gain 1 Energy.",2,2,2}, // 78
 {"Rampage","Deal 9 damage. This Rampage +5 damage for combat.",1,1,0}, // 79
 {"Setup Strike","Deal 7 damage. Gain 2 STR this turn.",1,0,0}, // 80
 {"Spite","Deal 5 damage. If lost HP this turn: hit twice.",0,1,0}, // 81
 {"Stampede","End turn: auto-play random Hand Attack.",2,1,2}, // 82
 {"Stoke","Exhaust Hand. Add random card per exhausted.",1,2,1}, // 83
 {"Stone Armor","Gain 4 Plating. End turn: gain Block.",1,1,2}, // 84
 {"Tank","MULTIPLAYER: double enemy damage to self, halve allies.",1,2,2}, // 85
 {"Tear Asunder","Deal 5 damage. +1 hit per HP-loss event this combat.",2,2,0}, // 86
 {"Thrash","Deal 4 twice. Random Attack exhausts; gain damage.",1,2,0}, // 87
 {"Unmovable","Each turn, double the first Block you gain from a card.",2,2,2}, // 88
 {"Unrelenting","Deal 14 damage. Next Attack costs 0.",2,1,0}, // 89
 {"Vicious","Applying Vulnerable draws 1 card.",1,1,2}, // 90
 {"Giant Rock","Deal 16 damage. (Generated token)",1,0,0}, // 91
 {"Grave Cleaver","Remove 6 enemy Block. Deal 14 damage.",2,1,0}, // 92
 {"Dusk Guard","Gain 6 Block, or 10 if any enemy intends to attack.",1,0,1}, // 93
 {"Mercy's Edge","Deal 8 damage. If the target is Weak, heal 3 HP.",1,1,0}, // 94
 {"Black Covenant","At turn end, deal 4 damage to all enemies per unspent Energy (up to 2). Ignores damage modifiers.",1,2,2}, // 95
#include "ninja_cards.inc"
#include "regent_cards.inc"
#include "new_cards.inc"
};
const int CARD_COUNT=(int)(sizeof(cards)/sizeof(cards[0]));
static const char *upgrade_descriptions[]={
 "Deal 9 damage","Gain 8 block","Deal 11 damage. Apply 3 Vulnerable.","15 damage to all enemies. Cost -1 per Attack played.",
 "Draw 4 cards. Cannot draw more this turn.","Gain 11 Block. Draw 1","Lose 3 HP. Gain 3 Energy.","Deal 38 damage",
 "Gain 3 Strength","Gain 36 Block. Exhaust.","Deal 8 damage to all enemies X times.","Deal 10 damage. Draw 2 cards.",
 "Gain 8 Block. Deal 8 damage.","Deal 6 damage, plus 4 base damage for each card in your Exhaust pile.","Lose 1 HP. Deal 13 damage to all enemies.","Deal 24 damage. Exhaust a random hand card.",
 "Gain 11 Block. If you exhausted a card this turn, gain 11 more Block.","Gain 9 Block. Choose one card to Exhaust.","Lose 1 HP. Gain 2 Strength. Exhaust 1 card.","Hit all for 7. Apply 2 Vulnerable.",
 "Gain 4 Block per card Exhausted.","Deal damage equal to your Block. Cost 0.","3 damage to a random enemy 4 times.","Gain 5 Block. Upgrade ALL cards in hand.",
 "Gain 16 Block. Reflect 6 per enemy hit this turn.","Deal 12 damage. Choose a discarded card and put it on top of your draw pile.","Deal 9 damage. Draw cards until you draw a card that is not an Attack.","Deal 8 twice",
 "Each turn: lose 1 HP. HP lost on your turn: deal 9 damage to all enemies.","Lose 2 HP. Deal 20 damage.","Apply 4 Vulnerable. Exhaust.","Gain 10 Block. Apply 2 Vulnerable.",
 "Deal 10 damage. Twice if target Vulnerable.","Each Attack this turn grants 5 Block.","Apply 2 Vulnerable. Gain 1 Strength per Vulnerable. Exhaust.","Deal 4 damage, plus 3 base damage for each turn of Vulnerable on the target.",
 "Deal 2 damage to ALL enemies 5 times.","If at least 3 cards are exhausted, deal 24 damage to all enemies.","Lose 6 HP. Gain 2 Energy. Draw 4 cards. Exhaust.","Deal 16 damage. Apply 2 Weak and 2 Vulnerable.",
 "Deal 14 damage. Double foe Vulnerable. Exhaust.","When you lose HP on your turn, gain 2 Strength.","Exhaust hand. Deal 10 per card.","Exhaust 1 card. Draw 3 cards.",
 "Deal 9 damage. Add a copy to discard.","Exhaust all non-Attacks. +7 Block each.","Lose 2 HP. Gain 22 Block.","Deal 12 damage. Fatal: gain 4 max HP.",
 "At turn start: +4 Strength.","Deal 6, +3 per Strike you own.",
 NULL,NULL,NULL,NULL,
"Innate. At turn start, return a random discarded Attack to your hand and upgrade it.",
"Cost 2. Block is retained between turns.",
"Deal 30 damage. Apply 7 Vulnerable.",
"Play the top X+1 cards of your draw pile.",
"Gain 8 Block. Half damage from Vulnerable foes.",
"Skills cost 0. Playing them Exhausts them.",
"Turn start: lose 1 HP; gain 10 Block.",
"Vulnerable foes take +50% damage.",
"Cost 1. When a card Exhausts, draw 1.",
"MULTIPLAYER: lose 1 HP; give ally Block. No Exhaust.",
"Draw 2 cards. On Exhaust: gain 3 Energy.",
"Gain 1 Energy per Attack in your hand. Then you cannot gain Energy this turn.",
"Deal 6 twice. Gain 4 STR; enemy +1 STR.",
"If Exhausted this turn, gain 4 Energy. Exhaust.",
"Cost 0. Play top Draw card; Exhaust it.",
"Draw a Strike; auto-play it against a random foe.",
"Deal 21 damage to all enemies. At turn end, play this card from your exhaust pile.",
"Cost 0. Add random free Attack. Exhaust.",
"Gain Block: deal 8 to random foe.",
"Innate. On 3rd Attack each turn, copy to Hand.",
"Deal 20 damage. Enemy -15 STR this turn.",
"Heal 13 HP. Exhaust.",
"Next 2 Attacks this turn play twice.",
"Transform hand Attacks into Giant Rock+.",
"Start turn: gain 2 Energy.",
"Deal 9 damage. This Rampage +9 each use.",
"Deal 9 damage. Gain 3 STR this turn.",
"Deal 5 damage. If lost HP this turn: hit 3 times.",
"Cost 1. End turn: auto-play random Attack.",
"Exhaust Hand. Add upgraded card per exhausted.",
"Gain 6 Plating. End turn: gain Block.",
"Multiplayer-only: protects allies.",
"Deal 7 damage. +1 hit per HP-loss event.",
"Deal 6 twice. Exhaust Attack, add damage.",
"Each turn, double the first Block you gain from a card.",
"Deal 20 damage. Next Attack costs 0.",
"Applying Vulnerable draws 2 cards.",
"Deal 20 damage. (Generated token)",
"Remove 9 enemy Block. Deal 19 damage.",
"Gain 9 Block, or 13 if any enemy intends to attack.",
"Deal 11 damage. If the target is Weak, heal 4 HP.",
"At turn end, deal 6 damage to all enemies per unspent Energy (up to 2). Ignores damage modifiers.",
#include "ninja_upgrades.inc"
#include "regent_upgrades.inc"
#include "new_upgrades.inc"
};
#include "ninja_meta.inc"
#include "regent_meta.inc"
#include "new_card_flags.h"
#include "new_potions.h"
static int card_cost_base(int raw){
 int id=CARD_ID(raw);
 if(id<0||id>=CARD_COUNT)return -2;
 if(id>=ASCENT_CARD_FIRST)return cards[id].cost;
 if(id>=REGENT_FIRST)return CARD_UP(raw)?regent_cards[id-REGENT_FIRST].upcost:cards[id].cost;
 if(id>=NINJA_FIRST&&id<NINJA_LAST)return CARD_UP(raw)?ninja_cards[id-NINJA_FIRST].upcost:cards[id].cost;
 if(id==21&&CARD_UP(raw))return 0;
 if(id==37)return 0;
 if(CARD_UP(raw)&&(id==55||id==59||id==62||id==65||id==68||
    id==69||id==71||id==82||id==85||id==88))return cards[id].cost-1;
 return cards[id].cost;
}
static const char *card_description_base(int raw){
 int id=CARD_ID(raw);
 if(id<0||id>=CARD_COUNT)return "Unknown card";
 if(CARD_UP(raw)&&id<CARD_COUNT&&upgrade_descriptions[id])return upgrade_descriptions[id];
 return cards[id].desc;
}

#include "card_mods.inc"


static void exhaust_card(Game*g,int raw);
static void resolve_card(Game*g,int index,int enemy,int replay_raw);
static int regent_flags(int raw);
static int regent_stamp(Game*g,int raw);
static void regent_created(Game*g);
static void regent_on_draw(Game*g,int raw);
static void regent_on_hit(Game*g,int e,int amount);
static void regent_after_shuffle(Game*g);
static void regent_start_turn(Game*g);
static void regent_before_turn(Game*g);
static void regent_end_turn(Game*g);
static int regent_base_damage(const Game*g,int raw);
static void regent_choose(Game*g,int index,int finish);
static void regent_play(Game*g,int index,int target);
static int regent_cost_now(const Game*g,int raw);
static void regent_gain_stars(Game*g,int n);
static int regent_processing;
static void regent_foreign_begin(Game*g,int raw);
static void regent_foreign_finish(Game*g,int raw);

static int ninja_flags(int raw);
static int ninja_stamp(Game*g,int raw);
static int ninja_owned_card(const Game*g,int id);
static void r151_self_loss(Game*g);
static void r151_exhaust(Game*g);
static void r151_played(Game*g,int type);
static void r151_status_snapshot(const Game*g,int out[6]);
static void r151_after_effect(Game*g,const int before[6]);
static void r151_skip_reward(Game*g);
static int ninja_base_damage(const Game*g,int raw,int in_hand);
static void ninja_on_draw(Game*g,int raw);
static void ninja_on_hit(Game*g,int enemy,int unblocked);
static void ninja_poison(Game*g,int enemy,int n);
static void ninja_poison_all(Game*g,int n);
static void ninja_process(Game*g);
static void ninja_start_turn_before(Game*g);
static void ninja_start_turn_after(Game*g);
static int ninja_prepare_retain(Game*g);
static void ninja_choose(Game*g,int index,int finish);


const RelicDef relic_defs[]={
 {"Cinder Heart","After each won combat, heal 6 HP."},
 {"Copper Ring","On the first turn of each combat, gain 1 extra Energy."},
 {"Spiked Mail","When an enemy attacks you, deal 4 damage to that enemy, even if you Block."},
 {"Glass Emblem","Each turn, every hit of your first Attack card deals +5 damage."},
 {"Stone Flask","On pickup, gain 14 maximum HP and heal 14 HP."},
 {"Fury Censer","Every 3 Attack cards played in one turn, gain 1 Strength for this combat."},
 {"Merchant Coin","Cards and relics cost 30 less Gold in shops."},
 {"Old Compass","Draw 2 extra cards on the first turn of each combat."},
 {"Resting Idol","Resting also lets you upgrade one card, if any are eligible."},
 {"Rust Amulet","Each turn, your first Attack applies 2 Weak to each enemy it hits."},
 {"Guardian Knot","Start each combat with 12 Block."},
 {"Ashen Crown","On pickup, gain 1 Energy per turn; lose 12 maximum HP."},
 {"Medal of Valor","After each won elite fight, gain 1 permanent Strength for future combats."},
 {"Locket","Every card offered as a combat reward is upgraded."},
 {"Burning Thorn","At turn start, if at half HP or less, gain 3 Strength. Once per combat."},
 {"Fortress Pin","Each card effect that grants Block grants 2 extra Block."},
 {"Gold Tooth","After each won combat, gain 20 extra Gold."},
 {"Violet Orb","Improves every potion you use: +5 healing, +5 Block or +5 base Fire damage; +1 Strength, +1 Energy or +1 card drawn."},
 {"Sage's Ink","Draw 1 extra card at the start of every turn."},
 {"Broken Clock","At combat start, apply 2 Vulnerable to all enemies."},
 {"Iron Seed","At the start of turns 3, 6, 9 and so on, heal 4 HP."},
 {"Wildflower","During combat, using a potion grants 12 Block."},
 {"Worn Chain","Every 3 Skill cards played in one turn, gain 1 Dexterity for this combat."},
 {"Last Ember","Once per act, lethal damage instead restores you to 25% of maximum HP."},
 {"Bloodstone","Every 3 Attack cards played in one turn, heal 3 HP."},
 {"Sun Token","Gain 1 Energy per turn. Draw 1 fewer card at each turn start."},
 {"Hollow Skull","While at half HP or less, each hit of your Attacks deals +3 damage."},
 {"Scavenger","Whenever you skip a combat card reward, heal 6 HP."},
 {"Vow of Steel","Start every combat with 1 additional Strength."},
 {"Sanctuary","Start every combat with 10 additional Block."},
 {"Old Compass","Draw 2 extra cards on the first turn of each combat."} /* legacy Seer's Oath ID; never offered */,
 {"Vital Offering","On pickup, gain 12 maximum HP and heal to full."},
 {"Pilgrim's Purse","On pickup, gain 100 Gold."},
 {"Ember Pact","Gain 1 additional Energy on the first turn of every combat."},
 {"Blood Vow","At the start of turns 3, 6, 9 and so on, gain 1 Strength."},
 {"Bulwark Oath","Start every combat with 1 additional Dexterity."},
 {"Quiet Mercy","After every won combat, heal 3 additional HP."},
 {"Enduring Might","Gain 2 permanent Strength for every future combat. Can be chosen again."},
 {"Enduring Vitality","Gain 10 maximum HP and heal to full. Can be chosen again."},
 {"Enduring Fortune","Gain 150 Gold. Can be chosen again."},
 {"Ashen Hourglass","Save up to 2 unspent Energy at turn end. Gain that Energy next turn. Does not carry between combats."},
 {"Ossuary Bell","Your first 3 cards Exhausted each turn each grant 3 Block. Stacks with exhaust Powers; ignores Dexterity."},
 {"Penitent's Needle","Once each turn, the first self-inflicted HP loss during your turn draws 1 card. Enemy hits and Poison do not trigger it."},
 {"Duelist's Reliquary","Once each turn, after playing both an Attack and a Skill, gain 1 Energy. Either order works."},
 {"Serpent Ring","At combat start, draw 2 extra cards."},
 {"Venom Skull","Each Poison application adds 1 extra Poison."},
 {"Mourning Bells","Each card discarded on your turn deals 3 damage to a random enemy."},
 {"Grave Bindings","Each card discarded on your turn grants 3 Block."},
 {"Plague Funnel","At combat start, apply 4 Poison to all enemies."},
 {"Hidden Scroll","At combat start, add 3 Shivs to your hand."},
 {"Paper Raven","Weak enemies deal 40% less attack damage to you."},
 {"Spiral Needle","Playing a Shiv grants 1 Dexterity for this turn."},
 {"Drake Ring","Draw 2 extra cards on each of your first 3 turns. Replaces Serpent Ring."},
 {"Bloodglass Phial","First self-inflicted HP loss on your turn: gain 1 Energy and 4 Block."},
 {"Quiet Bell","End a turn without playing an Attack: gain 1 extra Energy next turn."},
 {"Twinfang Brooch","Every third Attack played each turn applies 1 Poison to all enemies."},
 {"Sepulchral Spindle","After exhausting 3 cards in a combat, draw 2 and gain 1 Energy. Once per combat."},
 {"Hollow Chalice","After combat, heal 2 HP per Power played, up to 6 HP."},
 {"Mourner's Lens","First card each turn that adds enemy Weak, Vulnerable or Poison: draw 1 card."},
 {"Briar Rosary","End your turn with at least 20 Block: apply 2 Poison to all enemies."},
 {"Grave Ledger","Skipping a card reward grants 8 Gold. Every third skip upgrades a random unupgraded card."},
 {"Thornbound Seal","Start combat with 1 extra Strength per curse or status card in your deck."},
 {"Prism Bone","Play an Attack, Skill and Power in one combat: gain 2 Energy and draw 2. Once per combat."},
 {"Divine Right","At the start of each combat, gain 3 Stars. Stars remain between turns."},

#include "new_relics.inc"
#include "expedition_relics.inc"
};
const int RELIC_COUNT=(int)(sizeof(relic_defs)/sizeof(relic_defs[0]));
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define CLAMP(x,a,b) MAX(a,MIN(x,b))

uint32_t game_random(Game *g){uint32_t x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;g->rng=x?x:0x4563871bu;return g->rng;}
static int rnd(Game *g,int max){return max>0?(int)(game_random(g)%(uint32_t)max):0;}
int game_relic_id(const Game*g,int i){if(i<0||i>=MAX_RELICS)return -1;return i<LEGACY_RELIC_CAP?g->relics[i]:g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int*relic_slot(Game*g,int i){return i<LEGACY_RELIC_CAP?&g->relics[i]:&g->expedition.extra_relics[i-LEGACY_RELIC_CAP];}
static int relic(const Game *g,int id){for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==id)return 1;return 0;}
static void enter_map(Game*g){
 if(g->revision182.room_pending){g->revision182.room_pending=0;expedition_roll(g);}
 if(g->expedition.story_pending){g->expedition.story_pending=0;g->screen=EXPEDITION_STORY;g->selection=0;expedition_prepare_gift(g);return;}
 g->screen=MAP;g->scroll=0;g->selection=CLAMP(g->lane,0,2);
 if(g->floor<MAP_ROWS&&!game_route_available(g,g->selection))
   for(int k=0;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}
}
static void msg(Game*g,const char *s){snprintf(g->message,sizeof(g->message),"%s",s);}
static void add_card(Game*g,int c){if(CARD_ID(c)>=ASCENT_CARD_FIRST&&!ascent_card_available(g,CARD_ID(c)))return;
 if(g->deck_count<MAX_DECK)g->deck[g->deck_count++]=c;}
int game_card_is_playable(int raw){
 int id=CARD_ID(raw);
 return id>=0&&id<CARD_COUNT&&!(id>=CARD_STATUS_MIN&&id<=CARD_STATUS_MAX);
}
int game_card_is_upgradeable(int raw){
 return game_card_is_playable(raw)&&!CARD_UP(raw)&&CARD_ID(raw)!=CARD_GIANT_ROCK&&CARD_ID(raw)!=R_DEBRIS;
}
static int solo_reward_eligible(int id){
 return id>=3&&id<CARD_COUNT&&game_card_is_playable(id)&&
   id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&id!=N_FLANKING&&id!=N_SNEAKY&&cards[id].rarity<=3;
}
int game_collection_unlock_count(int tab){return tab==4?10:20;}
static int collection_base_card(int tab,int id){
 if(tab<0||tab>2||id<0||id>=ASCENT_CARD_FIRST||!game_card_is_playable(id))return 0;
 if(id==63||id==85||id==N_FLANKING||id==N_SNEAKY)return 0;
 return (tab==2&&id>=COLORLESS_FIRST)||(tab==0?id<NINJA_FIRST:tab==1?id>=NINJA_FIRST&&id<NINJA_LAST:id>=REGENT_FIRST&&id<REGENT_LAST);
}
int game_collection_id(int tab,int index){
 if(tab<0||tab>=5||index<0)return -1;
 int custom=game_collection_unlock_count(tab);
 if(index<custom)return tab<3?ASCENT_CARD_FIRST+20*tab+index:tab==3?6+index:64+index;
 index-=custom;
 if(tab==3)return index<6?index:-1;
 for(int id=0;id<(tab==4?RELIC_COUNT:ASCENT_CARD_FIRST);id++){
  int available=tab==4?((id<64||id>=74)&&id!=30&&(id<37||id>39)):collection_base_card(tab,id);
  if(available&&index--==0)return id;
 }
 return -1;
}
int game_collection_count(int tab){
 if(tab<0||tab>=5)return 0;
 if(tab==3)return 26;if(tab==4)return 90;
 int count=20;for(int id=0;id<ASCENT_CARD_FIRST;id++)count+=collection_base_card(tab,id);
 return count;
}
static int reward_of_rarity(Game*g,int rarity,int excluded0,int excluded1){
 int pool[512],weight[512],n=0,total=0;
 for(int id=3;id<CARD_COUNT;id++)if(solo_reward_eligible(id)&&ninja_owned_card(g,id)&&cards[id].rarity==rarity&&id!=excluded0&&id!=excluded1){
  int owned=0;for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==id){owned=1;break;}
  /* Upgraded copies count as owned. Duplicates remain possible, at one third
     the per-card weight of an unseen card in the same rarity tier. */
  pool[n]=id;weight[n]=owned?1:3;total+=weight[n++];
 }
 if(!n&&rarity==3)return reward_of_rarity(g,2,excluded0,excluded1);
 if(!n)return g->character==CHAR_NIGHTSHADE?N_BACKFLIP:g->character==CHAR_REGENT?R_DEFEND_REGENT:5;
 int roll=rnd(g,total);for(int i=0;i<n;i++){if(roll<weight[i])return pool[i];roll-=weight[i];}
 return pool[n-1];
}
/* STS2 base bands, with a persisted -5..40 rare offset. Ancient cards are
   reserved for character story gifts and explicit mystery rewards. */
static int roll_rarity(Game*g,int rare_base,int uncommon,int use_offset,int advance){
 int rare=rare_base+(use_offset?g->revision182.rare_offset:0),roll=rnd(g,100);
 int result=roll<rare?2:roll<rare+uncommon?1:0;
 if(advance)g->revision182.rare_offset=result==2?-5:MIN(40,g->revision182.rare_offset+1);
 return result;
}
static int card_reward_except(Game*g,int a,int b){return reward_of_rarity(g,roll_rarity(g,9,37,1,0),a,b);}
static int card_reward(Game*g){return reward_of_rarity(g,roll_rarity(g,3,34,0,0),-1,-1);}
static void fill_rewards(Game*g){
 for(int i=0;i<3;i++){
  int rare=g->battle_type==NODE_BOSS?2:roll_rarity(g,g->battle_type==NODE_ELITE?10:3,g->battle_type==NODE_ELITE?40:37,1,1);
  if(g->battle_type==NODE_BOSS)g->revision182.rare_offset=-5;
  int c=reward_of_rarity(g,rare,i>0?CARD_ID(g->reward[0]):-1,i>1?CARD_ID(g->reward[1]):-1);
  int upgraded=relic(g,13)||(rare<2&&rnd(g,100)<MIN(2,g->act-1)*25);
  g->reward[i]=c|(upgraded?UPBIT:0);
 }
}
static void add_relic(Game*g,int id){if(id<0||id>=RELIC_COUNT)return;if(id==NR_DRAKE){for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)==NR_SNAKE){*relic_slot(g,i)=NR_DRAKE;return;}}if(id==30)id=7;if(id>=37&&id<=39){if(id==37)g->permanent_strength+=2;else if(id==38){g->maxhp+=10;g->hp=g->maxhp;}else if(id==39)g->gold+=150;return;}if(g->relic_count>=MAX_RELICS||relic(g,id))return;*relic_slot(g,g->relic_count++)=id;if(id==4){g->maxhp+=14;g->hp+=14;}if(id==31){g->maxhp+=12;g->hp=g->maxhp;}if(id==32)g->gold+=100;if(id==11){g->max_energy++;g->maxhp=MAX(1,g->maxhp-12);g->hp=MIN(g->hp,g->maxhp);}if(id==25)g->max_energy++;if(id==72){g->maxhp+=10;g->hp+=10;}if(id==85){g->maxhp+=8;g->hp+=8;}}
static int random_relic_except(Game*g,int excluded){
 int pool[MAX_RELICS],n=0;if(g->relic_count>=MAX_RELICS)return -1;
 for(int id=1;id<RELIC_COUNT;id++)if((id<=27||(id>=40&&id<44)||(id>=53&&id<63)||(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51)||(id>=64&&id<74&&(g->ascent.relics&(1u<<(id-64))))||(id>=74&&id<94))&&id!=excluded&&!relic(g,id))pool[n++]=id;
 return n?pool[rnd(g,n)]:-1;
}
static int random_relic(Game*g){return random_relic_except(g,-1);}
static void offer_blessing(Game*g){
 /* One dependable act boon, one general relic, one build-focused relic.
    Each slot has a broad unowned pool; repeated stat gifts are the fallback. */
 int pools[3][64],counts[3]={0};
 for(int id=28;id<40;id++)if(id!=30&&(id>=37||!relic(g,id)))pools[0][counts[0]++]=id;
 for(int id=1;id<28;id++)if(id!=11&&id!=25&&!relic(g,id))pools[1][counts[1]++]=id;
 for(int id=40;id<63;id++)if(id!=44&&(id<45||id>52||g->character==CHAR_NIGHTSHADE)&&
     (id!=52||g->act>1)&&!relic(g,id))pools[2][counts[2]++]=id;
 for(int i=0;i<3;i++)g->reward[i]=counts[i]?pools[i][rnd(g,counts[i])]:37+i;
 if(g->act>1&&rnd(g,4)==0){
  if(g->character==CHAR_REGENT)g->reward[2]=1000+(rnd(g,2)?R_METEOR_SHOWER:R_THE_SEALED_THRONE);
  else if(g->character==CHAR_NIGHTSHADE)g->reward[2]=1000+(rnd(g,2)?N_SUPPRESS:N_WRAITH_FORM);
 }
 g->screen=BLESSING;g->selection=g->scroll=0;msg(g,"Choose one blessing for your ascent.");
}
static void begin_act(Game*g){g->revision182.room_pending=0;g->revision182.rare_offset=-5;g->screen=ACT_INTRO;g->scroll=0;g->selection=0;}
int game_moon_heal_now(const Game*g){return MIN(20,g->maxhp-g->hp);}
/* Deterministic branching edges: no save struct layout changes. Edges are
   seeded by the run and act, so maps persist correctly across reloads. */
static uint32_t route_hash(uint32_t z){z^=z>>16;z*=0x7feb352du;z^=z>>15;z*=0x846ca68bu;return z^(z>>16);}
/* Two distinct starts, three optional forks, and at most three merges per act.
   Every active room has an exit. Ordered neighbors prevent crossing paths. */
static unsigned route_layer(const Game*g,int wanted,int lane){
 unsigned live=5u;
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*0x9e3779b9u);
 for(int r=0;r<=wanted;r++){
  unsigned edges[3]={1u,2u,4u};uint32_t h=route_hash(seed+(unsigned)r*9199u);
  int fork=(r==1+(int)(seed&1u)||r==6+(int)((seed>>1)&1u)||r==10+(int)((seed>>2)&1u)||r==14+(int)((seed>>3)&1u));
  int merge=(r==4||r==8||r==12||r==16);
  if(fork&&live!=7u){
   int gap=!(live&2)?1:!(live&1)?0:2;
   int from=gap==1?((h&1)?0:2):1;
   if(live&(1u<<from))edges[from]|=1u<<gap;
  }else if(merge&&live==7u){int from=(h&1)?0:2;edges[from]=2u;}
  /* Inactive lanes still have a valid exit for compatible older saves. */
  if(r==wanted)return lane<0?live:edges[lane];
  unsigned next=0;for(int c=0;c<3;c++)if(live&(1u<<c))next|=edges[c];live=next;
 }
 return live;
}
unsigned game_route_mask(const Game*g,int row,int lane){
 if(!g||row<0||row>=MAP_ROWS-1||lane<0||lane>=3)return 0;
 return route_layer(g,row,lane);
}
unsigned game_route_row_mask(const Game*g,int row){
 if(!g||row<0||row>=MAP_ROWS)return 0;
 unsigned mask=route_layer(g,row,-1);
 if(row<g->floor&&g->map_selected[row]>=0)mask|=1u<<g->map_selected[row];
 if(row==g->floor&&g->floor>0)mask|=game_route_mask(g,g->floor-1,g->lane);
 return mask;
}
unsigned game_route_visible_mask(const Game *g,int row) {
 if(!g||row<0||row>=MAP_ROWS)return 0;
 if(row==g->floor&&g->floor>0)return game_route_mask(g,g->floor-1,g->lane);
 return game_route_row_mask(g,row);
}
int game_route_available(const Game *g,int lane) {
 if(!g||g->floor>=MAP_ROWS||lane<0||lane>=MAP_LANES)return 0;
 if(g->floor==0)return (game_route_row_mask(g,0)&(1u<<lane))!=0;
 return (game_route_mask(g,g->floor-1,g->lane)&(1u<<lane))!=0;
}
static void map_select_reachable(Game*g) {
 if(g->floor>=MAP_ROWS)return;
 if(game_route_available(g,g->selection))return;
 for(int i=0;i<MAP_LANES;i++)
  if(game_route_available(g,i)){g->selection=i;return;}
}
/* Keep the planar topology, but make each fork a real room/reward choice.
   A fork's two child lanes differ within two floors, before any merge. */
static void map_distinguish_forks(Game*g){
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);
 int first=(int)(seed&1u)*2;g->map[1][first]=NODE_EVENT;g->map[1][2-first]=NODE_FIGHT;
 for(int r=0;r<MAP_ROWS-1;r++){
  unsigned live=route_layer(g,r,-1);
  for(int lane=0;lane<3;lane++)if(live&(1u<<lane)){
   unsigned edges=route_layer(g,r,lane);if((edges&(edges-1))==0)continue;
   int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
   uint32_t h=route_hash(seed+(unsigned)r*7117u);
   if(h&1u){int temp=a;a=b;b=temp;}
   int choice=r+1;if(choice==2||choice==7||choice==15)choice++;
   if(choice==3){
    g->map[3][a]=NODE_SHOP;g->map[3][b]=(h&2u)?NODE_REST:NODE_EVENT;
    /* Each lane still has exactly one shop, on floor 4 or floor 10. */
    g->map[9][a]=(h&4u)?NODE_REST:NODE_EVENT;g->map[9][b]=NODE_SHOP;
   }else if(choice==11){g->map[choice][a]=NODE_REST;g->map[choice][b]=NODE_EVENT;}
   else {g->map[choice][a]=NODE_ELITE;g->map[choice][b]=NODE_FIGHT;}
  }
 }
}
static void map_generate(Game*g){
 uint32_t seed=route_hash(g->seed^(uint32_t)g->act*9973u);
 for(int r=0;r<MAP_ROWS;r++){
  g->map_selected[r]=-1;
  for(int c=0;c<3;c++){
   uint32_t h=route_hash(seed+(unsigned)(r*131+c*977));int n=NODE_FIGHT;
   if(r==1)n=(h%3)?NODE_EVENT:NODE_FIGHT;
   else if(r==3||r==9){
    int earlyshop=(int)(route_hash(seed+c*127u)&1u);
    n=((r==3)==earlyshop)?NODE_SHOP:((h&1)?NODE_EVENT:NODE_REST);
   }else if(r==5)n=(h%3==0)?NODE_EVENT:NODE_FIGHT;
   else if(r==7)n=NODE_TREASURE;
   else if(r==11)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==13)n=(h&1)?NODE_EVENT:NODE_REST;
   else if(r==15)n=(h%3)?NODE_EVENT:NODE_SHOP;
   else if(r==17)n=NODE_REST;
   else if(r>=4&&(h%100)<(unsigned)(r>=10?38:24))n=NODE_ELITE;
   g->map[r][c]=n;
  }
 }
 map_distinguish_forks(g);
 /* Guaranteed rests by floor 6 and floor 12, then before the boss. No route
    can stack the old random rest rolls or go an entire act without recovery. */
 for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<3;c++)if(g->map[r][c]==NODE_REST)g->map[r][c]=NODE_EVENT;
 int early=4+(seed&1u),middle=10+((seed>>1)&1u);
 for(int c=0;c<3;c++){g->map[early][c]=NODE_REST;g->map[middle][c]=NODE_REST;g->map[17][c]=NODE_REST;}
 /* An optional fourth camp is a visible recovery-versus-event choice. */
 if(seed&4u){int lane=(int)((seed>>3)%3u);g->map[14][lane]=NODE_REST;}
 /* A guaranteed camp may occupy a fork's original distinguishing room.
    Preserve a real choice on the following floor before the merge. */
 for(int r=0;r<MAP_ROWS-2;r++)for(int c=0;c<3;c++){
  unsigned edges=route_layer(g,r,c);if(!(route_layer(g,r,-1)&(1u<<c))||!(edges&(edges-1)))continue;
  int a=0,b=2;while(!(edges&(1u<<a)))a++;while(!(edges&(1u<<b)))b--;
  if(g->map[r+1][a]==g->map[r+1][b]&&g->map[r+2][a]==g->map[r+2][b]&&r+2!=early&&r+2!=middle&&r+2!=17){
   g->map[r+2][a]=NODE_FIGHT;g->map[r+2][b]=NODE_ELITE;
  }
 }
}
static void shuffle(Game*g,int *p,int count){for(int i=count-1;i>0;i--){int j=rnd(g,i+1),temp=p[i];p[i]=p[j];p[j]=temp;}}
static int has_strike_tag(const char *name);
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
 raw=regent_stamp(g,ninja_stamp(g,raw));
 if(g->hand_count<MAX_HAND)g->hand[g->hand_count++]=raw;
 else if(g->discard_count<MAX_PILE)
   g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);
 else return;
 regent_created(g);
}
static void gain_energy(Game*g,int n){
 if(n>0&&(g->screen==COMBAT||g->screen==CARD_CHOICE||g->screen==NINJA_CHOICE||g->screen==REGENT_CHOICE)&&!g->ic[IC_ENERGY_LOCK])
   g->energy=MIN(99,g->energy+n);
}
static void draw_card(Game*g){
 if((g->screen==COMBAT||g->screen==CARD_CHOICE)&&(g->reward_potion&1))return;
 if(g->hand_count>=MAX_HAND)return;
 if(g->regent.shuffle_pending){g->regent.shuffle_draws=MIN(MAX_PILE,g->regent.shuffle_draws+1);return;}
 if(!g->draw_count){if(!g->discard_count)return;memcpy(g->draw,g->discard,(size_t)g->discard_count*sizeof(int));g->draw_count=g->discard_count;g->discard_count=0;shuffle(g,g->draw,g->draw_count);regent_after_shuffle(g);if(g->regent.shuffle_pending){g->regent.shuffle_draws++;return;}}
 int drawn=g->draw[--g->draw_count];
 g->hand[g->hand_count++]=drawn;
 ninja_on_draw(g,drawn);regent_on_draw(g,drawn);
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
 }
}
static void reshuffle_battle(Game*g){g->draw_count=g->deck_count;memcpy(g->draw,g->deck,(size_t)g->deck_count*sizeof(int));for(int i=0;i<g->draw_count;i++)g->draw[i]=regent_stamp(g,ninja_stamp(g,g->draw[i]));shuffle(g,g->draw,g->draw_count);g->discard_count=g->exhaust_count=g->hand_count=0;
 if(g->character==CHAR_REGENT){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(regent_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(regent_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}
 if(g->character==CHAR_NIGHTSHADE){int ordered[MAX_PILE],n=0;for(int i=0;i<g->draw_count;i++)if(!(ninja_flags(g->draw[i])&4))ordered[n++]=g->draw[i];for(int i=0;i<g->draw_count;i++)if(ninja_flags(g->draw[i])&4)ordered[n++]=g->draw[i];memcpy(g->draw,ordered,(size_t)n*sizeof(int));}
}
static void heal(Game*g,int n){g->hp=MIN(g->maxhp,g->hp+MAX(n,0));}
static void player_death(Game*g){
 if(g->hp>0)return;
 if(relic(g,23)&&g->defense_event!=g->act){g->defense_event=g->act;g->hp=MAX(1,g->maxhp/4);msg(g,"Last Ember saved your life!");}
 else{g->hp=0;g->screen=DEFEAT;}
}
#define RIFT_INFERNO_POWER(g) ((g)->used_potion & 0xffff)
#define RIFT_INFERNO_COPIES(g) ((g)->used_potion >> 16 ? (g)->used_potion >> 16 : ((g)->used_potion > 0 ? 1 : 0))
static void lose_hp(Game*g,int n){
 n=MAX(n,0);if(g->screen==COMBAT&&g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);g->hp-=n;
 if(n>0)record_combat_fx(GFX_SELF,-2,-1,n,0);
 if(n>0&&g->screen==COMBAT){
  g->ic[IC_HPLOSS_COMBAT]++;
  if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
 }
 if(n>0&&g->screen==COMBAT&&g->shop_cards[0]>0)g->strength+=g->shop_cards[0];
 if(n>0&&g->screen==COMBAT&&RIFT_INFERNO_POWER(g)>0){
   /* Trigger once per HP-loss event, not once for each point of lost HP.
      Inferno's AoE is a Power effect: it ignores Strength, Weak, Vulnerable
      and on-attack relics but respects enemy Block. */
   const int power=RIFT_INFERNO_POWER(g);
   for(int z=0;z<g->enemy_count;z++){
     Enemy *foe=&g->enemies[z];
     if(foe->hp<=0)continue;
     int blocked=MIN(foe->block,power);
     foe->block-=blocked;
     foe->hp-=power-blocked;
     record_combat_fx(GFX_PROC,-2,z,power,blocked);
   }
 }
 player_death(g);
 if(n>0)r151_self_loss(g);
 if(n>0&&g->screen==COMBAT&&g->ic[IC_OWN_TURN]&&relic(g,42)&&!g->ic[IC_NEEDLE]){
  g->ic[IC_NEEDLE]=1;draw_card(g);
 }
}
static void damage_player(Game*g,int n){
 if(n<=0)return;
 if(g->vulnerable) n=n*3/2;
 if(g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);
 int blocked=MIN(g->block,n);
 record_combat_fx(GFX_ATTACK,combat_fx_source,-1,n,blocked);
 g->block-=blocked;n-=blocked;
 if(g->regent.reflect&&blocked>0&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int b=MIN(e->block,blocked);e->block-=b;e->hp-=blocked-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,blocked,b);}
 if(g->regent.gambit&&n>0)g->hp=0;
 if(n>0){g->hp-=n;
  if(g->screen==COMBAT){
   g->ic[IC_HPLOSS_COMBAT]++;
   if(g->ic[IC_OWN_TURN])g->ic[IC_HPLOSS_TURN]++;
  }
 }
 if(relic(g,2)&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int absorbed=MIN(4,e->block);e->block-=absorbed;e->hp-=4-absorbed;record_combat_fx(GFX_PROC,-1,combat_fx_source,4,absorbed);}
 if(g->ninja.v[NS_THORNS]&&combat_fx_source>=0&&combat_fx_source<g->enemy_count){Enemy*e=&g->enemies[combat_fx_source];int d=g->ninja.v[NS_THORNS],b=MIN(e->block,d);e->block-=b;e->hp-=d-b;record_combat_fx(GFX_PROC,-1,combat_fx_source,d,b);}
 if(n>0&&g->hp>0&&relic(g,78)&&!g->expedition.combat[E181_HURT_BLOCK]){g->expedition.combat[E181_HURT_BLOCK]=1;relic_block(g,5);}
 if(g->hp>0&&(long long)g->hp*4<=g->maxhp&&relic(g,89)&&!g->expedition.combat[E181_LAST_MERCY]){g->expedition.combat[E181_LAST_MERCY]=1;heal(g,12);}
 player_death(g);
}
static int attack_amount(const Game*g,int idx,int amt){
 const Enemy *e=&g->enemies[idx];
 amt=MAX(0,amt+(r140_attack_context?g->regent.attack_vigor:0)+g->strength+(r140_attack_context&&CARD_MOD(r150_card_raw)==MOD_KEEN?2:0));
 if(g->weak)amt=amt*3/4;
 if(e->vulnerable){
  amt=amt*3/2;
  if(g->ic[IC_CRUELTY])amt=amt*(100+g->ic[IC_CRUELTY])/100;
 }
 if(r140_attack_context&&relic(g,3)&&g->ic[IC_ATTACKS_THIS_TURN]==0)amt+=5;
 if(r140_attack_context&&relic(g,26)&&g->hp*2<=g->maxhp)amt+=3;
 if(r140_attack_context&&relic(g,87)&&e->hp*2<e->maxhp)amt+=4;
 if(r140_attack_context&&e->weak)for(int i=0;i<MIN(30,g->ninja.v[NS_TRACKING])&&amt<1000000000;i++)amt=(int)MIN(1000000000LL,(long long)amt*2);
 if(r140_attack_context&&g->ninja.v[NS_SHADOW_NOW])amt=(int)MIN(1000000000LL,(long long)amt*2);
 if(r140_attack_context&&CARD_ID(r150_card_raw)==R_SOVEREIGN_BLADE&&g->regent.conqueror[idx])amt=(int)MIN(1000000000LL,(long long)amt*2);
 return amt;
}
static void damage_enemy(Game*g,int idx,int amt,int ignore_block){
 if(idx<0||idx>=g->enemy_count||g->enemies[idx].hp<=0)return;
 Enemy *e=&g->enemies[idx];
 amt=attack_amount(g,idx,amt);
 preview_hit(idx,amt);
 record_combat_fx(GFX_ATTACK,-1,idx,amt,ignore_block?0:MIN(e->block,amt));
 if(!ignore_block){int b=MIN(e->block,amt);e->block-=b;amt-=b;}
 e->hp-=amt;
 ninja_on_hit(g,idx,amt);regent_on_hit(g,idx,amt);
 if(r140_attack_context&&relic(g,77)&&!g->expedition.combat[E181_WEAK_ATTACK]){g->expedition.combat[E181_WEAK_ATTACK]=1;e->weak=MIN(99,e->weak+1);}
 if(r140_attack_context&&relic(g,9)&&g->ic[IC_ATTACKS_THIS_TURN]==0&&!(r140_rust_targets&(1<<idx))){e->weak=MIN(99,e->weak+2);r140_rust_targets|=1<<idx;}

}
static void deal(Game*g,int idx,int n){damage_enemy(g,idx,n,0);}
static void apply_vulnerable(Game*g,Enemy*e,int n){
 if(!e||n<=0||e->hp<=0)return;
 e->vulnerable=MIN(99,e->vulnerable+n);
 if(g->ic[IC_VICIOUS]>0){
  int count=MIN(6,g->ic[IC_VICIOUS]);
  for(int i=0;i<count;i++)draw_card(g);
 }
}
static void proc_juggernaut(Game*g){
 if(g->ic[IC_JUGGERNAUT]<=0||g->screen!=COMBAT||g->ic[IC_GUARD])return;
 int foe=random_living_foe(g);
 if(foe<0)return;
 g->ic[IC_GUARD]=1;
 Enemy *e=&g->enemies[foe];
 int d=g->ic[IC_JUGGERNAUT],blocked=MIN(e->block,d);
 e->block-=blocked;e->hp-=d-blocked;
 record_combat_fx(GFX_PROC,-2,foe,d,blocked);
 g->ic[IC_GUARD]=0;
}
static void gain_block(Game*g,int n){
 int amount=MAX(0,n+g->dexterity+(relic(g,15)?2:0));
 if(amount>0&&g->ic[IC_UNMOVABLE]&&!g->ic[IC_UNMOVABLE_USED]){
  amount*=2;g->ic[IC_UNMOVABLE_USED]=1;
 }
 for(int i=0;i<MIN(30,g->ninja.v[NS_SHADOWMELD])&&amount<1000000000;i++)amount=(int)MIN(1000000000LL,(long long)amount*2);
 int gained=MIN(amount,999-g->block);
 if(gained>0)record_combat_fx(GFX_BLOCK,-1,-1,gained,0);
 if(preview_trace&&preview_phase){
  CardPreview*p=preview_trace;
  if(preview_nested)p->uncertain=1;
  if(!p->block_count||gained<p->block_low)p->block_low=gained;
  if(!p->block_count||gained>p->block_high)p->block_high=gained;
  if(!p->block_count)p->block_first=gained;
  if(p->block_count==1)p->block_second=gained;
  p->block_count++;p->block_total+=gained;
 }
 if(amount<=0)return;
 g->block=MIN(999,g->block+amount);
 proc_juggernaut(g);
}
static void card_block(Game*g,int n){if(g->regent.panic)return;gain_block(g,n+(CARD_MOD(r150_card_raw)==MOD_WARD?4:0));}
static void relic_block(Game*g,int n){
 for(int i=0;i<MIN(30,g->ninja.v[NS_SHADOWMELD])&&n<1000000000;i++)n=(int)MIN(1000000000LL,(long long)n*2);
 int amount=MIN(n,999-g->block);if(amount<=0)return;
 g->block+=amount;record_combat_fx(GFX_BLOCK,-1,-1,amount,0);proc_juggernaut(g);
}
static int random_relic(Game*g);
static void check_win(Game*g){
 if(g->screen!=COMBAT||g->ninja.task_count>0||g->regent.task_count||regent_processing)return;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)return;
 g->ic[37]=0;g->v151.card_done=0;g->v151.potion_offer=-1;
 if(relic(g,57))heal(g,2*MIN(3,g->v151.relic[4]));
 g->gold+=g->regent.powers[R_ROYALTIES-REGENT_FIRST];g->kill_count+=g->enemy_count;
 if(relic(g,81)&&(g->battle_type==NODE_ELITE||g->battle_type==NODE_BOSS))g->gold+=35;
 if(relic(g,84)&&g->expedition.combat[E181_POTION_USED])heal(g,4);
 if(relic(g,85))heal(g,1);
 if(g->battle_type==NODE_BOSS)ascent_record_act(g);
 if(g->battle_type==NODE_BOSS&&g->act==3&&!g->ascent.won){
  if(!ascent_complete(g,0)){g->screen=ASCENT_SAVE_ERROR;g->selection=0;return;}
  g->screen=ASCENT_ENDING;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }
 if(g->battle_type==NODE_BOSS&&g->act==4&&g->ascent.secret){
  if(!ascent_complete(g,1)){g->screen=ASCENT_SAVE_ERROR;g->selection=1;return;}
  g->screen=ASCENT_TRUE_END;g->selection=0;g->win_screen=0;g->relic_pending=0;return;
 }
 if(rnd(g,100)<22)g->v151.potion_offer=ascent_random_potion(g);
 g->gold+=g->reward_gold+(relic(g,16)?20:0)+(relic(g,67)?8:0);
 if(relic(g,68))heal(g,3);
 if(relic(g,0))heal(g,6);if(relic(g,36))heal(g,3);
 if(g->battle_type==NODE_ELITE&&relic(g,12))g->permanent_strength++;
 if(g->battle_type==NODE_BOSS&&g->act==7){g->screen=ABOMINATION_AFTER;g->selection=0;g->win_screen=0;return;}
 if(g->battle_type==NODE_BOSS){
  int gift=expedition_relic(g);
  g->relic_pending=1;g->relic_resume=REWARD;
  g->reward[0]=gift;
  g->win_screen=(!g->endless&&g->act>=3?2:1);
  g->selection=0;g->screen=RELIC_RESULT;
  msg(g,""); /* The relic screen owns all boss-reward text. */
  return;
 }
 g->screen=REWARD;g->selection=0;
 if(g->battle_type==NODE_ELITE){
  int gift=expedition_relic(g);
  if(gift>=0){g->reward[0]=gift;g->relic_pending=1;
   g->relic_resume=REWARD;g->screen=RELIC_RESULT;g->selection=0;
   msg(g,"Elite defeated.");
  }else msg(g,"Elite defeated. No relics remain.");
 }else{fill_rewards(g);msg(g,"Victory! Choose one reward.");}
}
/* Thirty-six deterministic, species-specific enemy profiles, balanced by act. */
typedef struct {const char *name;short hp,attack,tier; unsigned char moves[4];} EnemyDef;
static const EnemyDef enemy_defs[ENEMY_COUNT]={
 {"Ash Crawler",30,8,0,{0,0,1,6}},
 {"Bone Sentinel",35,8,0,{0,1,7,0}},
 {"Mist Gazer",32,9,0,{0,2,0,9}},
 {"Iron Wight",38,9,0,{0,3,1,0}},
 {"Grave Knight",53,11,0,{0,1,10,0}},
 {"Brine Witch",50,12,0,{6,0,2,1}},
 {"Stone Giant",87,13,1,{1,10,21,4}},
 {"Fallen Warden",155,18,1,{1,18,10,22}},
 {"THE HOLLOW KING",300,23,2,{4,10,22,20}},
 {"Cinder Hound",33,9,0,{7,0,6,0}},
 {"Crypt Archer",37,9,0,{0,3,0,1}},
 {"Glimmer Moth",29,8,0,{8,2,0,9}},
 {"Oathbreaker",42,10,0,{4,0,0,10}},
 {"Ember Seer",78,12,1,{5,19,9,18}},
 {"Bone Captain",85,11,1,{20,7,18,3}},
 {"THE FURNACE MAW",168,15,2,{1,19,10,4}},
 {"Rust Reaver",56,12,0,{0,10,1,0}},
 {"Sable Lurker",52,12,0,{7,8,0,2}},
 {"Mire Oracle",50,11,0,{6,9,5,0}},
 {"Wound Collector",62,12,0,{12,0,3,4}},
 {"Chain Keeper",66,14,0,{8,0,1,7}},
 {"Drowned Penitent",53,12,0,{13,2,0,9}},
 {"Ossuary Beast",126,15,1,{1,7,23,10}},
 {"Storm Golem",143,16,1,{13,16,18,21}},
 {"THE CRIMSON JUDGE",240,19,2,{3,18,11,21}},
 {"Void Disciple",82,17,0,{5,8,0,1}},
 {"Bone Harvester",92,17,0,{7,0,12,3}},
 {"Abyss Watcher",78,16,0,{6,9,2,10}},
 {"Rift Channeler",79,16,0,{11,5,9,0}},
 {"Pale Executioner",103,18,0,{10,0,3,1}},
 {"Crownless Saint",91,16,0,{13,12,3,0}},
 {"Entropy Knight",102,19,0,{0,4,7,10}},
 {"Twilight Shade",88,18,0,{8,6,2,7}},
 {"Infernal Colossus",173,19,1,{4,19,10,21}},
 {"Spectral Marshal",163,19,1,{20,3,7,22}},
 {"Rift Behemoth",185,20,1,{1,10,23,7}},
 {"Gallows Stalker",34,8,0,{0,15,1,0}},
 {"Candle Wretch",32,8,0,{16,0,2,1}},
 {"Sepulcher Guard",62,12,0,{15,1,0,17}},
 {"Thorn Penitent",55,12,0,{16,2,0,4}},
 {"Veil Duelist",88,17,0,{14,0,16,1}},
 {"Tomb Cantor",94,16,0,{17,0,6,14}},
 {"Oathless Lancer",90,12,1,{14,15,18,16}},
 {"Cathedral Horror",148,17,1,{17,23,16,21}},
 {"THE UNMAKER",340,22,2,{1,11,10,4}},
 {"THE UNBEGOTTEN",610,31,2,{13,16,11,10}},
 {"Crown Eater",174,19,1,{22,7,21,18}},
 {"Cinder Sexton",37,9,0,{1,0,5,14}},
 {"Bellback Vermin",33,8,0,{0,14,1,3}},
 {"Waxbound Acolyte",39,9,0,{1,5,0,2}},
 {"Pyre Castellan",86,12,1,{1,19,21,18}},
 {"Saltglass Eel",57,13,0,{0,14,2,7}},
 {"Silt Reliquary",65,12,0,{1,0,3,14}},
 {"Rime Cantor",53,12,0,{2,0,11,14}},
 {"Abyssal Sacristan",145,17,1,{1,23,21,16}},
 {"Halo Leech",85,17,0,{1,12,0,14}},
 {"Starved Herald",91,17,0,{2,0,10,14}},
 {"Throne Husk",99,18,0,{1,0,14,3}},
 {"Eclipse Justiciar",182,20,1,{13,18,22,16}},
};
/* Normal / elite / boss pools are separate so bosses never show up as ordinary foes. */
static const unsigned char enemy_normal[3][13]={{0,1,2,3,9,10,11,12,36,37,47,48,49},{4,5,16,17,18,19,20,21,38,39,51,52,53},{25,26,27,28,29,30,31,32,40,41,55,56,57}};
static const unsigned char enemy_elite[3][5]={{6,13,14,42,50},{7,22,23,43,54},{33,34,35,46,58}};
static const unsigned char enemy_boss[3]={15,24,8};
static void enemy_intent(Game*g,Enemy *e){
 const EnemyDef*d=&enemy_defs[e->art];
 if(e->art==45){
  int phase=e->hp<=e->maxhp/3?2:e->hp<=e->maxhp*2/3?1:0;
  if(phase>e->phase){e->strength+=5*(phase-e->phase);e->phase=phase;e->weak=e->vulnerable=0;e->poison/=2;e->block+=18*phase;}
 }else if(d->tier==2 && e->phase==0 && e->hp<=e->maxhp/2){e->phase=1;e->strength+=3;}
 unsigned turn=(unsigned)g->turn;
 int slot=(int)(turn%4u);
 if(e->phase&&d->tier==2)slot=(slot+1)%4;
 e->intent=d->moves[slot];
 if(e->art==45){static const unsigned char moves[3][4]={{13,16,11,10},{7,13,10,16},{10,7,13,16}};e->intent=moves[CLAMP(e->phase,0,2)][slot];}
 /* Most heavy openings are deferred; a deterministic one-in-eight exception
    keeps some danger while always showing the actual intended move. */
 if(g->turn==0&&(e->intent==4||e->intent==7||e->intent==10||e->intent==11)&&
    (route_hash(g->seed^(unsigned)e->art*101u^(unsigned)g->floor*37u)&7u))e->intent=0;
 long long percent=g->act==1?82:g->act==2?99:124LL+24LL*(g->act-3)+2LL*(g->act-3)*(g->act-3);
 if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=8;
 e->intent_value=(int)CLAMP((d->attack*percent+50)/100+g->floor/7+(e->phase?2:0),1,100000000);
}
static const char *intent_titles[INTENT_COUNT]={
 "STRIKE","GUARD","WEAKEN","EXPOSE","EMPOWER","HEX","VENOM",
 "DOUBLE","DRAIN","MEND","CRUSH","DREAD","LEECH","FORTIFY","PIERCE","SUNDER","FLURRY","ENTOMB","FRACTURE","SEAR","RALLY","PURGE","GLOOM","BLIGHT"
};
static const char *intent_details[INTENT_COUNT]={
 "One direct hit. Block reduces its damage.",
 "Raises enemy block. Enemy block expires next turn.",
 "Small hit and 2 Weak: your attacks deal 25% less.",
 "Full hit and 2 Vulnerable: you take 50% more.",
 "Raises enemy Strength by 2 and gains 8 Block.",
 "Half-damage hit, then adds a Burn to discard.",
 "Half-damage hit and adds 2 Poison to you.",
 "Two separate attacks, each around half the shown value.",
 "Half-damage hit; lose 1 energy next turn.",
 "Heals enemy health and grants some Block.",
 "Heavy attack and adds an unplayable Daze.",
 "Adds a Dread curse and gains Block.",
 "Half-damage hit and heals for HP actually lost.",
 "Raises enemy Block and Strength by 1.",
 "A light attack that ignores your Block. Weak still reduces it.",
 "Removes 4 of your Block, then makes a light attack.",
 "Three small attacks. Each hit is reduced separately by Block.",
 "Adds one unplayable Injury to discard for this combat and gains Block.",
 "A half-strength hit; applies 2 Weak and 2 Vulnerable.",
 "Two small attacks, then adds a Burn to your discard pile.",
 "All living enemies gain 1 Strength and 8 Block.",
 "Removes enemy Weak and Vulnerable, halves Poison, then gains Block.",
 "Applies 2 Weak and reduces your next turn Energy by 1. No direct hit.",
 "Adds 2 Poison and one Daze to your discard pile. No direct hit."
};
const char *game_intent_title(int n){return n>=0&&n<INTENT_COUNT?intent_titles[n]:"UNKNOWN";}
const char *game_intent_description(int n){return n>=0&&n<INTENT_COUNT?intent_details[n]:"No information";}
static const char *potion_titles[6]={
 "Healing","Strength","Guard","Fire","Energy","Insight"
};
static const char *potion_details[6]={
 "Recover 18 HP; cannot exceed maximum health.",
 "Gain 2 Strength this combat. Buffs each attack hit.",
 "Gain 20 Block, before any relic bonuses.",
 "Deal 20 damage to all living enemies.",
 "Gain 2 Energy immediately.",
 "Draw 3 cards from your draw/discard pile."
};
const char *game_potion_name(int id){return id>=6&&id<26?new_potion_names[id-6]:id>=0&&id<6?potion_titles[id]:"None";}
const char *game_potion_description(int id){return id>=6&&id<26?new_potion_rules[id-6]:id>=0&&id<6?potion_details[id]:"Empty slot";}
const char *game_potion_description_now(const Game*g,int id){
 static const char*enhanced[]={"Heal 23 HP (Violet Orb).","Gain 3 Strength this combat (Violet Orb).","Gain 25 Block (Violet Orb).","Deal 25 base damage to all enemies (Violet Orb).","Gain 3 Energy immediately (Violet Orb).","Draw 4 cards (Violet Orb)."};
 return g&&relic(g,17)&&id>=0&&id<6?enhanced[id]:game_potion_description(id);
}
static int strike_count(const Game*g);
static int card_base_damage(const Game *g,int raw,int target) {
 int id=CARD_ID(raw),up=CARD_UP(raw),base=0;
 if(id<0||id>=CARD_COUNT)return 0;
 if(id>=ASCENT_CARD_FIRST){int hits=1;
  switch(id){
   case 334:base=8+3*up;break;case 336:base=7+3*up;break;
   case 338:base=6+3*up+(g->ic[IC_HPLOSS_TURN]?4:0);break;
   case 339:base=10+4*up;break;case 341:base=9+4*up+2*MIN(100,g->ic[IC_HPLOSS_TURN]);break;
   case 343:base=12+4*up;hits=2;break;case 346:base=24+7*up;break;
   case 348:base=4+up;hits=5+up;break;case 350:base=27+8*up;break;case 352:base=45+13*up;break;
   case 354:base=6+3*up;break;case 357:base=8+3*up;break;case 360:base=11+5*up;break;
   case 363:base=5+2*up;hits=3;break;case 365:base=12+5*up;break;
   case 369:base=18+7*up+(target>=0&&target<g->enemy_count?g->enemies[target].poison:0);break;
   case 372:base=8+3*up;hits=5;break;case 374:base=7+3*up;break;case 376:base=9+3*up;break;
   case 379:base=5+3*up;break;case 381:base=12+5*up;break;case 384:base=8+3*up;hits=2;break;
   case 387:base=18+7*up+MIN(30+10*up,g->regent.forge);break;case 389:base=22+8*up;break;
   case 392:base=25+10*up+MIN(10,g->regent.stars)*(3+up);break;default:return 0;
  }
  return (base+(CARD_MOD(raw)==MOD_KEEN?2:0))*hits;
 }
 if(id>=REGENT_FIRST)return regent_base_damage(g,raw);
 if(id>=NINJA_FIRST&&id<NINJA_LAST)return ninja_base_damage(g,raw,1);
 switch(id){
 case 0:base=6+3*up;break;case 2:base=8+3*up;break;
 case 3:base=12+3*up;break;case 7:base=32+6*up;break;
 case 10:base=5+3*up;break;
 case 11:base=9+up;break;case 12:base=5+3*up;break;
 case 13:base=6+(3+up)*g->exhaust_count;break;case 14:base=9+4*up;break;
 case 15:base=18+6*up;break;case 19:base=4+3*up;break;
 case 21:base=g->block;break;
 case 22:base=3;break;case 25:base=9+3*up;break;case 26:base=6+3*up;break;case 37:base=g->exhaust_count>=3?18+6*up:0;break;case 40:base=10+4*up;break;
 case 27:base=5+3*up;break;case 29:base=15+5*up;break;
 case 32:base=8+2*up;break;case 35:base=4+(2+up)*(target>=0&&target<g->enemy_count?g->enemies[target].vulnerable:0);break;
 case 36:base=2;break;case 39:base=13+3*up;break;
 case 41:base=0;break;case 42:base=(7+3*up)*MAX(0,g->hand_count-1);break;
 case 44:base=6+3*up;break;case 47:base=10+2*up;break;
 case 49:base=6+(2+up)*strike_count(g);break;
 case 56:base=20+10*up;break;
 case 66:base=5+up;break;
 case 70:base=16+5*up;break;
 case 74:base=15+5*up;break;
 case 79:base=9+g->ic[IC_RAMPAGE];break;
 case 80:base=7+2*up;break;
 case 81:base=5;break;
 case 86:base=5+2*up;break;
 case 87:base=4+2*up;break;
 case 89:base=14+6*up;break;
 case 91:base=16+4*up;break;
 case 92:base=14+5*up;break;case 94:base=8+3*up;break;
 default:return 0;
 }
 if(CARD_MOD(raw)==MOD_KEEN)base+=id==42?2*MAX(0,g->hand_count-1):2;
 int dmg=base;
 if(id==27||id==22||id==36||id==32||id==66||id==81||id==86||id==87){int hits=id==27?2:id==22?3+up:id==36?4+up:id==66||id==87?2:id==81?(g->ic[IC_HPLOSS_TURN]?(2+up):1):id==86?MIN(20,1+g->ic[IC_HPLOSS_COMBAT]):(target>=0&&target<g->enemy_count&&g->enemies[target].vulnerable?2:1);dmg*=hits;}
 if(id==10)dmg*=g->energy;
 return dmg;
}
int game_effective_damage(const Game*g,int raw,int target){
 CardPreview p;game_card_preview(g,raw,target,&p);
 return target>=0&&target<g->enemy_count?p.total[target]:0;
}
static void init_enemy(Game*g,Enemy*e,int id){
 const EnemyDef*d=&enemy_defs[id];
 memset(e,0,sizeof(*e));e->art=id;
 snprintf(e->name,sizeof(e->name),"%s",d->name);
 long long percent=g->act==1?94:g->act==2?132:188LL+50LL*(g->act-3);
 if(g->act==1&&g->battle_type==NODE_FIGHT)percent+=10;
 if(d->tier==2)percent=g->act>=3?182LL+44LL*(g->act-3):84+g->act*24;else percent+=g->floor+MAX(0,g->floor-4)/2;
 if(g->act>3)percent+=5LL*(g->act-3)*(g->act-3);
 long long hp=(d->hp*percent+50)/100;
 if(g->enemy_count==2)hp=(hp*72+50)/100;
 e->maxhp=(int)MIN(1000000000LL,hp);
 if(e->maxhp<1)e->maxhp=1;
 e->hp=e->maxhp;
 enemy_intent(g,e);
}
const char *game_enemy_name(int id){return id>=0&&id<ENEMY_COUNT?enemy_defs[id].name:"Unknown";}
void game_start_battle(Game*g,int mode){
 g->screen=COMBAT;g->battle_type=mode;
 g->revision182.room_type=mode;g->revision182.room_pending=mode!=NODE_BOSS;
 /* Music selection uses a separate seed hash, preserving gameplay RNG. */
 unsigned music_hash=route_hash(g->seed^(unsigned)g->act*17041u^(unsigned)g->floor*6781u);
 g->combat_music=mode==NODE_ELITE?(music_hash&1?3:6):(music_hash%3==0?2:music_hash%3==1?4:5);
 g->turn=0;
 g->enemy_count=mode==NODE_FIGHT&&route_hash(g->seed^(unsigned)g->act*7919u^(unsigned)g->floor*101u)%4u==0?2:1;
 for(int i=0;i<g->enemy_count;i++){
  int act=MIN(2,g->act-1);
  int id=mode==NODE_BOSS?(g->act==7?45:g->act==4&&g->ascent.secret?44:enemy_boss[g->endless&&g->act>3?rnd(g,3):act]):
    mode==NODE_ELITE?enemy_elite[g->act>=4?rnd(g,3):act][rnd(g,5)]:
    g->act>=4?enemy_normal[rnd(g,3)][rnd(g,13)]:enemy_normal[act][rnd(g,13)];
  if(i>0&&id==g->enemies[0].art){
   int pool[39],n=0;for(int a=0;a<(g->act>=4?3:1);a++)for(int k=0;k<13;k++){
    int candidate=enemy_normal[g->act>=4?a:act][k];if(candidate!=id)pool[n++]=candidate;
   }id=pool[rnd(g,n)];
  }
  init_enemy(g,&g->enemies[i],id);
 }
 memset(g->ic,0,sizeof(g->ic));
 memset(g->expedition.combat,0,sizeof(g->expedition.combat));
 memset(&g->ninja,0,sizeof(g->ninja));memset(&g->regent,0,sizeof(g->regent));
 {int ledger=g->v151.relic[7],seen=g->v151.relic[R162_EVENT_SEEN],last=g->v151.relic[R162_EVENT_LAST];memset(&g->v151,0,sizeof(g->v151));g->v151.relic[7]=ledger;g->v151.relic[R162_EVENT_SEEN]=seen;g->v151.relic[R162_EVENT_LAST]=last;g->v151.potion_offer=-1;}
 g->reward_potion=0;g->shop_relic=0;g->shop_cards[0]=g->shop_cards[1]=g->shop_cards[2]=0;g->turn=0;g->turn_played=0;g->heal_after_battle=0;g->used_potion=0;g->selection=0;g->target=0;g->strength=g->permanent_strength+(relic(g,28)?1:0);g->low_hp_triggered=0;g->dexterity=relic(g,35)?1:0;
 if(relic(g,61))for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])>=50&&CARD_ID(g->deck[i])<=53)g->strength++;
 g->weak=g->vulnerable=g->poison=g->block=0;
 g->reward_gold=(mode==NODE_BOSS?95:mode==NODE_ELITE?48:18)+rnd(g,12);
 if(relic(g,70))g->strength++;if(relic(g,71))g->dexterity++;
 if(relic(g,65))for(int i=0;i<g->enemy_count;i++)g->enemies[i].poison+=2;
 if(relic(g,75))for(int i=0;i<g->enemy_count;i++)g->enemies[i].weak=2;
 if(relic(g,19))for(int i=0;i<g->enemy_count;i++)g->enemies[i].vulnerable=2;
 reshuffle_battle(g);msg(g,"The enemy is preparing an attack.");game_start_turn(g);
 if(relic(g,64))g->block+=6;if(relic(g,66))regent_gain_stars(g,3);
 if(relic(g,10))g->block+=12;if(relic(g,29))g->block+=10;
 int *boon=g->revision182.next_combat;int extra=boon[2],any=0;for(int i=0;i<5;i++)any|=boon[i];
 g->strength+=boon[0];g->block=MIN(999,g->block+boon[1]);g->weak+=boon[3];g->vulnerable+=boon[4];
 memset(boon,0,sizeof(g->revision182.next_combat));for(int i=0;i<extra;i++)draw_card(g);
 if(any)msg(g,"Your mystery bargains take effect. Inspect STATUS for details.");
}
static void check_win(Game*g);
void game_start_turn(Game*g){
 g->expedition.combat[E181_POWER_ENERGY]=g->expedition.combat[E181_WEAK_ATTACK]=g->expedition.combat[E181_HURT_BLOCK]=g->expedition.combat[E181_EXHAUST_HEAL]=0;
 if(g->screen!=COMBAT)return;
 g->turn++;g->v151.relic[0]=g->v151.relic[5]=0;
 ninja_start_turn_before(g);regent_before_turn(g);
 g->ic[IC_OWN_TURN]=1;
 g->ic[IC_HPLOSS_TURN]=0;
 g->ic[IC_ATTACKS_THIS_TURN]=0;g->ic[36]=0;
 g->ic[IC_BELL]=g->ic[IC_NEEDLE]=g->ic[IC_DUELIST]=0;
 g->ic[IC_UNMOVABLE_USED]=0;
 g->ic[IC_ENERGY_LOCK]=0;
 if(g->ic[IC_PLATING]>0)g->ic[IC_PLATING]--;
 if(g->heal_after_battle>0)g->strength+=g->heal_after_battle;
 if(RIFT_INFERNO_POWER(g)>0){
    /* Each copy loses 1 HP at turn start and triggers every copy's AoE. */
    const int copies=RIFT_INFERNO_COPIES(g);
    for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
    if(g->screen==COMBAT)check_win(g);
 }
 if(g->screen!=COMBAT)return;
 if(g->ic[IC_CRIMSON_COPIES]>0){
  int copies=g->ic[IC_CRIMSON_COPIES];
  for(int i=0;i<copies&&g->screen==COMBAT;i++)lose_hp(g,1);
  if(g->screen==COMBAT&&g->ic[IC_CRIMSON]>0){
   int old_block=g->block;g->block=MIN(999,g->block+g->ic[IC_CRIMSON]);
   record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);
   proc_juggernaut(g);
  }
 }
 if(g->screen!=COMBAT)return;
 g->energy=MAX(0,g->max_energy+(g->turn==1?(relic(g,1)+relic(g,33)):0)-g->energy_penalty+g->ic[IC_PYRE]+g->ic[IC_HOURGLASS]);g->energy_penalty=0;g->ic[IC_HOURGLASS]=0;gain_energy(g,g->v151.relic[1]);g->v151.relic[1]=0;
 if(relic(g,73)&&g->turn%3==0)gain_energy(g,1);
 if(relic(g,20)&&g->turn%3==0)heal(g,4);if(relic(g,34)&&g->turn%3==0)g->strength++;
 g->turn_played=0;g->reward_potion=0;g->shop_relic=0;g->shop_cards[1]=0;
 if(relic(g,14)&&!g->low_hp_triggered&&g->hp*2<=g->maxhp){g->strength+=3;g->low_hp_triggered=1;}
 int n=5+(g->turn==1&&relic(g,69)?1:0)+(relic(g,18)?1:0)+(g->turn==1?2*relic(g,7)+2*relic(g,NR_SNAKE):0)+(g->turn<=3?2*relic(g,NR_DRAKE):0)-(relic(g,25)?1:0)+g->ninja.v[NS_NEXT_DRAW];g->ninja.v[NS_NEXT_DRAW]=0;
 for(int i=0;i<n;i++)draw_card(g);
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
 g->selection=0;
 ninja_start_turn_after(g);regent_start_turn(g);
 if(relic(g,76)&&g->turn==2)gain_energy(g,1);
 if(relic(g,79)&&g->turn%4==0)g->strength+=2;
 if(relic(g,86)&&g->draw_count<=3)relic_block(g,6);
 if(g->expedition.combat[E181_NEXT_BLOCK]){relic_block(g,8);g->expedition.combat[E181_NEXT_BLOCK]=0;}
 if(relic(g,90)&&g->turn==3)ninja_poison_all(g,4);
 if(relic(g,91)&&g->turn==1){int powers=0;for(int i=0;i<g->deck_count;i++)powers+=cards[CARD_ID(g->deck[i])].type==2;if(powers>=3)draw_card(g);else relic_block(g,4);}
}
static int enemy_raw_damage(const Enemy *e){
 int v=e->intent_value;
 switch(e->intent){
 case 0:return MAX(0,v+e->strength);
 case 2:return MAX(0,v/2+e->strength);
 case 3:return MAX(0,v+e->strength);
 case 5:case 6:case 8:case 18:return MAX(0,v/2+e->strength);
 case 7:return MAX(0,v/2+e->strength);
 case 10:return MAX(0,v+v/4+e->strength);
 case 12:case 14:case 15:return MAX(0,v/2+e->strength);
 case 16:case 19:return MAX(0,v/3+e->strength);
 default:return 0;
 }
}
static int enemy_attack_value(const Enemy *e){
 int n=enemy_raw_damage(e);
 return e->weak?n*3/4:n;
}
int game_enemy_intent_hits(const Game *g,int index){
 if(!g||index<0||index>=g->enemy_count)return 0;
 const Enemy *e=&g->enemies[index];
 return e->hp>0&&enemy_raw_damage(e)>0?(e->intent==7||e->intent==19?2:e->intent==16?3:1):0;
}
int game_enemy_intent_damage(const Game *g,int index){
 int hits=game_enemy_intent_hits(g,index);
 if(!hits)return 0;
 int n=enemy_attack_value(&g->enemies[index]);
 if(g->enemies[index].weak&&relic(g,NR_KRANE))n=enemy_raw_damage(&g->enemies[index])*3/5;
 if(g->ic[IC_COLOSSUS]&&g->enemies[index].vulnerable)n=(n+1)/2;
 if(g->ic[IC_TANK])n*=2;
 if(g->vulnerable)n=n*3/2;
 if(g->ninja.v[NS_INTANGIBLE])n=MIN(n,1);
 return n*hits; /* Total before block. */
}
void game_enemy_intent_text(const Game*g,int index,char*out,int cap){
 if(!out||cap<=0)return;out[0]=0;
 if(!g||index<0||index>=g->enemy_count)return;
 const Enemy*e=&g->enemies[index];
 if(e->hp<=0){snprintf(out,cap,"Defeated");return;}
 int total=game_enemy_intent_damage(g,index),hits=game_enemy_intent_hits(g,index);
 switch(e->intent){
 case 0:snprintf(out,cap,"Deal %d DMG",total);break;
 case 1:snprintf(out,cap,"Gain %d Block",e->intent_value);break;
 case 2:snprintf(out,cap,"Deal %d DMG; apply 2 Weak",total);break;
 case 3:snprintf(out,cap,"Deal %d DMG; apply 2 Vulnerable",total);break;
 case 4:snprintf(out,cap,"Gain 2 Strength and 8 Block");break;
 case 5:snprintf(out,cap,"Deal %d DMG; add Burn to discard",total);break;
 case 6:snprintf(out,cap,"Deal %d DMG; apply 2 Poison",total);break;
 case 7:snprintf(out,cap,"Deal %d DMG twice",hits?total/hits:0);break;
 case 8:snprintf(out,cap,"Deal %d DMG; drain 1 Energy next turn",total);break;
 case 9:snprintf(out,cap,"Heal %d HP; gain %d Block",MIN(e->maxhp-e->hp,e->maxhp/9+3),e->intent_value/2);break;
 case 10:snprintf(out,cap,"Deal %d DMG; add Daze to discard",total);break;
 case 11:snprintf(out,cap,"Add Dread to discard; gain %d Block",e->intent_value/2);break;
 case 12:snprintf(out,cap,"Deal %d DMG; heal for HP lost",total);break;
 case 13:snprintf(out,cap,"Gain %d Block and 1 Strength",e->intent_value);break;
 case 14:snprintf(out,cap,"Deal %d DMG, ignoring Block",total);break;
 case 15:snprintf(out,cap,"Remove 4 Block, then deal %d DMG",total);break;
 case 16:snprintf(out,cap,"Deal %d DMG three times",hits?total/hits:0);break;
 case 17:snprintf(out,cap,"Add Injury to discard; gain %d Block",e->intent_value/2);break;
 case 18:snprintf(out,cap,"Deal %d DMG; apply 2 Weak and Vulnerable",total);break;
 case 19:snprintf(out,cap,"Deal %d DMG twice; add Burn",hits?total/hits:0);break;
 case 20:snprintf(out,cap,"All foes gain 1 Strength and 8 Block");break;
 case 21:snprintf(out,cap,"Cleanse Weak/Vulnerable, halve Poison; %d Block",e->intent_value/2);break;
 case 22:snprintf(out,cap,"Apply 2 Weak; drain 1 Energy next turn");break;
 case 23:snprintf(out,cap,"Apply 2 Poison; add Daze to discard");break;
 }
}
static void enemy_hit(Game*g,Enemy*e,int raw){
 if(e->hp<=0)return;
 if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;
 if(g->ic[IC_TANK])raw*=2;
 if(e->weak&&relic(g,NR_KRANE)){int full=enemy_raw_damage(e);raw=full*3/5;if(g->ic[IC_COLOSSUS]&&e->vulnerable)raw=(raw+1)/2;if(g->ic[IC_TANK])raw*=2;}
 int old_source=combat_fx_source;combat_fx_source=(int)(e-g->enemies);
 damage_player(g,raw);combat_fx_source=old_source;
 if(g->shop_cards[1]>0){
   int n=g->shop_cards[1],blocked=MIN(e->block,n);
   e->block-=blocked;e->hp-=n-blocked;
   record_combat_fx(GFX_PROC,-2,(int)(e-g->enemies),n,blocked);
 }
}
static void all_damage(Game*g,int n);
void game_prepare_end_turn(Game*g){
 if(g->screen!=COMBAT||g->ic[38]||g->ninja.task_count||g->regent.task_count)return;
 if(ninja_prepare_retain(g))return;
 regent_end_turn(g);if(g->screen!=COMBAT)return;
 g->ic[38]=1;
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
 if(g->ic[IC_COVENANT]>0){
  int damage=g->ic[IC_COVENANT]*CLAMP(g->energy,0,2);
  for(int e=0;e<g->enemy_count;e++)if(g->enemies[e].hp>0&&damage>0){
   int absorbed=MIN(g->enemies[e].block,damage);g->enemies[e].block-=absorbed;g->enemies[e].hp-=damage-absorbed;
   record_combat_fx(GFX_PROC,-1,e,damage,absorbed);
  }
 }
 for(int i=0;i<g->exhaust_count;i++)if(CARD_ID(g->exhaust[i])==70)
   all_damage(g,16+(CARD_UP(g->exhaust[i])?5:0)+(CARD_MOD(g->exhaust[i])==MOD_KEEN?2:0));
 if(g->ic[IC_PLATING]>0){
  int old_block=g->block;g->block=MIN(999,g->block+g->ic[IC_PLATING]);
  record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);
  proc_juggernaut(g);
 }
 if(relic(g,88)&&g->block==0)g->expedition.combat[E181_NEXT_BLOCK]=1;
 if(relic(g,54)&&!g->ic[IC_ATTACKS_THIS_TURN])g->v151.relic[1]=1;
 if(relic(g,59)&&g->block>=20)ninja_poison_all(g,2);
 check_win(g);
}
void game_end_turn(Game*g){
 if(g->screen!=COMBAT||g->ninja.task_count||g->regent.task_count)return;
 game_prepare_end_turn(g);
 if(g->screen!=COMBAT)return;
 g->ic[38]=0;
 if(g->ic[IC_TEMP_STRENGTH]>0){
  g->strength-=g->ic[IC_TEMP_STRENGTH];g->ic[IC_TEMP_STRENGTH]=0;
 }
 g->dexterity-=g->ninja.v[NS_TEMP_DEX];g->ninja.v[NS_TEMP_DEX]=0;
 g->ninja.v[NS_CORROSIVE]=g->ninja.v[NS_SHADOWMELD]=g->ninja.v[NS_BURST]=0;g->ninja.v[NS_STRANGLE0]=g->ninja.v[NS_STRANGLE1]=0;
 g->ic[IC_OWN_TURN]=0;
 g->ic[IC_HOURGLASS]=relic(g,40)?CLAMP(g->energy,0,2):0;
 int retained=0;
 for(int i=0;i<g->hand_count;i++){
  g->hand[i]&=~CARD_FREE;
  if(CARD_ID(g->hand[i])==50){lose_hp(g,2);}
  if(g->screen==DEFEAT)return;
  else if(CARD_ID(g->hand[i])==53)g->energy_penalty++;
  if(regent_flags(g->hand[i])&8){exhaust_card(g,g->hand[i]);continue;}
  if(game_card_is_retained(g,g->hand[i]))g->hand[retained++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=g->hand[i]&~(CARD_SLY_TURN|CARD_RETAIN_TURN);
 }
 g->hand_count=retained;g->regent.retain_all=0;g->regent.monologue=0;
 /* Hand Trick lasts for the whole turn, even after playing/reshuffling. */
 for(int i=0;i<g->draw_count;i++)g->draw[i]&=~CARD_SLY_TURN;
 for(int i=0;i<g->discard_count;i++)g->discard[i]&=~CARD_SLY_TURN;
 for(int i=0;i<g->exhaust_count;i++)g->exhaust[i]&=~CARD_SLY_TURN;
 /* Block shields the incoming enemy turn, then expires before the next draw. */
 for(int k=0;k<g->enemy_count;k++)g->enemies[k].block=0;
 for(int k=0;k<g->enemy_count;k++){
  Enemy *e=&g->enemies[k];if(e->hp<=0)continue;
  for(int p=0;p<=MIN(1000,g->ninja.v[NS_ACCELERANT])&&e->hp>0&&e->poison>0;p++){record_combat_fx(GFX_PROC,-1,k,e->poison,0);e->hp-=e->poison;e->poison--;}
  if(e->hp<=0)continue;
  switch(e->intent){
   case 0: {int dmg=enemy_attack_value(e);enemy_hit(g,e,dmg);break;}
   case 1:e->block+=e->intent_value;break;
   case 2:enemy_hit(g,e,enemy_attack_value(e));g->weak=MAX(g->weak,2);break;
   case 3:enemy_hit(g,e,enemy_attack_value(e));g->vulnerable=MAX(g->vulnerable,2);break;
   case 4:e->strength+=2;e->block+=8;break;
   case 5:enemy_hit(g,e,enemy_attack_value(e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 6:enemy_hit(g,e,enemy_attack_value(e));g->poison=MIN(12,g->poison+2);break;
   case 7:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(e));break;
   case 8:enemy_hit(g,e,enemy_attack_value(e));g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 9:e->hp=MIN(e->maxhp,e->hp+e->maxhp/9+3);e->block+=e->intent_value/2;break;
   case 10:enemy_hit(g,e,enemy_attack_value(e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;
   case 11:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=53;e->block+=e->intent_value/2;break;
   case 12:{int before=g->hp;enemy_hit(g,e,enemy_attack_value(e));e->hp=MIN(e->maxhp,e->hp+MAX(0,before-g->hp));break;}
   case 13:e->block+=e->intent_value;e->strength++;break;
   case 14:{int preserved=g->block;g->block=0;enemy_hit(g,e,enemy_attack_value(e));g->block+=preserved;break;}
   case 15:g->block=MAX(0,g->block-4);enemy_hit(g,e,enemy_attack_value(e));break;
   case 16:for(int hit=0;hit<3&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(e));break;
   case 17:if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=52;e->block+=e->intent_value/2;break;
   case 18:enemy_hit(g,e,enemy_attack_value(e));g->weak=MAX(g->weak,2);g->vulnerable=MAX(g->vulnerable,2);break;
   case 19:for(int hit=0;hit<2&&g->screen==COMBAT&&e->hp>0;hit++)enemy_hit(g,e,enemy_attack_value(e));if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=50;break;
   case 20:for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0){g->enemies[z].strength++;g->enemies[z].block+=8;}break;
   case 21:e->weak=e->vulnerable=0;e->poison/=2;e->block+=e->intent_value/2;break;
   case 22:g->weak=MAX(g->weak,2);g->energy_penalty=MIN(2,g->energy_penalty+1);break;
   case 23:g->poison=MIN(12,g->poison+2);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=51;break;
  }
  if(e->block>0)record_combat_fx(GFX_BLOCK,k,k,e->block,0);
  if(g->screen==DEFEAT)return;
  if(e->weak>0)e->weak--;
  if(e->vulnerable>0)e->vulnerable--;
 }
 if(g->poison>0){
   int inferno=g->used_potion;g->used_potion=0;
   lose_hp(g,g->poison);g->used_potion=inferno;
   g->poison--;if(g->screen==DEFEAT)return;
 }
 if(g->weak>0)g->weak--;
 if(g->vulnerable>0)g->vulnerable--;
 check_win(g);
 if(g->screen!=COMBAT)return;
 if(!g->ic[IC_BARRICADE]&&!g->ninja.v[NS_BLUR])g->block=0;
 if(g->ninja.v[NS_BLUR])g->ninja.v[NS_BLUR]--;
 if(g->ninja.v[NS_INTANGIBLE])g->ninja.v[NS_INTANGIBLE]--;
 g->ic[IC_COLOSSUS]=0;
 for(int i=0;i<g->enemy_count;i++){
  if(i<2&&g->ic[IC_MANGLE0+i]!=0){
   g->enemies[i].strength+=g->ic[IC_MANGLE0+i];
   g->ic[IC_MANGLE0+i]=0;
  }
 }
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)enemy_intent(g,&g->enemies[i]);
 game_start_turn(g);
}
static int has_strike_tag(const char *name){
 for(;*name;name++){
  const char *p=name,*key="Strike";
  while(*p&&*key&&*p==*key){p++;key++;}
  if(!*key)return 1;
 }
 return 0;
}
static int strike_count(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++){
  int id=CARD_ID(g->deck[i]);
  if(id>=0&&id<CARD_COUNT&&has_strike_tag(cards[id].name))count++;
 }
 return count;
}
static void all_damage(Game*g,int n){for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0)deal(g,i,n);}
static void exhaust_card(Game*g,int raw){
 if(g->exhaust_count<MAX_PILE){
  g->exhaust[g->exhaust_count++]=raw;
  r151_exhaust(g);
  g->reward_potion|=4;
  if(relic(g,41)&&g->ic[IC_BELL]<3){g->ic[IC_BELL]++;relic_block(g,3);}
  if(CARD_ID(raw)==64&&g->ic[IC_OWN_TURN])gain_energy(g,2+CARD_UP(raw));
  if(g->ic[IC_DARK_EMBRACE]&&g->ic[IC_CHAIN_DEPTH]<20){
    g->ic[IC_CHAIN_DEPTH]++;
    for(int j=0;j<g->ic[IC_DARK_EMBRACE];j++)draw_card(g);
    g->ic[IC_CHAIN_DEPTH]--;
  }
  if(g->shop_cards[2]>0){int old_block=g->block;g->block=MIN(999,g->block+g->shop_cards[2]);record_combat_fx(GFX_BLOCK,-1,-1,g->block-old_block,0);}
 }
}
static void select_live_enemy(Game*g,int direction);
int game_card_cost_now(const Game*g,int raw){
 int id=CARD_ID(raw);
 if(raw&CARD_FREE)return 0;
 if(g&&g->character==CHAR_REGENT&&g->regent.powers[R_VOID_FORM-REGENT_FIRST]&&g->regent.void_used<2*g->regent.powers[R_VOID_FORM-REGENT_FIRST])return 0;
 if(g&&id<CARD_COUNT&&g->ic[IC_CORRUPTION]&&cards[id].type==1)return 0;
 if(g&&id<CARD_COUNT&&cards[id].type==0&&g->ic[IC_UNRELENTING])return 0;
 if(g&&id>=REGENT_FIRST&&id<ASCENT_CARD_FIRST)return regent_cost_now(g,raw);
 if(g&&((id>=NINJA_FIRST&&id<NINJA_LAST)||(id>=354&&id<374))){if(cards[id].type==1&&g->ninja.v[NS_POUNCE])return 0;if(id==N_PINPOINT)return MAX(0,game_card_cost(raw)-g->ninja.v[NS_SKILLS_TURN]);if(id==N_UP_MY_SLEEVE)return MAX(0,game_card_cost(raw)-CARD_COUNTER(raw));}
 if(CARD_ID(raw)==3&&g){int cost=game_card_cost(raw)-g->shop_relic;return cost>0?cost:0;}
 return game_card_cost(raw);
}
#include "ninja_engine.inc"
#include "relics151.inc"
#include "regent_engine.inc"
static void autoplay_card(Game*g,int raw,int force_exhaust){
 if(g->screen!=COMBAT||g->hand_count>=MAX_HAND||g->ic[IC_CHAIN_DEPTH]>=20)return;
 int foe=random_living_foe(g);
 if(foe<0)return;
 int index=g->hand_count++;
 g->hand[index]=raw|CARD_FREE|(force_exhaust?CARD_FORCE_EXHAUST:0);
 g->ic[IC_CHAIN_DEPTH]++;
 g->ic[46]=(raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL))+1;
 game_play_card(g,index,foe);
 g->ic[IC_CHAIN_DEPTH]--;
}
static void autoplay_from_draw(Game*g,int limit,int force_exhaust){
 for(int i=0;i<limit&&g->screen==COMBAT&&g->draw_count>0&&
     g->ic[IC_CHAIN_DEPTH]<20;i++){
  int raw=g->draw[--g->draw_count];
  autoplay_card(g,raw,force_exhaust);
 }
}
static void resolve_card(Game*g,int index,int enemy,int replay_raw){
 if(g->screen!=COMBAT||(index>=0&&index>=g->hand_count)||(index<0&&replay_raw<0))return;
 int raw=index<0?replay_raw:g->hand[index],id=CARD_ID(raw),up=CARD_UP(raw),cost=game_card_cost_now(g,raw);
 /* Multiplayer-only ally target cards are deliberately not playable solo. */
 if(id==63||id==85){msg(g,"Requires multiplayer ally.");return;}
 if(cost==-2)return;
 int x_value=game_card_cost(raw)==-1?g->energy:0;
 if(id==10&&(raw&CARD_EPHEMERAL)&&repeated_x>=0)x_value=repeated_x;
 if(cost==-1)cost=g->energy;
 if(g->energy<cost){snprintf(g->message,sizeof(g->message),"Need %d more Energy.",cost-g->energy);return;}
 int saved_attack=r140_attack_context,saved_rust=r140_rust_targets,saved_mod=r150_card_raw;
 r150_card_raw=raw;
 r140_attack_context=cards[id].type==0;r140_rust_targets=0;
 int saved_fx_card=combat_fx_card;
 if(!preview_trace)combat_fx_card=raw;
 int saved_phase=preview_phase;
 if(preview_trace){
  preview_depth++;
  preview_phase=preview_depth==1||(preview_depth==2&&(raw&CARD_EPHEMERAL));
  if(preview_depth>1&&!preview_phase)preview_nested=1;
  if(preview_phase&&preview_depth==2)preview_trace->repeated++;
 }
 int original_raw=raw;regent_foreign_begin(g,raw);
 int ascent_custom=id>=ASCENT_CARD_FIRST,ascent_after=ascent_custom?g->ninja.v[NS_AFTERIMAGE]:0,ascent_serpent=ascent_custom?g->ninja.v[NS_SERPENT]:0;
 int repeat_this_attack=(!(raw&CARD_EPHEMERAL)&&CARD_MOD(raw)==MOD_ECHO)?1:0;
 if(g->character==CHAR_REGENT&&!(raw&CARD_EPHEMERAL))repeat_this_attack+=g->regent.instance_replay[CARD_INSTANCE(raw)];
 if(id<CARD_COUNT&&cards[id].type==0){
  if(g->ic[IC_UNRELENTING])g->ic[IC_UNRELENTING]=0;
  if(!(raw&CARD_EPHEMERAL)&&g->ic[IC_NEXT_ATTACK_REPEAT]>0){
   g->ic[IC_NEXT_ATTACK_REPEAT]--;
   repeat_this_attack++;
  }
 }
 if(ascent_custom&&cards[id].type==1){
  g->ninja.v[NS_POUNCE]=0;
  if(!(raw&CARD_EPHEMERAL)&&g->ninja.v[NS_BURST]){g->ninja.v[NS_BURST]--;repeat_this_attack++;}
 }
 if(enemy<0||enemy>=g->enemy_count||g->enemies[enemy].hp<=0){enemy=0;while(enemy<g->enemy_count-1&&g->enemies[enemy].hp<=0)enemy++;}
 g->energy-=cost;
 if(index>=0){g->hand_count--;
  for(int j=index;j<g->hand_count;j++)g->hand[j]=g->hand[j+1];
 }
 int r151_before[6];r151_status_snapshot(g,r151_before);
 int n=up?3:0,u=up,e=enemy;
 switch(id){
#include "new_effects.inc"
  case 0:deal(g,enemy,6+n);break;case 1:card_block(g,5+n);break;
  case 2:deal(g,enemy,8+n);apply_vulnerable(g,&g->enemies[enemy],2+up);break;
  case 3:all_damage(g,12+3*up);break;
  case 4:for(int j=0;j<3+up;j++)draw_card(g);g->reward_potion|=1;break;
  case 5:card_block(g,8+n);draw_card(g);break;
  case 6:lose_hp(g,3);gain_energy(g,2+up);break;
  case 7:deal(g,enemy,32+2*n);break;
  case 8:g->strength+=2+up;break;
  case 9:card_block(g,30+2*n);break;
  case 10:for(int j=0;j<x_value;j++)all_damage(g,5+3*up);break;
  case 11:deal(g,enemy,9+up);for(int j=0;j<1+up;j++)draw_card(g);break;
  case 12:card_block(g,5+n);deal(g,enemy,5+n);break;
  case 13:deal(g,enemy,6+(3+up)*g->exhaust_count);break;
  case 14:lose_hp(g,1);if(g->screen==COMBAT)all_damage(g,9+4*up);break;
  case 15:deal(g,enemy,18+6*up);if(g->hand_count>0){int k=rnd(g,g->hand_count),c=g->hand[k];for(int j=k;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];g->hand_count--;exhaust_card(g,c);
    {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}}break;
  case 16:card_block(g,8+3*up);if(g->reward_potion&4)card_block(g,8+3*up);break;
  case 17:{
 card_block(g,7+2*up);
 if(g->hand_count>0){
  if(up){g->next_screen=17;g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
  else {int ix=rnd(g,g->hand_count),c=g->hand[ix];
   for(int j=ix;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];
   g->hand_count--;exhaust_card(g,c);
   {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}}
 }break;
}
  case 18:
 lose_hp(g,1);
 if(g->screen!=DEFEAT&&g->hand_count>0){
   g->next_screen=18;g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;
 }else if(g->screen==COMBAT)g->strength+=1+up;
 break;
  case 19:all_damage(g,4+3*up);for(int z=0;z<g->enemy_count;z++)if(g->enemies[z].hp>0)apply_vulnerable(g,&g->enemies[z],1+up);break;
  case 20:g->shop_cards[2]+=3+up;break;
  case 21:deal(g,enemy,g->block);break;
  case 22:for(int j=0;j<3+up;j++){int count=0;for(int k=0;k<g->enemy_count;k++)if(g->enemies[k].hp>0)count++;if(!count)break;int k=(preview_trace&&preview_depth==1&&preview_choice>=0)?preview_choice%count:rnd(g,count);for(int q=0;q<g->enemy_count;q++)if(g->enemies[q].hp>0&&k--==0){deal(g,q,3);break;}}break;
  case 23:
 card_block(g,5);
 if(up){
  for(int z=0;z<g->hand_count;z++)
   if(game_card_is_upgradeable(g->hand[z]))g->hand[z]|=UPBIT;
 }else{
  for(int z=0;z<g->hand_count;z++)
   if(!CARD_UP(g->hand[z])&&CARD_ID(g->hand[z])<50){
    g->next_screen=23;g->scroll=0;g->selection=z;
    g->prior_screen=COMBAT;g->screen=CARD_CHOICE;break;
   }
 }break;
  case 24:card_block(g,12+4*up);g->shop_cards[1]+=4+2*up;break;
  case 25:{
 deal(g,enemy,9+3*up);
 int remaining=0;
 for(int q=0;q<g->enemy_count;q++)if(g->enemies[q].hp>0)remaining++;
 if(g->discard_count>0&&remaining>0){
   g->next_screen=25;g->ic[IC_PENDING_CARD]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);g->scroll=up;g->selection=0;
   g->prior_screen=COMBAT;g->screen=CARD_CHOICE;
 }break;
}
  case 26:{
 deal(g,enemy,6+3*up);
 while(g->hand_count<MAX_HAND&&(g->draw_count||g->discard_count)&&
       !(g->reward_potion&1)){
   int before=g->hand_count;draw_card(g);
   if(g->hand_count<=before)break;
   int newest=CARD_ID(g->hand[g->hand_count-1]);
   if(newest>=CARD_COUNT||cards[newest].type!=0)break;
 }break;
}
  case 27:deal(g,enemy,5+n);deal(g,enemy,5+n);break;
  case 28:{
    /* Playing the Power does NOT immediately cost HP. Start next turn. */
    int copies=RIFT_INFERNO_COPIES(g);
    int dmg=RIFT_INFERNO_POWER(g);
    copies=MIN(copies+1,30);
    dmg=MIN(dmg+6+3*up,32000);
    g->used_potion=(copies<<16)|dmg;
    break;
  }
  case 29:lose_hp(g,2);if(g->screen==COMBAT)deal(g,enemy,15+5*up);break;
  case 30:apply_vulnerable(g,&g->enemies[enemy],3+up);break;
  case 31:card_block(g,7+n);apply_vulnerable(g,&g->enemies[enemy],1+up);break;
  case 32:{int hits=g->enemies[enemy].vulnerable?2:1;for(int z=0;z<hits;z++)deal(g,enemy,8+2*up);break;}
  case 33:g->reward_potion+=((3+2*up)<<8);break;
  case 34:apply_vulnerable(g,&g->enemies[enemy],1+up);g->strength+=g->enemies[enemy].vulnerable;break;
  case 35:deal(g,enemy,4+(2+up)*g->enemies[enemy].vulnerable);break;
  case 36:for(int hit=0;hit<4+up;hit++)all_damage(g,2);break;
  case 37:if(g->exhaust_count>=3)all_damage(g,18+6*up);break;
  case 38:lose_hp(g,6);gain_energy(g,2);for(int j=0;j<3+up;j++)draw_card(g);break;
  case 39:deal(g,enemy,13+3*up);g->enemies[enemy].weak+=1+up;apply_vulnerable(g,&g->enemies[enemy],1+up);break;
  case 40:deal(g,enemy,10+4*up);g->enemies[enemy].vulnerable=MIN(99,g->enemies[enemy].vulnerable*2);break;
  case 41:g->shop_cards[0]+=1+up;break;
  case 42:{int count=g->hand_count;for(int z=0;z<count;z++)exhaust_card(g,g->hand[z]);g->hand_count=0;for(int z=0;z<count;z++)deal(g,enemy,7+3*up);break;}
  case 43:
 if(g->hand_count>0){g->next_screen=43;g->scroll=up;g->selection=0;
  g->prior_screen=COMBAT;g->screen=CARD_CHOICE;}
 else for(int j=0;j<2+up;j++)draw_card(g);
 break;
  case 44:deal(g,enemy,6+3*up);if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL);break;
  case 45:{int ix=0;while(ix<g->hand_count){int c=g->hand[ix];if(cards[CARD_ID(c)].type!=0){for(int j=ix;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];g->hand_count--;exhaust_card(g,c);card_block(g,5+2*up);}else ix++;}break;}
  case 46:lose_hp(g,2);card_block(g,16+2*n);break;
  case 47:{int was=g->enemies[enemy].hp;deal(g,enemy,10+2*up);if(was>0&&g->enemies[enemy].hp<=0){g->maxhp+=3+up;heal(g,3+up);}break;}
  case 48:g->heal_after_battle+=3+up;break;
  case 49:deal(g,enemy,6+(2+up)*strike_count(g));break;
  case 54:g->ic[IC_AGGRESSION]++;break;
  case 55:g->ic[IC_BARRICADE]=1;break;
  case 56:deal(g,enemy,20+10*up);
    apply_vulnerable(g,&g->enemies[enemy],5+2*up);break;
  case 57:autoplay_from_draw(g,MIN(30,x_value+up),0);break;
  case 58:card_block(g,5+3*up);g->ic[IC_COLOSSUS]=1;break;
  case 59:g->ic[IC_CORRUPTION]=1;break;
  case 60:g->ic[IC_CRIMSON]+=8+2*up;g->ic[IC_CRIMSON_COPIES]++;break;
  case 61:g->ic[IC_CRUELTY]=MIN(100,g->ic[IC_CRUELTY]+25+25*up);break;
  case 62:g->ic[IC_DARK_EMBRACE]++;break;
  case 63:break; /* Multiplayer-only; rejected before spending. */
  case 64:draw_card(g);draw_card(g);break;
  case 65:{
    int n=0;
    for(int i=0;i<g->hand_count;i++)
     if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)n++;
    gain_energy(g,n);
    g->ic[IC_ENERGY_LOCK]=1;break;
  }
  case 66:deal(g,enemy,5+up);deal(g,enemy,5+up);
    g->strength+=3+up;g->enemies[enemy].strength++;break;
  case 67:if(g->reward_potion&4)gain_energy(g,3+up);break;
  case 68:autoplay_from_draw(g,1,1);break;
  case 69:g->ic[IC_HELLRAISER]++;break;
  case 70:all_damage(g,16+5*up);break;
  case 71:{
    for(int tries=0;tries<150;tries++){
      int card=rnd(g,CARD_COUNT);
      if(card<CARD_COUNT&&ninja_owned_card(g,card)&&cards[card].type==0&&
         game_card_is_playable(card)&&card!=CARD_GIANT_ROCK&&card!=56){
        add_generated(g,card|CARD_FREE);break;
      }
    }break;
  }
  case 72:g->ic[IC_JUGGERNAUT]+=6+2*up;break;
  case 73:g->ic[IC_JUGGLING]=1;break;
  case 74:{
    deal(g,enemy,15+5*up);
    int loss=10+5*up;
    g->enemies[enemy].strength-=loss;
    if(enemy<2)g->ic[IC_MANGLE0+enemy]+=loss;
    break;
  }
  case 75:heal(g,10+3*up);break;
  case 76:g->ic[IC_NEXT_ATTACK_REPEAT]+=1+up;break;
  case 77:
   for(int i=0;i<g->hand_count;i++)
    if(CARD_ID(g->hand[i])<CARD_COUNT&&
       cards[CARD_ID(g->hand[i])].type==0)
      g->hand[i]=CARD_GIANT_ROCK|(up?UPBIT:0);
   break;
  case 78:g->ic[IC_PYRE]+=1+up;break;
  case 79:deal(g,enemy,9+g->ic[IC_RAMPAGE]);
    g->ic[IC_RAMPAGE]+=5+4*up;break;
  case 80:deal(g,enemy,7+2*up);g->strength+=2+up;
    g->ic[IC_TEMP_STRENGTH]+=2+up;break;
  case 81:{
    int hits=g->ic[IC_HPLOSS_TURN]?2+up:1;
    for(int i=0;i<hits&&g->enemies[enemy].hp>0;i++)deal(g,enemy,5);
    break;
  }
  case 82:g->ic[IC_STAMPEDE]++;break;
  case 83:{
    int n=g->hand_count;
    for(int i=0;i<n;i++)exhaust_card(g,g->hand[i]);
    g->hand_count=0;
    for(int i=0;i<n&&g->hand_count<MAX_HAND;i++){
      for(int tries=0;tries<140;tries++){
       int id=rnd(g,CARD_COUNT);
       if(game_card_is_playable(id)&&ninja_owned_card(g,id)&&id!=CARD_GIANT_ROCK&&id!=63&&id!=85&&id!=N_FLANKING&&id!=N_SNEAKY){
        add_generated(g,id|(up?UPBIT:0));break;
       }
      }
    }
    break;
  }
  case 84:g->ic[IC_PLATING]+=4+2*up;break;
  case 85:break; /* Multiplayer-only; rejected before spending. */
  case 86:{
    int hits=MIN(20,1+g->ic[IC_HPLOSS_COMBAT]);
    for(int i=0;i<hits&&g->enemies[enemy].hp>0;i++)deal(g,enemy,5+2*up);
    break;
  }
  case 87:{
    int added=0,count=0;
    for(int i=0;i<g->hand_count;i++)
     if(CARD_ID(g->hand[i])<CARD_COUNT&&cards[CARD_ID(g->hand[i])].type==0)count++;
    if(count>0){
     int which=(preview_trace&&preview_depth==1&&preview_choice>=0)?preview_choice%count:rnd(g,count);
     for(int i=0;i<g->hand_count;i++){
      int c=g->hand[i];
      if(cards[CARD_ID(c)].type==0&&which--==0){
       added=card_base_damage(g,c,enemy);
       for(int j=i;j+1<g->hand_count;j++)g->hand[j]=g->hand[j+1];
       g->hand_count--;exhaust_card(g,c);
       {char notice[72];snprintf(notice,sizeof(notice),"Random exhaust: %s",cards[CARD_ID(c)].name);msg(g,notice);}
       break;
      }
     }
    }
    deal(g,enemy,4+2*up+added);
    if(g->enemies[enemy].hp>0)deal(g,enemy,4+2*up+added);break;
  }
  case 88:g->ic[IC_UNMOVABLE]=1;break;
  case 89:deal(g,enemy,14+6*up);g->ic[IC_UNRELENTING]=1;break;
  case 90:g->ic[IC_VICIOUS]+=1+up;break;
  case 91:deal(g,enemy,16+4*up);break;
  case 92:g->enemies[enemy].block=MAX(0,g->enemies[enemy].block-6-3*up);deal(g,enemy,14+5*up);break;
  case 93:{int threatened=0;for(int e=0;e<g->enemy_count;e++)threatened|=game_enemy_intent_hits(g,e)>0;card_block(g,6+3*up+(threatened?4:0));break;}
  case 94:{int weak=g->enemies[enemy].weak;deal(g,enemy,8+3*up);if(weak)heal(g,3+up);break;}
  case 95:g->ic[IC_COVENANT]=MIN(1000000,g->ic[IC_COVENANT]+4+2*up);break;
 }
 r151_after_effect(g,r151_before);
 if(cards[id].type==2){g->ic[id>=93?47:33+id/31]|=(int)(1u<<(id%31));record_combat_fx(GFX_POWER,-1,-1,0,0);}
 if(CARD_MOD(raw)==MOD_KINDLED&&g->screen==COMBAT)draw_card(g);
 if(preview_trace)preview_phase=0;
 int must_exhaust=(id==9||id==30||id==34||id==38||id==40||
   id==42||id==47||id==67||id==71||id==75||id==83||
   (id==63&&!up)|| (raw&CARD_FORCE_EXHAUST) || (id>=ASCENT_CARD_FIRST&&id<ASCENT_CARD_LAST&&new_exhaust[id-ASCENT_CARD_FIRST]) ||
   (g->ic[IC_CORRUPTION]&&cards[id].type==1)||CARD_MOD(raw)==MOD_FLEETING||CARD_MOD(raw)==MOD_KINDLED);
 if(ascent_custom&&cards[id].type==1&&g->ninja.v[NS_PLANNER])raw|=CARD_SLY;
 if(must_exhaust&&!(raw&CARD_EPHEMERAL)){exhaust_card(g,raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL));}
 else if(cards[id].type!=2&&!(raw&CARD_EPHEMERAL)&&!(id==25&&g->screen==CARD_CHOICE)&&g->discard_count<MAX_PILE)g->discard[g->discard_count++]=raw&~(CARD_FREE|CARD_FORCE_EXHAUST);
 if(g->selection>=g->hand_count)g->selection=MAX(0,g->hand_count-1);
 g->turn_played++;
 if(cards[id].type==0){
  g->ic[IC_ATTACKS_THIS_TURN]++;
  if(g->ic[IC_ATTACKS_THIS_TURN]%3==0){if(relic(g,5))g->strength++;if(relic(g,24))heal(g,3);}
  if(g->ic[IC_JUGGLING]&&g->ic[IC_ATTACKS_THIS_TURN]==3)
    add_generated(g,(original_raw&~(CARD_FREE|CARD_FORCE_EXHAUST|CARD_EPHEMERAL)));
  g->shop_relic++;
  int rage=(g->reward_potion>>8)&255;
  if(rage)gain_block(g,rage);
 }
 if(cards[id].type==1&&++g->ic[36]%3==0&&relic(g,22))g->dexterity++;
 if(relic(g,43)&&(cards[id].type==0||cards[id].type==1)){
  g->ic[IC_DUELIST]|=cards[id].type==0?1:2;
  if(g->ic[IC_DUELIST]==3){g->ic[IC_DUELIST]|=4;gain_energy(g,1);}
 }
 r151_played(g,cards[id].type);regent_foreign_finish(g,raw);
 if(ascent_custom){
  if(cards[id].type==1)g->ninja.v[NS_SKILLS_TURN]++;
  if(ascent_after)relic_block(g,ascent_after);
  if(ascent_serpent)ninja_proc(g,random_living_foe(g),ascent_serpent,0);
  for(int foe=0;foe<g->enemy_count;foe++)if(g->ninja.v[NS_STRANGLE0+foe])ninja_proc(g,foe,g->ninja.v[NS_STRANGLE0+foe],1);
 }
 for(int repeat=0;repeat<repeat_this_attack&&g->screen==COMBAT&&g->ic[IC_CHAIN_DEPTH]<20;repeat++){
   int saved_x=repeated_x;repeated_x=id==10?x_value:-1;
   /* A replay resolves directly: it never consumes a hand slot, draws an
      extra card through a temporary vacancy, or creates a pile duplicate. */
   int foe=enemy;if(g->enemies[foe].hp<=0)foe=random_living_foe(g);
   if(foe>=0){g->ic[IC_CHAIN_DEPTH]++;
    resolve_card(g,-1,foe,original_raw|CARD_EPHEMERAL|CARD_FREE);g->ic[IC_CHAIN_DEPTH]--;}
   repeated_x=saved_x;
 }
 if(g->hp<=0&&g->screen!=DEFEAT)g->screen=DEFEAT;
 if(g->screen==COMBAT){
  if(g->target<0||g->target>=g->enemy_count||
     g->enemies[g->target].hp<=0)select_live_enemy(g,1);
  check_win(g);
 }
 r140_attack_context=saved_attack;r140_rust_targets=saved_rust;r150_card_raw=saved_mod;
 if(!preview_trace)combat_fx_card=saved_fx_card;
 if(preview_trace){preview_depth--;preview_phase=saved_phase;}
}
void game_play_card(Game*g,int index,int enemy){
 if(index<0)return;if(index<g->hand_count&&CARD_ID(g->hand[index])>=ASCENT_CARD_FIRST){resolve_card(g,index,enemy,-1);return;}if(index<g->hand_count&&(g->character==CHAR_REGENT||CARD_ID(g->hand[index])>=REGENT_FIRST)){regent_play(g,index,enemy);return;}if(index<g->hand_count&&CARD_ID(g->hand[index])>=NINJA_FIRST){ninja_play_from_hand(g,index,enemy);return;}resolve_card(g,index,enemy,-1);
}
static void settle_card_choice(Game*g,int idx){
 int up=g->scroll;
 if(g->screen!=CARD_CHOICE||idx<0||
 idx>=(g->next_screen==25?g->discard_count:g->hand_count))return;
 if(g->next_screen==25){
  int c=g->discard[idx];
  for(int i=idx;i+1<g->discard_count;i++)g->discard[i]=g->discard[i+1];
  g->discard_count--;
  if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=c;
  g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
  int headbutt=g->ic[IC_PENDING_CARD]?g->ic[IC_PENDING_CARD]:25|(up?UPBIT:0);g->ic[IC_PENDING_CARD]=0;
  if(CARD_MOD(headbutt)==MOD_FLEETING||CARD_MOD(headbutt)==MOD_KINDLED)exhaust_card(g,headbutt);
  else if(g->discard_count<MAX_PILE)g->discard[g->discard_count++]=headbutt;
  g->selection=0;r151_finish_choice(g);check_win(g);return;
 }
 int pending=g->next_screen,c=g->hand[idx];
 if(pending==23){
  if(!game_card_is_upgradeable(c)){msg(g,"Choose an unupgraded card.");return;}
  g->hand[idx]|=UPBIT;
 }else if(pending==18||pending==43||pending==17||pending==26){
  for(int i=idx;i+1<g->hand_count;i++)g->hand[i]=g->hand[i+1];
  g->hand_count--;
  if(pending==26){if(g->draw_count<MAX_PILE)g->draw[g->draw_count++]=c;}
  else exhaust_card(g,c);
 }
 g->screen=COMBAT;g->prior_screen=COMBAT;g->scroll=0;g->next_screen=0;
 g->selection=MIN(idx,g->hand_count);
 if(pending==18)g->strength+=1+up;
 if(pending==43)for(int i=0;i<2+up;i++)draw_card(g);
 r151_finish_choice(g);
 if(g->screen==COMBAT)check_win(g);
}
const char *node_name(int type){static const char*names[]={"BATTLE","ELITE","MYSTERY","MERCHANT","CAMPFIRE","RELIC","BOSS"};return type>=0&&type<=6?names[type]:"UNKNOWN";}
static void encounter(Game*g,int node){
 g->revision182.room_type=node;g->revision182.room_pending=node!=NODE_BOSS;
 g->selection=0;
 switch(node){
 case NODE_FIGHT:case NODE_ELITE:game_start_battle(g,node);break;
 case NODE_BOSS:game_story_start(g,1);break;
 case NODE_EVENT:game_begin_event(g,game_next_event(g));break;
 case NODE_SHOP:g->screen=SHOP;for(int i=0;i<3;i++)g->shop_cards[i]=card_reward_except(g,i>0?g->shop_cards[0]:-1,i>1?g->shop_cards[1]:-1);g->shop_relic=random_relic(g);break;
 case NODE_REST:g->screen=REST;break;
 case NODE_TREASURE:{
   int r=expedition_relic(g);
   if(r>=0){
      g->relic_pending=1;g->relic_resume=MAP;g->reward[0]=r;g->win_screen=0;g->screen=RELIC_RESULT;
      snprintf(g->message,sizeof(g->message),"FOUND RELIC: %s",relic_defs[r].name);
   }else{msg(g,"The chest was empty.");enter_map(g);}
   break;
 }
 }
}
void game_choose_node(Game*g,int lane){
 if(g->screen!=MAP||g->floor>=MAP_ROWS)return;
 if(lane<0||lane>=MAP_LANES)return;
 if(!game_route_available(g,lane))return;
 g->lane=lane;g->map_selected[g->floor]=lane;
 int type=g->map[g->floor][lane];g->floor++;encounter(g,type);
}
void game_new(Game*g,uint32_t seed){
 memset(g,0,sizeof(*g));g->magic=0x52494654u;g->version=GAME_VERSION;
 g->revision182.rare_offset=-5;g->v151.potion_offer=-1;g->seed=seed?seed:0x1affadeu;g->rng=g->seed;
 g->screen=TITLE;g->selection=1;g->act=1;g->floor=0;g->lane=1;
 g->maxhp=g->hp=80;g->max_energy=3;g->gold=95;
 for(int i=0;i<5;i++)add_card(g,0);
 for(int i=0;i<4;i++)add_card(g,1);
 add_card(g,2);add_relic(g,0);
 g->potion_count=2;g->potions[0]=0;g->potions[1]=1;
 map_generate(g);msg(g,"A lone Warden climbs the ruined tower.");
}
const char *game_character_name(const Game*g){return g&&g->character==CHAR_REGENT?"Proxy":g&&g->character==CHAR_NIGHTSHADE?"Vesper":"Warden";}
void game_choose_character(Game*g,int character){
 if(!g||character<0||character>2)return;
 if(character==CHAR_REGENT){g->character=CHAR_REGENT;g->deck_count=g->relic_count=g->potion_count=0;g->max_energy=3;g->maxhp=g->hp=75;g->gold=99;for(int i=0;i<4;i++){add_card(g,R_STRIKE_REGENT);add_card(g,R_DEFEND_REGENT);}add_card(g,R_FALLING_STAR);add_card(g,R_VENERATE);add_relic(g,63);msg(g,"The Proxy claims the ruined tower.");return;}
 g->character=character;g->deck_count=g->relic_count=0;g->potion_count=0;
 g->max_energy=3;g->maxhp=g->hp=character==CHAR_NIGHTSHADE?70:80;g->gold=character==CHAR_NIGHTSHADE?99:95;
 for(int i=0;i<5;i++)add_card(g,character==CHAR_NIGHTSHADE?N_STRIKE_SILENT:0);
 for(int i=0;i<(character==CHAR_NIGHTSHADE?5:4);i++)add_card(g,character==CHAR_NIGHTSHADE?N_DEFEND_SILENT:1);
 if(character==CHAR_NIGHTSHADE){add_card(g,N_NEUTRALIZE);add_card(g,N_SURVIVOR);add_relic(g,NR_SNAKE);}
 else{add_card(g,2);add_relic(g,0);g->potion_count=2;g->potions[0]=0;g->potions[1]=1;}
 msg(g,character==CHAR_NIGHTSHADE?"Vesper enters the ruined tower.":"A lone Warden climbs the ruined tower.");
}
int game_story_kind(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 if(expedition_scene(screen))return screen==ABOMINATION_BEFORE?2:1;
 if(ascent_story_screen(screen))return screen==ASCENT_BOSS?2:1;
 return (screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING)?1:screen==STORY_BOSS?2:0;
}
static uint32_t story_hash(uint32_t x){x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
void game_story_start(Game*g,int boss){
 if(boss&&g->act==7){g->screen=ABOMINATION_BEFORE;g->selection=0;return;}
 if(boss&&g->act==4&&g->ascent.secret){g->screen=ASCENT_BOSS;g->selection=0;return;}
 if(boss&&g->act>3){game_start_battle(g,NODE_BOSS);return;}
 g->screen=boss?STORY_BOSS:STORY_ACT_BEFORE_BLESSING;g->selection=0;
 g->scroll=(int)(story_hash(g->seed^((uint32_t)g->character+1u)*0x9e3779b9u^((uint32_t)g->act)*0x85ebca6bu^(boss?0x170b055u:0x170ac7u))%(boss?5u:10u));
}
const char*game_story_text(const Game*g){
 int c=CLAMP(g->character,0,2),a=CLAMP(g->act-1,0,3);
 return game_story_kind(g)==2?r170_boss[c][MIN(a,2)][CLAMP(g->scroll,0,4)][CLAMP(g->selection,0,2)]:r170_entry[c][a][CLAMP(g->scroll,0,9)];
}
const char*game_story_speaker(const Game*g){
 static const char*names[]={"WARDEN","VESPER","PROXY"};
 return game_story_kind(g)==2&&g->selection==1?game_enemy_name(enemy_boss[CLAMP(g->act-1,0,2)]):names[CLAMP(g->character,0,2)];
}
void game_update(Game*g){if(g->screen==COMBAT&&g->ninja.task_count)ninja_process(g);if(g->screen==COMBAT&&(g->regent.task_count||g->regent.shuffle_pending))regent_process(g);if(g->screen==ACT_INTRO&&++g->scroll>=120)game_story_start(g,0);}
int game_music_silent(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 return (expedition_scene(screen)&&screen!=ABOMINATION_BEFORE)||screen==ACT_INTRO||screen==STORY_ACT||screen==STORY_ACT_BEFORE_BLESSING||(ascent_story_screen(screen)&&screen!=ASCENT_BOSS);
}
int game_music_track(const Game*g){
 int screen=g->screen==PAUSE_MENU?game_pause_source():g->screen;
 if(screen==STORY_BOSS||screen==ASCENT_BOSS||screen==ABOMINATION_BEFORE)return 0;
 if(screen>=ASCENT_SLOTS_VIEW&&screen<=ASCENT_OPTIONS)return 0;
 if(screen==TITLE||screen==NEW_RUN_CONFIRM||screen==HELP||screen==CHARACTER_SELECT)return 0;
 if(screen==COMBAT||screen==CARD_CHOICE||screen==NINJA_CHOICE||screen==REGENT_CHOICE||(screen==DECK_VIEW&&g->prior_screen==COMBAT)){
  if(g->battle_type==NODE_ELITE)return g->combat_music==6?6:3;
  if(g->battle_type==NODE_BOSS)return 0;
  return g->combat_music==4?4:g->combat_music==5?5:2;
 }
 return g->act==1?1:g->act==2?7:g->act==3?8:9;
}
static void shop_buy(Game*g,int choice){
 int discount=relic(g,6)?30:0;
 if(choice<3){if(g->shop_cards[choice]<0){msg(g,"Already sold.");return;}int price=50+25*cards[g->shop_cards[choice]].rarity-discount;if(g->gold>=price&&g->deck_count<MAX_DECK){g->gold-=price;add_card(g,g->shop_cards[choice]);g->shop_cards[choice]=-1;msg(g,"Purchased a card.");}else msg(g,"Not enough gold.");}
 else if(choice==3){int price=155-discount;if(g->gold>=price&&g->shop_relic>=0){g->gold-=price;add_relic(g,g->shop_relic);g->shop_relic=-1;msg(g,"Relic purchased.");}else msg(g,"Not enough gold.");}
 else if(choice==4){if(g->gold>=60&&g->deck_count>1){
   g->screen=DECK_VIEW;g->prior_screen=SHOP;g->selection=0;g->deck_readonly=0;g->shop_selection=4;
   msg(g,"Choose card to remove; charge only when confirmed.");
 }else msg(g,"Not enough gold.");}
 else if(choice==5){if(g->gold>=50&&g->potion_count<MAX_POTIONS){
   g->gold-=50;int potion=ascent_random_potion(g);g->potions[g->potion_count++]=potion;
   snprintf(g->message,sizeof(g->message),"Potion gained: %s",game_potion_name(potion));
 }else msg(g,g->potion_count==MAX_POTIONS?"Belt full. No gold spent.":"Not enough gold.");}
 else if(choice==6){enter_map(g);}

}
static int count_upgradeable(const Game*g){
 int count=0;
 for(int i=0;i<g->deck_count;i++)
  if(game_card_is_upgradeable(g->deck[i]))count++;
 return count;
}
#include "events150.inc"
static void rest_pick(Game*g,int option){
 if(option==0){heal(g,g->maxhp*3/10);if(relic(g,8)&&count_upgradeable(g)){g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=g->scroll=0;msg(g,"Rested. Choose an upgrade.");}else{enter_map(g);msg(g,"Recovered at the campfire.");}}
 if(option==1){
   if(!count_upgradeable(g)){msg(g,"All cards already upgraded.");return;}
   g->screen=DECK_VIEW;g->prior_screen=REST;g->selection=0;g->scroll=0;
   msg(g,"Choose an unupgraded card.");
 }
}
int game_inventory_available(const Game*g){
 switch(g->screen){case COMBAT:case MAP:case SHOP:case REST:case EVENT:case EVENT_CARD:case EVENT_RESULT:case REWARD:case BOSS_REWARD:case REWARD_PREVIEW:case RELIC_RESULT:case DECK_VIEW:case POTION_VIEW:case RELIC_VIEW:case BLESSING:return 1;default:return 0;}
}
void game_discard_potion(Game*g,int index){
 if(!game_inventory_available(g)||index<0||index>=g->potion_count)return;
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;msg(g,"Potion discarded.");
}
void game_use_potion(Game*g,int index){
 int combat=g->screen==COMBAT;
 int outside=game_inventory_available(g)&&!combat;
 if((!combat&&!outside)||g->potion_count<=0)return;
 if(index<0||index>=g->potion_count)return;
 int id=g->potions[index];int bonus=relic(g,17)?5:0;
 if(!combat&&id!=0&&id!=11&&id!=19&&id!=21){msg(g,"Use this potion during combat.");return;}
 if(!combat&&(id==0||id==11)&&g->hp==g->maxhp){msg(g,"Already at full health. Potion kept.");return;}
 int old_hp=g->hp,old_strength=g->strength,old_block=g->block,old_energy=g->energy,old_hand=g->hand_count;
 for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];
 g->potion_count--;
 if(combat)g->expedition.combat[E181_POTION_USED]=1;
 if(id==0)heal(g,18+bonus);
 if(id==1)g->strength+=2+(bonus>0);
 if(id==2)relic_block(g,20+bonus);
 if(id==3)all_damage(g,20+bonus);
 if(id==4)gain_energy(g,2+(bonus>0));
 if(id==5){for(int i=0;i<3+(bonus>0);i++)draw_card(g);}
 if(id>=6&&id<26){switch(id){
#include "new_potion_effects.inc"
 }snprintf(g->message,sizeof(g->message),"Used %s.",game_potion_name(id));}
 if(combat&&relic(g,21))relic_block(g,12);
 switch(id){
 case 0:snprintf(g->message,sizeof(g->message),"Healing potion: +%d HP",g->hp-old_hp);break;
 case 1:snprintf(g->message,sizeof(g->message),"Strength +%d (now %d)",g->strength-old_strength,g->strength);break;
 case 2:snprintf(g->message,sizeof(g->message),"Guard potion: +%d Block",g->block-old_block);break;
 case 3:snprintf(g->message,sizeof(g->message),"Fire potion used");break;
 case 4:snprintf(g->message,sizeof(g->message),"Energy potion: +%d Energy",g->energy-old_energy);break;
 case 5:snprintf(g->message,sizeof(g->message),"Insight potion: drew %d",MAX(0,g->hand_count-old_hand));break;
 }
 if(combat)check_win(g);
}
static int grid_down(int selected,int count);
#include "inventory151.inc"
static int ascent_action(Game*,int);
static int resume_available;
void game_set_resume_available(int available){resume_available=!!available;}
int game_resume_available(void){return resume_available;}
static int paused_from_screen=MAP;
static int paused_selection;
int game_pause_source(void){return paused_from_screen;}
int game_pause_selection(void){return paused_selection;}
static void select_live_enemy(Game*g,int direction){
 if(g->enemy_count<=0){g->target=0;return;}
 if(g->target<0||g->target>=g->enemy_count)g->target=0;
 int active=0;
 for(int i=0;i<g->enemy_count;i++)active+=g->enemies[i].hp>0;
 if(active==0)return;
 for(int k=0;k<g->enemy_count;k++){
    g->target=(g->target+direction+g->enemy_count)%g->enemy_count;
    if(g->enemies[g->target].hp>0)return;
 }
}
/* Preserve the column where possible; a short final row clamps to its last card.
   Down on the last row stays put instead of moving sideways. */
static int grid_down(int selected,int count){
 return (selected/3+1)*3<count?MIN(selected+3,count-1):selected;
}
void game_action(Game*g,int b){
 if(expedition_action(g,b)||ascent_action(g,b))return;
 if(g->screen==COMBAT)g->ic[46]=0;
 if(b==BTN_PAUSE){
   if(g->screen==PAUSE_MENU){g->screen=paused_from_screen;return;}
   if(g->screen!=TITLE&&g->screen!=HELP&&g->screen!=NEW_RUN_CONFIRM&&g->screen!=CHARACTER_SELECT&&g->screen!=VICTORY&&g->screen!=DEFEAT){
     paused_from_screen=g->screen;paused_selection=0;g->screen=PAUSE_MENU;return;
   }
 }
 int n;
 switch(g->screen){
 /* Legacy post-blessing scenes must not award a second relic on resume. */
 case STORY_ACT:if(b==BTN_OK)enter_map(g);break;
 case STORY_ACT_BEFORE_BLESSING:if(b==BTN_OK)offer_blessing(g);break;
 case STORY_BOSS:if(b==BTN_OK&&++g->selection>=3)game_start_battle(g,NODE_BOSS);break;
 case ACT_INTRO:if(b==BTN_OK)game_story_start(g,0);break;
 case TITLE:
  if(b==BTN_UP)g->selection=(CLAMP(g->selection,0,3)+3)%4;
  else if(b==BTN_DOWN)g->selection=(CLAMP(g->selection,0,3)+1)%4;
  else if(b==BTN_OK){
   if(g->selection<2){g->screen=ASCENT_SLOTS_VIEW;g->scroll=g->selection;g->selection=0;}
   else {g->screen=g->selection==2?ASCENT_COLLECTION:ASCENT_OPTIONS;g->selection=g->scroll=0;}
  }break;
 case CHARACTER_SELECT:
  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=(g->selection+((b==BTN_LEFT||b==BTN_UP)?2:1))%3;
  else if(b==BTN_OK){int c=g->selection,slot=g->master_index;unsigned seed=game_random(g);Game old=*g;
   if(!ascent_begin(g,slot,c,seed)){*g=old;msg(g,"Cannot write save slot. Please try again.");}}
  else if(b==BTN_BACK){g->screen=ASCENT_SLOTS_VIEW;g->scroll=1;g->selection=g->master_index;}break;
 case REGENT_CHOICE:{int n=game_regent_choice_count(g);
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;
  else if(b==BTN_OK)regent_choose(g,g->selection,0);
  else if(b==BTN_BACK)regent_choose(g,0,1);break;}
 case NINJA_CHOICE:{int n=g->hand_count;
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,n);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<n)g->selection++;
  else if(b==BTN_OK)ninja_choose(g,g->selection,0);
  else if(b==BTN_BACK)ninja_choose(g,0,1);break;}
 case HELP:if(b==BTN_BACK||b==BTN_OK)g->screen=g->prior_screen;break;
 case MAP:
  if(b==BTN_UP&&g->floor+g->scroll<MAP_ROWS-1) {
    g->scroll++;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(b==BTN_DOWN&&g->scroll>0){
    g->scroll--;
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    if(!(visible&(1u<<g->selection)))for(int j=0;j<MAP_LANES;j++)
      if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_LEFT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection-1;j>=0;j--)if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_RIGHT){
    unsigned visible=game_route_visible_mask(g,g->floor+g->scroll);
    for(int j=g->selection+1;j<MAP_LANES;j++)if(visible&(1u<<j)){g->selection=j;break;}
  }
  else if(g->scroll>0&&b==BTN_OK)
    ; /* Future rooms are inspectable without a notification. */
  else if(b==BTN_LEFT){for(int k=g->selection-1;k>=0;k--)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_RIGHT){for(int k=g->selection+1;k<MAP_LANES;k++)if(game_route_available(g,k)){g->selection=k;break;}}
  else if(b==BTN_OK){if(g->floor==MAP_ROWS){game_story_start(g,1);}else game_choose_node(g,g->selection);}
  else if(b==BTN_ALT){g->prior_screen=MAP;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;}
  else if(b==BTN_START){g->screen=RELIC_VIEW;g->prior_screen=MAP;g->selection=0;}
  else if(b==BTN_POTION){g->shop_selection=g->selection;g->prior_screen=MAP;g->screen=POTION_VIEW;g->selection=0;msg(g,"");}
  break;
 case CARD_CHOICE:{
  int choices=g->next_screen==25?g->discard_count:g->hand_count;
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,choices);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<choices)g->selection++;
  else if(b==BTN_OK)settle_card_choice(g,g->selection);
  else if(b==BTN_BACK)msg(g,"Choose a card to finish the effect.");
  break;}
 case COMBAT:
  if(g->ninja.task_count){ninja_process(g);break;}
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(g->hand_count,g->selection+1);
  else if(b==BTN_UP)select_live_enemy(g,-1);
  else if(b==BTN_DOWN)select_live_enemy(g,1);
  else if(b==BTN_OK){if(g->selection>=g->hand_count)game_end_turn(g);else game_play_card(g,g->selection,g->target);}
  else if(b==BTN_BACK)game_end_turn(g);
  else if(b==BTN_POTION)game_use_potion(g,g->potion_count-1);
  else if(b==BTN_ALT){
    g->master_index=g->selection;
    g->prior_screen=COMBAT;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;
  }
  break;
 case REWARD:
  if(g->selection==3)g->selection=5;
  if(b==BTN_DOWN){if(g->selection<3)g->selection=g->v151.potion_offer>=0?4:5;else g->selection=5;}
  else if(b==BTN_UP){if(g->selection==5&&g->v151.potion_offer>=0)g->selection=4;else if(g->selection>=4&&!g->v151.card_done)g->selection=1;}
  else if(b==BTN_LEFT&&g->selection<3&&!g->v151.card_done)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT&&g->selection<3&&!g->v151.card_done)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK||b==BTN_BACK){
    if(b==BTN_BACK)g->selection=5;
    if(g->selection<3&&!g->v151.card_done){
      if(g->deck_count>=MAX_DECK){msg(g,"Deck full. You can leave this card reward.");break;}
      add_card(g,g->reward[g->selection]);g->v151.card_done=1;
      if(g->ninja.v[NS_BONUS_REWARDS]>0){g->ninja.v[NS_BONUS_REWARDS]--;fill_rewards(g);g->v151.card_done=0;g->selection=0;msg(g,"The Hunt: choose an extra card reward.");break;}
      g->selection=g->v151.potion_offer>=0?4:5;break;
    }
    if(g->selection==4&&g->v151.potion_offer>=0){
      if(g->potion_count>=MAX_POTIONS){msg(g,"Belt full. Discard a potion in inventory.");break;}
      g->potions[g->potion_count++]=g->v151.potion_offer;g->ic[37]=g->v151.potion_offer+1;g->v151.potion_offer=-1;g->selection=g->v151.card_done?5:1;break;
    }
    if(g->selection!=5)break;
    if(!g->v151.card_done)r151_skip_reward(g);
    /* Leaving forfeits all remaining card picks and the unclaimed potion. */
    g->v151.card_done=1;g->v151.potion_offer=-1;g->ninja.v[NS_BONUS_REWARDS]=0;
    if(g->win_screen){
      int final=g->win_screen==2;g->win_screen=0;
      if(final){g->screen=VICTORY;msg(g,"THE SPIRE HAS FALLEN");}
      else{g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;map_generate(g);begin_act(g);}
    }else{enter_map(g);snprintf(g->message,sizeof(g->message),"Onward, %s.",game_character_name(g));}
  }
  break;
 case REST:
  if(b==BTN_LEFT||b==BTN_RIGHT)g->selection=1-g->selection;
  else if(b==BTN_OK)rest_pick(g,g->selection);
  break;
 case SHOP:
  if(b==BTN_ALT){g->shop_selection=g->selection;g->deck_readonly=1;
    g->prior_screen=SHOP;g->screen=DECK_VIEW;g->selection=0;g->scroll=0;}
  else if(b==BTN_POTION||b==BTN_START){g->shop_selection=g->selection;g->prior_screen=SHOP;g->screen=POTION_VIEW;g->selection=0;msg(g,"");}
  else if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(6,g->selection+1);
  else if(b==BTN_OK)shop_buy(g,g->selection);
  else if(b==BTN_BACK)enter_map(g);
  break;
 case EVENT:
  if(b==BTN_LEFT||b==BTN_UP)g->selection=(g->selection+game_event_choice_count(g)-1)%game_event_choice_count(g);
  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%game_event_choice_count(g);
  else if(b==BTN_BACK)do_event(g,game_event_choice_count(g)-1);
  else if(b==BTN_OK)do_event(g,g->selection);
  break;
 case EVENT_CARD:
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,g->deck_count);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)g->selection++;
  else if(b==BTN_BACK){g->screen=EVENT;g->selection=g->shop_selection;msg(g,"");}
  else if(b==BTN_OK)event_card_commit(g);
  break;
 case DECK_VIEW:
  if(g->prior_screen==COMBAT){
    if(b==BTN_LEFT){g->scroll=(g->scroll+3)%4;g->selection=0;}
    else if(b==BTN_RIGHT){g->scroll=(g->scroll+1)%4;g->selection=0;}
    else if(b==BTN_BACK){
      g->screen=COMBAT;
      g->selection=MIN(g->master_index,g->hand_count);
    }else{
      int count=g->scroll==0?g->draw_count:
         g->scroll==1?g->hand_count:
         g->scroll==2?g->discard_count:g->exhaust_count;
      if(b==BTN_UP&&g->selection>=3)g->selection-=3;
      if(b==BTN_DOWN)g->selection=grid_down(g->selection,count);
    }
    break;
  }
  if(b==BTN_UP&&g->selection>=3)g->selection-=3;
  else if(b==BTN_DOWN)g->selection=grid_down(g->selection,g->deck_count);
  else if(b==BTN_LEFT&&g->selection%3>0)g->selection--;
  else if(b==BTN_RIGHT&&g->selection%3<2&&g->selection+1<g->deck_count)
    g->selection++;
  else if(b==BTN_BACK){
    if(g->prior_screen==MAP||g->prior_screen==EVENT)enter_map(g);
    else{g->screen=g->prior_screen;g->selection=(g->prior_screen==SHOP||g->prior_screen==REWARD)?g->shop_selection:0;g->deck_readonly=0;}
   }
  else if(b==BTN_OK){
   if((g->prior_screen==REST||g->prior_screen==EVENT)&&g->deck_count>0){
    int idx=g->selection;
    if(idx<0||idx>=g->deck_count)break;
    if(!game_card_is_upgradeable(g->deck[idx])){
      msg(g,"Card cannot be upgraded again.");break;
    }
    g->deck[idx]|=UPBIT;
    if(g->prior_screen==EVENT && --g->next_screen>0 && count_upgradeable(g)>0){
      snprintf(g->message,sizeof(g->message),"Upgraded %s. Choose %d more.",
               cards[CARD_ID(g->deck[idx])].name,g->next_screen);
    }else{
      char result[72];
      snprintf(result,sizeof(result),"Upgraded %s.",cards[CARD_ID(g->deck[idx])].name);
      enter_map(g);msg(g,result);
    }
   }
   else if(g->prior_screen==SHOP&&!g->deck_readonly&&g->deck_count>1){if(g->gold<60){g->screen=SHOP;msg(g,"Not enough gold for removal.");break;}n=g->selection;g->gold-=60;for(int i=n;i<g->deck_count-1;i++)g->deck[i]=g->deck[i+1];g->deck_count--;g->screen=SHOP;g->selection=4;msg(g,"Card removed. 60 gold paid.");}
  }break;
 case BOSS_REWARD:
  if(b==BTN_DOWN)g->selection=3;
  else if(b==BTN_UP&&g->selection==3)g->selection=1;
  else if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(3,g->selection+1);
  else if(b==BTN_OK||b==BTN_BACK){
    int id=(b==BTN_OK&&g->selection<3)?g->reward[g->selection]:-1;
    if(id>=0)add_relic(g,id);
    g->win_screen=(!g->endless&&g->act>=3?2:1);fill_rewards(g);
    g->screen=REWARD;g->selection=0;
  }
  break;
 case RELIC_RESULT:
  if(b==BTN_LEFT||b==BTN_RIGHT)g->selection=1-g->selection;
  else if(b==BTN_OK||b==BTN_BACK){
    if(g->relic_pending&&b==BTN_OK&&g->selection==0&&g->reward[0]>=0)
      add_relic(g,g->reward[0]);
    g->relic_pending=0;
    if(g->win_screen||g->relic_resume==REWARD){
      fill_rewards(g);g->selection=0;g->screen=REWARD;
      msg(g,"Choose a card or skip.");
    }else enter_map(g);
    g->relic_resume=0;
  }
  break;
 case BLESSING:
  if(b==BTN_LEFT)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_RIGHT)g->selection=MIN(2,g->selection+1);
  else if(b==BTN_OK){int gift=g->reward[g->selection];if(gift>=1000){int card=gift-1000,replaced=0;if(card==R_METEOR_SHOWER)for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==R_FALLING_STAR){g->deck[i]=(g->deck[i]&~511)|R_METEOR_SHOWER;replaced=1;break;}if(card==N_SUPPRESS)for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])==N_NEUTRALIZE){g->deck[i]=(g->deck[i]&~511)|N_SUPPRESS;replaced=1;break;}if(!replaced){if(g->deck_count>=MAX_DECK){msg(g,"Deck full. Choose another blessing.");break;}add_card(g,card);}}else add_relic(g,gift);enter_map(g);}
  break;
 case REWARD_PREVIEW:
  if(b==BTN_LEFT)g->selection=(g->selection+2)%3;
  else if(b==BTN_RIGHT)g->selection=(g->selection+1)%3;
  else if(b==BTN_BACK||b==BTN_POTION||b==BTN_OK)g->screen=REWARD;
  break;
 case POTION_VIEW:
  if(b==BTN_UP){g->selection=MAX(0,g->selection-1);msg(g,"");}
  else if(b==BTN_DOWN){g->selection=MIN(MAX(0,g->potion_count-1),g->selection+1);msg(g,"");}
  else if(b==BTN_OK){game_use_potion(g,g->selection);g->selection=MIN(g->selection,MAX(0,g->potion_count-1));}
  else if(b==BTN_BACK||b==BTN_POTION||b==BTN_START){g->screen=g->prior_screen==MAP?MAP:SHOP;g->selection=g->shop_selection;msg(g,"");}
  break;
 case PAUSE_MENU:
   if(b==BTN_UP||b==BTN_DOWN)paused_selection=1-paused_selection;
   else if(b==BTN_BACK||(b==BTN_OK&&paused_selection==1))g->screen=paused_from_screen;
   break;
 case EVENT_RESULT:
   if(b==BTN_OK||b==BTN_BACK)enter_map(g);
   break;
 case RELIC_VIEW:
  if(b==BTN_UP)g->selection=MAX(0,g->selection-1);
  else if(b==BTN_DOWN)g->selection=MIN(MAX(0,g->relic_count-1),g->selection+1);
  else if(b==BTN_BACK||b==BTN_OK)enter_map(g);
  break;
 case VICTORY:
  if(b==BTN_LEFT||b==BTN_RIGHT||b==BTN_UP||b==BTN_DOWN)g->selection=1-CLAMP(g->selection,0,1);
  else if(b==BTN_BACK||(b==BTN_OK&&g->selection==1)){uint32_t seed=game_random(g);game_new(g,seed);}
  else if(b==BTN_OK){
   g->endless=1;g->hp=g->maxhp;g->act++;g->floor=0;g->lane=1;g->win_screen=0;g->relic_pending=0;
   map_generate(g);begin_act(g);
  }
  break;
 case DEFEAT:
  if(b==BTN_OK||b==BTN_BACK){uint32_t seed=game_random(g);game_new(g,seed);}
  break;
 }
}
static int valid_card_raw(int raw){
 int id=CARD_ID(raw),mod=CARD_MOD(raw);
 if(raw<0||(raw&~CARD_RAW_MASK)||id>=CARD_COUNT||mod>=MOD_COUNT)return 0;
 if(mod&&!game_can_modify(raw&~(CARD_MOD_MASK|UPBIT),mod))return 0;
 return 1;
}
int game_validate(const Game*g){
 if(!g||g->character<0||g->character>2||g->ninja.task_count<0||g->ninja.task_count>NINJA_TASK_MAX||g->endless<0||g->endless>1||g->magic!=0x52494654u||g->version!=GAME_VERSION)return 0;
 if(g->ascent.id){
  if(g->ascent.slot>=4||g->ascent.cards>ASCENT_ALL_CARDS||g->ascent.potions>0xfffffu||g->ascent.relics>1023||g->ascent.secret>1||g->ascent.won>1||g->ascent.ascended>1)return 0;
  for(int i=0;i<2;i++){int a=g->ascent.unlocked_cards[i],b=g->ascent.unlocked_potions[i];if(a!=-1&&(a<334+20*g->character||a>=354+20*g->character))return 0;if(b!=-1&&(b<6||b>=26))return 0;}
  if(g->ascent.unlocked_relic!=-1&&(g->ascent.unlocked_relic<64||g->ascent.unlocked_relic>=74))return 0;
 }
 if(g->revision182.rare_offset< -5||g->revision182.rare_offset>40||g->revision182.room_pending<0||g->revision182.room_pending>1||g->revision182.room_type<0||g->revision182.room_type>NODE_BOSS)return 0;
 if(g->revision182.event_seen_hi>127u)return 0;
 const int next_limits[5]={30,99,5,5,5};for(int i=0;i<5;i++)if(g->revision182.next_combat[i]<0||g->revision182.next_combat[i]>next_limits[i])return 0;
 if(g->act<1||g->act>100000||(!g->endless&&g->act>3)||g->floor<0||g->floor>MAP_ROWS||g->lane<0||g->lane>=3)return 0;
 if(g->maxhp<=0||g->hp<0||g->hp>g->maxhp||g->gold<0)return 0;
 if(g->screen<TITLE||g->screen>MYSTERY_CARDS||g->deck_count<1||g->deck_count>MAX_DECK)return 0;
 if((g->screen==STORY_ACT||g->screen==STORY_ACT_BEFORE_BLESSING)&&(g->selection!=0||g->scroll<0||g->scroll>9))return 0;
 if(g->screen==STORY_BOSS&&(g->act>3||g->selection<0||g->selection>2||g->scroll<0||g->scroll>4))return 0;
 if(g->hand_count<0||g->hand_count>MAX_HAND||g->draw_count<0||g->draw_count>MAX_PILE||g->discard_count<0||g->discard_count>MAX_PILE||g->exhaust_count<0||g->exhaust_count>MAX_PILE)return 0;
 if(g->enemy_count<0||g->enemy_count>MAX_ENEMIES||g->potion_count<0||g->potion_count>MAX_POTIONS)return 0;
 for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].art<0||g->enemies[i].art>=ENEMY_COUNT||g->enemies[i].maxhp<1||g->enemies[i].hp>g->enemies[i].maxhp||g->enemies[i].intent<0||g->enemies[i].intent>=INTENT_COUNT)return 0;
 if(g->relic_count<0||g->relic_count>MAX_RELICS)return 0;
 for(int i=0;i<g->deck_count;i++)if(!valid_card_raw(g->deck[i]))return 0;
 for(int i=0;i<g->hand_count;i++)if(!valid_card_raw(g->hand[i]))return 0;
 for(int i=0;i<g->draw_count;i++)if(!valid_card_raw(g->draw[i]))return 0;
 for(int i=0;i<g->discard_count;i++)if(!valid_card_raw(g->discard[i]))return 0;
 for(int i=0;i<g->exhaust_count;i++)if(!valid_card_raw(g->exhaust[i]))return 0;
 for(int i=0;i<g->relic_count;i++)if(game_relic_id(g,i)<0||game_relic_id(g,i)>=RELIC_COUNT)return 0;
 if(g->expedition.story_seen>0xfffffu||g->expedition.story_pending<0||g->expedition.story_pending>1||g->expedition.story_id<0||g->expedition.story_id>=20)return 0;
 if(expedition_scene(g->screen)&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==EXPEDITION_GIFT&&(g->selection<0||g->selection>1||g->expedition.gift_kind<0||g->expedition.gift_kind>1))return 0;
 if(g->screen==EXPEDITION_GIFT){int id=g->expedition.gift;if(g->expedition.gift_kind?(id<0||id>=RELIC_COUNT):!valid_card_raw(id))return 0;}
 if(g->screen==MYSTERY_CARDS){if(g->selection<0||g->selection>3)return 0;for(int i=0;i<3;i++)if(!valid_card_raw(g->reward[i])||cards[CARD_ID(g->reward[i])].rarity!=2)return 0;}
 for(int i=0;i<g->potion_count;i++)if(g->potions[i]<0||g->potions[i]>25)return 0;
 for(int r=0;r<MAP_ROWS;r++)for(int c=0;c<MAP_LANES;c++)if(g->map[r][c]<NODE_FIGHT||g->map[r][c]>NODE_BOSS)return 0;
 if(g->screen==CARD_CHOICE&&
 ((g->next_screen==25?g->discard_count:g->hand_count)<1||g->selection<0||
 g->selection>=(g->next_screen==25?g->discard_count:g->hand_count)||
 (g->next_screen!=17&&g->next_screen!=18&&g->next_screen!=23&&g->next_screen!=25&&g->next_screen!=43)))return 0;
 if(g->ninja.nightmare_count<0||g->ninja.nightmare_count>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.nightmare_count;i++)if(!valid_card_raw(g->ninja.nightmare_raw[i])||g->ninja.nightmare_n[i]<1||g->ninja.nightmare_n[i]>MAX_HAND)return 0;
 for(int i=0;i<g->ninja.task_count;i++){const NinjaTask*t=&g->ninja.tasks[i];if(t->kind<NT_PLAY||t->kind>NT_ENDTURN||!valid_card_raw(t->raw)||t->target<0||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->ninja.choice_count;i++)if(i>=MAX_HAND||!valid_card_raw(g->ninja.choice_raws[i]))return 0;
 if(g->screen==CHARACTER_SELECT&&(g->selection<0||g->selection>2))return 0;
 for(int i=0;i<10;i++)if(g->regent.automation_phase[i]<0||g->regent.automation_phase[i]>1000000)return 0;
 for(int i=0;i<5;i++)if(g->regent.panache_phase[i]<0||g->regent.panache_phase[i]>100000000)return 0;
 if(g->regent.option_count<0||g->regent.option_count>MAX_PILE||g->regent.return_count<0||g->regent.return_count>MAX_PILE||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX||g->regent.shuffle_pending<0||g->regent.shuffle_pending>1||g->regent.shuffle_draws<0||g->regent.shuffle_draws>MAX_PILE)return 0;
 for(int i=0;i<g->regent.task_count;i++){const RegentTask*t=&g->regent.tasks[i];if(t->kind<RT_PLAY||t->kind>RT_SEEK_THREE||!valid_card_raw(t->raw)||t->target< -1||t->target>=MAX_ENEMIES||t->n<0||t->n>1000000||t->aux<0||t->aux>1000000||t->depth<0||t->depth>3)return 0;}
 for(int i=0;i<g->regent.option_count;i++)if(!valid_card_raw(g->regent.options[i]))return 0;
 for(int i=0;i<g->regent.return_count;i++)if(!valid_card_raw(g->regent.return_cards[i]))return 0;
 for(int i=0;i<512;i++)if(g->regent.instance_cost[i]<0||g->regent.instance_cost[i]>99||g->regent.instance_damage[i]<0||g->regent.instance_damage[i]>1000000||g->regent.instance_replay[i]<0||g->regent.instance_replay[i]>20)return 0;
 for(int i=0;i<EXTRA_LAST-REGENT_FIRST;i++)if(g->regent.powers[i]<0||g->regent.powers[i]>1000000||g->regent.played[i]<0||g->regent.played[i]>1000000)return 0;
 if(g->screen==REGENT_CHOICE&&(g->regent.choice_kind<1||g->regent.choice_kind>RC_SEEK_THREE||g->regent.choice_remaining<1||g->selection<0||g->selection>=game_regent_choice_count(g)))return 0;
 if(g->regent.stars<0||g->regent.stars>999||g->regent.task_count<0||g->regent.task_count>REGENT_TASK_MAX)return 0;
 if(g->screen==NINJA_CHOICE&&(g->ninja.choice_kind<1||g->ninja.choice_kind>4||g->ninja.choice_remaining<1||g->ninja.choice_count<0||g->ninja.choice_count>MAX_HAND||g->selection<0||g->selection>=g->hand_count))return 0;
 if(g->screen==ASCENT_COLLECTION&&(g->scroll<0||g->scroll>=5||g->selection<0||g->selection>=game_collection_count(g->scroll)))return 0;
 if(g->screen==MAP&&(g->selection<0||g->selection>=MAP_LANES))return 0;
 if(g->screen==COMBAT&&(g->selection<0||g->selection>g->hand_count||g->target<0||g->target>=MAX(1,g->enemy_count)))return 0;
 if(g->v151.potion_offer< -1||g->v151.potion_offer>25||g->v151.card_done<0||g->v151.card_done>1)return 0;
 if(g->screen==REWARD&&(g->selection<0||g->selection>5))return 0;
 if(g->screen==BOSS_REWARD&&(g->selection<0||g->selection>3))return 0;
 if((g->screen==BLESSING||g->screen==REWARD_PREVIEW)&&(g->selection<0||g->selection>2))return 0;
 if(g->screen==BLESSING)for(int i=0;i<3;i++)if(!((g->reward[i]>=1&&g->reward[i]<63&&g->reward[i]!=44&&(g->reward[i]<45||g->reward[i]>52||g->character==CHAR_NIGHTSHADE))||(g->character==CHAR_REGENT&&(g->reward[i]==1000+R_METEOR_SHOWER||g->reward[i]==1000+R_THE_SEALED_THRONE))||(g->character==CHAR_NIGHTSHADE&&(g->reward[i]==NR_DRAKE||g->reward[i]==1000+N_SUPPRESS||g->reward[i]==1000+N_WRAITH_FORM))))return 0;
 if(g->screen==REWARD_PREVIEW)for(int i=0;i<3;i++)if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;
 if(g->screen==RELIC_RESULT&&(g->selection<0||g->selection>1||g->reward[0]<-1||g->reward[0]>=RELIC_COUNT))return 0;
 if(g->screen==REST&&(g->selection<0||g->selection>1))return 0;
 if(g->screen==EVENT&&(g->selection<0||g->selection>=game_event_choice_count(g)||g->effect_turn<0||g->effect_turn>=EVENT_COUNT))return 0;
 if(g->screen==EVENT_RESULT&&g->reward[0]!=-1&&!valid_card_raw(g->reward[0]))return 0;
 if(g->screen==EVENT&&(g->effect_turn==2||g->effect_turn==6||g->effect_turn==34))for(int i=0;i<2;i++)if(!valid_card_raw(g->reward[i])||!game_card_is_playable(g->reward[i]))return 0;
 if(g->screen==EVENT&&g->effect_turn==17&&(!valid_card_raw(g->reward[0])||!game_card_is_playable(g->reward[0])))return 0;
 if(g->screen==EVENT_CARD&&(g->effect_turn<0||g->effect_turn>=EVENT_COUNT||g->next_screen<1||g->next_screen>EVENT_TRANSFORM||g->selection<0||g->selection>=g->deck_count||g->shop_selection<0||g->shop_selection>=game_event_choice_count(g)-1))return 0;
 if(g->screen==SHOP&&(g->selection<0||g->selection>7))return 0;
 if(g->screen==DECK_VIEW){
   int count=g->prior_screen==COMBAT?
      (g->scroll==0?g->draw_count:
       g->scroll==1?g->hand_count:
       g->scroll==2?g->discard_count:g->exhaust_count):g->deck_count;
   if(g->selection<0||g->selection>=MAX(1,count))return 0;
 }
 if(g->screen==REWARD)for(int i=0;i<3;i++)if(!solo_reward_eligible(CARD_ID(g->reward[i])))return 0;
 if(g->screen==BOSS_REWARD)for(int i=0;i<3;i++)if(g->reward[i]<-1||g->reward[i]>=RELIC_COUNT)return 0;
 return 1;
}
/* Save is a checksummed snapshot: reject truncated/corrupted or incompatible saves. */
static uint32_t hash_bytes(const unsigned char*p,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
int game_save(const Game*g,const char*path){if(!game_validate(g))return 0;FILE*f=fopen(path,"wb");if(!f)return 0;uint32_t check=hash_bytes((const unsigned char*)g,sizeof(*g));int ok=fwrite(&check,4,1,f)==1&&fwrite(g,sizeof(*g),1,f)==1;return fclose(f)==0&&ok;}
#include "save_v20.inc"
#include "save_legacy.inc"
#include "save_v22.inc"
int game_load(Game*g,const char*path){
 unsigned char raw[sizeof(Game)];Game temp;uint32_t check;
 FILE*f=fopen(path,"rb");if(!f)return 0;
 int ok=fread(&check,4,1,f)==1;size_t n=ok?fread(raw,1,sizeof(raw),f):0;
 unsigned char extra;int trailing=fread(&extra,1,1,f);fclose(f);
 if(!ok||trailing!=0||n<16||check!=hash_bytes(raw,n))return 0;
 memset(&temp,0,sizeof(temp));uint32_t version;memcpy(&version,raw+4,4);
 if(version==GAME_VERSION||version==182){if(n!=sizeof(Game))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}
 else if(version==181){if(n!=offsetof(Game,revision182))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}
 else if(version==180){if(n!=offsetof(Game,expedition))return 0;memcpy(&temp,raw,n);if(temp.relic_count>64)return 0;temp.version=GAME_VERSION;}
 else if(version==23){if(n!=offsetof(Game,ascent))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}
 else if(version==22){if(n!=offsetof(Game,regent))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;}
 else if(version==21){if(n!=offsetof(Game,v151))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;temp.v151.potion_offer=-1;}
 else {if(version==20){if(!load_v20(&temp,raw,n))return 0;}else if(!load_legacy_run(&temp,raw,n,version))return 0;if(!migrate_raw_cards(&temp))return 0;temp.v151.potion_offer=-1;}
 if(version<23&&!migrate_raw22(&temp))return 0;
 if(version<182){temp.revision182.rare_offset=-5;revision182_migrate_room(&temp);}
 if(temp.screen==POTION_VIEW&&temp.prior_screen!=MAP)temp.prior_screen=SHOP;
 if(version==180&&temp.screen==RELIC_RESULT&&temp.reward[0]<0)temp.reward[0]=37;
 if(!game_validate(&temp))return 0;
 /* Retire the duplicate blessing without renumbering any saved relic IDs. */
 int count=0,compass=0;
 for(int i=0;i<temp.relic_count;i++){
  int id=game_relic_id(&temp,i)==30?7:game_relic_id(&temp,i);
  if(id==7&&compass)continue;
  if(id==7)compass=1;
  *relic_slot(&temp,count++)=id;
 }
 temp.relic_count=count;
 /* A saved, unclaimed Seer's Oath becomes a repeatable Strength blessing;
    it remains a legal blessing slot even when Compass is already owned. */
 if(temp.screen==BLESSING)for(int i=0;i<3;i++)if(temp.reward[i]==30){
  int replacement=37;
  for(;replacement<40;replacement++){int used=0;for(int j=0;j<3;j++)used|=temp.reward[j]==replacement;if(!used)break;}
  temp.reward[i]=replacement<40?replacement:37;
 }
 if(temp.battle_type==NODE_BOSS&&temp.win_screen&&
    (temp.screen==REWARD||temp.screen==REWARD_PREVIEW||(temp.screen==DECK_VIEW&&temp.prior_screen==REWARD))){
  int invalid=0;for(int i=0;i<3;i++)invalid|=cards[CARD_ID(temp.reward[i])].rarity!=2;
  if(invalid)fill_rewards(&temp);
 }
 *g=temp;return 1;
}

/* Persistent, versioned lifetime profile. One result per unique run seed. */
void profile_init(RiftProfile*p){
 memset(p,0,sizeof(*p));p->magic=0x52415046u;p->version=1;
}
static int profile_valid(const RiftProfile*p){
 return p&&p->magic==0x52415046u&&p->version==1&&
  p->wins<=p->runs&&p->losses<=p->runs&&p->wins+p->losses==p->runs;
}
int profile_record(RiftProfile*p,const Game*g){
 if(!profile_valid(p)||!g||(g->screen!=VICTORY&&g->screen!=DEFEAT))return 0;
 if(p->runs&&p->last_seed==g->seed)return 0;
 p->runs++;
 if(g->screen==VICTORY)p->wins++;else p->losses++;
 unsigned distance=g->screen==VICTORY?3u*(MAP_ROWS+1):(unsigned)MIN(4294967295ULL,(unsigned long long)(g->act-1)*(MAP_ROWS+1)+g->floor);
 if(distance>p->furthest_floor)p->furthest_floor=distance;
 if(g->kill_count>0)p->total_kills+=(uint32_t)g->kill_count;
 if(g->kill_count>(int)p->best_kills)p->best_kills=(uint32_t)g->kill_count;
 if(g->deck_count>(int)p->best_deck)p->best_deck=(uint32_t)g->deck_count;
 p->last_seed=g->seed;p->last_result=(uint32_t)g->screen;
 return 1;
}
int profile_save(const RiftProfile*p,const char*path){
 if(!profile_valid(p))return 0;
 FILE*f=fopen(path,"wb");if(!f)return 0;
 uint32_t checksum=hash_bytes((const unsigned char*)p,sizeof(*p));
 int ok=fwrite(&checksum,sizeof(checksum),1,f)==1&&fwrite(p,sizeof(*p),1,f)==1;
 return fclose(f)==0&&ok;
}
int profile_load(RiftProfile*p,const char*path){
 RiftProfile stored;uint32_t checksum;
 FILE*f=fopen(path,"rb");if(!f)return 0;
 int ok=fread(&checksum,sizeof(checksum),1,f)==1&&fread(&stored,sizeof(stored),1,f)==1;
 if(fclose(f)!=0)ok=0;
 if(!ok||checksum!=hash_bytes((const unsigned char*)&stored,sizeof(stored))||!profile_valid(&stored))return 0;
 *p=stored;return 1;
}
/* Temporary status and persistent Powers are separate; each Power appears once. */
static int power_present(const Game*g,int id){
 if(id>=ASCENT_CARD_FIRST)return 0;
 if(id>=REGENT_FIRST)return g->regent.played[id-REGENT_FIRST];
 if(id>=NINJA_FIRST&&id<NINJA_LAST)return g->ninja.powers[id-NINJA_FIRST];
 if(g->ic[id>=93?47:33+id/31]&(int)(1u<<(id%31)))return 1;
 switch(id){case 20:return g->shop_cards[2];case 28:return g->used_potion;case 41:return g->shop_cards[0];case 48:return g->heal_after_battle;
 case 95:return g->ic[IC_COVENANT];case 54:return g->ic[IC_AGGRESSION];case 55:return g->ic[IC_BARRICADE];case 59:return g->ic[IC_CORRUPTION];case 60:return g->ic[IC_CRIMSON_COPIES];case 61:return g->ic[IC_CRUELTY];case 62:return g->ic[IC_DARK_EMBRACE];case 69:return g->ic[IC_HELLRAISER];case 72:return g->ic[IC_JUGGERNAUT];case 73:return g->ic[IC_JUGGLING];case 78:return g->ic[IC_PYRE];case 82:return g->ic[IC_STAMPEDE];case 84:return g->ic[IC_PLATING];case 85:return g->ic[IC_TANK];case 88:return g->ic[IC_UNMOVABLE];case 90:return g->ic[IC_VICIOUS];default:return 0;}
}
static void power_line(const Game*g,int id,char*out,int cap){
 if(id>=ASCENT_CARD_FIRST){if(cap>0)out[0]=0;return;}
 if(id>=REGENT_FIRST){int n=g->regent.powers[id-REGENT_FIRST];const char*rule="active";
 switch(id){
 case R_ARSENAL:rule="+%d Strength per created card";break;
 case R_BLACK_HOLE:rule="%d damage/all per Star gain or spend";break;
 case R_CHILD_OF_THE_STARS:rule="%d Block for each Star spent";break;
 case R_FURNACE:rule="Forge %d each turn start";break;
 case R_GENESIS:rule="+%d Stars each turn start";break;
 case R_HAMMER_TIME:rule="Forge gains %d additional copies";break;
 case R_MONARCHS_GAZE:rule="each hit: foe loses %d Strength this turn";break;
 case R_NEUTRON_AEGIS:case C_ETERNAL_ARMOR:rule="Plating granted; see current Plating";break;
 case R_ORBIT:rule="gain %d Energy per 4 Energy spent";break;
 case R_PALE_BLUE_DOT:rule="5+ plays: draw %d extra next turn";break;
 case R_PARRY:rule="Sovereign Blade gains %d Block";break;
 case R_PILLAR_OF_CREATION:rule="%d Block per created card";break;
 case R_ROYALTIES:rule="gain %d Gold at combat end";break;
 case R_SEEKING_EDGE:rule="Sovereign Blade hits ALL enemies";break;
 case R_SPECTRUM_SHIFT:rule="create %d Colorless cards/turn";break;
 case R_SWORD_SAGE:rule="Sovereign Blade replays %d times";break;
 case R_THE_SEALED_THRONE:rule="gain %d Stars per card played";break;
 case R_TYRANNY:rule="draw %d, then Exhaust that many/turn";break;
 case R_VOID_FORM:n*=2;rule="first %d manual plays/turn are free";break;
 case C_AUTOMATION:rule="gain %d Energy per 10 cards drawn";break;
 case C_CALAMITY:rule="create %d Attacks per Attack played";break;
 case C_ENTROPY:rule="Transform %d cards each turn start";break;
 case C_FASTEN:rule="Defend cards gain +%d Block";break;
 case C_MAYHEM:rule="autoplay %d top draw cards/turn";break;
 case C_NOSTALGIA:rule="first Attack/Skill goes atop Draw";break;
 case C_PANACHE:rule="%d damage/all per 5 cards this turn";break;
 case C_PREP_TIME:rule="gain %d Vigor each turn start";break;
 case C_PROWESS:rule="Strength and Dexterity granted";break;
 case C_ROLLING_BOULDER:rule="%d damage/all next turn; grows +5/copy";break;
 case C_STRATAGEM:rule="pick %d draw cards after a shuffle";break;
 }
 char detail[150];snprintf(detail,sizeof(detail),rule,n);snprintf(out,cap,"%s: %s",cards[id].name,detail);return;}
 if(id>=NINJA_FIRST&&id<NINJA_LAST){
 const char*rule="active";int n=g->ninja.powers[id-NINJA_FIRST];
 switch(id){
 case N_ABRASIVE:n=g->ninja.v[NS_THORNS];rule="%d Thorns; reflects each enemy hit";break;
 case N_ACCELERANT:n=g->ninja.v[NS_ACCELERANT]+1;rule="Poison triggers %d times";break;
 case N_ACCURACY:n=g->ninja.v[NS_ACCURACY];rule="Shivs gain %d damage";break;
 case N_AFTERIMAGE:n=g->ninja.v[NS_AFTERIMAGE];rule="%d Block per card played";break;
 case N_ENVENOM:n=g->ninja.v[NS_ENVENOM];rule="%d Poison per unblocked Attack hit";break;
 case N_FAN_OF_KNIVES:rule="Shivs hit ALL enemies";break;
 case N_FOOTWORK:rule="Dexterity granted; see current Dexterity";break;
 case N_INFINITE_BLADES:n=g->ninja.v[NS_BLADES];rule="%d Shivs each turn start";break;
 case N_MASTER_PLANNER:rule="played Skills permanently gain Sly";break;
 case N_NOXIOUS_FUMES:n=g->ninja.v[NS_FUMES];rule="%d Poison to all each turn start";break;
 case N_OUTBREAK:n=g->ninja.v[NS_OUTBREAK];rule="%d damage/all per 3 Poison applications";break;
 case N_PHANTOM_BLADES:n=g->ninja.v[NS_PHANTOM];rule="Retain Shivs; +%d damage on first/turn";break;
 case N_SERPENT_FORM:n=g->ninja.v[NS_SERPENT];rule="%d random damage per card played";break;
 case N_SPEEDSTER:n=g->ninja.v[NS_SPEEDSTER];rule="%d damage/all per extra card drawn";break;
 case N_TOOLS_OF_THE_TRADE:n=g->ninja.v[NS_TOOLS];rule="draw/discard %d each turn start";break;
 case N_TRACKING:n=1<<MIN(20,g->ninja.v[NS_TRACKING]);rule="Attacks deal %dx damage to Weak foes";break;
 case N_WELL_LAID_PLANS:n=g->ninja.v[NS_PLANS];rule="Retain up to %d cards each turn";break;
 case N_WRAITH_FORM:n=g->ninja.v[NS_WRAITH];rule="lose %d Dexterity each turn start";break;
 }
 char detail[140];snprintf(detail,sizeof(detail),rule,n);snprintf(out,cap,"%s: %s",cards[id].name,detail);return;
 }
 const char*fmt=0;int a=0,b=0;
 switch(id){
 case 95:fmt="Black Covenant: %d damage/foe per saved Energy (max 2)";a=g->ic[IC_COVENANT];break;
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
 case 90:fmt="Vicious: draw %d when applying Vulnerable";a=g->ic[IC_VICIOUS];break;
 case 88:fmt=g->ic[IC_UNMOVABLE_USED]?"Unmovable: already used this turn":"Unmovable: next card Block gain doubled";break;
 default:snprintf(out,(size_t)cap,"%s: played this combat",cards[id].name);return;
 }
 snprintf(out,(size_t)cap,fmt,a,b);
}
static int status_entries(const Game*g,int wanted,char*out,int cap){
 int n=0;
#define STATUS_FLAG(value,label) do {if(value){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,"%s",label);n++;}}while(0)
#define STATUS(value,format) do {int v=(value);if(v){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,format,v);n++;}}while(0)
 STATUS(g->revision182.next_combat[0],"Next combat: +%d Strength for that fight");
 STATUS(g->revision182.next_combat[1],"Next combat: +%d starting Block");
 STATUS(g->revision182.next_combat[2],"Next combat: draw %d extra opening cards");
 STATUS(g->revision182.next_combat[3],"Next combat: start with %d Weak");
 STATUS(g->revision182.next_combat[4],"Next combat: start with %d Vulnerable");
 if(g->character==CHAR_REGENT){STATUS(g->regent.stars,"Stars: %d available; saved between turns");STATUS(g->regent.forge,"Forge: +%d Sovereign Blade damage");STATUS(g->regent.created,"Cards created this combat: %d");STATUS(g->regent.vigor,"Vigor: +%d damage on next Attack");STATUS(g->regent.next_stars,"Gain %d Stars next turn");STATUS(g->regent.next_energy,"Gain %d Energy next turn");STATUS(g->regent.next_draw,"Draw %d extra cards next turn");STATUS_FLAG(g->regent.reflect,"Reflect: return blocked attack damage");STATUS(g->regent.panic,"Panic: no card Block for %d turn starts");}
 STATUS(g->strength,"Strength: %d damage per hit");
 STATUS(g->dexterity,"Dexterity: %d Block from cards");
 STATUS(g->weak,"Weak: %d turns; attacks deal 25%% less");
 STATUS(g->vulnerable,"Vulnerable: %d turns; take 50%% more damage");
 STATUS(g->poison,"Poison: lose %d HP at turn end");
 STATUS(g->energy_penalty,"Energy drain: -%d Energy next turn");
 STATUS(g->ic[IC_HOURGLASS],"Hourglass: %d saved Energy next turn");
 if(relic(g,40)){char label[80];snprintf(label,sizeof(label),"Hourglass: will save %d Energy if ending now",CLAMP(g->energy,0,2));STATUS_FLAG(g->screen==COMBAT,label);}
 if(relic(g,41)){char label[80];snprintf(label,sizeof(label),"Ossuary Bell: %d of 3 triggers left",MAX(0,3-g->ic[IC_BELL]));STATUS_FLAG(g->screen==COMBAT,label);}
 STATUS_FLAG(relic(g,42)&&!g->ic[IC_NEEDLE],"Needle: next self-inflicted HP loss draws 1");
 STATUS_FLAG(relic(g,43)&&!(g->ic[IC_DUELIST]&4),"Duelist: play Attack + Skill for 1 Energy");
 STATUS(g->shop_cards[1],"Flame Barrier: reflect %d per hit this turn");
 STATUS(g->reward_potion>>8,"Rage: +%d Block per Attack this turn");
 STATUS_FLAG(g->ic[IC_COLOSSUS],"Colossus: halve damage from Vulnerable foes");
 STATUS_FLAG(g->reward_potion&1,"Draw locked for this turn");
 STATUS(g->ic[IC_RAMPAGE],"Rampage: +%d damage this combat");
 STATUS_FLAG(g->ic[IC_ENERGY_LOCK],"Energy locked for this turn");
 STATUS(g->ic[IC_TEMP_STRENGTH],"Temporary Strength: %d expires this turn");
 STATUS_FLAG(g->ic[IC_UNRELENTING],"Unrelenting: next Attack costs 0");
 STATUS(g->ic[IC_NEXT_ATTACK_REPEAT],"One-Two Punch: repeat next %d Attack(s)");
 STATUS(g->ninja.v[NS_INTANGIBLE],"Intangible: damage capped at 1 for %d turns");
 STATUS(g->ninja.v[NS_TEMP_DEX],"Temporary Dexterity: %d expires this turn");
 STATUS(g->ninja.v[NS_BLUR],"Blur: keep Block for %d turn starts");
 STATUS(g->ninja.v[NS_BURST],"Burst: next %d Skills play twice");
 STATUS_FLAG(g->ninja.v[NS_POUNCE],"Pounce: next Skill costs 0");
 STATUS_FLAG(g->ninja.v[NS_SHADOWMELD],"Shadowmeld: double Block gained this turn");
 STATUS_FLAG(g->ninja.v[NS_SHADOW_NOW],"Shadow Step: double Attack damage this turn");
 STATUS_FLAG(g->ninja.v[NS_SHADOW_NEXT],"Shadow Step: double Attack damage next turn");
 STATUS(g->ninja.v[NS_CORROSIVE],"Corrosive Wave: draw applies %d Poison to all");
 STATUS(g->ninja.v[NS_NEXT_DRAW],"Predator: draw %d extra cards next turn");
 STATUS(g->ninja.v[NS_NEXT_BLOCK],"Dodge and Roll: %d base Block next turn");
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
int game_status_count(const Game*g){return status_entries(g,-1,0,0);}
void game_status_line(const Game*g,int index,char*out,int cap){
 if(out&&cap>0)out[0]=0;status_entries(g,index,out,cap);
}

#include "card_preview.inc"

#include "ascent_storage.inc"
#include "ascent_flow.inc"
#include "expedition_flow.inc"
