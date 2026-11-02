#include <stdio.h>
#include <string.h>
static int fail_account,fail_finale;
static FILE*qa_fopen(const char*path,const char*mode){
 if(mode[0]=='w'){
  if(fail_account&&strstr(path,"ASCENT18_ACCOUNT")){fail_account--;return 0;}
  if(fail_finale&&strstr(path,"ASCENT18_FINALE")){fail_finale--;return 0;}
 }
 return fopen(path,mode);
}
#define fopen qa_fopen
#define main previous_revision_main
#include "../v186r3/revision.c"
#undef main
#undef fopen
static void fresh(char*root){assert(mkdtemp(root)&&ascent_open(root));}
static void next(Game*g,int count){for(int n=0;n<count;n++)game_action(g,BTN_OK);}
static void five_win_gate(void){
 char root[]="/tmp/r186r4-five-XXXXXX";fresh(root);Game g,h;
 for(int c=0;c<3;c++)for(int win=1;win<=11;win++){
  assert(ascent_begin(&g,c,c,18604+c*100+win));unsigned before=ascent_account.cards[c];clear(&g,3);
  assert(g.screen==ASCENT_ENDING&&ascent_account.wins[c]==(unsigned)win);
  assert(ascent_secret(c)==(win>=5)&&g.ascent.secret==(unsigned)(win>=5));
  assert(ascent_popcount(ascent_account.cards[c])==MIN(win,10)*2);
  assert(ascent_popcount(ascent_account.cards[c]^before)==(win<=10?2:0));
  if(win<=10){assert(g.ascent.unlocked_cards[0]!=g.ascent.unlocked_cards[1]);assert(cards[g.ascent.unlocked_cards[0]].rarity!=cards[g.ascent.unlocked_cards[1]].rarity);}
  else assert(g.ascent.unlocked_cards[0]==-1&&g.ascent.unlocked_cards[1]==-1);
  assert(ascent_checkpoint(&g)&&ascent_resume(&h,c));assert(ascent_complete(&h,0)&&ascent_account.wins[c]==(unsigned)win);
  next(&g,3);assert(g.screen==ASCENT_CARDS);next(&g,3);assert(g.screen==(win>=5?ASCENT_REVELATION:VICTORY));
  if(win>=5){next(&g,3);assert(g.act==4&&g.ascent.secret);}
 }
 assert(ascent_open(root));for(int c=0;c<3;c++)assert(ascent_secret(c)&&ascent_account.wins[c]==11&&ascent_account.cards[c]==ASCENT_ALL_CARDS);
 ascent_account.wins[1]=4;assert(!ascent_secret(1)&&ascent_secret(0)&&ascent_secret(2));
 assert(ascent_begin(&g,1,1,44));g.act=3;g.screen=ASCENT_REVELATION;g.selection=1;g.ascent.secret=1;
 assert(ascent_checkpoint(&g)&&ascent_resume(&h,1));assert(h.screen==ASCENT_ENDING&&!h.ascent.secret);
 assert(game_save(&g,"/tmp/r186r4-old.dat")&&game_load(&h,"/tmp/r186r4-old.dat")&&h.screen==ASCENT_ENDING);
 ascent_account.wins[1]=5;assert(ascent_commit_account());g.screen=ASCENT_RELIC;g.ascent.secret=0;assert(ascent_checkpoint(&g)&&ascent_resume(&h,1));assert(h.ascent.secret);next(&h,1);assert(h.screen==ASCENT_REVELATION);
 puts("PASS all characters: first/fourth/fifth/sixth/tenth/eleventh Act 3 routes; exactly two distinct-rarity cards per win through ten; no unlocks lost at five; independent character gate; cached old save repair; idempotent wins.");
}
static void finale_orders(void){
 const int orders[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
 Game g,h,stale,combat;
 for(int order=0;order<6;order++){
  char root[]="/tmp/r186r4-orders-XXXXXX";fresh(root);
  for(int c=0;c<3;c++)ascent_account.wins[c]=5;
  for(int i=0;i<3;i++){
   int c=orders[order][i];assert(ascent_begin(&g,(order+i)%4,c,300+order*3+i));
   g.act=4;g.endless=1;game_start_battle(&g,NODE_BOSS);combat=g;assert(ascent_checkpoint(&g));
   clear(&g,4);assert(g.screen==ASCENT_TRUE_END&&g.ascent.ascended);
   assert(ascent_account.ascended==((1u<<orders[order][0])|(i>0?1u<<orders[order][1]:0)|(i>1?1u<<orders[order][2]:0)));
   if(i<2){next(&g,3);assert(g.screen==VICTORY&&!ascent_finale.state);continue;}
   assert(ascent_finale_owner(&g)&&ascent_finale.state==FINALE_PREPARED);
   /* Power loss after account receipt but before the winning run snapshot. */
   assert(ascent_open(root)&&ascent_resume(&h,g.ascent.slot));assert(h.screen==COMBAT&&!h.ascent.ascended);
   clear(&h,4);assert(h.screen==ASCENT_TRUE_END&&ascent_finale_owner(&h));g=h;
   assert(game_save(&g,"/tmp/r186r4-before-finale.dat"));next(&g,3);assert(g.screen==ASCENT_FINALE&&g.selection==0&&ascent_finale.state==FINALE_ACTIVE);
   assert(game_save(&g,"/tmp/r186r4-finale-stale.dat"));stale=g;
   for(int page=0;page<ASCENT_FINALE_PAGES;page++){
    assert(g.selection==page&&g.screen==ASCENT_FINALE&&game_validate(&g));
    assert(game_story_kind(&g)==1&&game_music_silent(&g));
    assert(ascent_checkpoint(&g)&&ascent_open(root)&&ascent_resume(&h,g.ascent.slot));assert(h.screen==ASCENT_FINALE&&h.selection==page);g=h;
    assert(game_load(&h,"/tmp/r186r4-finale-stale.dat")&&h.screen==ASCENT_FINALE&&h.selection==page);
    next(&g,1);
   }
   assert(g.screen==VICTORY&&ascent_finale.state==FINALE_DONE);
   assert(ascent_open(root)&&game_load(&h,"/tmp/r186r4-finale-stale.dat")&&h.screen==VICTORY);
   assert(game_load(&h,"/tmp/r186r4-before-finale.dat"));next(&h,3);assert(h.screen==VICTORY);
   next(&stale,1);assert(stale.screen==VICTORY);clear(&combat,4);next(&combat,3);assert(combat.screen==VICTORY);
  }
  for(int c=0;c<3;c++){
   assert(ascent_begin(&g,c,c,700+c));clear(&g,4);next(&g,3);assert(g.screen==VICTORY&&ascent_finale.state==FINALE_DONE);
  }
 }
 puts("PASS all six character clear orders and all four save slots: first/second clear skip shared finale, third first-clear gets all five pages, every page cold-resumes, stale saves/receipts and subsequent character wins never replay it.");
}
static void legacy_and_custom(void){
 Game g;char root[]="/tmp/r186r4-legacy-XXXXXX";fresh(root);
 ascent_account.ascended=7;for(int c=0;c<3;c++)ascent_account.wins[c]=10;assert(ascent_commit_account()&&ascent_open(root));
 for(int c=0;c<3;c++){assert(ascent_begin(&g,c,c,800+c));clear(&g,4);next(&g,3);assert(g.screen==VICTORY&&!ascent_finale.state);}
 char custom[]="/tmp/r186r4-custom-XXXXXX";fresh(custom);ascent_account.ascended=3;ascent_account.wins[2]=5;
 assert(ascent_begin(&g,0,2,900));g.revision184.custom=1;clear(&g,4);next(&g,3);assert(g.screen==VICTORY&&ascent_account.ascended==3&&!ascent_finale.state);
 puts("PASS existing all-three-clear profiles do not receive a retroactive replay; custom runs do not award or consume the finale.");
}
static void write_failures(void){
 Game g,h;char root[]="/tmp/r186r4-failures-XXXXXX";fresh(root);ascent_account.ascended=3;ascent_account.wins[2]=5;
 assert(ascent_begin(&g,3,2,1000));fail_finale=1;clear(&g,4);
 assert(g.screen==ASCENT_SAVE_ERROR&&ascent_account.ascended==3&&!ascent_finale.state);
 fail_account=1;next(&g,1);assert(g.screen==ASCENT_SAVE_ERROR&&ascent_account.ascended==3&&ascent_finale.state==FINALE_PREPARED&&!ascent_finale_owner(&g));
 assert(ascent_open(root));next(&g,1);assert(g.screen==ASCENT_TRUE_END&&ascent_account.ascended==7&&ascent_finale_owner(&g));
 next(&g,2);fail_finale=1;next(&g,1);assert(g.screen==ASCENT_TRUE_END&&g.selection==2&&ascent_finale.state==FINALE_PREPARED);next(&g,1);assert(g.screen==ASCENT_FINALE&&g.selection==0);
 fail_finale=1;next(&g,1);assert(g.screen==ASCENT_FINALE&&g.selection==0&&ascent_finale.page==0);next(&g,4);assert(g.selection==4);
 fail_finale=1;next(&g,1);assert(g.screen==ASCENT_FINALE&&g.selection==4&&ascent_finale.state==FINALE_ACTIVE);next(&g,1);assert(g.screen==VICTORY&&ascent_finale.state==FINALE_DONE);
 assert(ascent_checkpoint(&g)&&ascent_open(root)&&ascent_resume(&h,3)&&h.screen==VICTORY);
 puts("PASS injected preparation, account-commit, scene-entry, page-progress and final-completion write failures: retry preserves rewards and cannot lose or duplicate the finale.");
}
int main(void){five_win_gate();finale_orders();legacy_and_custom();write_failures();gifts();forge();damage();puts("PASS five-win secret access, ten-win unlock progression and one-time Three as One finale.");return 0;}
