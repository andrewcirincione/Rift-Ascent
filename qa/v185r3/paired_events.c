/* Fork identical naturally reached event states into all legal choices, then
 * follow each for four rooms using the same decision policy. Collect with the
 * baseline game, replay the exact same input file with both game versions. */
#define main campaign_main
#include "campaigns.c"
#undef main
static int target(int e){return e==0||e==7||e==9||e==28||e==38;}
typedef struct{int character,seed;Game game;} State;
static int rollout(Game*g,int room){for(int n=0;n<2500;n++){if(g->hp<=0||g->screen==DEFEAT)return 0;if(g->act>3||g->screen==VICTORY||g->act*MAP_ROWS+g->floor>=room+4)return 1;if(!act(g))return -1;}return -1;}
int main(int argc,char**argv){assert(argc==3);char root[]="/tmp/rift-paired-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));int collect=!strcmp(argv[1],"--collect");FILE*f=fopen(argv[2],collect?"wb":"rb");assert(f);
 if(collect){int samples[EVENT_COUNT]={0};for(int c=0;c<3;c++)for(int seed=1;seed<=100;seed++){State s={.character=c,.seed=seed};Game*g=&s.game;ascent_account.cards[c]=ASCENT_ALL_CARDS;ascent_account.potions=0xfffff;ascent_account.relics=1023;assert(ascent_begin(g,0,c,seed*7919u+180000));for(int n=0;n<15000&&g->hp>0&&g->act<=3&&g->screen!=DEFEAT&&g->screen!=VICTORY;n++){if(g->screen==EVENT&&target(g->effect_turn)&&samples[g->effect_turn]<60){samples[g->effect_turn]++;assert(fwrite(&s,sizeof(s),1,f)==1);}if(!act(g))break;}}for(int i=0;i<EVENT_COUNT;i++)if(target(i))fprintf(stderr,"Collected %d %s states\n",samples[i],game_event_title(i));}
 else{puts("character,seed,event,choice,initial_hp,initial_gold,initial_maxhp,survived,end_hp,end_gold,end_maxhp,end_deck_count");State s;while(fread(&s,sizeof(s),1,f)==1){Game*g=&s.game;for(int c=0;c<game_event_choice_count(g);c++)if(game_event_choice_available(g,c)){AscentAccount account=ascent_account;Game h=*g;h.selection=c;game_action(&h,BTN_OK);if(h.screen==EVENT_CARD)event_card(&h);int survived=rollout(&h,g->act*MAP_ROWS+g->floor);ascent_account=account;printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s.character,s.seed,g->effect_turn,c,g->hp,g->gold,g->maxhp,survived,h.hp,h.gold,h.maxhp,h.deck_count);}}}
 fclose(f);return 0;}
