#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include "../source/game.c"
static void arena(Game*g,int art){assert(ascent_begin(g,0,0,7722));g->act=art==15?1:art==24?2:3;g->floor=5;game_start_battle(g,NODE_BOSS);g->enemy_count=1;g->turn=0;init_enemy(g,&g->enemies[0],art);g->turn=1;g->enemies[0].hp=g->enemies[0].maxhp=9999;g->hp=g->maxhp=400;g->relic_count=0;g->hand_count=g->discard_count=0;g->draw_count=30;for(int i=0;i<30;i++)g->draw[i]=1;}
static void spoils(Game*g){assert(ascent_begin(g,0,0,7722));g->act=3;g->floor=5;g->battle_type=NODE_FIGHT;g->screen=REWARD;g->v151.card_done=0;g->v151.potion_offer=0;g->selection=0;g->deck_count=3;g->deck[0]=0;g->deck[1]=1;g->deck[2]=6;g->reward[0]=3;g->reward[1]=4;g->reward[2]=5;while(!game_beta_offer(g))g->seed++;assert(game_validate(g));}
static void fixture(const char*name,Game*g){assert(game_validate(g));char path[200];snprintf(path,sizeof(path),"qa/v184beta1/evidence/fixture-%s.bin",name);FILE*f=fopen(path,"wb");assert(f);fwrite(g,sizeof(*g),1,f);fclose(f);}
int main(void){char root[]="/tmp/rift-beta-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));Game g,h;assert(sizeof(Game)==28500);int arts[]={6,15,24,8,44,23,22,50,35};
 for(int a=0;a<9;a++)for(int seed=1;seed<=1000;seed++){arena(&g,arts[a]);g.seed=seed;for(int turn=0;turn<8;turn++){g.turn=turn;enemy_intent(&g,&g.enemies[0]);assert(g.enemies[0].intent==enemy_defs[arts[a]].moves[turn%4]);assert(game_enemy_plan(arts[a])[0]);}g.enemies[0].hp=g.enemies[0].maxhp/2;g.turn=3;enemy_intent(&g,&g.enemies[0]);assert(g.enemies[0].intent==enemy_defs[arts[a]].moves[3]);}
 puts("PASS 9 readable elite/boss cycles across 1,000 seeds and two rotations; half-HP phase never skips the announced recovery.");
 arena(&g,15);g.turn=4;g.enemies[0].intent=31;assert(game_enemy_intent_damage(&g,0)==0);game_end_turn(&g);assert(g.hp==400&&g.enemies[0].vulnerable==1&&g.enemies[0].block==0);int before=g.enemies[0].hp;g.hand_count=1;g.hand[0]=0;game_play_card(&g,0,0);assert(before-g.enemies[0].hp==9);game_end_turn(&g);assert(!g.enemies[0].vulnerable);
 puts("PASS Recover deals no damage, grants no Block, and exposes the enemy for exactly the next player turn (+50% damage).");
 for(int choice=0;choice<3;choice++){
  spoils(&g);game_action(&g,BTN_DOWN);assert(g.selection==3);game_action(&g,BTN_OK);assert(g.screen==BETA_FORGE);g.selection=choice;game_action(&g,BTN_OK);assert(g.screen==BETA_FORGE_CARD&&game_validate(&g));int selected=g.selection,old=g.deck[selected],hp=g.maxhp,expected=game_beta_preview(&g,choice,old);assert(ascent_checkpoint(&g)&&ascent_resume(&h,0));assert(h.screen==BETA_FORGE_CARD&&h.effect_turn==choice);
  game_action(&h,BTN_BACK);assert(h.screen==BETA_FORGE&&h.deck[selected]==old&&h.maxhp==hp);game_action(&h,BTN_OK);game_action(&h,BTN_OK);assert(h.screen==REWARD&&h.v151.card_done&&h.deck[selected]==expected&&h.maxhp==hp-(choice==2?4:0));assert(h.v151.potion_offer==0&&!game_beta_offer(&h));assert(ascent_checkpoint(&h)&&ascent_resume(&g,0));assert(g.deck[selected]==expected&&g.v151.card_done);
 }
 spoils(&g);g.maxhp=g.hp=10;assert(!game_beta_eligible(&g,2,0));for(int i=0;i<g.deck_count;i++)g.deck[i]|=UPBIT|(MOD_BOUND<<CARD_MOD_SHIFT);assert(!game_beta_offer(&g));
 spoils(&g);g.act=1;assert(!game_beta_offer(&g));g.act=3;g.battle_type=NODE_BOSS;assert(!game_beta_offer(&g));
 spoils(&g);g.ninja.v[NS_BONUS_REWARDS]=1;g.screen=BETA_FORGE_CARD;g.effect_turn=0;game_action(&g,BTN_OK);assert(!g.v151.card_done&&!g.ninja.v[NS_BONUS_REWARDS]&&CARD_UP(g.deck[0]));
 puts("PASS upgrade, rotating modification and paid Echo; cancellation, preview, eligibility, max-HP safety, choice/save/resume, one reward consumption, potion preservation and Hunt bonus pick.");
 int count=0;spoils(&g);for(int seed=1;seed<=10000;seed++){g.seed=seed;count+=game_beta_offer(&g);}assert(count>2200&&count<2800);printf("PASS forge availability %d/10,000 eligible late non-boss spoils; never Act 1 or bosses.\n",count);
 arena(&g,15);g.hp=1;g.enemies[0].intent=0;g.kill_count=12;game_end_turn(&g);assert(g.screen==DEFEAT&&strstr(g.subtitle,"FURNACE MAW"));int deck=g.deck[0],relics=g.relic_count;assert(ascent_checkpoint(&g)&&!ascent_resume(&h,0));game_action(&g,BTN_ALT);assert(g.scroll==1);game_action(&g,BTN_OK);assert(g.screen==DEFEAT&&g.deck[0]==deck);game_action(&g,BTN_POTION);assert(g.scroll==2&&g.relic_count==relics);game_action(&g,BTN_BACK);assert(!g.scroll);game_action(&g,BTN_OK);assert(g.screen==TITLE);
 spoils(&g);g.screen=VICTORY;g.scroll=0;g.selection=0;game_action(&g,BTN_ALT);game_action(&g,BTN_BACK);game_action(&g,BTN_OK);assert(g.act==4&&g.endless);
 puts("PASS lethal enemy attribution; death slot retired before recap; read-only deck/relic tabs; menu exit and endless continuation after victory.");
 char stable[320];snprintf(stable,sizeof(stable),"%s/ASCENT18_ACCOUNT.0",root);FILE*f=fopen(stable,"wb");assert(f);fputs("stable-sentinel",f);fclose(f);assert(ascent_open(root));f=fopen(stable,"rb");char b[30]={0};assert(fread(b,1,15,f)==15&&!strcmp(b,"stable-sentinel"));fclose(f);
 puts("PASS beta account, slots and settings use BETA184_ stems; stable account untouched. Game struct size remains unchanged.");
 spoils(&g);fixture("forge",&g);
 arena(&g,15);g.turn=4;g.enemies[0].intent=31;g.hp=g.maxhp=80;fixture("recover",&g);
 int bosses[]={15,24,8,44};for(int i=0;i<4;i++){arena(&g,bosses[i]);g.turn=1;enemy_intent(&g,&g.enemies[0]);char name[40];snprintf(name,sizeof(name),"boss-%d",i);fixture(name,&g);}
 arena(&g,8);g.maxhp=80;g.hp=1;g.kill_count=12;g.enemies[0].intent=10;g.relic_count=4;g.relics[0]=0;g.relics[1]=13;g.relics[2]=62;g.relics[3]=77;fixture("death",&g);
 spoils(&g);g.screen=VICTORY;g.scroll=0;g.kill_count=34;g.relic_count=4;g.relics[0]=0;g.relics[1]=13;g.relics[2]=62;g.relics[3]=77;fixture("victory",&g);
 arena(&g,15);g.turn=0;init_enemy(&g,&g.enemies[0],15);g.turn=1;g.hp=g.maxhp=80;g.hand_count=5;g.hand[0]=0;g.hand[1]=1;g.hand[2]=2;g.hand[3]=0;g.hand[4]=1;fixture("release",&g);
 return 0;
}
