#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../v186/source/game.c"
#include "../v186/source/ascent_dialogue.h"
static void clear(Game*g,int act){g->act=act;g->endless=act>3;game_start_battle(g,NODE_BOSS);for(int i=0;i<g->enemy_count;i++)g->enemies[i].hp=0;check_win(g);}
static void gate(void){
 char root[]="/tmp/r186r3-gates-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;
 for(int c=0;c<3;c++){
  for(int win=0;win<11;win++){
   /* An imported complete collection must not open the secret early. */
   ascent_account.cards[c]=ASCENT_ALL_CARDS;
   assert(ascent_begin(&g,c,c,18000+c*100+win));clear(&g,3);
   assert(g.screen==ASCENT_ENDING&&ascent_account.wins[c]==(unsigned)win+1);
   assert(ascent_secret(c)==(win>=9)&&g.ascent.secret==(unsigned)(win>=9));
   assert(ascent_checkpoint(&g)&&ascent_resume(&h,c));assert(ascent_complete(&h,0)&&ascent_account.wins[c]==(unsigned)win+1);
   for(int p=0;p<3;p++){assert(!strstr(ascent_dialogue[c][0][p],"UNMAKER")&&!strstr(ascent_dialogue[c][0][p],"keep climbing"));game_action(&g,BTN_OK);}
   assert(g.screen==ASCENT_CARDS);for(int p=0;p<3;p++)game_action(&g,BTN_OK);
   assert(g.screen==(win>=9?ASCENT_REVELATION:VICTORY));
   if(win>=9){for(int p=0;p<3;p++)game_action(&g,BTN_OK);assert(g.act==4);clear(&g,4);assert(g.screen==ASCENT_TRUE_END&&ascent_account.wins[c]==(unsigned)win+1);}
  }
 }
 assert(ascent_open(root));for(int c=0;c<3;c++)assert(ascent_account.wins[c]==11);
 ascent_account.wins[1]=0;ascent_account.cards[1]=ASCENT_ALL_CARDS;
 assert(!ascent_secret(1)&&ascent_secret(0)&&ascent_secret(2));
 assert(ascent_begin(&g,1,1,19000));g.act=3;g.screen=ASCENT_REVELATION;g.selection=1;g.ascent.secret=1;
 assert(ascent_checkpoint(&g)&&ascent_resume(&h,1));assert(h.screen==ASCENT_ENDING&&!h.ascent.secret&&h.selection==0);
 assert(game_save(&g,"/tmp/r186r3-stale.dat")&&game_load(&h,"/tmp/r186r3-stale.dat"));assert(h.screen==ASCENT_ENDING&&!h.ascent.secret);
 g.screen=PAUSE_MENU;g.prior_screen=ASCENT_REVELATION;g.selection=0;
 assert(ascent_checkpoint(&g)&&ascent_resume(&h,1));assert(h.prior_screen==ASCENT_ENDING&&!h.ascent.secret);
 g.screen=ASCENT_REVELATION;game_action(&g,BTN_OK);assert(g.screen==ASCENT_ENDING&&g.act==3);
 ascent_account.wins[1]=9;ascent_account.cards[1]=0;assert(ascent_begin(&g,1,1,19001));clear(&g,3);assert(ascent_secret(1)&&g.ascent.secret);
 puts("PASS 33 real Act 3 win routes; first/ninth/tenth/eleventh thresholds for all characters; receipts idempotent; per-character gate independent of cards; cold resume and stale/paused/standalone ending repair; secret boss resolution.");
}
static void gifts(void){
 Game g,h;int upgraded[7]={0},rare[7]={0},total[7]={0};
 for(int a=1;a<=7;a++)for(int seed=1;seed<=500;seed++)for(int c=0;c<3;c++){
  game_new(&g,seed);game_choose_character(&g,c);g.act=a;g.endless=a>3;g.ascent.cards=ASCENT_ALL_CARDS;
  g.screen=EXPEDITION_GIFT;g.expedition.story_id=0;expedition_prepare_gift(&g);assert(g.expedition.gift_kind==0);
  int raw=g.expedition.gift;assert(combat_reward_eligible(CARD_ID(raw))&&ninja_owned_card(&g,CARD_ID(raw)));
  if(a==1)assert(cards[CARD_ID(raw)].rarity==1&&!CARD_UP(raw));if(a>=3)assert(cards[CARD_ID(raw)].rarity>=2);
  if(a>=4)assert(CARD_UP(raw)||!game_card_is_upgradeable(raw));
  total[a-1]++;upgraded[a-1]+=!!CARD_UP(raw);rare[a-1]+=cards[CARD_ID(raw)].rarity>=2;
  unsigned rng=g.rng;g.selection=1;game_action(&g,BTN_LEFT);assert(g.expedition.gift==raw&&g.rng==rng);
  g.expedition.story_id=1;g.relic_count=0;expedition_prepare_gift(&g);int id=g.expedition.gift;
  if(a==1)assert(id==expedition_stories[c][1].relic);else assert(id==74||id==75||id==76||id==77||id==83||id==85||id==87||id==89||id==93);
  add_relic(&g,id);expedition_prepare_gift(&g);assert(g.expedition.gift!=id);
 }
 for(int a=1;a<=7;a++)printf("Act %d story cards: Rare/Ancient %d/%d, upgraded %d/%d\n",a,rare[a-1],total[a-1],upgraded[a-1],total[a-1]);
 for(int a=1;a<=7;a++)for(int c=0;c<3;c++)for(int story=0;story<20;story++){
  game_new(&g,700+story);game_choose_character(&g,c);g.act=a;g.endless=a>3;g.screen=EXPEDITION_GIFT;g.expedition.story_id=story;expedition_prepare_gift(&g);
  assert(game_beta_offer(&g)==(story==5||story==6||story==15||story==16));assert(game_validate(&g));
  if(!g.expedition.gift_kind){int raw=g.expedition.gift;assert(game_save(&g,"/tmp/r186r3-gift.dat")&&game_load(&h,"/tmp/r186r3-gift.dat"));assert(h.expedition.gift==raw);int n=h.deck_count;h.selection=0;game_action(&h,BTN_OK);assert(h.deck_count==n+1&&h.deck[n]==raw);}
 }
 game_new(&g,777);g.act=7;g.relic_count=MAX_RELICS;g.expedition.story_id=1;expedition_prepare_gift(&g);assert(g.expedition.gift_kind==1&&g.expedition.gift==39);
 for(int a=1;a<=7;a++)for(int c=0;c<3;c++){
  game_new(&g,700);game_choose_character(&g,c);g.act=a;g.endless=a>3;expedition_mystery(&g);for(int i=0;i<3;i++){assert(cards[CARD_ID(g.reward[i])].rarity==2);for(int j=0;j<i;j++)assert(CARD_ID(g.reward[i])!=CARD_ID(g.reward[j]));if(a>=4)assert(CARD_UP(g.reward[i])||!game_card_is_upgradeable(g.reward[i]));}assert(game_validate(&g));
 }
 puts("PASS 10,500 act-scaled card gifts; stronger unowned relic pools; exact four Forge outcomes; frozen preview/save/claim; full-inventory fallback; upgraded cross-character rares remain distinct and legal.");
}
static void forge(void){
 Game g,h;for(int a=1;a<=7;a++)for(int choice=0;choice<3;choice++){
  game_new(&g,42);g.act=a;g.endless=a>3;g.hp=g.maxhp=80;g.screen=BETA_FORGE;g.expedition.gift_kind=2;g.deck_count=3;g.deck[0]=0;g.deck[1]=1;g.deck[2]=8;g.selection=choice;
  int cost=MAX(0,5-a);assert(game_beta_echo_cost(&g)==cost);if(a>=2)assert(game_beta_modifier(&g,1)!=MOD_FLEETING&&game_beta_modifier(&g,1)!=MOD_KINDLED);
  game_action(&g,BTN_OK);assert(g.screen==BETA_FORGE_CARD);int index=g.selection,raw=g.deck[index],preview=game_beta_preview(&g,choice,raw);
  assert(game_save(&g,"/tmp/r186r3-forge.dat")&&game_load(&h,"/tmp/r186r3-forge.dat"));game_action(&g,BTN_BACK);assert(g.maxhp==80&&g.deck[index]==raw);
  game_action(&h,BTN_OK);assert(h.maxhp==80-(choice==2?cost:0)&&h.deck[index]==preview&&h.screen==MAP);
 }
 for(int a=1;a<=7;a++)for(int id=0;id<EVENT_COUNT;id++)for(int option=0;option<event_offer_counts[id];option++){
  game_new(&g,18603+id);g.act=a;g.endless=a>3;game_begin_event(&g,id);g.hp=g.maxhp=100;g.gold=200;g.deck_count=3;g.deck[0]=0;g.deck[1]=1;g.deck[2]=8;
  EventOffer o=event_offer(&g,id,option);if(o.action<=0||o.action>=MOD_COUNT)continue;
  assert(o.gold<=event_offers[id][option].gold&&o.hp<=event_offers[id][option].hp&&o.maxhp<=event_offers[id][option].maxhp);
  char title[128],detail[200];unsigned rng=g.rng;game_event_option(&g,option,title,sizeof(title),detail,sizeof(detail));assert(g.rng==rng);
  assert(game_event_choice_available(&g,option));g.selection=option;game_action(&g,BTN_OK);assert(g.screen==EVENT_CARD&&g.gold==200&&g.hp==100&&g.maxhp==100);
  int i=g.selection;assert(game_save(&g,"/tmp/r186r3-event.dat")&&game_load(&h,"/tmp/r186r3-event.dat"));game_action(&g,BTN_BACK);assert(g.gold==200&&g.hp==100&&g.maxhp==100);
  game_action(&h,BTN_OK);assert(h.screen==EVENT_RESULT&&CARD_MOD(h.deck[i])==o.action&&h.gold==200-o.gold&&h.maxhp==100-o.maxhp&&h.hp==MIN(100-o.hp,h.maxhp));
 }
 puts("PASS all acts: three Journey Forge choices, reduced Echo cost, safer Temper pool, preview/cancel/save/exact payment; every mystery modification uses one shared adjusted quote and commit price.");
}
static void damage(void){
 Game g,h;int checked=0;for(int a=1;a<=7;a++)for(int n=1;n<=3;n++)for(int type=0;type<3;type++)for(int weak=0;weak<2;weak++){
  game_new(&g,99);g.act=a;g.endless=a>3;game_start_battle(&g,type==0?NODE_FIGHT:type==1?NODE_ELITE:NODE_BOSS);g.enemy_count=n;g.relic_count=0;g.hp=g.maxhp=10000;g.poison=0;g.block=17;g.vulnerable=weak;g.hand_count=0;g.draw_count=30;for(int i=0;i<30;i++)g.draw[i]=1;
  int shown=0;for(int e=0;e<n;e++){init_enemy(&g,&g.enemies[e],e);g.enemies[e].hp=g.enemies[e].maxhp=10000;g.enemies[e].intent=0;g.enemies[e].intent_value=100;g.enemies[e].strength=0;g.enemies[e].weak=weak;shown+=game_enemy_intent_damage(&g,e);}
  assert(game_save(&g,"/tmp/r186r3-damage.dat")&&game_load(&h,"/tmp/r186r3-damage.dat"));game_end_turn(&h);assert(h.hp==10000-MAX(0,shown-17));checked++;
 }
 for(int a=4;a<=7;a++)for(int type=0;type<3;type++){
  game_new(&g,99);g.act=a;g.endless=1;g.floor=0;g.battle_type=type==0?NODE_FIGHT:type==1?NODE_ELITE:NODE_BOSS;g.enemy_count=1;
  init_enemy(&g,&g.enemies[0],type==0?0:type==1?14:15);init_enemy(&g,&g.enemies[1],type==0?27:type==1?34:8);
  assert(g.enemies[0].maxhp>=g.enemies[1].maxhp);assert(g.enemies[0].intent_value>=g.enemies[1].intent_value);
 }
 printf("PASS %d intent/actual/save checks across all acts, encounter sizes and weaknesses; returning enemies meet the Act 3 stat floor.\n",checked);
}
int main(void){gate();gifts();forge();damage();puts("PASS progression, reward scaling, tenth-win endings and save compatibility.");return 0;}
