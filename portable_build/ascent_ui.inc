/* Menus use the same generated frames, type and measured pixel layout as 1.71-r2. */
static const char*const ascent_names[3]={"WARDEN","VESPER","PROXY"};
static void ascent_heading(const char*title){
 dark_rect(0,0,W,H,150);gilded_panel(10,36,460,216,1);
 r134_center(240,13,title,GOLD,&r138_title10);
}
static void ascent_potion_icon(int x,int y,int size,int id){
 /* Healing, Strength, Guard, Fire, Energy and Insight share thematically
    matching bottles with the new pool instead of one universal bottle. */
 static const unsigned char legacy_art[6]={11,17,24,20,15,22};
 if(id>=0&&id<6)id=legacy_art[id];
 if(id>=6&&id<26)r134_blit(x,y,size,size,ascent_potion_art[id-6],80,80,255);
 else r134_blit(x,y,size,size,r134_utility_icons[0],40,40,255);
}
static void ascent_badges(int x,int y,int character){
 const AscentAccount*a=ascent_profile();char b[24];
 r134_blit(x+7,y+4,27,29,ascent_badge_fire,40,40,255);
 snprintf(b,sizeof(b),"%u",a->wins[character]);r135_shadow_text(x+20-r134_width(b,&r134_f8)/2,y+18,b,WHITE,&r134_f8);
 r134_blit(x+111,y+4,27,29,ascent_badge_rift,40,40,255);
 snprintf(b,sizeof(b),"%u",a->best_act[character]);r135_shadow_text(x+124-r134_width(b,&r134_f8)/2,y+18,b,WHITE,&r134_f8);
 if(a->ascended&(1u<<character))r135_shadow_text(x+64,y+31,"A",RGB(255,210,89),&r134_f18b);
}
static void ascent_slots_view(const Game*g){
 ascent_heading(g->scroll==2?"CUSTOM RUN / SAVE SLOT":g->scroll?"BEGIN ASCENT / SAVE SLOT":"RESUME / SAVE SLOT");
 for(int i=0;i<ASCENT_SLOTS;i++){
  const AscentSlot*s=ascent_slot(i);int y=46+i*45;char b[100];
  if(i==g->selection)r134_blit(27,y-2,426,42,r151_selection,420,60,255);
  snprintf(b,sizeof(b),"SLOT %d   %s%s",i+1,s->present?ascent_names[CLAMP(s->character,0,2)]:"EMPTY",s->present&&s->custom?" / CUSTOM":"");r134_text(78,y+8,b,i==g->selection?GOLD:WHITE,&r134_f9b);
  if(s->present){snprintf(b,sizeof(b),"Act %d / Floor %d     HP %d / %d",s->act,s->floor+1,s->hp,s->maxhp);r134_text(78,y+24,b,MUTED,&r134_f9);}
 }
 r134_center(240,235,g->message[0]?g->message:"Unlocks are shared across all four save slots.",GOLD,&r134_f8);
 footer("X CONFIRM   O BACK");
}
static void ascent_overwrite_view(const Game*g){
 ascent_heading("REPLACE THIS SAVED RUN?");char b[80];snprintf(b,sizeof(b),"SLOT %d",g->master_index+1);
 r134_center(240,61,b,GOLD,&r138_title10);
 r134_center(240,87,"Only this run will be replaced.",WHITE,&r134_f9);
 r134_center(240,104,"All character unlocks and wins are kept.",MUTED,&r134_f9);
 button(131,133,218,31,"KEEP SAVED RUN",g->selection==0);
 button(131,179,218,31,"BEGIN NEW RUN",g->selection==1);
 footer("X CONFIRM   O CANCEL");
}
static void ascent_options_view(const Game*g){
 ascent_heading("OPTIONS");
 button(85,66,310,35,ascent_fast()?"COMBAT SPEED: FAST":"COMBAT SPEED: NORMAL",g->selection==0);
 button(85,116,310,35,ascent_debug()?"DEBUG SHORTCUTS: ON":"DEBUG SHORTCUTS: OFF",g->selection==1);
 r134_center(240,173,g->selection?"L: defeat enemies   R: next act":"Fast shortens combat animations and action pauses.",WHITE,&r134_f8);
 r134_center(240,207,g->message[0]?g->message:"Options are saved for all slots.",MUTED,&r134_f8);
 footer("X TOGGLE   O BACK");
}
static unsigned ascent_collection_mask(int tab){const AscentAccount*a=ascent_profile();return tab<3?a->cards[tab]:tab==3?a->potions:tab==4?a->relics:0xfffffu;}
static void ascent_collection_view(const Game*g){
 const char*labels[]={"WARDEN","VESPER","PROXY","POTIONS","RELICS"};
 int tab=CLAMP(g->scroll,0,4),custom=game_collection_unlock_count(tab),count=game_collection_count(tab);
 int index=CLAMP(g->selection,0,count-1),base=index>=custom;
 int first=base?custom+(index-custom)/6*6:index/6*6,last=base?count:custom;
 unsigned mask=ascent_collection_mask(tab);char b[100];
 snprintf(b,sizeof(b),"UNLOCKS / %d OF %d",ascent_popcount(mask),custom);ascent_heading(b);
 for(int i=0;i<5;i++)button(23+i*87,45,85,23,labels[i],i==tab);
 r134_text(28,73,base?"BASE GAME / NO UNLOCK NEEDED":"EARNED BY COMPLETING ACT 3",MUTED,&r134_f8);
 for(int n=first;n<last&&n<first+6;n++){
  int pos=n-first,x=24+pos%3*92,y=83+pos/3*74,open=game_collection_unlocked(tab,n);
  int id=game_collection_id(tab,n);if(n==index)y-=5;gilded_panel(x,y,85,69,n==index);
  if(open){if(tab<3)r134_card_art(x+8,y+5,69,32,id);else if(tab==3)ascent_potion_icon(x+20,y+5,46,id);else r150b_relic_icon(x+20,y+5,46,id);}
  else r134_center(x+42,y+24,"?",MUTED,&r134_f18b);
  if(open&&tab<3)r134_wrap(x+8,y+39,69,2,9,cards[id].name,n==index?WHITE:MUTED,&r134_f8,1);
  else{snprintf(b,sizeof(b),"%02d%s",n-(base?custom:0)+1,open?"":" LOCKED");r134_center(x+42,y+55,b,open?GOLD:MUTED,&r134_f8);}
  if(n==index)r134_blit(x+23,y+1,40,23,r138_choice,42,28,255);else dark_rect(x+5,y+5,75,59,60);
 }
 if(game_collection_unlocked(tab,index)){
  int id=game_collection_id(tab,index);
  if(tab<3)card_render(315,76,143,164,id,1,1);
  else{if(tab==3)ascent_potion_icon(363,86,48,id);else r150b_relic_icon(363,86,48,id);
   r134_wrap(317,142,136,3,12,tab==3?game_potion_name(id):relic_defs[id].name,GOLD,&r134_f9b,0);
   r134_wrap(317,184,136,5,11,tab==3?game_potion_description(id):relic_defs[id].desc,WHITE,&r134_f9,0);
  }
 }else{r134_center(385,113,"UNDISCOVERED",GOLD,&r134_f9);r134_wrap(319,145,134,5,12,tab<3?"Complete Act 3 with this character to unlock two cards.":tab==3?"Complete Act 3 with any character to unlock two potions.":"Complete Act 3 with any character to unlock a relic.",MUTED,&r134_f9,0);}
 snprintf(b,sizeof(b),"BASE: %d %s%s",count-custom,tab<3?"CARDS":tab==3?"POTIONS":"RELICS",tab==2?" (53 COLORLESS)":"");
 r135_shadow_text(28,234,b,GOLD,&r134_f8);
 footer(base?"L/R TABS   SQ UNLOCKABLES   O BACK":"L/R TABS   SQ BASE GAME   O BACK");
}
static void ascent_unlock_view(const Game*g){
 int kind=g->screen==ASCENT_CARDS?0:g->screen==ASCENT_POTIONS?1:2;
 ascent_heading(kind==0?"CARDS UNLOCKED":kind==1?"POTIONS UNLOCKED":"RELIC UNLOCKED");
 int n=kind==2?1:2,any=0;
 for(int i=0;i<n;i++){
  int id=kind==0?g->ascent.unlocked_cards[i]:kind==1?g->ascent.unlocked_potions[i]:g->ascent.unlocked_relic;
  if(id<0)continue;any=1;int x=n==1?137:37+i*222;
  if(kind==0)card_render(x+30,48,145,183,id,1,1);
  else{gilded_panel(x,49,205,184,1);if(kind==1)ascent_potion_icon(x+72,62,60,id);else r150b_relic_icon(x+72,62,60,id);
   r134_wrap(x+17,132,171,2,13,kind==1?game_potion_name(id):relic_defs[id].name,GOLD,&r138_title9,0);
   r134_wrap(x+17,166,171,5,11,kind==1?game_potion_description(id):relic_defs[id].desc,WHITE,&r134_f9,0);
  }
 }
 if(!any){r134_center(240,105,"COLLECTION COMPLETE",GOLD,&r138_title10);r134_center(240,141,kind==0?"All 20 cards for this character are unlocked.":kind==1?"All 20 new potions are unlocked.":"All 10 new relics are unlocked.",WHITE,&r134_f9);}
 r134_center(240,240,"Available in future runs across every save slot.",MUTED,&r134_f8);
 footer(kind==0?"X NEXT / POTIONS":kind==1?"X NEXT / RELIC":"X CONTINUE");
}
static void ascent_scene_view(const Game*g){
 int c=CLAMP(g->character,0,2),kind=g->screen==ASCENT_ENDING?0:g->screen==ASCENT_REVELATION?1:g->screen==ASCENT_BOSS?2:3;
 rect(0,0,W,H,RGB(0,0,0));art_blit(72,38,336,126,ascent_scene_art[c][kind],336,126);
 r134_text(72,175,kind==2?"BEFORE THE FIRST THRONE":kind==3?"THE TRUE ASCENT":ascent_names[c],GOLD,&r138_title10);
 r134_wrap(72,194,336,4,12,ascent_dialogue[c][kind][CLAMP(g->selection,0,2)],WHITE,&r134_f9,0);
 r134_blit(343,247,14,14,r135_cross,14,14,255);r134_text(364,250,"CONTINUE",MUTED,&r134_f8);
}
