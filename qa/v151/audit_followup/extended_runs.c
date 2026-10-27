/* Matched-seed full runs using public game actions, with state validation,
 * exact save/load checkpoints and deterministic continuation checks. */
#define main old_playtest_main
#include "../playtest.c"
#undef main
#include <assert.h>
static int character,variant;
static unsigned long screens[32],card_seen[256],relic_seen[MAX_RELICS],events[EVENT_COUNT],mods[MOD_COUNT];
static void step(Game*g){
 if(g->screen==CHARACTER_SELECT){g->selection=character;game_action(g,BTN_OK);}
 else if(g->screen==BLESSING){g->selection=variant%3;game_action(g,BTN_OK);}
 else if(g->screen==COMBAT)play_turn(g);
 else outside(g);
}
static void failure(const Game*g,int seed,int actions,const char*why){
 fprintf(stderr,"FAIL %s character=%d policy=%d seed=%d actions=%d screen=%d act=%d floor=%d turn=%d hp=%d selection=%d target=%d tasks=%d\n",why,character,mode,seed,actions,g->screen,g->act,g->floor,g->turn,g->hp,g->selection,g->target,g->ninja.task_count);
 FILE*f=fopen("qa/v151/audit_followup/failure.dat","wb");if(f){fwrite(g,sizeof(*g),1,f);fclose(f);}exit(1);
}
int main(int argc,char**argv){
 int runs=argc>1?atoi(argv[1]):500,start=argc>2?atoi(argv[2]):10001;character=argc>3?atoi(argv[3]):1;mode=argc>4?atoi(argv[4]):0;
 int wins=0,deaths[3]={0},reached[3]={0},checkpoints=0;long actions=0,floors=0;
 char path[120];snprintf(path,sizeof(path),"/tmp/rift-audit-%d-%d-%d.dat",character,mode,start);
 for(int i=0;i<runs;i++){
  Game g;variant=i;game_new(&g,(unsigned)(start+i)*2654435761u);int n=0,seenact=1,same=0;reached[0]++;
  while(g.screen!=VICTORY&&g.screen!=DEFEAT&&n<10000){
   if(!game_validate(&g))failure(&g,start+i,n,"invalid live state");
   if(g.screen!=TITLE&&g.screen!=CHARACTER_SELECT)for(int j=0;j<g.deck_count;j++){int id=CARD_ID(g.deck[j]);if(!(id>=50&&id<=53)&&(id>=NINJA_FIRST)!=g.character)failure(&g,start+i,n,"wrong character card pool");}
   screens[g.screen]++;for(int j=0;j<g.deck_count;j++){card_seen[CARD_ID(g.deck[j])]++;mods[CARD_MOD(g.deck[j])]++;}for(int j=0;j<g.relic_count;j++)relic_seen[g.relics[j]]++;if(g.screen==EVENT)events[g.effect_turn]++;
   Game before=g;
   if(n%73==0||g.screen==NINJA_CHOICE||g.screen==CARD_CHOICE){
    Game loaded;
    if(!game_save(&g,path)||!game_load(&loaded,path)||memcmp(&g,&loaded,sizeof(g)))failure(&g,start+i,n,"save/load mismatch");
    step(&g);step(&loaded);if(memcmp(&g,&loaded,sizeof(g)))failure(&g,start+i,n,"resume divergence");checkpoints++;
   }else step(&g);
   n++;actions++;same=memcmp(&before,&g,sizeof(g))?0:same+1;
   if(same>8)failure(&g,start+i,n,"no progress");
   if(g.act>seenact&&g.act<=3){reached[g.act-1]++;seenact=g.act;}
  }
  if(!game_validate(&g))failure(&g,start+i,n,"invalid terminal state");
  if(g.screen==VICTORY)wins++;else if(g.screen==DEFEAT)deaths[g.act-1]++;else failure(&g,start+i,n,"run action limit");
  floors+=(g.act-1)*(MAP_ROWS+1)+g.floor;
 }
 remove(path);
 printf("PASS character=%s runs=%d start=%d policy=%d wins=%d rate=%.3f actions=%ld save_resume=%d mean_floor=%.2f\n",character?"Nightshade":"Warden",runs,start,mode,wins,(double)wins/runs,actions,checkpoints,(double)floors/runs);
 for(int i=0;i<3;i++)printf("act=%d reached=%d deaths=%d\n",i+1,reached[i],deaths[i]);
 printf("screens:");for(int i=0;i<32;i++)if(screens[i])printf(" %d:%lu",i,screens[i]);puts("");
 printf("cards:");for(int i=0;i<CARD_COUNT;i++)if(card_seen[i])printf(" %d",i);puts("");
 printf("relics:");for(int i=0;i<RELIC_COUNT;i++)if(relic_seen[i])printf(" %d",i);puts("");
 printf("events:");for(int i=0;i<EVENT_COUNT;i++)printf(" %d:%lu",i,events[i]);puts("");
 printf("mods:");for(int i=0;i<MOD_COUNT;i++)printf(" %d:%lu",i,mods[i]);puts("");return 0;
}
