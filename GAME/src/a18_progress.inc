/* A1.8 account journal. Profile and each run slot have two independently
 * checksummed generations. Never overwrite the newest valid generation.
 * Nothing here opens or writes another release's save files. */
static A18Profile a18_account;
static A18SlotInfo a18_slot_info[A18_SLOT_COUNT];
static unsigned a18_profile_generation,a18_slot_generation[A18_SLOT_COUNT];
static char a18_storage_root[192];
static int a18_ready,a18_dirty;
static int r180_debug_enabled;
int game_debug_enabled(void){return r180_debug_enabled;}
static void r180_load_options(void){
 r180_debug_enabled=0;
 if(!a18_storage_root[0])return;
 char path[240];snprintf(path,sizeof(path),"%s/A18_OPTIONS.DAT",a18_storage_root);
 FILE*f=fopen(path,"rb");if(!f)return;
 unsigned data[4],extra;int ok=fread(data,sizeof(data),1,f)==1;
 if(ok&&fread(&extra,1,1,f))ok=0;
 fclose(f);
 if(ok&&data[0]==0x5231384fu&&data[1]==1&&data[2]<=1&&
    data[3]==(data[0]^data[1]^data[2]^0xa57c39e1u))r180_debug_enabled=(int)data[2];
}
int game_set_debug_enabled(int enabled){
 enabled=enabled!=0;
 if(a18_storage_root[0]){
  char path[240];snprintf(path,sizeof(path),"%s/A18_OPTIONS.DAT",a18_storage_root);
  unsigned data[4]={0x5231384fu,1,(unsigned)enabled,0};
  data[3]=data[0]^data[1]^data[2]^0xa57c39e1u;
  FILE*f=fopen(path,"wb");if(!f)return 0;
  int ok=fwrite(data,sizeof(data),1,f)==1;if(fclose(f))ok=0;
  if(!ok)return 0;
 }
 r180_debug_enabled=enabled;return 1;
}
static unsigned a18_checksum(const void*ptr,size_t size){
 const unsigned char*p=ptr;unsigned h=2166136261u;
 for(size_t i=0;i<size;i++)h=(h^p[i])*16777619u;return h;
}
int a18_popcount(unsigned v){int n=0;while(v){n+=v&1u;v>>=1;}return n;}
int a18_is_card(int raw){int id=CARD_ID(raw);return id>=A18_CARD_FIRST&&id<A18_CARD_LAST;}
int a18_card_character(int id){return a18_is_card(id)?(CARD_ID(id)-A18_CARD_FIRST)/20:-1;}
static void a18_account_reset(void){
 memset(&a18_account,0,sizeof(a18_account));a18_account.magic=0x41313850u;
 a18_account.version=1;a18_account.next_serial=1;a18_ready=1;a18_dirty=1;
}
const A18Profile*a18_profile(void){if(!a18_ready)a18_account_reset();return &a18_account;}
const A18SlotInfo*a18_slot(int slot){return slot>=0&&slot<A18_SLOT_COUNT?&a18_slot_info[slot]:0;}
int a18_card_available(const Game*g,int id){
 if(!a18_is_card(id))return 1;
 return g&&a18_card_character(id)==g->character&&(g->a18.available_cards&(1u<<((id-A18_CARD_FIRST)%20)));
}
int a18_potion_available(const Game*g,int id){return id>=0&&id<6?1:g&&id>=6&&id<26&&(g->a18.available_potions&(1u<<(id-6)));}
int a18_relic_available(const Game*g,int id){return id<64?1:g&&id>=64&&id<74&&(g->a18.available_relics&(1u<<(id-64)));}
int a18_random_potion(Game*g){int pool[26],n=0;for(int id=0;id<26;id++)if(a18_potion_available(g,id))pool[n++]=id;return pool[rnd(g,n)];}
int a18_secret_unlocked(int character){const A18Profile*p=a18_profile();return character>=0&&character<3&&p->cards[character]==A18_CARD_MASK&&p->wins[character]>=10;}
static int a18_profile_valid(const A18Profile*p){
 if(p->magic!=0x41313850u||p->version!=1||!p->next_serial||(p->potions&~A18_CARD_MASK)||(p->relics&~A18_RELIC_MASK)||(p->true_endings&~7u))return 0;
 for(int c=0;c<3;c++)if(p->cards[c]&~A18_CARD_MASK)return 0;
 return 1;
}
typedef struct {unsigned magic,version,generation,size,checksum;} A18Journal;
static void a18_path(char*out,int cap,const char*name,int half){snprintf(out,cap,"%s/%s.%d",a18_storage_root,name,half);}
static unsigned a18_read_journal(const char*name,int half,void*out,unsigned bytes,int profile){
 char path[240];a18_path(path,sizeof(path),name,half);FILE*f=fopen(path,"rb");if(!f)return 0;
 A18Journal h;int ok=fread(&h,sizeof(h),1,f)==1&&h.magic==0x4131384au&&h.version==1&&h.generation&&h.size==bytes;
 if(ok)ok=fread(out,bytes,1,f)==1;unsigned char extra;if(ok&&fread(&extra,1,1,f))ok=0;
 fclose(f);if(!ok||h.checksum!=a18_checksum(out,bytes))return 0;
 if(profile?!a18_profile_valid(out):!game_validate(out))return 0;return h.generation;
}
static int a18_write_journal(const char*name,unsigned*generation,const void*data,unsigned bytes){
 if(!a18_storage_root[0]){(*generation)++;return 1;}
 unsigned next=*generation+1;if(!next)return 0;
 char path[240];a18_path(path,sizeof(path),name,(int)(next&1u));FILE*f=fopen(path,"wb");if(!f)return 0;
 A18Journal h={0x4131384au,1,next,bytes,a18_checksum(data,bytes)};
 int ok=fwrite(&h,sizeof(h),1,f)==1&&fwrite(data,bytes,1,f)==1;
 if(fclose(f)!=0)ok=0;if(ok)*generation=next;return ok;
}
static int a18_flush_profile(void){
 if(!a18_dirty)return 1;
 if(!a18_write_journal("A18_PROFILE",&a18_profile_generation,&a18_account,sizeof(a18_account)))return 0;
 a18_dirty=0;return 1;
}
static void a18_set_slot_info(int slot,const Game*g){
 A18SlotInfo*i=&a18_slot_info[slot];i->occupied=1;i->corrupt=0;i->character=g->character;i->act=g->act;i->floor=g->floor;i->hp=g->hp;i->maxhp=g->maxhp;i->screen=g->screen;i->serial=g->a18.serial;
}
static int a18_read_slot(int slot,Game*out){
 static Game first,second;char name[32];snprintf(name,sizeof(name),"A18_SLOT_%d",slot+1);
 unsigned a=a18_read_journal(name,0,&first,sizeof(first),0),b=a18_read_journal(name,1,&second,sizeof(second),0);
 if(!a&&!b)return 0;
 const Game*g=b>a?&second:&first;if(g->a18.slot!=slot||!g->a18.serial)return 0;
 *out=*g;a18_slot_generation[slot]=b>a?b:a;a18_set_slot_info(slot,out);return 1;
}
int a18_storage_open(const char*root){
 a18_account_reset();a18_profile_generation=0;memset(a18_slot_generation,0,sizeof(a18_slot_generation));memset(a18_slot_info,0,sizeof(a18_slot_info));
 snprintf(a18_storage_root,sizeof(a18_storage_root),"%s",root?root:"");
 r180_load_options();
 if(!a18_storage_root[0]){game_set_resume_available(0);return 1;}
 A18Profile a,b;unsigned ag=a18_read_journal("A18_PROFILE",0,&a,sizeof(a),1),bg=a18_read_journal("A18_PROFILE",1,&b,sizeof(b),1);
 if(ag||bg){a18_account=bg>ag?b:a;a18_profile_generation=bg>ag?bg:ag;a18_dirty=0;}
 int available=0;static Game saved;
 for(int i=0;i<4;i++)if(a18_read_slot(i,&saved)){
  available++;if(saved.a18.serial>=a18_account.next_serial){a18_account.next_serial=saved.a18.serial+1;a18_dirty=1;}
 }
 /* Optional legacy import reads only a SAVE.DAT manually copied into this
    dedicated A1.8 directory. The source file is never overwritten/deleted. */
 if(!available){char path[240];snprintf(path,sizeof(path),"%s/SAVE.DAT",a18_storage_root);
  if(game_load(&saved,path)&&saved.screen!=TITLE&&saved.screen!=HELP&&saved.screen!=CHARACTER_SELECT&&saved.screen!=NEW_RUN_CONFIRM){
   saved.a18.slot=0;saved.a18.serial=a18_account.next_serial++;a18_dirty=1;
   saved.a18.available_cards=a18_account.cards[saved.character];saved.a18.available_potions=a18_account.potions;saved.a18.available_relics=a18_account.relics;
   if(a18_checkpoint(&saved))available=1;
  }
 }
 game_set_resume_available(available>0);return a18_flush_profile();
}
static void a18_restore_receipt(Game*g,const A18Receipt*r){
 g->a18.act3_claimed=1;
 for(int i=0;i<2;i++){g->a18.card_awards[i]=r->cards[i];g->a18.potion_awards[i]=r->potions[i];}
 g->a18.relic_award=r->relic;g->a18.award_win_number=(int)a18_account.wins[g->character];
}
static void a18_reconcile(Game*g){
 if(!g->a18.serial||g->a18.slot<0||g->a18.slot>=4)return;
 A18Receipt*r=&a18_account.receipts[g->a18.slot];int c=g->character;
 /* A valid slot can repair a profile whose newest generation was damaged.
    Award IDs and win number were committed in that same run snapshot. */
 if(g->a18.act3_claimed&&r->serial<g->a18.serial){
  r->serial=g->a18.serial;
  for(int i=0;i<2;i++){
   int card=g->a18.card_awards[i],potion=g->a18.potion_awards[i];r->cards[i]=card;r->potions[i]=potion;
   if(a18_card_character(card)==c)a18_account.cards[c]|=1u<<((card-334)%20);
   if(potion>=6&&potion<26)a18_account.potions|=1u<<(potion-6);
  }
  r->relic=g->a18.relic_award;if(r->relic>=64&&r->relic<74)a18_account.relics|=1u<<(r->relic-64);
  if(g->a18.award_win_number>(int)a18_account.wins[c])a18_account.wins[c]=g->a18.award_win_number;
  a18_dirty=1;
 }
 if(g->a18.act4_claimed&&r->act4_serial<g->a18.serial){
  r->act4_serial=g->a18.serial;
  /* Legacy clear totals are retained only for save compatibility. */
  a18_dirty=1;
 }
 if(g->a18.true_claimed&&r->true_serial<g->a18.serial){r->true_serial=g->a18.serial;a18_account.true_endings|=1u<<c;a18_dirty=1;}
 if(g->a18.completed_acts>(int)a18_account.best_acts[c]){a18_account.best_acts[c]=g->a18.completed_acts;a18_dirty=1;}
}
int a18_checkpoint(Game*g){
 if(!g||g->a18.slot<0||g->a18.slot>=4||!g->a18.serial)return 1;
 if(!game_validate(g))return 0;
 if(!a18_flush_profile()){msg(g,"Progress could not be saved. Please retry.");return 0;}
 char name[32];snprintf(name,sizeof(name),"A18_SLOT_%d",g->a18.slot+1);
 if(!a18_write_journal(name,&a18_slot_generation[g->a18.slot],g,sizeof(*g))){msg(g,"Run could not be saved. Please retry.");return 0;}
 a18_set_slot_info(g->a18.slot,g);game_set_resume_available(1);return 1;
}
int a18_resume_slot(Game*g,int slot){
 if(slot<0||slot>=4||!a18_read_slot(slot,g))return 0;
 a18_reconcile(g);return a18_flush_profile();
}
int a18_begin_run(Game*g,int slot,int character,unsigned seed){
 if(slot<0||slot>=4||character<0||character>2)return 0;
 a18_profile();if(a18_account.next_serial==0xffffffffu)return 0;
 static Game next;game_new(&next,seed);game_choose_character(&next,character);
 next.a18.slot=slot;next.a18.serial=a18_account.next_serial++;
 next.a18.available_cards=a18_account.cards[character];next.a18.available_potions=a18_account.potions;next.a18.available_relics=a18_account.relics;
 next.a18.secret_route=a18_secret_unlocked(character);a18_account.runs++;a18_dirty=1;
 begin_act(&next);
 if(!a18_checkpoint(&next))return 0;*g=next;return 1;
}
static int a18_choose_locked(Game*g,unsigned mask,int count){int pool[20],n=0;for(int i=0;i<count;i++)if(!(mask&(1u<<i)))pool[n++]=i;return n?pool[rnd(g,n)]:-1;}
static void a18_choose_cards(Game*g,int out[2]){
 int c=g->character,left[4]={0},pairs[12][2],n=0;
 unsigned mask=a18_account.cards[c];out[0]=out[1]=-1;
 for(int i=0;i<20;i++)if(!(mask&(1u<<i)))left[cards[334+c*20+i].rarity]++;
 int total=left[0]+left[1]+left[2]+left[3];
 for(int a=0;a<4;a++)for(int b=a+1;b<4;b++)if(left[a]&&left[b]){
  int feasible=1;for(int r=0;r<4;r++)if(left[r]-(r==a)-(r==b)>(total-2)/2)feasible=0;
  if(feasible){pairs[n][0]=a;pairs[n++][1]=b;}
 }
 if(n){int p=rnd(g,n);for(int j=0;j<2;j++){int pool[20],k=0;for(int i=0;i<20;i++)if(!(mask&(1u<<i))&&cards[334+c*20+i].rarity==pairs[p][j])pool[k++]=i;int i=pool[rnd(g,k)];out[j]=334+c*20+i;mask|=1u<<i;}}
 a18_account.cards[c]=mask;
}
static void a18_record_boss(Game*g){
 if(preview_trace)return;
 a18_profile();
 if(g->act==4&&!g->a18.act4_claimed){
  int c=g->character;A18Receipt*r=g->a18.slot>=0&&g->a18.slot<4?&a18_account.receipts[g->a18.slot]:0;
  if(!r||r->act4_serial<g->a18.serial){
   if(r)r->act4_serial=g->a18.serial;a18_dirty=1;
  }
  g->a18.act4_claimed=1;g->a18.act4_clear_number=a18_account.act4_clears[c];
 }
 g->a18.completed_acts=MAX(g->a18.completed_acts,g->act);
 if(g->a18.completed_acts>(int)a18_account.best_acts[g->character]){a18_account.best_acts[g->character]=g->a18.completed_acts;a18_dirty=1;}
}
static void a18_finish_act3(Game*g){
 g->screen=A18_ENDING;g->selection=0;g->win_screen=g->relic_pending=0;
 if(preview_trace)return;
 a18_profile();a18_record_boss(g);
 if(g->a18.slot<0||g->a18.slot>=4){g->a18.slot=0;g->a18.serial=a18_account.next_serial++;}
 if(!g->a18.serial)g->a18.serial=a18_account.next_serial++;
 A18Receipt*r=&a18_account.receipts[g->a18.slot];
 if(r->serial==g->a18.serial){a18_restore_receipt(g,r);g->a18.secret_route=a18_secret_unlocked(g->character);return;}
 if(g->a18.act3_claimed||r->serial>g->a18.serial){g->a18.act3_claimed=1;for(int i=0;i<2;i++)g->a18.card_awards[i]=g->a18.potion_awards[i]=-1;g->a18.relic_award=-1;return;}
 r->serial=g->a18.serial;a18_account.wins[g->character]++;
 a18_choose_cards(g,r->cards);
 for(int i=0;i<2;i++){int p=a18_choose_locked(g,a18_account.potions,20);r->potions[i]=p<0?-1:p+6;if(p>=0)a18_account.potions|=1u<<p;}
 int relic_id=a18_choose_locked(g,a18_account.relics,10);r->relic=relic_id<0?-1:relic_id+64;if(relic_id>=0)a18_account.relics|=1u<<relic_id;
 a18_restore_receipt(g,r);g->a18.secret_route=a18_secret_unlocked(g->character);a18_dirty=1;
}
static void a18_finish_true(Game*g){
 g->screen=A18_TRUE_END;g->selection=0;g->win_screen=g->relic_pending=0;g->a18.true_claimed=1;
 if(preview_trace)return;a18_record_boss(g);
 a18_account.true_endings|=1u<<g->character;
 if(g->a18.slot>=0&&g->a18.slot<4)a18_account.receipts[g->a18.slot].true_serial=g->a18.serial;
 a18_dirty=1;
}
static void a18_next_act(Game*g){g->endless=1;g->act++;g->floor=0;g->lane=1;g->hp=g->maxhp;map_generate(g);begin_act(g);}
int game_debug_available(const Game*g){
 if(!g||g->hp<=0)return 0;
 switch(g->screen){
  case COMBAT:case MAP:case REST:case SHOP:case EVENT:case EVENT_RESULT:
  case REWARD:case RELIC_RESULT:case BLESSING:case ACT_INTRO:
  case STORY_ACT:case STORY_ACT_BEFORE_BLESSING:case STORY_BOSS:
  case A18_BOSS_INTRO:return 1;
  default:return 0;
 }
}
static int r180_debug_action(Game*g,int b){
 if(b!=BTN_DEBUG_KILL&&b!=BTN_DEBUG_ACT)return 0;
 if(!game_debug_enabled()||!game_debug_available(g))return 1;
 if(b==BTN_DEBUG_KILL){
  if(g->screen!=COMBAT)return 1;
  g->ninja.task_count=g->regent.task_count=g->regent.shuffle_pending=0;
  for(int i=0;i<g->enemy_count;i++)if(g->enemies[i].hp>0){
   int hp=g->enemies[i].hp;g->enemies[i].hp=0;g->enemies[i].block=0;
   record_combat_fx(GFX_ATTACK,-1,i,hp,0);
  }
  check_win(g);msg(g,"Debug: encounter cleared.");
 }else if(g->act<10000){
  /* Skipping is not a boss defeat: do not award wins/unlocks/records. */
  g->ninja.task_count=g->regent.task_count=g->regent.shuffle_pending=0;
  g->win_screen=g->relic_pending=g->ic[37]=g->ic[38]=0;
  g->enemy_count=0;g->screen=MAP;g->a18.secret_route=a18_secret_unlocked(g->character);
  a18_next_act(g);msg(g,"Debug: advanced to the next act.");
 }
 return 1;
}
static int a18_action(Game*g,int b){
 if(r180_debug_action(g,b))return 1;
 /* HELP's legacy screen ID is reused without changing persisted Game bytes. */
 if(g->screen==HELP){
  if(b==BTN_UP||b==BTN_DOWN)g->selection=1-CLAMP(g->selection,0,1);
  else if((b==BTN_LEFT||b==BTN_RIGHT||(b==BTN_OK&&g->selection==0))&&g->selection==0){
   if(!game_set_debug_enabled(!game_debug_enabled()))msg(g,"Could not save options. Setting unchanged.");
   else msg(g,"");
  }else if(b==BTN_BACK||(b==BTN_OK&&g->selection==1)){g->screen=TITLE;g->selection=3;msg(g,"");}
  return 1;
 }
 if(g->screen==TITLE){
  if(b==BTN_UP)g->selection=(g->selection+3)%4;
  else if(b==BTN_DOWN)g->selection=(g->selection+1)%4;
  else if(b==BTN_OK){
   if(g->selection<2){g->a18.menu_mode=g->selection==1;g->screen=A18_SLOTS;g->selection=0;}
   else if(g->selection==2){g->screen=A18_COLLECTION;g->selection=0;g->a18.collection_tab=0;g->a18.collection_character=0;}
   else {g->screen=HELP;g->prior_screen=TITLE;g->selection=0;msg(g,"");}
  }
  return 1;
 }
 if(g->screen==A18_SLOTS){
  if(b==BTN_UP)g->selection=(g->selection+3)%4;
  else if(b==BTN_DOWN)g->selection=(g->selection+1)%4;
  else if(b==BTN_BACK){g->screen=TITLE;g->selection=g->a18.menu_mode?1:0;}
  else if(b==BTN_OK){int slot=g->selection;
   if(!g->a18.menu_mode){if(!a18_resume_slot(g,slot))msg(g,"This slot has no valid saved run.");}
   else {g->a18.slot=slot;g->selection=0;g->screen=a18_slot_info[slot].occupied?A18_SLOT_CONFIRM:CHARACTER_SELECT;}
  }return 1;
 }
 if(g->screen==A18_SLOT_CONFIRM){
  if(b==BTN_UP||b==BTN_DOWN)g->selection=1-g->selection;
  else if(b==BTN_BACK||(b==BTN_OK&&!g->selection)){g->screen=A18_SLOTS;g->selection=g->a18.slot;}
  else if(b==BTN_OK){g->screen=CHARACTER_SELECT;g->selection=0;}
  return 1;
 }
 if(g->screen==CHARACTER_SELECT){
  if(b==BTN_LEFT||b==BTN_UP)g->selection=(g->selection+2)%3;
  else if(b==BTN_RIGHT||b==BTN_DOWN)g->selection=(g->selection+1)%3;
  else if(b==BTN_BACK){g->screen=A18_SLOTS;g->selection=MAX(0,g->a18.slot);}
  else if(b==BTN_OK){int slot=g->a18.slot;if(slot<0||slot>=4)slot=0;unsigned seed=game_random(g);if(!a18_begin_run(g,slot,g->selection,seed))msg(g,"Could not save new run. Existing slot preserved.");}
  return 1;
 }
 if(g->screen==A18_COLLECTION){
  int count=g->a18.collection_tab==2?10:20;
  if(b==BTN_LEFT||b==BTN_RIGHT){g->a18.collection_tab=(g->a18.collection_tab+(b==BTN_LEFT?2:1))%3;g->selection=0;}
  else if(b==BTN_TAB_LEFT||b==BTN_TAB_RIGHT){g->a18.collection_character=(g->a18.collection_character+(b==BTN_TAB_LEFT?2:1))%3;g->selection=0;}
  else if(b==BTN_UP)g->selection=(g->selection+count-1)%count;
  else if(b==BTN_DOWN)g->selection=(g->selection+1)%count;
  else if(b==BTN_BACK){g->screen=TITLE;g->selection=2;}
  return 1;
 }
 if(g->screen>=A18_ENDING&&g->screen<=A18_TRUE_END){
  if(b!=BTN_OK)return 1;
  if(!a18_checkpoint(g))return 1;
  switch(g->screen){
  case A18_ENDING:if(++g->selection>=3){g->screen=A18_UNLOCK_CARDS;g->selection=0;}break;
  case A18_UNLOCK_CARDS:g->screen=A18_UNLOCK_POTIONS;g->selection=0;break;
  case A18_UNLOCK_POTIONS:g->screen=A18_UNLOCK_RELIC;g->selection=0;break;
  case A18_UNLOCK_RELIC:g->screen=g->a18.secret_route?A18_BEYOND:VICTORY;g->selection=0;break;
  case A18_BEYOND:if(++g->selection>=3)a18_next_act(g);break;
  case A18_BOSS_INTRO:if(++g->selection>=3)game_start_battle(g,NODE_BOSS);break;
  case A18_TRUE_END:if(++g->selection>=3){g->screen=VICTORY;g->selection=0;}break;
  }
  a18_checkpoint(g);return 1;
 }
 return 0;
}
int a18_is_ending(int screen){return screen==A18_ENDING||screen==A18_TRUE_END;}
int a18_run_valid(const Game*g){
 const A18Run*r=&g->a18;
 if(r->act4_claimed<0||r->act4_claimed>1)return 0;
 if(r->slot< -1||r->slot>=4||(r->available_cards&~A18_CARD_MASK)||(r->available_potions&~A18_CARD_MASK)||(r->available_relics&~A18_RELIC_MASK)||r->act3_claimed<0||r->act3_claimed>1||r->secret_route<0||r->secret_route>1||r->true_claimed<0||r->true_claimed>1)return 0;
 for(int i=0;i<20;i++)if(r->powers[i]<0||r->powers[i]>1000000)return 0;
 for(int i=0;i<2;i++)if((r->card_awards[i]!=-1&&r->card_awards[i]!=0&&a18_card_character(r->card_awards[i])!=g->character)||(r->potion_awards[i]!=-1&&r->potion_awards[i]!=0&&(r->potion_awards[i]<6||r->potion_awards[i]>25)))return 0;
 if(g->screen==A18_SLOTS&&(g->selection<0||g->selection>=4))return 0;
 if(g->screen==A18_SLOT_CONFIRM&&(g->selection<0||g->selection>1))return 0;
 if(g->screen==A18_COLLECTION&&(g->a18.collection_tab<0||g->a18.collection_tab>2||g->a18.collection_character<0||g->a18.collection_character>2||g->selection<0||g->selection>=(g->a18.collection_tab==2?10:20)))return 0;
 if(g->screen>=A18_ENDING&&g->screen<=A18_TRUE_END&&(g->selection<0||g->selection>2))return 0;
 return 1;
}
