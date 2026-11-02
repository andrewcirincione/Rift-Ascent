#include <assert.h>
#include <stdio.h>
#include "../source/game.c"
int main(void){Game g,h;int open,tab,scroll,potion;
 const int allowed[]={COMBAT,MAP,SHOP,REST,EVENT,EVENT_CARD,EVENT_RESULT,REWARD,BOSS_REWARD,REWARD_PREVIEW,RELIC_RESULT,BLESSING,DECK_VIEW,POTION_VIEW,RELIC_VIEW,CARD_CHOICE,NINJA_CHOICE,REGENT_CHOICE,EXPEDITION_GIFT,MYSTERY_CARDS,BETA_FORGE,BETA_FORGE_CARD,VICTORY,DEFEAT,ASCENT_CARDS,ASCENT_POTIONS,ASCENT_RELIC,ASCENT_SAVE_ERROR};
 for(unsigned i=0;i<sizeof(allowed)/sizeof(*allowed);i++){
  game_new(&g,777);game_choose_character(&g,0);g.screen=allowed[i];g.selection=2;g.scroll=1;g.effect_turn=2;g.next_screen=23;h=g;
  open=tab=scroll=potion=0;assert(game_inventory_available(&g));assert(game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion)&&open);
  assert(game_inventory_action(&g,BTN_ALT,&open,&tab,&scroll,&potion)&&tab==5);
  game_inventory_action(&g,BTN_DOWN,&open,&tab,&scroll,&potion);game_inventory_action(&g,BTN_OK,&open,&tab,&scroll,&potion);
  game_inventory_action(&g,BTN_TAB_LEFT,&open,&tab,&scroll,&potion);assert(tab==4);
  game_inventory_action(&g,BTN_BACK,&open,&tab,&scroll,&potion);assert(!open&&!memcmp(&g,&h,sizeof(g)));
  if(g.screen!=COMBAT){assert(game_inventory_action(&g,BTN_ALT,&open,&tab,&scroll,&potion)&&open&&tab==5);game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion);assert(!open&&!memcmp(&g,&h,sizeof(g)));}
 }
 printf("PASS %zu in-run screens: deck/inventory browsing preserves the complete underlying Game, selection and pending effect.\n",sizeof(allowed)/sizeof(*allowed));
 const int blocked[]={TITLE,CHARACTER_SELECT,ASCENT_SLOTS_VIEW,ASCENT_OVERWRITE,ASCENT_OPTIONS,ASCENT_COLLECTION,CUSTOM_CARDS,CUSTOM_ITEMS,CUSTOM_QUANTITY,CUSTOM_CONFIRM,ACT_INTRO,STORY_ACT,STORY_ACT_BEFORE_BLESSING,STORY_BOSS,ASCENT_ENDING,ASCENT_REVELATION,ASCENT_BOSS,ASCENT_TRUE_END,EXPEDITION_STORY,ABOMINATION_BEFORE,ABOMINATION_AFTER,MYSTERY_STORY};
 for(unsigned i=0;i<sizeof(blocked)/sizeof(*blocked);i++){g.screen=blocked[i];h=g;open=1;assert(!game_inventory_available(&g)&&!game_inventory_action(&g,BTN_START,&open,&tab,&scroll,&potion)&&!open&&!memcmp(&g,&h,sizeof(g)));}
 const int readonly[]={CARD_CHOICE,NINJA_CHOICE,REGENT_CHOICE,VICTORY,DEFEAT,ASCENT_CARDS,ASCENT_POTIONS,ASCENT_RELIC,ASCENT_SAVE_ERROR};
 for(unsigned i=0;i<sizeof(readonly)/sizeof(*readonly);i++){game_new(&g,777);g.screen=readonly[i];g.hp=20;g.potion_count=1;g.potions[0]=0;h=g;open=1;tab=3;scroll=potion=0;assert(!game_inventory_can_change(&g));game_use_potion(&g,0);game_discard_potion(&g,0);game_inventory_action(&g,BTN_OK,&open,&tab,&scroll,&potion);game_inventory_action(&g,BTN_POTION,&open,&tab,&scroll,&potion);assert(!memcmp(&g,&h,sizeof(g)));}
 game_new(&g,777);g.screen=BETA_FORGE;g.selection=2;g.hp=20;g.potion_count=1;g.potions[0]=0;open=1;tab=3;scroll=potion=0;assert(game_inventory_action(&g,BTN_OK,&open,&tab,&scroll,&potion)==2&&g.hp==38&&!g.potion_count&&g.screen==BETA_FORGE&&g.selection==2);
 game_new(&g,777);g.screen=BETA_FORGE;g.selection=2;game_action(&g,BTN_PAUSE);assert(g.screen==PAUSE_MENU&&game_inventory_available(&g)&&!game_inventory_can_change(&g));h=g;open=0;game_inventory_action(&g,BTN_ALT,&open,&tab,&scroll,&potion);game_inventory_action(&g,BTN_BACK,&open,&tab,&scroll,&potion);assert(!memcmp(&g,&h,sizeof(g)));game_action(&g,BTN_BACK);assert(g.screen==BETA_FORGE&&g.selection==2);
 g.screen=EXPEDITION_STORY;game_action(&g,BTN_PAUSE);assert(g.screen==PAUSE_MENU&&!game_inventory_available(&g));
 puts("PASS cutscenes/setup stay closed; unfinished effects and recaps remain view-only; Forge healing and pause return preserve focus.");return 0;}
