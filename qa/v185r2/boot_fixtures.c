/* Synthetic cold-boot saves, never the owner's files. */
#include <assert.h>
#include <sys/stat.h>
#include "../v185/source/game.c"
static Game184 freeze(const Game*g){
 Game184 o;memset(&o,0,sizeof(o));
 memcpy(&o,g,offsetof(Game184,enemies));memcpy(o.enemies,g->enemies,sizeof(o.enemies));
 memcpy(&o.next_screen,&g->next_screen,offsetof(Game184,regent)-offsetof(Game184,next_screen));
 memcpy(&o.regent,&g->regent,offsetof(RegentState184,hit_count));
 memcpy(o.regent.hit_count,g->regent.hit_count,sizeof(o.regent.hit_count));
 memcpy(o.regent.conqueror,g->regent.conqueror,sizeof(o.regent.conqueror));
 memcpy(&o.regent.vigor,&g->regent.vigor,sizeof(RegentState184)-offsetof(RegentState184,vigor));
 memcpy(&o.ascent,&g->ascent,sizeof(o)-offsetof(Game184,ascent));o.version=184;return o;
}
int main(int argc,char**argv){
 assert(argc==2);char path[256];Game g;AscentAccount account;
 const char*names[]={"fresh","stable185","stable184","beta184","legacy184","corrupt"};
 for(int i=0;i<6;i++){
  snprintf(path,sizeof(path),"%s/%s",argv[1],names[i]);assert(!mkdir(path,0700));
  assert(ascent_open(path));if(i==0)continue;
  game_new(&g,18502);game_choose_character(&g,2);g.screen=MAP;g.ascent.id=1;g.ascent.slot=0;
  g.regent.stars=11;g.ascent.cards=31;
  for(int j=0;j<2;j++)g.ascent.unlocked_cards[j]=g.ascent.unlocked_potions[j]=-1;
  g.ascent.unlocked_relic=-1;assert(game_validate(&g));
  account=*ascent_profile();account.next_run=2;account.cards[2]=31;
  Game184 old=freeze(&g);
  if(i==1||i==2){
   assert(ascent_write("ASCENT18_ACCOUNT",&account,sizeof(account),2));
   assert(ascent_write("ASCENT18_RUN_1",i==1?(void*)&g:(void*)&old,i==1?sizeof(g):sizeof(old),3));
  }else if(i==3){
   assert(ascent_write("BETA184_ACCOUNT",&account,sizeof(account),2));
   assert(ascent_write("BETA184_RUN_1",&old,sizeof(old),3));
  }else if(i==4){
   char file[320];snprintf(file,sizeof(file),"%s/SAVE.DAT",path);FILE*f=fopen(file,"wb");assert(f);
   unsigned checksum=hash_bytes((void*)&old,sizeof(old));
   assert(fwrite(&checksum,4,1,f)==1&&fwrite(&old,sizeof(old),1,f)==1&&!fclose(f));
  }else{
   unsigned junk=0xdeadbeef;
   assert(ascent_write("ASCENT18_RUN_1",&junk,sizeof(junk),3));
   assert(ascent_write("ASCENT18_ACCOUNT",&junk,sizeof(junk),2));
  }
 }
 puts("PASS generated six isolated cold-boot save scenarios");return 0;
}
