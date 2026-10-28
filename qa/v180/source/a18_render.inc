/* A1.8 pages use the existing Gothic UI textures and font renderer. Every
 * content picture below resolves a separately generated, hashed asset. */
static const char*a18_char_names[3]={"WARDEN","VESPER","PROXY"};
static const char*a18_rarity_names[4]={"COMMON","UNCOMMON","RARE","ANCIENT"};
static const char*a18_dialogue[3][4][3]={
 {
  {"I gave your gods every chance to kneel.","They chose the floor.","This throne is a grave. I am done burying myself in it."},
  {"Ten victories. Three broken kingdoms. Still I hear the chain.","The throne was never the master. Something beyond it holds the leash.","Then I go beyond it. Let the last jailer learn my name."},
  {"THE IRON JAILER: Every oath you broke was another link in my chain.","WARDEN: Then you should have built it from something stronger than gods.","Keep your key. I brought a hammer."},
  {"The last chain breaks. For the first time, the world does not pull him back.","No throne. No jailer. No voice telling me what I am.","I did not come this far to rule the ruins. I came to walk out of them."}
 },
 {
  {"You built a kingdom on the backs of the forgotten.","I learned every name you tried to erase.","Now listen to the silence where yours used to be."},
  {"Ten times I cut the throat of this kingdom. Ten times it learned to speak again.","There is a witness behind the stars, writing every ending before I reach it.","Let it watch closely. This is the last page it gets."},
  {"THE SILENT WITNESS: I saw your death before your first breath.","VESPER: Then you should have looked behind you.","You do not get to write my ending."},
  {"The witness falls silent. Every stolen name returns with the dawn.","No one remembers the hand that opened the cage.","Good. Let them remember the sky."}
 },
 {
  {"I wore your crown because you mistook obedience for consent.","Your stars answer to no king.","Neither do I."},
  {"Ten crowns shattered. And still the heavens demand a sovereign.","There was a first king before the stars. Every throne is only his reflection.","Then I will teach the heavens to live without one."},
  {"THE FIRST SOVEREIGN: Everything you command was borrowed from me.","PROXY: Then collect your debt.","I will pay you in broken stars."},
  {"The first crown fractures. Across the void, a thousand empty thrones turn to dust.","The stars are not subjects. They never were.","Let them burn without permission."}
 }
};
static void a18_heading(const char*title,const char*subtitle){
 dark_rect(0,0,W,H,100);gilded_panel(10,9,460,243,1);
 r134_center(240,19,title,GOLD,&r138_title10);
 if(subtitle&&*subtitle)r134_center(240,36,subtitle,MUTED,&r134_f9);
}
static void a18_story_screen(const Game*g){
 int c=CLAMP(g->character,0,2),kind=g->screen==A18_BEYOND?1:g->screen==A18_BOSS_INTRO?2:g->screen==A18_TRUE_END?3:0;
 rect(0,0,W,H,RGB(0,0,0));art_blit(0,0,480,180,a18_story_art[c][kind],480,180);
 dark_rect(0,0,480,26,135);
 char heading[80];snprintf(heading,sizeof(heading),"%s / %s",a18_char_names[c],kind==0?"THE THIRD THRONE":kind==1?"BEYOND THE RIFT":kind==2?"THE LAST GATE": "TRUE ENDING");
 r134_center(240,8,heading,GOLD,&r138_title10);
 rect(0,180,480,92,RGB(4,5,10));
 r134_wrap(26,194,428,4,13,a18_dialogue[c][kind][CLAMP(g->selection,0,2)],WHITE,&r134_f10,0);
 char page[20];snprintf(page,sizeof(page),"%d / 3",g->selection+1);r134_text(28,247,page,MUTED,&r134_f8);
 r134_blit(365,242,14,14,r135_cross,14,14,255);r134_text(386,246,"CONTINUE",GOLD,&r134_f9);
}
static void a18_slots_screen(const Game*g){
 a18_heading(g->a18.menu_mode?"BEGIN ASCENT / SAVE SLOTS":"RESUME / SAVE SLOTS","All slots share character records and unlocks");
 for(int i=0;i<4;i++){
  const A18SlotInfo*s=a18_slot(i);int y=56+i*43,chosen=g->selection==i;
  if(chosen)r134_blit(28,y-2,424,40,r151_selection,420,60,245);
  char b[128];snprintf(b,sizeof(b),"SLOT %d  /  %s",i+1,s->occupied?a18_char_names[CLAMP(s->character,0,2)]:"EMPTY");
  r134_text(78,y+2,b,chosen?GOLD:WHITE,&r138_title9);
  if(s->occupied){snprintf(b,sizeof(b),"Act %d  -  Floor %d  -  HP %d/%d%s",s->act,s->floor+1,s->hp,s->maxhp,s->screen==DEFEAT?"  / FALLEN":s->screen==VICTORY?"  / COMPLETE":"");r134_wrap(78,y+20,324,1,10,b,MUTED,&r134_f8,1);}
  else r134_text(78,y+20,g->a18.menu_mode?"Start a new journey here":"No saved run",MUTED,&r134_f9);
 }
 footer("UP/DOWN SLOT    X SELECT    O BACK");
}
static void a18_overwrite_screen(const Game*g){
 a18_heading("REPLACE THIS RUN?","Account-wide wins and unlocks are never erased");
 char b[64];snprintf(b,sizeof(b),"Only the run in SLOT %d will be replaced.",g->a18.slot+1);r134_center(240,88,b,WHITE,&r134_f10);
 r134_center(240,108,"The old run remains until a new character is confirmed.",MUTED,&r134_f9);
 button(128,140,224,34,"KEEP SAVED RUN",g->selection==0);
 button(128,184,224,34,"CHOOSE NEW CHARACTER",g->selection==1);
 footer("UP/DOWN CHOOSE   X CONFIRM   O CANCEL");
}
static void a18_unlock_screen(const Game*g){
 card_combat_context=0;char subtitle[96];snprintf(subtitle,sizeof(subtitle),"%s WIN %d / Available in future runs",a18_char_names[g->character],g->a18.award_win_number);
 const char*title=g->screen==A18_UNLOCK_CARDS?"NEW CARDS UNLOCKED":g->screen==A18_UNLOCK_POTIONS?"NEW POTIONS UNLOCKED":"NEW RELIC UNLOCKED";
 a18_heading(title,subtitle);
 if(g->screen==A18_UNLOCK_CARDS){
  int any=0;
  for(int i=0;i<2;i++){int raw=g->a18.card_awards[i];if(raw<334||raw>=394)continue;any=1;
   int x=63+i*191;R135CardLayout l=r135_card_layout(raw,164);
   card_render(x,53,164,l.height,raw,1,1);
   r134_center(x+82,220,a18_rarity_names[cards[raw].rarity],cards[raw].rarity==3?PURPLE:GOLD,&r138_title8);
  }
  if(!any){r134_center(240,111,"ALL 20 CHARACTER CARDS UNLOCKED",GOLD,&r138_title10);r134_center(240,139,"Your full custom set remains in future reward pools.",WHITE,&r134_f9);}
 }else if(g->screen==A18_UNLOCK_POTIONS){
  int any=0;for(int i=0;i<2;i++){int id=g->a18.potion_awards[i];if(id<6||id>=26)continue;any=1;int x=29+i*226;
   art_blit(x+65,56,88,88,a18_potion_art[id-6],88,88);
   r134_center(x+108,151,game_potion_name(id),GOLD,&r138_title9);
   r134_wrap(x+10,173,202,5,11,game_potion_description(id),WHITE,&r134_f9,0);
  }if(!any){r134_center(240,116,"ALL 20 POTIONS UNLOCKED",GOLD,&r138_title10);r134_center(240,146,"Every new potion is available in future runs.",WHITE,&r134_f9);}
 }else{
  int id=g->a18.relic_award;
  if(id>=64&&id<74){art_blit(196,57,88,88,a18_relic_art[id-64],88,88);r134_center(240,151,relic_defs[id].name,GOLD,&r138_title10);r134_wrap(84,176,312,4,12,relic_defs[id].desc,WHITE,&r134_f10,0);}
  else {r134_center(240,116,"ALL 10 RELICS UNLOCKED",GOLD,&r138_title10);r134_center(240,146,"Every new relic is available in future runs.",WHITE,&r134_f9);}
 }
 button(168,229,144,24,"NEXT",1);footer("X CONTINUE");
}
static void a18_collection_screen(const Game*g){
 const A18Profile*p=a18_profile();int tab=g->a18.collection_tab,c=g->a18.collection_character,n=tab==2?10:20,pick=CLAMP(g->selection,0,n-1),first=CLAMP(pick-3,0,n-7);char b[128];
 a18_heading("THE ARCHIVE / UNLOCKS","");
 const char*tabs[3]={"CARDS","POTIONS","RELICS"};for(int i=0;i<3;i++)button(18+i*153,35,140,24,tabs[i],i==tab);
 unsigned mask=tab==0?p->cards[c]:tab==1?p->potions:p->relics;
 snprintf(b,sizeof(b),tab==0?"%s  %d / 20":"%s  %d / %d",tab==0?a18_char_names[c]:tabs[tab],a18_popcount(mask),n);r134_text(28,65,b,GOLD,&r134_f9b);
 for(int i=first;i<n&&i<first+7;i++){
  int y=83+(i-first)*22,open=(mask&(1u<<i))!=0;const char*name=tab==0?cards[334+c*20+i].name:tab==1?game_potion_name(i+6):relic_defs[i+64].name;
  if(i==pick)r134_blit(23,y-3,216,22,r151_selection,420,60,240);
  snprintf(b,sizeof(b),"%02d  %s",i+1,open?name:"LOCKED");r134_text(31,y+1,b,i==pick?GOLD:open?WHITE:MUTED,&r134_f9);
 }
 int open=(mask&(1u<<pick))!=0;
 if(open){
  if(tab==0){card_combat_context=0;int raw=334+c*20+pick;R135CardLayout l=r135_card_layout(raw,178);card_render(270,67,178,l.height,raw,1,1);r134_center(359,238,a18_rarity_names[cards[raw].rarity],GOLD,&r134_f9);}
  else{const char*name=tab==1?game_potion_name(pick+6):relic_defs[pick+64].name;const char*rules=tab==1?game_potion_description(pick+6):relic_defs[pick+64].desc;
   art_blit(308,73,88,88,tab==1?a18_potion_art[pick]:a18_relic_art[pick],88,88);r134_wrap(256,167,195,2,13,name,GOLD,&r138_title10,0);r134_wrap(256,196,195,5,11,rules,WHITE,&r134_f9,0);}
 }else{
  gilded_panel(258,77,196,156,0);r134_center(356,108,"LOCKED",MUTED,&r138_title10);
  r134_wrap(278,137,156,6,13,tab==0?"Defeat Act 3 with this character to reveal two cards of different rarities.":tab==1?"Defeat Act 3 with any character to reveal two new potions.":"Defeat Act 3 with any character to reveal one new relic.",WHITE,&r134_f10,0);
 }
 footer(tab==0?"LEFT/RIGHT TAB   L/R CHARACTER   UP/DOWN ITEM   O BACK":"LEFT/RIGHT TAB   UP/DOWN ITEM   O BACK");
}
static int a18_render_screen(const Game*g){
 if(g->screen==A18_SLOTS)a18_slots_screen(g);
 else if(g->screen==A18_SLOT_CONFIRM)a18_overwrite_screen(g);
 else if(g->screen==A18_COLLECTION)a18_collection_screen(g);
 else if(g->screen==A18_ENDING||g->screen==A18_BEYOND||g->screen==A18_BOSS_INTRO||g->screen==A18_TRUE_END)a18_story_screen(g);
 else if(g->screen==A18_UNLOCK_CARDS||g->screen==A18_UNLOCK_POTIONS||g->screen==A18_UNLOCK_RELIC)a18_unlock_screen(g);
 else return 0;return 1;
}
