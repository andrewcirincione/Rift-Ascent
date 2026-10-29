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
 ascent_heading(g->scroll?"BEGIN ASCENT / SAVE SLOT":"RESUME / SAVE SLOT");
 for(int i=0;i<ASCENT_SLOTS;i++){
  const AscentSlot*s=ascent_slot(i);int y=46+i*45;char b[100];
  if(i==g->selection)r134_blit(27,y,426,42,r140_selection,208,28,255);
  snprintf(b,sizeof(b),"SLOT %d   %s",i+1,s->present?ascent_names[CLAMP(s->character,0,2)]:"EMPTY");r134_text(44,y+7,b,i==g->selection?GOLD:WHITE,&r134_f9b);
  if(s->present){snprintf(b,sizeof(b),"Act %d / Floor %d     HP %d / %d",s->act,s->floor+1,s->hp,s->maxhp);r134_text(44,y+23,b,MUTED,&r134_f9);}
 }
 r134_center(240,235,g->message[0]?g->message:"Unlocks are shared across all four save slots.",GOLD,&r134_f8);
 footer("UP/DOWN CHOOSE   X CONFIRM   O BACK");
}
static void ascent_overwrite_view(const Game*g){
 ascent_heading("REPLACE THIS SAVED RUN?");char b[80];snprintf(b,sizeof(b),"SLOT %d",g->master_index+1);
 r134_center(240,61,b,GOLD,&r138_title10);
 r134_center(240,87,"Only this run will be replaced.",WHITE,&r134_f9);
 r134_center(240,104,"All character unlocks and wins are kept.",MUTED,&r134_f9);
 button(131,133,218,31,"KEEP SAVED RUN",g->selection==0);
 button(131,179,218,31,"BEGIN NEW RUN",g->selection==1);
 footer("UP/DOWN CHOOSE   X CONFIRM   O CANCEL");
}
static void ascent_options_view(const Game*g){
 ascent_heading("OPTIONS");button(100,81,280,42,ascent_debug()?"DEBUG SHORTCUTS: ON":"DEBUG SHORTCUTS: OFF",1);
 r134_center(240,147,"L: defeat all enemies during combat",WHITE,&r134_f9);
 r134_center(240,168,"R: advance to the next act",WHITE,&r134_f9);
 r134_center(240,207,g->message[0]?g->message:"Debug is off by default and saved for all slots.",MUTED,&r134_f8);
 footer("X TOGGLE   O BACK");
}
static unsigned ascent_collection_mask(int tab){const AscentAccount*a=ascent_profile();return tab<3?a->cards[tab]:tab==3?a->potions:a->relics;}
static void ascent_collection_view(const Game*g){
 const char*labels[]={"WARDEN","VESPER","PROXY","POTIONS","RELICS"};
 int tab=CLAMP(g->scroll,0,4),count=tab==4?10:20,index=CLAMP(g->selection,0,count-1),first=index/6*6;
 unsigned mask=ascent_collection_mask(tab);char b[80];snprintf(b,sizeof(b),"UNLOCKS / %s  %d / %d",labels[tab],ascent_popcount(mask),count);ascent_heading(b);
 for(int i=0;i<5;i++){int x=21+i*89;r134_text(x,43,labels[i],i==tab?GOLD:MUTED,&r134_f8);}
 for(int n=first;n<count&&n<first+6;n++){
  int pos=n-first,x=24+pos%3*92,y=70+pos/3*82,open=(mask&(1u<<n))!=0;
  gilded_panel(x,y,85,76,n==index);
  if(open){if(tab<3)r134_card_art(x+8,y+9,69,46,334+tab*20+n);else if(tab==3)ascent_potion_icon(x+20,y+8,46,n+6);else r150b_relic_icon(x+20,y+8,46,n+64);}
  else r134_center(x+42,y+28,"?",MUTED,&r134_f18b);
  snprintf(b,sizeof(b),"%02d%s",n+1,open?"":" LOCKED");r134_center(x+42,y+59,b,open?GOLD:MUTED,&r134_f8);
 }
 if(mask&(1u<<index)){
  if(tab<3){int raw=334+tab*20+index;card_render(315,67,143,177,raw,1,1);}
  else{int id=index+(tab==3?6:64);if(tab==3)ascent_potion_icon(363,69,48,id);else r150b_relic_icon(363,69,48,id);
   r134_wrap(317,131,136,3,12,tab==3?game_potion_name(id):relic_defs[id].name,GOLD,&r134_f9b,0);
   r134_wrap(317,173,136,6,11,tab==3?game_potion_description(id):relic_defs[id].desc,WHITE,&r134_f9,0);
  }
 }else{r134_center(385,113,"UNDISCOVERED",GOLD,&r134_f9);r134_wrap(319,145,134,5,12,tab<3?"Complete Act 3 with this character to unlock two cards.":tab==3?"Complete Act 3 with any character to unlock two potions.":"Complete Act 3 with any character to unlock a relic.",MUTED,&r134_f9,0);}
 footer("D-PAD BROWSE   L/R CATEGORY   O BACK");
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
