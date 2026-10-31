/* Relocate the two expanded arrays without reinterpreting older snapshots. */
static int migrate_layout184(Game*out,const void*data,size_t n){
 Game184 old;memset(&old,0,sizeof(old));if(n>sizeof(old)||n<16)return 0;memcpy(&old,data,n);
 size_t expected=old.version==184?sizeof(old):old.version==182||old.version==183?offsetof(Game184,revision184):old.version==181?offsetof(Game184,revision182):old.version==180?offsetof(Game184,expedition):old.version==23?offsetof(Game184,ascent):old.version==22?offsetof(Game184,regent):old.version==21?offsetof(Game184,v151):0;
 if(!expected||n!=expected||old.enemy_count<0||old.enemy_count>2)return 0;
 memset(out,0,sizeof(*out));
 memcpy(out,&old,offsetof(Game184,enemies));
 memcpy(out->enemies,old.enemies,sizeof(old.enemies));
 memcpy(&out->next_screen,&old.next_screen,offsetof(Game184,regent)-offsetof(Game184,next_screen));
 memcpy(&out->regent,&old.regent,offsetof(RegentState184,hit_count));
 memcpy(out->regent.hit_count,old.regent.hit_count,sizeof(old.regent.hit_count));
 memcpy(out->regent.conqueror,old.regent.conqueror,sizeof(old.regent.conqueror));
 memcpy(&out->regent.vigor,&old.regent.vigor,sizeof(RegentState184)-offsetof(RegentState184,vigor));
 memcpy(&out->ascent,&old.ascent,sizeof(Game184)-offsetof(Game184,ascent));
 out->version=GAME_VERSION;
 /* An interrupted beta reward forge returns to its original unclaimed spoils. */
 if(out->screen==BETA_FORGE||out->screen==BETA_FORGE_CARD){out->screen=REWARD;out->selection=0;}
 if(out->prior_screen==BETA_FORGE||out->prior_screen==BETA_FORGE_CARD)out->prior_screen=REWARD;
 if(out->screen==REWARD&&out->selection==3)out->selection=5;
 if(old.version==21)out->v151.potion_offer=-1;
 if(old.version==182&&out->screen==ASCENT_COLLECTION&&out->scroll==5){out->scroll=4;out->selection+=70;}
 if(old.version<182){out->revision182.rare_offset=-5;revision182_migrate_room(out);}
 if(old.version==180&&out->screen==RELIC_RESULT&&out->relic_pending&&out->reward[0]<0)out->reward[0]=37;
 return 1;
}
