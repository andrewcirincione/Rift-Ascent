#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../../qa/v186/source/game.c"
static void fixture(Game*g,int id,int enemies,int echo){
 game_new(g,711);g->character=dist_cards[id].character<0?0:dist_cards[id].character;g->relic_count=0;game_start_battle(g,NODE_FIGHT);
 g->enemy_count=enemies;g->hp=g->maxhp=200;g->energy=20;g->regent.stars=echo;g->block=0;g->strength=2;g->dexterity=1;g->hand_count=3;g->hand[0]=id;g->hand[1]=0;g->hand[2]=1;g->draw_count=3;g->draw[0]=2;g->draw[1]=0;g->draw[2]=1;g->discard_count=2;g->discard[0]=0;g->discard[1]=1;g->exhaust_count=0;g->selection=g->target=0;
 for(int e=0;e<enemies;e++){init_enemy(g,&g->enemies[e],e);g->enemies[e].hp=g->enemies[e].maxhp=10000;g->enemies[e].block=3;g->enemies[e].poison=e+2;g->enemies[e].vulnerable=1;g->enemies[e].weak=0;}
}
static int op_sum(int raw,int kind){const DistCard*d=&dist_cards[CARD_ID(raw)];int n=0;for(int j=0;j<d->count;j++)if(d->op[j].kind==kind)n+=d->op[j].n+CARD_UP(raw)*d->op[j].u;return n;}
static void catalogue_matrix(void){
 Game g,before;long n=0;unsigned account_before;
 char root[]="/tmp/rift-dist-rules-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));account_before=ascent_account.generation;
 for(int id=0;id<CARD_COUNT;id++)if(game_card_is_playable(id))for(int up=0;up<2;up++)for(int mod=0;mod<MOD_COUNT;mod++)if(!mod||game_can_modify(id|(up?UPBIT:0),mod))for(int foes=1;foes<=3;foes++)for(int echo=0;echo<=9;echo+=3){
  int raw=id|(up?UPBIT:0)|(mod<<CARD_MOD_SHIFT);fixture(&g,id,foes,echo);g.hand[0]=raw;before=g;
  CardPreview p;game_card_preview(&g,raw,foes-1,&p);assert(!memcmp(&g,&before,sizeof(g)));assert(ascent_account.generation==account_before);
  game_play_card(&g,0,foes-1);
  assert(g.regent.stars>=0&&g.regent.stars<=9&&g.energy>=0&&g.block>=0&&g.block<=999&&g.hp<=g.maxhp);
  assert(g.hand_count>=0&&g.hand_count<=MAX_HAND&&g.draw_count<=MAX_PILE&&g.discard_count<=MAX_PILE&&g.exhaust_count<=MAX_PILE);
  for(int e=0;e<foes;e++){int delta=before.enemies[e].hp-g.enemies[e].hp;/* No reaction powers are present; preview observes every direct hit. */assert(delta==MAX(0,p.total[e]-(dist_has_op(raw,DC_STRIP)?0:3)));}
  assert(g.block==p.block_total);assert(g.turn_played==(mod==MOD_ECHO?2:1));
  if(cards[id].type==2)assert(g.exhaust_count==0);
  else if(dist_cards[id].exile||mod==MOD_FLEETING||mod==MOD_KINDLED)assert(g.exhaust_count==1);
  assert(g.dist.next_draw>=0&&g.dist.next_draw<=MAX_HAND);n++;
 }
 printf("PASS %ld card cases: all playable slots, upgrades, legal forge modifiers, one/two/three foes, 0/3/6/9 Echo; preview immutability, damage/Guard parity, bounded piles/resources, exile and replay.\n",n);
}
static void resources_and_relics(void){
 Game g;fixture(&g,0,3,0);g.character=CHAR_WARDEN;g.hand_count=1;g.hand[0]=1;game_play_card(&g,0,0);assert(g.regent.stars==2);
 fixture(&g,0,3,0);g.character=CHAR_VESPER;g.hand_count=2;g.hand[0]=0;g.hand[1]=1;game_play_card(&g,0,0);game_play_card(&g,0,0);assert(g.regent.stars==3);
 fixture(&g,0,3,0);g.character=CHAR_PROXY;g.dist.bank=2;regent_start_turn(&g);assert(g.regent.stars==3);g.dist.bank=9;g.regent.stars=8;regent_start_turn(&g);assert(g.regent.stars==9);
 for(int id=0;id<RELIC_COUNT;id++){fixture(&g,0,3,0);add_relic(&g,id);for(int ev=0;ev<8;ev++)dist_relic_trigger(&g,ev);for(int t=1;t<=4;t++){g.turn=t;dist_relic_trigger(&g,1);}for(int type=0;type<3;type++){g.turn_played=4;g.dist.cards_played=6;g.dist.skills=3;dist_relic_play(&g,type,1,3);}assert(g.hp>0&&g.hp<=g.maxhp&&g.regent.stars<=9&&g.block<=999);}
 puts("PASS character Echo generators, cap, all relic pickup/turn/event/card trigger contracts.");
}
static void persistence(void){
 char root[]="/tmp/rift-dist-save-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;assert(ascent_begin(&g,0,0,900));g.screen=MAP;game_start_battle(&g,NODE_FIGHT);g.regent.stars=7;g.dist.power[0]=4;for(int id=0;id<CARD_COUNT;id++)if(dist_has_op(id,DC_POWER)&&dist_cards[id].op[0].aux==0){g.dist.power_raw[0]=id;break;}g.dist.next_guard=9;g.dist.last_type=1;g.dist.relic_used[5]=1;
 assert(game_validate(&g));assert(ascent_checkpoint(&g));assert(ascent_open(root)&&ascent_resume(&h,0));assert(!memcmp(&g.dist,&h.dist,sizeof(g.dist))&&h.regent.stars==7);
 assert(game_save(&g,"/tmp/rift-dist-roundtrip.bin")&&game_load(&h,"/tmp/rift-dist-roundtrip.bin"));assert(!memcmp(&g.dist,&h.dist,sizeof(g.dist)));
 puts("PASS live Echo, power/once flags and deferred effects round-trip through alternating and standalone saves.");
}
static void burdens(void){
 for(int id=50;id<=53;id++)for(int echo=0;echo<=3;echo+=3){Game g;fixture(&g,0,1,echo);g.character=CHAR_WARDEN;g.hand_count=1;g.hand[0]=id;g.hp=g.maxhp=200;g.block=10;g.enemies[0].poison=0;g.enemies[0].intent=0;g.enemies[0].intent_value=14;
 int incoming=game_enemy_intent_damage(&g,0),guard=10-(id==52?2:0),loss=(id==50?1:id==53&&echo<2?2:0),remaining=echo;
 if(id==50)remaining=MAX(0,echo-1);if(id==51)remaining=MAX(0,echo-2);if(id==53&&echo>=2)remaining=echo-2;
 game_end_turn(&g);assert(g.hp==200-loss-MAX(0,incoming-guard));assert(g.regent.stars==remaining);
 }
 puts("PASS all four redesigned burdens at empty/stocked Echo: HP, Guard and resource payments match printed rules.");
}
int main(void){catalogue_matrix();resources_and_relics();persistence();burdens();return 0;}
