/* Fresh 1.8 storage. Alternating checksummed generations avoid overwriting the
 * last valid snapshot. Account receipts are committed BEFORE the run result,
 * so a power cut between writes cannot award a completion twice. */
static AscentAccount ascent_account;
static AscentSlot ascent_slots[ASCENT_SLOTS];
static char ascent_root[256]=".";
static int ascent_ready;
static unsigned ascent_fast_setting;
static int ascent_path(char*out,int cap,const char*stem,unsigned index){
 return snprintf(out,cap,"%s/%s.%u",ascent_root,stem,index)<cap;
}
static int ascent_read(const char*stem,unsigned side,void*dst,unsigned bytes,unsigned*generation){
 char path[320];unsigned head[5];if(!ascent_path(path,sizeof(path),stem,side))return 0;
 FILE*f=fopen(path,"rb");if(!f)return 0;
 int ok=fread(head,sizeof(head),1,f)==1&&head[0]==0x52413846u&&head[1]==1&&head[3]==bytes;
 if(ok)ok=fread(dst,bytes,1,f)==1;
 unsigned char extra;if(ok)ok=fread(&extra,1,1,f)==0;
 if(fclose(f))ok=0;
 if(ok)ok=hash_bytes(dst,bytes)==head[4];
 if(ok)*generation=head[2];return ok;
}
static int ascent_write(const char*stem,const void*data,unsigned bytes,unsigned generation){
 char path[320];unsigned h[5]={0x52413846u,1,generation,bytes,hash_bytes(data,bytes)};
 if(!ascent_path(path,sizeof(path),stem,generation&1u))return 0;
 FILE*f=fopen(path,"wb");if(!f)return 0;
 int ok=fwrite(h,sizeof(h),1,f)==1&&fwrite(data,bytes,1,f)==1;
 if(fclose(f))ok=0;return ok;
}
static int ascent_account_valid(const AscentAccount*a){
 if(a->magic!=0x52413841u||a->version!=1||!a->next_run||a->debug>1||a->ascended>7||a->potions>0xfffffu||a->relics>1023)return 0;
 for(int c=0;c<3;c++)if(a->cards[c]>ASCENT_ALL_CARDS||a->best_act[c]>100000)return 0;
 return 1;
}
static int ascent_commit_account(void){
 unsigned gen=ascent_account.generation+1;
 AscentAccount verify;unsigned vg=0;
 if(!ascent_write("ASCENT18_ACCOUNT",&ascent_account,sizeof(ascent_account),gen)||
    !ascent_read("ASCENT18_ACCOUNT",gen&1u,&verify,sizeof(verify),&vg)||vg!=gen||
    memcmp(&verify,&ascent_account,sizeof(verify)))return 0;
 ascent_account.generation=gen;return 1;
}
const AscentAccount*ascent_profile(void){return &ascent_account;}
const AscentSlot*ascent_slot(int n){return n>=0&&n<ASCENT_SLOTS?&ascent_slots[n]:0;}
int ascent_popcount(unsigned n){int count=0;for(;n;n&=n-1)count++;return count;}
int ascent_secret(int c){return c>=0&&c<3&&ascent_account.cards[c]==ASCENT_ALL_CARDS;}
int ascent_fast(void){return ascent_fast_setting!=0;}
int ascent_set_fast(int enabled){unsigned value=!!enabled;if(!ascent_write("ASCENT18_SPEED",&value,sizeof(value),0))return 0;ascent_fast_setting=value;return 1;}
int ascent_debug(void){return ascent_account.debug!=0;}
int ascent_set_debug(int enabled){unsigned old=ascent_account.debug;ascent_account.debug=!!enabled;if(ascent_commit_account())return 1;ascent_account.debug=old;return 0;}
static void ascent_slot_info(int slot,const Game*g,unsigned gen){
 ascent_slots[slot]=(AscentSlot){1,g->character,g->act,g->floor,g->hp,g->maxhp,g->screen,gen,g->revision184.custom};
}
static void revision182_migrate_room(Game*g){
 int s=g->screen;if(s==DECK_VIEW||s==POTION_VIEW||s==RELIC_VIEW)s=g->prior_screen;
 int in_room=s==COMBAT||s==REWARD||s==RELIC_RESULT||s==EVENT||s==EVENT_CARD||s==EVENT_RESULT||s==SHOP||s==REST||s==MYSTERY_STORY||s==MYSTERY_CARDS;
 if(in_room&&g->floor>0){int lane=CLAMP(g->map_selected[g->floor-1],0,2);int node=g->map[g->floor-1][lane];
  g->revision182.room_type=node;g->revision182.room_pending=node!=NODE_BOSS&&!g->win_screen&&!g->expedition.story_pending;
 }
 if(g->battle_type==NODE_ELITE&&g->combat_music==5)g->combat_music=6;
 else if(g->battle_type==NODE_FIGHT&&g->combat_music==6)g->combat_music=5;
}
static int ascent_read_run(const char*stem,unsigned side,Game*g,unsigned*generation){
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,sizeof(*g),generation)){
  
  return game_validate(g);
 }
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,offsetof(Game,revision184),generation)&&(g->version==182||g->version==183)){
  if(g->version==182&&g->screen==ASCENT_COLLECTION&&g->scroll==5){g->scroll=4;g->selection=70+g->selection;}
  g->version=GAME_VERSION;
  /* The old topology remains stable until the next act. */
  return game_validate(g);
 }
 memset(g,0,sizeof(*g));
 if(ascent_read(stem,side,g,offsetof(Game,revision182),generation)&&g->version==181){
  g->version=GAME_VERSION;g->revision182.rare_offset=-5;revision182_migrate_room(g);return game_validate(g);
 }
 memset(g,0,sizeof(*g));
 if(!ascent_read(stem,side,g,offsetof(Game,expedition),generation)||g->version!=180||g->relic_count>64)return 0;
 g->version=GAME_VERSION;g->revision182.rare_offset=-5;revision182_migrate_room(g);
 if(g->screen==RELIC_RESULT&&g->relic_pending&&g->reward[0]<0)g->reward[0]=37;
 return game_validate(g);
}
/* A durable death receipt blocks both alternating snapshots before unlinking.
   A surviving old generation can therefore never revive a dead run. */
static unsigned ascent_dead_id(int slot){char stem[40];unsigned id=0,gen;snprintf(stem,sizeof(stem),"ASCENT18_DEAD_%d",slot+1);if(!ascent_read(stem,0,&id,sizeof(id),&gen))return 0;return id;}
int ascent_forget(Game*g){
 if(!g->ascent.id||g->ascent.slot>=ASCENT_SLOTS)return 0;
 unsigned slot=g->ascent.slot,id=g->ascent.id;char stem[40],path[320];
 snprintf(stem,sizeof(stem),"ASCENT18_DEAD_%u",slot+1);
 if(!ascent_write(stem,&id,sizeof(id),0))return 0;
 snprintf(stem,sizeof(stem),"ASCENT18_RUN_%u",slot+1);
 for(int side=0;side<2;side++)if(ascent_path(path,sizeof(path),stem,side))remove(path);
 memset(&ascent_slots[slot],0,sizeof(ascent_slots[slot]));
 int any=0;for(int i=0;i<ASCENT_SLOTS;i++)any|=ascent_slots[i].present;game_set_resume_available(any);return 1;
}
int ascent_resume(Game*g,int slot){
 if(slot<0||slot>=ASCENT_SLOTS)return 0;
 char stem[40];snprintf(stem,sizeof(stem),"ASCENT18_RUN_%d",slot+1);
 Game a,b;unsigned ga=0,gb=0;int va=ascent_read_run(stem,0,&a,&ga),vb=ascent_read_run(stem,1,&b,&gb);
 unsigned dead=ascent_dead_id(slot);
 if(va&&(a.screen==DEFEAT||a.hp<=0)){dead=MAX(dead,a.ascent.id);ascent_forget(&a);}
 if(vb&&(b.screen==DEFEAT||b.hp<=0)){dead=MAX(dead,b.ascent.id);ascent_forget(&b);}
 va=va&&a.screen!=DEFEAT&&a.hp>0&&a.ascent.id>dead&&game_validate(&a)&&a.ascent.id&&a.ascent.slot==(unsigned)slot;
 vb=vb&&b.screen!=DEFEAT&&b.hp>0&&b.ascent.id>dead&&game_validate(&b)&&b.ascent.id&&b.ascent.slot==(unsigned)slot;
 if(!va&&!vb)return 0;
 *g=vb&&(!va||gb>ga)?b:a;unsigned gen=vb&&(!va||gb>ga)?gb:ga;
 ascent_slot_info(slot,g,gen);return 1;
}
int ascent_checkpoint(Game*g){
 if(g->screen==DEFEAT||g->hp<=0)return ascent_forget(g);
 if(!ascent_ready||!g->ascent.id||g->ascent.slot>=ASCENT_SLOTS||!game_validate(g))return 0;
 char stem[40];unsigned slot=g->ascent.slot,gen=ascent_slots[slot].generation+1;
 snprintf(stem,sizeof(stem),"ASCENT18_RUN_%u",slot+1);
 if(!ascent_write(stem,g,sizeof(*g),gen))return 0;
 Game verify;unsigned vg;
 if(!ascent_read(stem,gen&1,&verify,sizeof(verify),&vg)||vg!=gen||memcmp(&verify,g,sizeof(*g)))return 0;
 ascent_slot_info(slot,g,gen);return 1;
}
int ascent_open(const char*root){
 if(!root||strlen(root)>=sizeof(ascent_root))return 0;
 snprintf(ascent_root,sizeof(ascent_root),"%s",root);
 AscentAccount a,b;unsigned ga=0,gb=0;
 int va=ascent_read("ASCENT18_ACCOUNT",0,&a,sizeof(a),&ga),vb=ascent_read("ASCENT18_ACCOUNT",1,&b,sizeof(b),&gb);
 va=va&&ascent_account_valid(&a);vb=vb&&ascent_account_valid(&b);
 memset(&ascent_account,0,sizeof(ascent_account));
 if(va||vb){ascent_account=vb&&(!va||gb>ga)?b:a;ascent_account.generation=vb&&(!va||gb>ga)?gb:ga;}
 else {ascent_account.magic=0x52413841u;ascent_account.version=1;ascent_account.next_run=1;}
 ascent_ready=1;ascent_fast_setting=0;{unsigned gen;ascent_read("ASCENT18_SPEED",0,&ascent_fast_setting,sizeof(ascent_fast_setting),&gen);if(ascent_fast_setting>1)ascent_fast_setting=0;}memset(ascent_slots,0,sizeof(ascent_slots));
 for(int i=0;i<ASCENT_SLOTS;i++){Game saved;ascent_resume(&saved,i);}
 /* Import the stable single-save prefix once. The old SAVE.DAT is untouched. */
 if(!va&&!vb&&!ascent_slots[0].present){
  char path[320];Game oldrun;snprintf(path,sizeof(path),"%s/SAVE.DAT",ascent_root);
  if(game_load(&oldrun,path)&&oldrun.screen!=TITLE&&oldrun.screen!=DEFEAT){
   oldrun.ascent.id=ascent_account.next_run++;oldrun.ascent.slot=0;
   for(int i=0;i<2;i++)oldrun.ascent.unlocked_cards[i]=oldrun.ascent.unlocked_potions[i]=-1;
   oldrun.ascent.unlocked_relic=-1;
   if(ascent_commit_account())ascent_checkpoint(&oldrun);
  }
 }
 return 1;
}
static int ascent_begin_mode(Game*g,int slot,int character,unsigned seed,int custom){
 if(!ascent_ready||slot<0||slot>=ASCENT_SLOTS||character<0||character>2)return 0;
 AscentAccount old=ascent_account;
 if(ascent_account.next_run==0xffffffffu)return 0;
 unsigned id=ascent_account.next_run++;
 if(!ascent_commit_account()){ascent_account=old;return 0;}
 game_new(g,seed);game_choose_character(g,character);
 g->ascent.id=id;g->ascent.slot=(unsigned)slot;g->ascent.cards=ascent_account.cards[character];
 g->ascent.potions=ascent_account.potions;g->ascent.relics=ascent_account.relics;g->ascent.secret=ascent_secret(character);
 for(int i=0;i<2;i++)g->ascent.unlocked_cards[i]=g->ascent.unlocked_potions[i]=-1;
 g->ascent.unlocked_relic=-1;begin_act(g);
 if(custom){g->revision184.custom=g->revision184.building=1;g->ascent.cards=ASCENT_ALL_CARDS;g->ascent.potions=0xfffff;g->ascent.relics=1023;
  g->deck_count=g->relic_count=g->potion_count=0;g->screen=CUSTOM_CARDS;g->selection=g->scroll=0;msg(g,"");}

 return ascent_checkpoint(g);
}
int ascent_begin(Game*g,int slot,int character,unsigned seed){return ascent_begin_mode(g,slot,character,seed,0);}
int ascent_card_available(const Game*g,int id){
 if(g->revision184.custom)return id>=0&&id<ASCENT_CARD_LAST;
 if(id<ASCENT_CARD_FIRST)return 1;if(id>=ASCENT_CARD_LAST)return 0;
 return (id-ASCENT_CARD_FIRST)/20==g->character&&(g->ascent.cards&(1u<<((id-ASCENT_CARD_FIRST)%20)));
}
static int ascent_random_potion(Game*g){
 int pool[26],n=0;for(int i=0;i<26;i++)if(i<6||(g->ascent.potions&(1u<<(i-6))))pool[n++]=i;
 return pool[rnd(g,n)];
}
static int ascent_pick_locked(unsigned mask,int start,int count,Game*g){
 int ids[20],n=0;for(int i=start;i<start+count;i++)if(!(mask&(1u<<i)))ids[n++]=i;
 return n?ids[rnd(g,n)]:-1;
}
/* Pick a random DIFFERENT-rarity pair only when the remaining distribution can
 * still be paired. This guarantees 20 unique cards on exactly ten victories. */
static void ascent_unlock_cards(Game*g,int out[2]){
 unsigned *mask=&ascent_account.cards[g->character];int starts[4]={0,6,12,18},sizes[4]={6,6,6,2};
 int remaining[4],pairs[6][2],n=0,total=0;
 for(int r=0;r<4;r++){remaining[r]=0;for(int i=starts[r];i<starts[r]+sizes[r];i++)remaining[r]+=!(*mask&(1u<<i));total+=remaining[r];}
 out[0]=out[1]=-1;if(!total)return;
 for(int a=0;a<4;a++)for(int b=a+1;b<4;b++)if(remaining[a]&&remaining[b]){
  int possible=1;for(int r=0;r<4;r++)if(remaining[r]-(r==a)-(r==b)>(total-2)/2)possible=0;
  if(possible){pairs[n][0]=a;pairs[n++][1]=b;}
 }
 if(!n)return;int which=rnd(g,n);
 for(int j=0;j<2;j++){int r=pairs[which][j],id=ascent_pick_locked(*mask,starts[r],sizes[r],g);*mask|=1u<<id;out[j]=ASCENT_CARD_FIRST+20*g->character+id;}
}
static void ascent_receipt_to_run(Game*g,const AscentReceipt*r){
 g->ascent.won=r->won;g->ascent.ascended=r->ascended;
 for(int i=0;i<2;i++){g->ascent.unlocked_cards[i]=r->cards[i];g->ascent.unlocked_potions[i]=r->potions[i];}
 g->ascent.unlocked_relic=r->relic;g->ascent.secret=ascent_secret(g->character);
}
static int ascent_complete(Game*g,int true_end){
 if(preview_trace)return 1;
 if(g->revision184.custom){if(true_end)g->ascent.ascended=1;else g->ascent.won=1;return 1;}
 if(!g->ascent.id||g->ascent.slot>=ASCENT_SLOTS)return 0;
 AscentAccount old=ascent_account;AscentReceipt*r=&ascent_account.receipts[g->ascent.slot];
 if(r->run>g->ascent.id){msg(g,"This run has been superseded in its save slot.");return 0;}
 if(r->run!=g->ascent.id){memset(r,0,sizeof(*r));r->run=g->ascent.id;r->cards[0]=r->cards[1]=r->potions[0]=r->potions[1]=r->relic=-1;}
 if(true_end){if(!r->ascended){r->ascended=1;ascent_account.ascended|=1u<<g->character;}}
 else if(!r->won){
  r->won=1;ascent_account.wins[g->character]++;ascent_unlock_cards(g,r->cards);
  for(int i=0;i<2;i++){int id=ascent_pick_locked(ascent_account.potions,0,20,g);r->potions[i]=id<0?-1:id+6;if(id>=0)ascent_account.potions|=1u<<id;}
  int id=ascent_pick_locked(ascent_account.relics,0,10,g);r->relic=id<0?-1:id+64;if(id>=0)ascent_account.relics|=1u<<id;
 }
 if((unsigned)g->act>ascent_account.best_act[g->character])ascent_account.best_act[g->character]=(unsigned)g->act;
 if(memcmp(&old,&ascent_account,sizeof(old))&&!ascent_commit_account()){ascent_account=old;msg(g,"Could not save unlocks. Press X to retry.");return 0;}
 ascent_receipt_to_run(g,r);return 1;
}
static void ascent_record_act(Game*g){
 if(preview_trace||!g->ascent.id||g->revision184.custom)return;
 unsigned old=ascent_account.best_act[g->character];
 if((unsigned)g->act>old){ascent_account.best_act[g->character]=g->act;if(!ascent_commit_account())ascent_account.best_act[g->character]=old;}
}
