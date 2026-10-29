#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../source/game.c"
int main(void){Game g,b;game_new(&g,181);g.relic_count=0;g.enemy_count=1;g.battle_type=NODE_FIGHT;g.floor=8;
 int prev_hp=0,prev_attack=0;
 for(int act=1;act<=50;act++){g.act=act;g.endless=act>3;init_enemy(&g,&g.enemies[0],25);Enemy*e=&g.enemies[0];long long oldpct=act==1?104:act==2?132:188LL+50LL*(act-3);oldpct+=g.floor+(g.floor-4)/2;int oldhp=(enemy_defs[25].hp*oldpct+50)/100;long long oldatk=act==1?90:act==2?99:124LL+24LL*(act-3);int attack=(enemy_defs[25].attack*oldatk+50)/100+g.floor/7;assert(act<=3?(e->maxhp==oldhp&&e->intent_value==attack):(e->maxhp>oldhp&&e->intent_value>=attack));assert(e->maxhp>prev_hp&&e->intent_value>=prev_attack);prev_hp=e->maxhp;prev_attack=e->intent_value;if(act==3||act==4||act==7||act==10)printf("Act %d HP %d (old %d), intent %d (old %d)\n",act,e->maxhp,oldhp,e->intent_value,attack);}
 g.act=100000;for(int id=0;id<ENEMY_COUNT;id++){init_enemy(&g,&g.enemies[0],id);assert(g.enemies[0].maxhp<=1000000000&&g.enemies[0].intent_value<=100000000);}
 char root[]="/tmp/rift-184r2-boundaries-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
 for(int c=0;c<3;c++)for(int act=3;act<=4;act++){assert(ascent_begin(&g,c,c,200+c));add_relic(&g,81);add_relic(&g,84);add_relic(&g,85);g.act=act;g.endless=act>3;g.ascent.secret=act==4;game_start_battle(&g,NODE_BOSS);g.hp=20;g.expedition.combat[E181_POTION_USED]=1;int gold=g.gold;g.enemies[0].hp=0;check_win(&g);assert(g.hp==25&&g.gold==gold+35);}
 assert(ascent_begin(&g,0,0,300));g.screen=MAP;g.deck_count=MAX_DECK;for(int i=0;i<MAX_DECK;i++)g.deck[i]=0;g.expedition.story_id=0;expedition_prepare_gift(&g);assert(g.expedition.gift_kind==1&&g.expedition.gift==39);
 expedition_mystery(&g);for(int i=0;i<3;i++)game_action(&g,BTN_OK);b=g;game_action(&g,BTN_OK);assert(g.screen==MYSTERY_CARDS&&g.deck_count==MAX_DECK);game_action(&g,BTN_BACK);assert(g.screen==MAP);
 g.screen=RELIC_RESULT;g.relic_pending=1;g.reward[0]=-1;g.version=180;assert(ascent_write("ASCENT18_RUN_1",&g,offsetof(Game,expedition),100));assert(ascent_resume(&b,0)&&b.reward[0]==37);
 game_new(&g,555);int mystery=0;for(int i=0;i<25000;i++)mystery+=game_next_event(&g)==EVENT_COUNT;assert(mystery>900&&mystery<1100);printf("Mystery event %d/25000 rooms\n",mystery);
 puts("PASS unchanged Acts 1-3, increasing Endless pressure, bounded extreme acts, Act 3/4 relic benefits, full decks, old blank-offer recovery, 4% mystery rooms");return 0;}
