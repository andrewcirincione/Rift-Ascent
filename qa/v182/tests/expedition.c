#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../source/game.c"
static void clean(Game*g,int c){game_new(g,181);game_choose_character(g,c);g->relic_count=0;g->screen=MAP;game_start_battle(g,NODE_FIGHT);g->revision182.room_pending=0;g->hp=50;g->maxhp=100;g->gold=0;g->block=g->strength=g->dexterity=0;g->energy=20;g->target=0;g->hand_count=g->discard_count=0;g->draw_count=20;for(int i=0;i<20;i++)g->draw[i]=c==0?0:c==1?N_STRIKE_SILENT:R_STRIKE_REGENT;g->regent.stars=20;for(int i=0;i<g->enemy_count;i++){g->enemies[i].hp=g->enemies[i].maxhp=10000;g->enemies[i].poison=g->enemies[i].weak=g->enemies[i].vulnerable=g->enemies[i].block=0;}}
static void play(Game*g,int raw){g->hand_count=1;g->hand[0]=raw;g->selection=0;game_play_card(g,0,0);}
static void win(Game*g){for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp=0;check_win(g);}
static int owner(int id){return id<NINJA_FIRST?0:id<NINJA_LAST?1:2;}
int main(void){setbuf(stdout,NULL);char root[]="/tmp/rift-182-expedition-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,b;
 assert(offsetof(Game,expedition)==28224&&RELIC_COUNT==94);
 for(int c=0;c<3;c++){
  clean(&g,c);g.hand_count=3;g.hand[0]=65;g.hand[1]=g.hand[2]=0;g.energy=10;game_play_card(&g,0,0);assert(g.ic[IC_ENERGY_LOCK]&&g.energy==10);int e=g.energy;
  play(&g,6);assert(g.energy==e);play(&g,38);assert(g.energy==e);
  g.potions[0]=4;g.potion_count=1;game_use_potion(&g,0);assert(g.energy==e);
  for(int id=74;id<94;id++)add_relic(&g,id);
  g.expedition.combat[E181_CARDS]=4;r151_played(&g,0);r151_played(&g,2);assert(g.energy==e);
  exhaust_card(&g,64);assert(g.energy==e);
  for(int s=COMBAT;s<=REGENT_CHOICE;s++)if(s==COMBAT||s==CARD_CHOICE||s==NINJA_CHOICE||s==REGENT_CHOICE){g.screen=s;gain_energy(&g,9);assert(g.energy==e);}
  g.screen=COMBAT;game_start_turn(&g);assert(!g.ic[IC_ENERGY_LOCK]);e=g.energy;gain_energy(&g,2);assert(g.energy==e+2);
 }
 puts("PASS Expect a Fight: all characters, Bloodletting, Offering, potion, relics, exhaust, choices; next-turn reset");
 for(int id=74;id<94;id++){
  clean(&g,0);add_relic(&g,id);
  switch(id){
   case 74:for(int i=0;i<4;i++)r151_played(&g,1);assert(g.energy==20);r151_played(&g,1);assert(g.energy==21);break;
   case 75:game_start_battle(&g,NODE_FIGHT);for(int i=0;i<g.enemy_count;i++)assert(g.enemies[i].weak==2);break;
   case 76:g.turn=1;game_start_turn(&g);assert(g.energy==g.max_energy+1);break;
   case 77:play(&g,0);assert(g.enemies[0].weak==1);play(&g,0);assert(g.enemies[0].weak==1);game_start_turn(&g);play(&g,0);assert(g.enemies[0].weak==2);break;
   case 78:damage_player(&g,10);assert(g.hp==40&&g.block==5);damage_player(&g,10);assert(g.hp==35&&g.block==0);break;
   case 79:g.turn=3;game_start_turn(&g);assert(g.strength==2);break;
   case 80:exhaust_card(&g,0);exhaust_card(&g,0);assert(g.hp==52);game_start_turn(&g);exhaust_card(&g,0);assert(g.hp==54);break;
   case 81:g.battle_type=NODE_ELITE;b=g;b.relic_count=0;win(&g);win(&b);assert(g.gold==b.gold+35);break;
   case 82:r151_played(&g,2);r151_played(&g,2);assert(g.energy==21);game_start_turn(&g);r151_played(&g,2);assert(g.energy==g.max_energy+1);break;
   case 83:for(int i=0;i<10;i++)r151_played(&g,1);for(int i=0;i<g.enemy_count;i++)assert(g.enemies[i].weak==3&&g.enemies[i].vulnerable==3);r151_played(&g,1);assert(g.enemies[0].weak==3);break;
   case 84:g.potion_count=1;g.potions[0]=4;game_use_potion(&g,0);game_start_turn(&g);b=g;b.relic_count=0;win(&g);win(&b);assert(g.hp==b.hp+4);break;
   case 85:assert(g.maxhp==108&&g.hp==58);win(&g);assert(g.hp==59);break;
   case 86:g.draw_count=7;game_start_turn(&g);assert(g.block==6);break;
   case 87:g.enemies[0].hp=4999;play(&g,0);assert(g.enemies[0].hp==4989);break;
   case 88:g.block=0;for(int i=0;i<g.enemy_count;i++){g.enemies[i].intent=1;g.enemies[i].intent_value=0;}game_end_turn(&g);assert(g.block==8&&!g.expedition.combat[E181_NEXT_BLOCK]);break;
   case 89:g.hp=30;damage_player(&g,6);assert(g.hp==36);damage_player(&g,12);assert(g.hp==24);break;
   case 90:g.turn=2;game_start_turn(&g);for(int i=0;i<g.enemy_count;i++)assert(g.enemies[i].poison==4);break;
   case 91:game_start_battle(&g,NODE_FIGHT);assert(g.block==4);g.deck_count=3;g.deck[0]=g.deck[1]=g.deck[2]=8;game_start_battle(&g,NODE_FIGHT);assert(g.block==0);break;
   case 92:r151_skip_reward(&g);assert(g.maxhp==101&&g.hp==51);break;
   case 93:r151_played(&g,0);r151_played(&g,0);assert(g.energy==21);game_start_turn(&g);r151_played(&g,0);assert(g.energy==g.max_energy);break;
  }
 }
 puts("PASS 20 base relic effects and once-per-turn/combat boundaries");
 for(int c=0;c<3;c++)for(int id=0;id<20;id++){
  assert(ascent_begin(&g,c,c,181+id));g.screen=REWARD;g.expedition.story_pending=1;g.expedition.story_id=id;enter_map(&g);assert(g.screen==EXPEDITION_STORY&&!g.expedition.story_pending);assert(game_validate(&g));
  int gift=g.expedition.gift,kind=g.expedition.gift_kind;assert(ascent_checkpoint(&g)&&ascent_resume(&b,c));assert(b.expedition.gift==gift&&b.screen==g.screen);
  for(int page=0;page<3;page++){assert(strlen(expedition_line(&g))>25);game_action(&g,BTN_OK);}assert(g.screen==EXPEDITION_GIFT&&game_validate(&g));
  int count=kind?g.relic_count:g.deck_count;game_action(&g,BTN_OK);assert(g.screen==MAP&&(kind?g.relic_count:g.deck_count)==count+1);
 }
 clean(&g,0);int rolled=0;unsigned seen=0;int bag=0;
 for(int i=0;i<100000;i++){g.expedition.story_pending=0;expedition_roll(&g);if(g.expedition.story_pending){rolled++;int id=g.expedition.story_id;assert(!(seen&(1u<<id)));seen|=1u<<id;if(++bag==20){assert(seen==0xfffffu);seen=0;bag=0;}}}
 assert(rolled>850&&rolled<1150);g.revision182.room_type=NODE_BOSS;g.expedition.story_pending=0;for(int i=0;i<10000;i++)expedition_roll(&g);assert(!g.expedition.story_pending);
 printf("PASS 60 stories/180 pages, persisted gifts, single claims; %d/100000 rolls, no repeats per bag, no boss stories\n",rolled);
 for(int c=0;c<3;c++)for(int seed=1;seed<=300;seed++){
  clean(&g,c);g.rng=seed;expedition_mystery(&g);unsigned own=0;
  for(int i=0;i<3;i++){int id=g.reward[i];assert(cards[id].rarity==2&&owner(id)!=c);own|=1u<<owner(id);for(int j=0;j<i;j++)assert(id!=g.reward[j]);}
  assert(own==(7u^(1u<<c)));for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==MYSTERY_CARDS&&game_validate(&g));int n=g.deck_count,id=g.reward[0];game_action(&g,BTN_OK);assert(g.screen==MAP&&g.deck_count==n+1&&g.deck[n]==id);
 }
 for(int from=0;from<3;from++)for(int j=0;j<8;j++){int id=expedition_foreign[from][j];if(id<0)continue;assert(cards[id].rarity==2);for(int c=0;c<3;c++)if(c!=from)for(int up=0;up<2;up++){
  clean(&g,c);g.energy=30;g.regent.stars=0;g.hand_count=4;g.hand[0]=id|(up?UPBIT:0);g.hand[1]=0;g.hand[2]=1;g.hand[3]=COLORLESS_FIRST;
  game_play_card(&g,0,0);for(int guard=0;g.screen!=COMBAT&&guard<30;guard++)game_action(&g,BTN_OK);assert(g.screen==COMBAT&&game_validate(&g)&&g.turn_played>0);
 }}
 puts("PASS 900 foreign rare offers, both other heroes represented, every offered card resolves for both recipients base/upgraded");
 for(int c=0;c<3;c++){
  assert(ascent_begin(&g,c,c,900+c));g.act=7;g.endless=1;game_story_start(&g,1);assert(g.screen==ABOMINATION_BEFORE);for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==COMBAT&&g.enemies[0].art==45&&g.enemies[0].maxhp>2500);
  Enemy*e=&g.enemies[0];e->hp=e->maxhp/2;enemy_intent(&g,e);assert(e->phase==1&&e->strength==5);e->hp=e->maxhp/4;enemy_intent(&g,e);assert(e->phase==2&&e->strength==10);win(&g);assert(g.screen==ABOMINATION_AFTER);assert(ascent_checkpoint(&g)&&ascent_resume(&b,c)&&b.screen==ABOMINATION_AFTER);for(int i=0;i<3;i++)game_action(&g,BTN_OK);assert(g.screen==RELIC_RESULT&&g.reward[0]>=0);game_action(&g,BTN_OK);assert(g.screen==REWARD);g.v151.potion_offer=-1;g.selection=3;game_action(&g,BTN_OK);assert(g.act==8&&g.screen==ACT_INTRO);
 }
 puts("PASS Act 7 before / three boss phases / after / relic / cards / Act 8 for every hero");
 assert(ascent_begin(&g,3,0,1000));g.screen=MAP;g.ascent.relics=1023;for(int id=0;id<RELIC_COUNT;id++)add_relic(&g,id);assert(g.relic_count>64&&g.relic_count<=MAX_RELICS);assert(game_validate(&g)&&ascent_checkpoint(&g)&&ascent_resume(&b,3));assert(!memcmp(g.expedition.extra_relics,b.expedition.extra_relics,sizeof(g.expedition.extra_relics)));assert(random_relic(&g)==-1);g.act=6;g.endless=1;game_start_battle(&g,NODE_BOSS);win(&g);assert(g.screen==RELIC_RESULT&&g.reward[0]>=37&&g.reward[0]<=39);int offer=g.reward[0];game_action(&g,BTN_OK);assert(g.screen==REWARD);assert(offer>=0);
 clean(&g,0);int found[20]={0};for(int i=0;i<20000;i++){int id=random_relic(&g);if(id>=74)found[id-74]++;assert(id<64||id>=74);}for(int i=0;i<20;i++)assert(found[i]);
 puts("PASS >64 relic storage/persistence, exhaustion blessings, all 20 new relics available with zero account unlocks");
 /* A real 1.8 snapshot prefix is migrated without touching its global account. */
 assert(ascent_begin(&g,0,1,333));g.screen=MAP;g.gold=777;g.version=180;unsigned gen=ascent_slot(0)->generation+1;assert(ascent_write("ASCENT18_RUN_1",&g,offsetof(Game,expedition),gen));AscentAccount account=ascent_account;assert(ascent_resume(&b,0));assert(b.version==182&&b.gold==777&&b.character==1&&!memcmp(&account,&ascent_account,sizeof(account)));assert(ascent_checkpoint(&b)&&ascent_resume(&g,0)&&g.gold==777);
 puts("PASS exact 28224-byte v1.8 run migration and unchanged global account");
 return 0;
}
