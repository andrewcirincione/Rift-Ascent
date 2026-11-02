/* Legal starter-deck campaigns through Act 7. The same policy and seeds
   run against the released baseline and candidate; no assisted combat. */
#define main old_policy_main
#include "policy.c"
#undef main
int main(int argc,char**argv){
 int runs=argc>1?atoi(argv[1]):100,offset=argc>2?atoi(argv[2]):10000;
 int first=argc>3?atoi(argv[3]):0,last=argc>3?first+1:3;
 char root[]="/tmp/r186r3-campaigns-XXXXXX";assert(mkdtemp(root)&&ascent_open(root));
 AscentAccount profile=ascent_account;
 for(int c=0;c<3;c++){profile.cards[c]=ASCENT_ALL_CARDS;profile.wins[c]=10;}
 profile.potions=0xfffff;profile.relics=1023;
 int entered[8]={0},lost[8]={0},won3=0,won7=0,stalls=0,total=0;
 puts("character,seed,reached,won3,won7,result,floor,hp,actions,combats,turns");
 for(int c=first;c<last;c++)for(int seed=offset+1;seed<=offset+runs;seed++){
  ascent_account=profile;Game g;assert(ascent_begin(&g,0,c,seed*7919u+18440u));
  int reached=1,win3=0,win7=0,result=0,actions=0,battles=0,turns=0,spam=0,lastturn=-1,lastscreen=-1;entered[1]++;
  for(;actions<35000;actions++){
   if(g.act>reached){while(reached<MIN(g.act,7))entered[++reached]++;}
   if(g.screen==ASCENT_ENDING||g.ascent.won||g.act>3)win3=1;
   if(g.act>7){win7=1;result=1;break;}
   if(g.hp<=0||g.screen==DEFEAT)break;
   if(g.screen==COMBAT){if(g.turn==lastturn)spam++;else spam=0;if(spam>40){game_update(&g);game_end_turn(&g);choices(&g);spam=0;continue;}if(lastscreen!=COMBAT&&g.turn==1)battles++;if(g.turn!=lastturn){turns++;lastturn=g.turn;}}
   lastscreen=g.screen;if(!act(&g)){result=-1;break;}
  }
  if(actions>=35000){result=-1;fprintf(stderr,"STALLED c=%d seed=%d screen=%d message=%s\n",c,seed,g.screen,g.message);FILE*f=fopen("/tmp/r5-stall.bin","wb");if(f){fwrite(&g,sizeof(g),1,f);fclose(f);}}
  total++;won3+=win3;won7+=win7;stalls+=result<0;if(!win7&&result>=0)lost[reached]++;
  printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",c,seed,reached,win3,win7,result,g.floor,g.hp,actions,battles,turns);fflush(stdout);
 }
 fprintf(stderr,"Campaigns %d; Act 3 wins %d (%.2f%%); Act 7 wins %d (%.2f%%); stalls %d\n",total,won3,100.0*won3/total,won7,100.0*won7/total,stalls);
 for(int a=1;a<=7;a++)fprintf(stderr,"Act %d: entered %d; lost %d (%.2f%% of entrants)\n",a,entered[a],lost[a],entered[a]?100.0*lost[a]/entered[a]:0);
 return stalls?2:0;
}
