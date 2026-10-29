#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>
#include "../source/game.c"
static void save(const char*name,const void*data,size_t size){char path[240];snprintf(path,sizeof(path),"qa/rebuild180/evidence/%s.bin",name);FILE*f=fopen(path,"wb");assert(f&&fwrite(data,size,1,f)==1);fclose(f);}
int main(void){char temp[]="/tmp/rift-fixtures-XXXXXX";assert(mkdtemp(temp));assert(ascent_open(temp));Game g[3];
 for(int c=0;c<3;c++){ascent_account.cards[c]=ASCENT_ALL_CARDS^(1u<<17)^(1u<<19);ascent_account.wins[c]=9;ascent_account.best_act[c]=3;}
 ascent_account.potions=(1u<<18)-1;ascent_account.relics=511;ascent_account.debug=1;
 for(int c=0;c<3;c++){assert(ascent_begin(&g[c],c,c,100+c));g[c].act=3;game_start_battle(&g[c],NODE_BOSS);g[c].hp=g[c].maxhp=800;g[c].energy=10;g[c].regent.stars=20;g[c].hand_count=2;g[c].hand[0]=334+c*20;g[c].hand[1]=335+c*20;g[c].selection=0;g[c].enemies[0].hp=1;char name[60];snprintf(name,sizeof(name),"fixture-boss3-%d",c);save(name,&g[c],sizeof(Game));}
 save("fixture-account",&ascent_account,sizeof(ascent_account));
 Game a=g[0];a.act=4;a.endless=1;a.ascent.secret=1;a.ascent.won=1;game_story_start(&a,1);a.screen=ASCENT_BOSS;a.selection=0;save("fixture-boss4-before",&a,sizeof(a));
 a=g[0];a.act=1;game_start_battle(&a,NODE_FIGHT);a.hp=a.maxhp=800;a.hand_count=3;a.hand[0]=334;a.hand[1]=335;a.hand[2]=336;a.selection=0;save("fixture-combat",&a,sizeof(a));
 for(int act=1;act<=5;act++){a=g[0];a.act=act;a.endless=act>3;a.screen=MAP;a.selection=0;char n[60];snprintf(n,sizeof(n),"fixture-map-%d",act);save(n,&a,sizeof(a));}
 for(int group=0;group<2;group++){a=g[0];a.act=1;a.screen=POTION_VIEW;a.prior_screen=MAP;a.selection=0;a.potion_count=3;a.message[0]=0;for(int i=0;i<3;i++)a.potions[i]=3*group+i;char n[60];snprintf(n,sizeof(n),"fixture-old-potions-%d",group);save(n,&a,sizeof(a));}
 for(int id=0;id<4;id++){a=g[0];a.act=1;a.screen=RELIC_RESULT;a.reward[0]=(int[]){0,7,17,35}[id];a.relic_pending=1;a.selection=0;char n[60];snprintf(n,sizeof(n),"fixture-old-relic-%d",id);save(n,&a,sizeof(a));}
 FILE*f=fopen("qa/rebuild180/evidence/state-layout.json","w");assert(f);fprintf(f,"{\"size\":%zu,\"account_size\":%zu",sizeof(Game),sizeof(AscentAccount));
#define OFF(x) fprintf(f,",\"" #x "\":%zu",offsetof(Game,x))
 OFF(screen);OFF(selection);OFF(scroll);OFF(act);OFF(character);OFF(hp);OFF(maxhp);OFF(ascent);OFF(enemy_count);OFF(enemies);OFF(floor);OFF(battle_type);OFF(combat_music);OFF(potion_count);OFF(potions);OFF(hand_count);OFF(hand);OFF(reward);OFF(prior_screen);
 fprintf(f,",\"enemy_size\":%zu,\"enemy_hp\":%zu,\"enemy_art\":%zu,\"account_cards\":%zu,\"account_wins\":%zu,\"account_ascended\":%zu,\"account_debug\":%zu,\"screens\":{",sizeof(Enemy),offsetof(Enemy,hp),offsetof(Enemy,art),offsetof(AscentAccount,cards),offsetof(AscentAccount,wins),offsetof(AscentAccount,ascended),offsetof(AscentAccount,debug));
#define SCREEN(x) fprintf(f,"\"" #x "\":%d,",x)
 SCREEN(TITLE);SCREEN(MAP);SCREEN(COMBAT);SCREEN(VICTORY);SCREEN(ASCENT_SLOTS_VIEW);SCREEN(ASCENT_OPTIONS);SCREEN(ASCENT_COLLECTION);SCREEN(ASCENT_ENDING);SCREEN(ASCENT_CARDS);SCREEN(ASCENT_POTIONS);SCREEN(ASCENT_RELIC);SCREEN(ASCENT_REVELATION);SCREEN(ASCENT_BOSS);SCREEN(ASCENT_TRUE_END);SCREEN(CHARACTER_SELECT);SCREEN(ACT_INTRO);SCREEN(BLESSING);SCREEN(STORY_ACT_BEFORE_BLESSING);fprintf(f,"\"DEFEAT\":%d}}\n",DEFEAT);fclose(f);return 0;}
