#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
static int listed(Game*g,int id){for(int tab=0;tab<3;tab++)for(int i=0;i<custom_count(g,tab);i++)if(custom_id(g,tab,i)==id)return 1;return 0;}
static void combat(Game*g,int id,int up,int varied){
 game_new(g,1842+id);game_choose_character(g,(id-334)/20);g->revision184.custom=1;g->relic_count=0;g->deck_count=2;g->deck[0]=id|(up?UPBIT:0);g->deck[1]=0;game_start_battle(g,NODE_FIGHT);
 g->enemy_count=varied?2:1;for(int e=0;e<g->enemy_count;e++){g->enemies[e].hp=g->enemies[e].maxhp=10000;g->enemies[e].block=0;g->enemies[e].poison=varied?5:0;g->enemies[e].vulnerable=0;g->enemies[e].weak=0;}
 g->hp=varied?40:80;g->maxhp=80;g->strength=varied?3:0;g->dexterity=varied?2:0;g->block=0;g->energy=20;g->regent.stars=varied?10:0;g->regent.forge=varied?15:0;
 g->hand_count=2;g->hand[0]=g->deck[0];g->hand[1]=0;g->draw_count=5;for(int i=0;i<5;i++)g->draw[i]=0;g->discard_count=g->exhaust_count=0;
}
int main(void){char root[]="/tmp/rift184r2-test-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;
 assert(ascent_begin_mode(&g,0,0,1842,1));assert(!listed(&g,334)&&!listed(&g,354)&&!listed(&g,374));
 ascent_account.cards[0]=1;ascent_account.cards[1]=2;ascent_account.cards[2]=4;
 assert(listed(&g,334)&&listed(&g,355)&&listed(&g,376));assert(!listed(&g,335)&&!listed(&g,354)&&!listed(&g,374));
 g.screen=CUSTOM_QUANTITY;g.prior_screen=CUSTOM_CARDS;g.revision184.tab=0;g.revision184.item=335|UPBIT;g.revision184.quantity=1;custom_apply_quantity(&g);assert(!g.deck_count);
 /* Loading a draft produced by original 1.8.4 must not destroy its slot. */
 g.screen=CUSTOM_CARDS;g.revision184.tab=2;g.selection=399;g.deck_count=1;g.deck[0]=393;assert(ascent_write("ASCENT18_RUN_1",&g,sizeof(g),99));assert(ascent_resume(&h,0)&&h.selection<custom_count(&h,2)&&h.deck[0]==393);g=h;g.screen=CUSTOM_CONFIRM;g.selection=0;game_action(&g,BTN_OK);assert(g.screen==CUSTOM_CARDS&&g.revision184.tab==3&&g.revision184.building);
 g.deck[0]=0;g.screen=CUSTOM_CONFIRM;g.selection=1;game_action(&g,BTN_OK);assert(g.screen==CUSTOM_CARDS&&g.revision184.tab==3&&g.deck_count==1);
 puts("PASS custom catalogs and quantity/launch validation enforce each character's actual account unlocks; old drafts remain resumable/removable; Back to Loadout opens Your Deck.");
 game_new(&g,1842);g.screen=REWARD;g.reward[0]=R_DEFEND_REGENT;g.reward[1]=R_VENERATE;g.reward[2]=R_STRIKE_REGENT;assert(game_validate(&g));
 unsigned families_seen=0;
 for(int mask=1;mask<8;mask++)for(int all=0;all<2;all++){
  for(int c=0;c<3;c++)ascent_account.cards[c]=all?ASCENT_ALL_CARDS:0;
  game_new(&g,1842+mask);g.revision184.custom=1;g.deck_count=0;
  const int ids[]={0,N_STRIKE_SILENT,R_STRIKE_REGENT};for(int c=0;c<3;c++)if(mask&(1<<c))g.deck[g.deck_count++]=ids[c]|UPBIT;
  assert(custom_deck_characters(&g)==(unsigned)mask);
  for(int mode=NODE_FIGHT;mode<=NODE_BOSS;mode++)if(mode==NODE_FIGHT||mode==NODE_ELITE||mode==NODE_BOSS){g.battle_type=mode;
   for(int rep=0;rep<300;rep++){fill_rewards(&g);for(int i=0;i<3;i++){int id=CARD_ID(g.reward[i]),c=card_character(id);assert(c>=0&&(mask&(1<<c)));assert(ascent_card_available(&g,id)&&combat_reward_eligible(id));if(mode==NODE_BOSS)assert(cards[id].rarity>=2);families_seen|=1u<<c;}}
  }
  /* Removing a character's last card updates the pool immediately. */
  if(g.deck_count>1){int removed=card_character(CARD_ID(g.deck[--g.deck_count]));assert(!(custom_deck_characters(&g)&(1u<<removed)));}
 }
 assert(families_seen==7);puts("PASS 37,800 custom reward offers across all seven deck-family combinations, locked/unlocked accounts, normal/elite/boss rarities and upgraded cards; removed families disappear; no starter/token boss cards.");
 for(int c=0;c<3;c++)ascent_account.cards[c]=ASCENT_ALL_CARDS;
 FILE*f=fopen("qa/v184r2/evidence/card-balance.csv","w");assert(f);fputs("id,name,upgrade,scenario,cost,damage,block,hp_change,stars_change,forge_change,hand_delta,exhausted\n",f);
 for(int id=334;id<394;id++)for(int up=0;up<2;up++)for(int varied=0;varied<2;varied++){
  combat(&g,id,up,varied);h=g;CardPreview cp;game_card_preview(&g,id|(up?UPBIT:0),0,&cp);assert(!memcmp(&g,&h,sizeof(g)));game_play_card(&g,0,0);assert(game_validate(&g));
  int damage=0,exhausted=0;for(int e=0;e<g.enemy_count;e++){int d=10000-g.enemies[e].hp;damage+=d;assert(cp.total[e]==d);}
  for(int i=0;i<g.exhaust_count;i++)exhausted|=CARD_ID(g.exhaust[i])==id;
  assert(exhausted==new_exhaust[id-334]);assert(g.block==cp.block_total);
  fprintf(f,"%d,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d\n",id,cards[id].name,up,varied?"two_foes_poison_strength_stars":"baseline",game_card_cost(id|(up?UPBIT:0)),damage,g.block,g.hp-h.hp,g.regent.stars-h.regent.stars,g.regent.forge-h.regent.forge,g.hand_count-h.hand_count,exhausted);
 }
 fclose(f);puts("PASS all 60 new cards, base and upgraded, resolve and preview identically across 240 actual combat cases; cost, damage, Block, healing, Stars, Forge, draw and Exhaust recorded.");
 const int loop_cards[]={356,377};for(int k=0;k<2;k++)for(int up=0;up<2;up++){
  int id=loop_cards[k];combat(&g,id,up,0);g.energy=0;g.hand_count=2;g.hand[0]=g.hand[1]=id|(up?UPBIT:0);g.draw_count=g.discard_count=g.exhaust_count=0;
  int plays=0;while(g.hand_count&&plays<100){game_play_card(&g,0,0);plays++;}assert(plays==2&&g.exhaust_count==2&&!g.hand_count&&g.energy==0);
 }
 combat(&g,364,0,0);g.energy=3;g.hand_count=1;game_play_card(&g,0,0);assert(g.energy==2);
 combat(&g,364,0,0);g.energy=3;game_play_card(&g,0,0);assert(g.energy==4);
 puts("PASS zero-Energy Quiet Step/Starlit Writ decks terminate after two copies instead of cycling unlimited Block/Stars; Threadcutter requires a real discard.");
 const int menus[]={TITLE,CHARACTER_SELECT,ASCENT_SLOTS_VIEW,ASCENT_COLLECTION,ASCENT_OPTIONS,CUSTOM_CARDS,CUSTOM_CONFIRM};game_new(&g,1842);
 for(unsigned i=0;i<sizeof(menus)/sizeof(*menus);i++){g.screen=menus[i];assert(game_music_track(&g)==10);}
 for(int act=1;act<=8;act++)for(int secret=0;secret<2;secret++){
  g.act=act;g.ascent.secret=secret;g.screen=COMBAT;g.battle_type=NODE_BOSS;assert(game_music_track(&g)==(act==4&&secret?11:0));
  g.screen=MAP;assert(game_music_track(&g)==(act==1?1:act==2?7:act==3?8:9));
 }
 g.screen=ASCENT_BOSS;assert(game_music_track(&g)==11);g.screen=ABOMINATION_BEFORE;assert(game_music_track(&g)==0);
 g.screen=COMBAT;g.battle_type=NODE_FIGHT;g.combat_music=5;assert(game_music_track(&g)==5);g.battle_type=NODE_ELITE;g.combat_music=6;assert(game_music_track(&g)==6);
 puts("PASS new main-menu and Act 4 secret-boss themes; existing regular, elite, map, Act 1-3/7 boss routing retained.");return 0;}
