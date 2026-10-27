#include "source/game.c"
#include <assert.h>
static int modded(int id,int mod){return id|(mod<<CARD_MOD_SHIFT);}
static void arena(Game*g,int raw){
 game_new(g,150);game_start_battle(g,NODE_FIGHT);g->enemy_count=2;
 for(int i=0;i<2;i++){g->enemies[i]=g->enemies[0];g->enemies[i].hp=g->enemies[i].maxhp=1000000;g->enemies[i].block=g->enemies[i].strength=g->enemies[i].weak=g->enemies[i].vulnerable=g->enemies[i].poison=0;g->enemies[i].intent=1;g->enemies[i].intent_value=0;}
 g->hand_count=1;g->hand[0]=raw;g->energy=9;g->draw_count=4;for(int i=0;i<4;i++)g->draw[i]=1;
 g->discard_count=g->exhaust_count=0;g->selection=g->target=0;g->relic_count=0;g->hp=g->maxhp=500;g->strength=g->dexterity=g->block=0;memset(g->ic,0,sizeof(g->ic));g->ic[IC_OWN_TURN]=1;
}
static int pile_count(const Game*g,int raw){int n=0;for(int i=0;i<g->hand_count;i++)n+=g->hand[i]==raw;for(int i=0;i<g->draw_count;i++)n+=g->draw[i]==raw;for(int i=0;i<g->discard_count;i++)n+=g->discard[i]==raw;for(int i=0;i<g->exhaust_count;i++)n+=g->exhaust[i]==raw;return n;}
int main(void){Game g,b,h;int cases=0;
 assert(sizeof(Game)==3260&&MAX_RELICS==40);
 for(int mod=1;mod<MOD_COUNT;mod++)for(int id=0;id<CARD_COUNT;id++)for(int up=0;up<2;up++){
  int raw=id|(up?UPBIT:0);if(!game_can_modify(raw,mod))continue;raw|=mod<<CARD_MOD_SHIFT;
  for(int str=-2;str<=20;str+=11)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++){
   arena(&g,raw);g.strength=str;g.weak=weak;g.dexterity=str/2;g.enemies[0].vulnerable=vul;g.hand[g.hand_count++]=0;g.hand[g.hand_count++]=1;
   b=g;CardPreview preview;game_card_preview(&g,raw,0,&preview);assert(!memcmp(&g,&b,sizeof(g)));
   char text[384];game_card_description_now(&g,raw,0,text,sizeof(text));assert(strstr(text,game_mod_name(mod)));
   game_combat_fx_begin();game_play_card(&g,0,0);game_combat_fx_end();int count;const GameCombatFx*fx=game_combat_fx_events(&count);
   int dmg[2]={0},block=0;for(int i=0;i<count;i++){if(fx[i].kind==GFX_ATTACK&&fx[i].source==-1&&fx[i].target>=0)dmg[fx[i].target]+=fx[i].amount;if(fx[i].kind==GFX_BLOCK&&fx[i].source==-1)block+=fx[i].amount;}
   if(!preview.random&&!preview.uncertain){assert(dmg[0]==preview.total[0]);assert(dmg[1]==preview.total[1]);assert(block>=preview.block_total);}
   assert(g.hand_count<=MAX_HAND&&g.energy>=0);cases++;
  }
 }
 printf("PASS %d modified base/upgraded card previews vs actual damage, RNG immutability, cost and pile bounds\n",cases);
 // Explicit arithmetic: Keen then Strength then Weak then Vulnerable, every hit.
 cases=0;
 for(int str=-3;str<=20;str++)for(int weak=0;weak<2;weak++)for(int vul=0;vul<2;vul++)for(int up=0;up<2;up++){
  int raw=modded(27,MOD_KEEN)|(up?UPBIT:0);arena(&g,raw);g.strength=str;g.weak=weak;g.enemies[0].vulnerable=vul;
  int per=MAX(0,5+3*up+2+str);if(weak)per=per*3/4;if(vul)per=per*3/2;
  game_play_card(&g,0,0);assert(1000000-g.enemies[0].hp==per*2);cases++;
 }
 for(int dex=-6;dex<=20;dex++)for(int up=0;up<2;up++)for(int fortress=0;fortress<2;fortress++){
  arena(&g,modded(1,MOD_WARD)|(up?UPBIT:0));g.dexterity=dex;if(fortress)add_relic(&g,15);game_play_card(&g,0,0);assert(g.block==MAX(0,5+3*up+4+dex+fortress*2));cases++;
 }
 printf("PASS %d independent Keen/Weak/Vulnerable and Ward/Dexterity/Fortress arithmetic cases\n",cases);
 for(int up=0;up<2;up++){
  int raw=modded(0,MOD_ECHO)|(up?UPBIT:0);arena(&g,raw);g.ic[IC_NEXT_ATTACK_REPEAT]=1;g.energy=2;game_play_card(&g,0,0);assert(g.energy==0);assert(g.enemies[0].hp==1000000-3*(6+3*up));assert(pile_count(&g,raw)==1);assert(g.ic[IC_NEXT_ATTACK_REPEAT]==0);
  raw=modded(47,MOD_ECHO);arena(&g,raw);game_play_card(&g,0,0);assert(g.exhaust_count==1&&g.discard_count==0);
  raw=modded(44,MOD_ECHO);arena(&g,raw);game_play_card(&g,0,0);assert(g.discard_count==3&&pile_count(&g,raw)==3); // original plus one explicit Anger copy per play
 }
 arena(&g,modded(26,MOD_ECHO));g.hand_count=10;for(int i=1;i<10;i++)g.hand[i]=0;g.draw_count=20;for(int i=0;i<20;i++)g.draw[i]=0;game_play_card(&g,0,0);assert(g.hand_count==10&&g.draw_count==19); // repeat has no artificial vacancy
 arena(&g,modded(0,MOD_FLEETING));g.energy=0;game_play_card(&g,0,0);assert(g.exhaust_count==1&&g.energy==0&&g.discard_count==0);
 arena(&g,modded(1,MOD_FLEETING));g.ic[IC_CORRUPTION]=1;g.ic[IC_DARK_EMBRACE]=1;game_play_card(&g,0,0);assert(g.exhaust_count==1&&g.hand_count==1);
 arena(&g,modded(0,MOD_KINDLED));game_play_card(&g,0,0);assert(g.exhaust_count==1&&g.hand_count==1);
 arena(&g,modded(0,MOD_BOUND));g.hand[g.hand_count++]=0;g.hand[0]|=CARD_FREE;g.draw_count=0;g.discard_count=0;game_end_turn(&g);assert(pile_count(&g,modded(0,MOD_BOUND))==1);assert(!(g.hand[0]&CARD_FREE));assert(g.hand[0]==modded(0,MOD_BOUND));
 assert(game_save(&g,"qa/v150/evidence/retain-save.dat"));assert(game_load(&h,"qa/v150/evidence/retain-save.dat"));assert(!memcmp(&g,&h,sizeof(g)));
 arena(&g,modded(25,MOD_KEEN));g.discard_count=1;g.discard[0]=1;game_play_card(&g,0,0);assert(g.screen==CARD_CHOICE);game_action(&g,BTN_OK);assert(g.discard_count==1&&g.discard[0]==modded(25,MOD_KEEN));
 puts("PASS Echo + One-Two Punch, exhaust/Anger conservation, full hands, Fleeting/Corruption, Kindled, Retain/save, modified Headbutt");
 arena(&g,0);g.hand_count=0;int automatic=modded(0,MOD_KEEN)|UPBIT;
 autoplay_card(&g,automatic,0);assert(g.ic[46]==automatic+1&&g.discard[0]==automatic&&g.enemies[0].hp==1000000-11);
 puts("PASS autoplay preserves the upgrade and modification in its displayed card and physical discard");
 // Unique relics coexist with all 36 existing obtainable relics.
 game_new(&g,150);for(int i=0;i<RELIC_COUNT;i++)if(i!=30&&(i<37||i>39))add_relic(&g,i);assert(g.relic_count==40);
 for(int a=0;a<RELIC_COUNT;a++)for(int c=40;c<44;c++)if(a!=30&&(a<37||a>39)&&a!=c){arena(&g,0);add_relic(&g,a);add_relic(&g,c);add_relic(&g,c);assert(g.relic_count==2);}
 for(int e=0;e<8;e++){arena(&g,0);add_relic(&g,40);g.energy=e;game_end_turn(&g);assert(g.energy==3+MIN(e,2));assert(g.ic[IC_HOURGLASS]==0);game_start_battle(&g,NODE_FIGHT);assert(g.energy==3);}
 arena(&g,0);add_relic(&g,41);g.dexterity=20;g.shop_cards[2]=3;for(int i=0;i<5;i++)exhaust_card(&g,0);assert(g.block==24&&g.ic[IC_BELL]==3); // 9 Bell + 15 Feel No Pain; no Dex
 arena(&g,6);add_relic(&g,42);g.hand[1]=6;g.hand_count=2;game_play_card(&g,0,0);assert(g.hand_count==2&&g.ic[IC_NEEDLE]==1);game_play_card(&g,0,0);assert(g.hand_count==1);
 arena(&g,0);add_relic(&g,42);g.ic[IC_OWN_TURN]=0;lose_hp(&g,2);assert(g.hand_count==1&&g.ic[IC_NEEDLE]==0);
 for(int order=0;order<2;order++){arena(&g,order?1:0);add_relic(&g,43);g.hand_count=4;g.hand[1]=order?0:1;g.hand[2]=0;g.hand[3]=1;g.energy=4;game_play_card(&g,0,0);assert(g.energy==3);game_play_card(&g,0,0);assert(g.energy==3);game_play_card(&g,0,0);game_play_card(&g,0,0);assert(g.energy==1);}
 puts("PASS 40 relic inventory, every new/old pair, Hourglass cap/reset, Bell + Feel No Pain, Needle once/own-turn, Duelist both orders/cap");
 // v19 migration without changing struct layout or losing deck/potion/relic data.
 assert(game_load(&h,"qa/v150/fixtures/state-map-chest.dat"));assert(h.version==GAME_VERSION&&h.screen==MAP&&h.deck_count==10);
 puts("PASS 1.4.0 save migration, expanded 3260-byte ABI");
}
