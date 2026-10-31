#include <assert.h>
#include <stdio.h>
#include "../v185/source/game.c"
static void fixture(const char*n,Game*g){char p[180];assert(game_validate(g));snprintf(p,sizeof(p),"qa/v185/evidence/fixture-%s.bin",n);FILE*f=fopen(p,"wb");assert(f&&fwrite(g,sizeof(*g),1,f)==1);fclose(f);}
int main(void){char root[]="/tmp/rift185r3-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;
 for(int c=0;c<3;c++){int forge=0;for(int id=0;id<20;id++){game_new(&g,777);game_choose_character(&g,c);g.screen=EXPEDITION_GIFT;g.selection=0;g.expedition.story_id=id;expedition_prepare_gift(&g);forge+=g.expedition.gift_kind==2;assert(game_beta_offer(&g)==(g.expedition.gift_kind==2));for(int n=0;n<4;n++){game_action(&g,BTN_LEFT);assert(g.selection==((n+1)%2));}if(g.expedition.gift_kind!=2){game_action(&g,BTN_OK);assert(g.screen!=BETA_FORGE);}}assert(forge==4);}
 puts("PASS each character: 4/20 story outcomes offer Forge; all outcomes have exactly two navigation choices.");
 game_new(&g,777);game_choose_character(&g,0);g.screen=EXPEDITION_GIFT;g.selection=2;g.expedition.gift_kind=1;g.expedition.gift=13;assert(game_save(&g,"/tmp/r3-oldgift.dat")&&game_load(&h,"/tmp/r3-oldgift.dat")&&h.selection==1);int relics=h.relic_count;game_action(&h,BTN_OK);assert(h.relic_count==relics&&h.screen==MAP);
 game_new(&g,777);game_choose_character(&g,0);g.screen=EXPEDITION_GIFT;g.selection=0;g.expedition.story_id=5;for(int i=0;i<g.deck_count;i++)g.deck[i]|=UPBIT|(MOD_BOUND<<CARD_MOD_SHIFT);expedition_prepare_gift(&g);assert(g.expedition.gift_kind!=2);g.expedition.gift_kind=2;relics=g.relic_count;game_action(&g,BTN_OK);assert(g.screen==EXPEDITION_GIFT&&g.relic_count==relics);game_action(&g,BTN_BACK);assert(g.screen==MAP);
 puts("PASS old third-choice save migrates to Leave; unavailable Forge falls back to a gift; stale Forge cannot award an unrelated relic.");
 for(int option=0;option<3;option++){game_new(&g,777);game_choose_character(&g,0);g.screen=EVENT;g.effect_turn=9;g.selection=option;g.hp=30;g.gold=20;int deck=g.deck_count;game_action(&g,BTN_OK);assert(g.hp==30-(option==0?6:0)&&g.gold==20+(option==0?150:option==1?180:0)&&g.deck_count==deck+(option==1));}
 game_new(&g,777);g.screen=EVENT;g.effect_turn=9;g.hp=6;assert(!game_event_choice_available(&g,0));g.hp=7;assert(game_event_choice_available(&g,0));g.deck_count=MAX_DECK;assert(!game_event_choice_available(&g,1)&&game_event_choice_available(&g,2));
 for(int event=28;event<=38;event+=10){game_new(&g,777);g.screen=EVENT;g.effect_turn=event;g.selection=0;int hp=g.hp;game_action(&g,BTN_OK);assert(g.hp==hp-(event==28?3:4));game_start_battle(&g,NODE_ELITE);assert(g.strength==(event==28?3:2)&&g.vulnerable==(event==28?1:0));for(int i=0;i<5;i++)assert(!g.revision182.next_combat[i]);game_start_battle(&g,NODE_ELITE);assert(g.strength==0&&g.vulnerable==0);}
 puts("PASS Cinder blood/burden/Leave values and affordability; Red Communion and Candle boons apply only once.");
 assert(ascent_begin(&g,0,0,777));g.screen=EXPEDITION_GIFT;g.expedition.gift_kind=1;g.expedition.gift=13;g.selection=0;g.revision182.room_pending=0;fixture("ordinary-gift",&g);
 g.screen=EVENT;g.effect_turn=9;g.hp=30;g.gold=20;fixture("cinder",&g);
 assert(ascent_begin(&g,0,0,777));g.act=2;g.floor=8;game_start_battle(&g,NODE_ELITE);fixture("elite",&g);game_start_battle(&g,NODE_BOSS);fixture("boss",&g);
 puts("PASS 1.8.5-r3 behavior regression suite.");return 0;
}
