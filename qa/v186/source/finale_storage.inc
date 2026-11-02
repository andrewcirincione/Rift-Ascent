/* Independent receipt: old Game and AscentAccount layouts remain unchanged.
   PREPARED is written before the account's third first-clear. ACTIVE records
   page progress; DONE can never be claimed by a later playthrough. */
enum {FINALE_PREPARED=1,FINALE_ACTIVE,FINALE_DONE};
typedef struct {unsigned run,slot,character,state,page;} AscentFinale;
static AscentFinale ascent_finale;
static unsigned ascent_finale_generation;
static int ascent_finale_valid(const AscentFinale*f){
 return f->run&&f->slot<ASCENT_SLOTS&&f->character<3&&
  ((f->state==FINALE_PREPARED&&f->page==0)||
   (f->state==FINALE_ACTIVE&&f->page<ASCENT_FINALE_PAGES)||
   (f->state==FINALE_DONE&&f->page==ASCENT_FINALE_PAGES));
}
static void ascent_finale_open(void){
 AscentFinale a,b;unsigned ga=0,gb=0;
 int va=ascent_read("RIFTD186_FINALE",0,&a,sizeof(a),&ga)&&ascent_finale_valid(&a);
 int vb=ascent_read("RIFTD186_FINALE",1,&b,sizeof(b),&gb)&&ascent_finale_valid(&b);
 memset(&ascent_finale,0,sizeof(ascent_finale));ascent_finale_generation=0;
 if(va||vb){ascent_finale=vb&&(!va||gb>ga)?b:a;ascent_finale_generation=vb&&(!va||gb>ga)?gb:ga;}
}
static int ascent_finale_write(const AscentFinale*next){
 AscentFinale verify;unsigned gen=ascent_finale_generation+1,vg=0;
 if(!ascent_write("RIFTD186_FINALE",next,sizeof(*next),gen)||
    !ascent_read("RIFTD186_FINALE",gen&1,&verify,sizeof(verify),&vg)||vg!=gen||
    memcmp(next,&verify,sizeof(verify)))return 0;
 ascent_finale=*next;ascent_finale_generation=gen;return 1;
}
static int ascent_finale_prepare(const Game*g){
 if(ascent_finale.state==FINALE_DONE)return 1;
 AscentFinale next={g->ascent.id,g->ascent.slot,(unsigned)g->character,FINALE_PREPARED,0};
 if(!memcmp(&next,&ascent_finale,sizeof(next)))return 1;
 return ascent_finale_write(&next);
}
static int ascent_finale_owner(const Game*g){
 if(g->revision184.custom||g->act!=4||!g->ascent.ascended||g->ascent.slot>=ASCENT_SLOTS||
    ascent_account.ascended!=7||!ascent_finale_valid(&ascent_finale))return 0;
 const AscentReceipt*r=&ascent_account.receipts[g->ascent.slot];
 return ascent_finale.run==g->ascent.id&&ascent_finale.slot==g->ascent.slot&&
  ascent_finale.character==(unsigned)g->character&&r->run==g->ascent.id&&r->ascended;
}
static void ascent_finale_victory(Game*g){g->screen=VICTORY;g->selection=g->scroll=g->master_index=0;}
static int ascent_finale_advance(Game*g,unsigned page){
 if(!ascent_finale_owner(g)||ascent_finale.state==FINALE_DONE)return 0;
 AscentFinale next=ascent_finale;next.page=page;next.state=page==ASCENT_FINALE_PAGES?FINALE_DONE:FINALE_ACTIVE;
 if(!ascent_finale_valid(&next)||!ascent_finale_write(&next)){msg(g,"Could not save finale. Press X to retry.");return 0;}
 msg(g,"");
 if(next.state==FINALE_DONE)ascent_finale_victory(g);else{g->screen=ASCENT_FINALE;g->selection=(int)page;}
 return 1;
}
static void ascent_finale_repair(Game*g){
 int *screen=(g->screen==PAUSE_MENU||g->screen==DECK_VIEW||g->screen==POTION_VIEW||g->screen==RELIC_VIEW)?&g->prior_screen:&g->screen;
 if(*screen!=ASCENT_FINALE&&*screen!=ASCENT_TRUE_END)return;
 int owner=ascent_finale_owner(g);
 if(*screen==ASCENT_FINALE&&(!owner||ascent_finale.state==FINALE_DONE)){
  *screen=VICTORY;g->selection=g->scroll=g->master_index=0;
 }else if(owner&&ascent_finale.state==FINALE_ACTIVE){
  *screen=ASCENT_FINALE;g->selection=(int)ascent_finale.page;
 }
}
