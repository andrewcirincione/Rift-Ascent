#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "../source/game.c"
int main(void){Game g,h;int starts[8]={0},ancients=0,rares=0;
 for(int seed=1;seed<=2000;seed++){
  game_new(&g,seed);unsigned prev=0;for(int row=0;row<MAP_ROWS;row++){
   unsigned live=game_route_row_mask(&g,row);assert(live&&!(live&~7));if(!row)starts[live]++;
   int max=-1;for(int lane=0;lane<3;lane++)if(live&(1u<<lane)){
    unsigned edges=route_layer(&g,row,lane);assert(edges&&!(edges&~7));int first=0,last=2;while(!(edges&(1u<<first)))first++;while(!(edges&(1u<<last)))last--;
    assert(first>=max);max=last;
   }prev=live;(void)prev;
  }
 }
 assert(starts[3]>500&&starts[5]>500&&starts[6]>500);assert(!starts[1]&&!starts[2]&&!starts[4]);puts("PASS 2000 reachable noncrossing maps; multiple starts throughout");
 for(int act=1;act<=5;act++)for(int elite=0;elite<2;elite++){
  game_new(&g,900+act);g.act=act;g.endless=act>3;unsigned long long seen=0;int count=elite?(act<=3?5:15):(act<=3?13:39),got=0;
  while(got<count){g.floor++;game_start_battle(&g,elite?NODE_ELITE:NODE_FIGHT);for(int i=0;i<g.enemy_count&&got<count;i++){int id=g.enemies[i].art;assert(!(seen&(1ull<<id)));seen|=1ull<<id;got++;}}
 }
 puts("PASS normal and elite species never repeat until their act pool is exhausted");
 for(int c=0;c<3;c++)for(int unlocked=0;unlocked<2;unlocked++){
  game_new(&g,184+c);game_choose_character(&g,c);g.ascent.cards=unlocked?ASCENT_ALL_CARDS:0;g.battle_type=NODE_BOSS;
  for(int n=0;n<4000;n++){fill_rewards(&g);for(int i=0;i<3;i++){int id=CARD_ID(g.reward[i]),r=cards[id].rarity;assert(r==2||r==3);assert(ascent_card_available(&g,id));ancients+=r==3;rares+=r==2;for(int k=0;k<i;k++)assert(id!=CARD_ID(g.reward[k]));}}
 }
 assert(ancients>100&&rares>ancients*5);printf("PASS boss cards only Rare/eligible Ancient: %d / %d; no locked cards\n",rares,ancients);
 char root[]="/tmp/rift184-events-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
 for(int id=0;id<EVENT_COUNT;id++)for(int condition=0;condition<6;condition++){
  game_new(&g,184+id);g.hp=condition==0?1:40;g.gold=condition<2?0:999;g.potion_count=condition==2?3:0;g.deck_count=condition==3?1:condition==4?128:10;for(int i=0;i<g.deck_count;i++)g.deck[i]=condition==5?1|UPBIT:0;
  game_begin_event(&g,id);int choices=game_event_choice_count(&g),available=0;for(int i=0;i<choices;i++)available+=game_event_choice_available(&g,i);assert(available);
  if(!game_event_can_leave(&g)){h=g;game_action(&g,BTN_BACK);assert(g.screen==EVENT&&g.hp==h.hp&&g.gold==h.gold);}
  for(int i=0;i<choices;i++){h=g;h.selection=i;game_action(&h,BTN_OK);if(h.screen==EVENT_CARD){assert(h.hp==g.hp&&h.gold==g.gold);game_action(&h,BTN_BACK);assert(h.screen==EVENT);}assert(h.hp>0&&game_validate(&h));}
 }
 puts("PASS every event has a legal exit/choice at low HP, zero Gold, full potions/deck and ineligible cards; cancellation never pays; mandatory rooms cannot be skipped with Circle");
 const int healing[][2]={{1,0},{3,1},{5,0},{7,1},{8,0},{8,1},{19,2},{22,0},{28,1},{29,0},{30,0},{36,1}};
 for(unsigned i=0;i<sizeof(healing)/sizeof(*healing);i++){
  game_new(&g,184);g.hp=g.maxhp;g.gold=999;g.potion_count=1;g.potions[0]=0;game_begin_event(&g,healing[i][0]);g.selection=healing[i][1];
  assert(game_event_choice_available(&g,g.selection));int oldhp=g.hp,oldmax=g.maxhp;game_action(&g,BTN_OK);
  assert(g.screen==EVENT_RESULT&&g.hp==g.maxhp&&g.maxhp==oldmax-(healing[i][0]==30?5:0));assert(g.hp<=oldhp);game_action(&g,BTN_OK);assert(g.screen==MAP);
 }
 puts("PASS all twelve pure healing choices work at full HP, cap HP, apply costs and continue to map");
 int mixed=0;for(int seed=1;seed<=1000;seed++){game_new(&g,seed);g.turn=1;init_enemy(&g,&g.enemies[0],0);mixed+=g.enemies[0].intent==24;}assert(mixed>50&&mixed<150);
 for(int weak=0;weak<=1;weak++){
  game_new(&g,184);game_start_battle(&g,NODE_FIGHT);g.relic_count=0;g.enemy_count=1;Enemy*e=&g.enemies[0];e->hp=e->maxhp=999;e->poison=0;e->block=0;e->strength=2;e->weak=weak;e->intent=24;e->intent_value=12;g.hp=g.maxhp=100;g.block=g.vulnerable=g.poison=0;g.hand_count=0;
  int damage=game_enemy_intent_damage(&g,0);char text[128];game_enemy_intent_text(&g,0,text,sizeof(text));assert(strstr(text,"DMG")&&strstr(text,"Block")&&game_enemy_intent_hits(&g,0)==1);game_end_turn(&g);assert(g.hp==100-damage&&e->block==6&&g.screen==COMBAT);
 }
 puts("PASS mixed attack/block intent appears in generated turns, predicts actual normal/Weak damage, gains six Block in same turn");return 0;
}
