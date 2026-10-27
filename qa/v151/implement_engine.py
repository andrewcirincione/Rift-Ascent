from pathlib import Path
q=Path(__file__).parent/'source'
def edit(name,old,new,count=1):
 p=q/name;s=p.read_text();assert s.count(old)==count,(name,old[:60],s.count(old));p.write_text(s.replace(old,new))
edit('game.h','#define GAME_VERSION 21','#define GAME_VERSION 22')
edit('game.h',' int character;NinjaState ninja;',' int character;NinjaState ninja;\n /* Appended to preserve the complete v21 prefix for migration. */\n struct {int potion_offer,card_done;int relic[16];} v151;')
edit('game.h','void game_use_potion(Game *g,int index);','void game_use_potion(Game*g,int index);\nint game_inventory_available(const Game*g);\nvoid game_discard_potion(Game*g,int index);')
# Relics fit the existing 64-slot save array.
entries=''' {"Bloodglass Phial","First self-inflicted HP loss on your turn: gain 1 Energy and 4 Block."},
 {"Quiet Bell","End a turn without playing an Attack: gain 1 extra Energy next turn."},
 {"Twinfang Brooch","Every third Attack played each turn applies 1 Poison to all enemies."},
 {"Sepulchral Spindle","After exhausting 3 cards in a combat, draw 2 and gain 1 Energy. Once per combat."},
 {"Hollow Chalice","After combat, heal 2 HP per Power played, up to 6 HP."},
 {"Mourner's Lens","First card each turn that adds enemy Weak, Vulnerable or Poison: draw 1 card."},
 {"Briar Rosary","End your turn with at least 20 Block: apply 2 Poison to all enemies."},
 {"Grave Ledger","Skipping a card reward grants 8 Gold. Every third skip upgrades a random unupgraded card."},
 {"Thornbound Seal","Start combat with 1 extra Strength per curse or status card in your deck."},
 {"Prism Bone","Play an Attack, Skill and Power in one combat: gain 2 Energy and draw 2. Once per combat."},
'''
edit('game.c',' {"Drake Ring","Draw 2 extra cards on each of your first 3 turns. Replaces Serpent Ring."},',' {"Drake Ring","Draw 2 extra cards on each of your first 3 turns. Replaces Serpent Ring."},\n'+entries)
edit('game.c','static int ninja_owned_card(const Game*g,int id);','static int ninja_owned_card(const Game*g,int id);\nstatic void r151_self_loss(Game*g);\nstatic void r151_exhaust(Game*g);\nstatic void r151_played(Game*g,int type);\nstatic void r151_status_snapshot(const Game*g,int out[6]);\nstatic void r151_after_effect(Game*g,const int before[6]);\nstatic void r151_skip_reward(Game*g);')
edit('game.c','(id<=27||(id>=40&&id<44)||(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51))','(id<=27||(id>=40&&id<44)||(id>=53&&id<63)||(g->character==CHAR_NIGHTSHADE&&id>=45&&id<=51))')
edit('game.c',' player_death(g);\n if(n>0&&g->screen==COMBAT&&g->ic[IC_OWN_TURN]&&relic(g,42)',' player_death(g);\n if(n>0)r151_self_loss(g);\n if(n>0&&g->screen==COMBAT&&g->ic[IC_OWN_TURN]&&relic(g,42)')
edit('game.c',' g->ic[37]=0;\n g->kill_count',' g->ic[37]=0;g->v151.card_done=0;g->v151.potion_offer=-1;\n if(relic(g,57))heal(g,2*MIN(3,g->v151.relic[4]));\n g->kill_count')
# Roll offer once per fight, even when the belt is full. Boss rewards follow relic choice.
edit('game.c',' g->gold+=g->reward_gold+(relic(g,16)?20:0);',' if(rnd(g,100)<22)g->v151.potion_offer=rnd(g,6);\n g->gold+=g->reward_gold+(relic(g,16)?20:0);')
edit('game.c','fill_rewards(g);if(g->potion_count<MAX_POTIONS&&rnd(g,100)<22){int potion=rnd(g,6);g->potions[g->potion_count++]=potion;g->ic[37]=potion+1;}g->screen=REWARD;g->selection=0;','fill_rewards(g);g->screen=REWARD;g->selection=0;')
edit('game.c',' memset(&g->ninja,0,sizeof(g->ninja));',' memset(&g->ninja,0,sizeof(g->ninja));\n {int ledger=g->v151.relic[7];memset(&g->v151,0,sizeof(g->v151));g->v151.relic[7]=ledger;g->v151.potion_offer=-1;}')
edit('game.c',' g->weak=g->vulnerable=g->poison=g->block=0;',' if(relic(g,61))for(int i=0;i<g->deck_count;i++)if(CARD_ID(g->deck[i])>=50&&CARD_ID(g->deck[i])<=53)g->strength++;\n g->weak=g->vulnerable=g->poison=g->block=0;')
edit('game.c',' g->turn++;\n ninja_start_turn_before(g);',' g->turn++;g->v151.relic[0]=g->v151.relic[5]=0;\n ninja_start_turn_before(g);')
edit('game.c','g->energy_penalty=0;g->ic[IC_HOURGLASS]=0;','g->energy_penalty=0;g->ic[IC_HOURGLASS]=0;gain_energy(g,g->v151.relic[1]);g->v151.relic[1]=0;')
edit('game.c',' g->ic[38]=1;',' g->ic[38]=1;\n if(relic(g,54)&&!g->ic[IC_ATTACKS_THIS_TURN])g->v151.relic[1]=1;\n if(relic(g,59)&&g->block>=20)ninja_poison_all(g,2);')
edit('game.c','  g->exhaust[g->exhaust_count++]=raw;','  g->exhaust[g->exhaust_count++]=raw;\n  r151_exhaust(g);')
edit('game.c','#include "ninja_engine.inc"','#include "ninja_engine.inc"\n#include "relics151.inc"')
edit('game.c',' int n=up?3:0;\n switch(id){',' int r151_before[6];r151_status_snapshot(g,r151_before);\n int n=up?3:0;\n switch(id){')
edit('game.c',' if(cards[id].type==2){g->ic[id>=93?47:33+id/31]',' r151_after_effect(g,r151_before);\n if(cards[id].type==2){g->ic[id>=93?47:33+id/31]')
edit('game.c',' for(int repeat=0;repeat<repeat_this_attack',' r151_played(g,cards[id].type);\n for(int repeat=0;repeat<repeat_this_attack')
edit('ninja_engine.inc',' int dmg=ninja_base_damage(g,raw,0),block=ninja_block_base(g,raw);',' int r151_before[6];r151_status_snapshot(g,r151_before);\n int dmg=ninja_base_damage(g,raw,0),block=ninja_block_base(g,raw);')
edit('ninja_engine.inc','#undef NP',' r151_after_effect(g,r151_before);\n#undef NP')
edit('ninja_engine.inc',' /* Only powers that existed before this card fire for this play. */',' r151_played(g,type);\n /* Only powers that existed before this card fire for this play. */')
# Native inventory API: overlays preserve underlying Game screen and selection.
edit('game.c','void game_use_potion(Game*g,int index){','int game_inventory_available(const Game*g){\n switch(g->screen){case COMBAT:case MAP:case SHOP:case REST:case EVENT:case EVENT_CARD:case EVENT_RESULT:case REWARD:case REWARD_PREVIEW:case RELIC_RESULT:case DECK_VIEW:case POTION_VIEW:case RELIC_VIEW:case BLESSING:return 1;default:return 0;}\n}\nvoid game_discard_potion(Game*g,int index){\n if(!game_inventory_available(g)||index<0||index>=g->potion_count)return;\n for(int i=index;i+1<g->potion_count;i++)g->potions[i]=g->potions[i+1];g->potion_count--;msg(g,"Potion discarded.");\n}\nvoid game_use_potion(Game*g,int index){')
edit('game.c',' int outside=g->screen==MAP||g->screen==SHOP||\n   (g->screen==POTION_VIEW&&(g->prior_screen==MAP||g->prior_screen==SHOP));',' int outside=game_inventory_available(g)&&!combat;')
# v21 saves preserve the prefix; older bit migrations remain separate.
edit('game.c',' else {if(version==20){',' else if(version==21){if(n!=offsetof(Game,v151))return 0;memcpy(&temp,raw,n);temp.version=GAME_VERSION;temp.v151.potion_offer=-1;}\n else {if(version==20){')
edit('game.c','if(!migrate_raw_cards(&temp))return 0;}','if(!migrate_raw_cards(&temp))return 0;temp.v151.potion_offer=-1;}')
edit('game.c',' if(g->screen==REWARD&&(g->selection<0||g->selection>3))return 0;',' if(g->v151.potion_offer< -1||g->v151.potion_offer>5||g->v151.card_done<0||g->v151.card_done>1)return 0;\n if(g->screen==REWARD&&(g->selection<0||g->selection>5))return 0;')
edit('events150.inc',' One modification per card.','',6)
