/* Matched boss-entry states collected from legal, unassisted campaigns. */
#define main policy_main
#ifdef STRONG_POLICY
#include "../../v185r3/campaigns.c"
#else
#include "../../v185r3/reference_campaigns.c"
#endif
#undef main
typedef struct {int character,seed,act;Game game;} Entry;
int main(int argc,char**argv){
 assert(argc>=3);char root[]="/tmp/r186-boss-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
 AscentAccount empty=*ascent_profile();for(int c=0;c<3;c++)empty.cards[c]=ASCENT_ALL_CARDS;
 empty.potions=0xfffff;empty.relics=1023;int n3=0,n7=0;
 if(!strcmp(argv[1],"capture")){
  FILE*f=fopen(argv[2],"wb");assert(f);int runs=argc>3?atoi(argv[3]):1000,offset=argc>4?atoi(argv[4]):30000;
  for(int c=0;c<3;c++)for(int seed=offset+1;seed<=offset+runs;seed++){
   ascent_account=empty;Game g;assert(ascent_begin(&g,0,c,seed*7919u+18440u));int seen3=0,seen7=0,spam=0,lastturn=-1;
   for(int actions=0;actions<35000;actions++){
    if(g.hp<=0||g.screen==DEFEAT||g.act>7)break;
    if(g.selection==2&&((g.screen==STORY_BOSS&&g.act==3&&!seen3)||(g.screen==ABOMINATION_BEFORE&&!seen7))){
     Entry e={c,seed,g.act,g};assert(fwrite(&e,sizeof(e),1,f)==1);
     if(g.act==3){seen3=1;n3++;}else{seen7=1;n7++;break;}
    }
    if(g.screen==COMBAT){if(g.turn==lastturn)spam++;else spam=0;lastturn=g.turn;if(spam>40){game_end_turn(&g);choices(&g);spam=0;continue;}}
    if(!act(&g))break;
   }
  }
  fclose(f);fprintf(stderr,"Captured natural pre-boss states: Act 3 %d; Act 7 %d; campaigns %d\n",n3,n7,3*runs);return 0;
 }
 FILE*f=fopen(argv[2],"rb");assert(f);Entry e;int wins3=0,wins7=0,stalls=0;
 puts("character,seed,act,enemies,result,hp_before,hp_after,turns");
 while(fread(&e,sizeof(e),1,f)==1){
  ascent_account=empty;Game g=e.game;g.version=GAME_VERSION;
  int hp=g.hp;game_action(&g,BTN_OK);assert(g.screen==COMBAT&&g.battle_type==NODE_BOSS);
  int enemies=g.enemy_count,steps=0,spam=0,lastturn=-1;
  while(g.hp>0&&(g.screen==COMBAT||g.screen==NINJA_CHOICE||g.screen==REGENT_CHOICE||g.screen==CARD_CHOICE)&&steps++<6000){
   if(g.screen==COMBAT){if(g.turn==lastturn)spam++;else spam=0;lastturn=g.turn;if(spam>40){game_end_turn(&g);choices(&g);spam=0;continue;}}
   assert(act(&g));
  }
  int win=g.hp>0&&g.screen!=DEFEAT&&steps<6000;stalls+=steps>=6000;
  if(e.act==3){n3++;wins3+=win;}else{n7++;wins7+=win;}
  printf("%d,%d,%d,%d,%d,%d,%d,%d\n",e.character,e.seed,e.act,enemies,win,hp,g.hp,g.turn);
 }
 fclose(f);fprintf(stderr,"Act 3 %d/%d (%.2f%%); Act 7 %d/%d (%.2f%%); stalls %d\n",wins3,n3,n3?100.0*wins3/n3:0,wins7,n7,n7?100.0*wins7/n7:0,stalls);
 return stalls?2:0;
}
