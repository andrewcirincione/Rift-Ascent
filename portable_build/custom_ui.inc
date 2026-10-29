static void custom_item_preview(int id,int card,int potion){
 if(id<0)return;
 if(card)card_render(315,76,143,164,id,1,1);
 else{if(potion)ascent_potion_icon(358,83,56,id);else r150b_relic_icon(358,83,56,id);
  r134_wrap(315,145,142,3,12,potion?game_potion_name(id):relic_defs[id].name,GOLD,&r134_f9b,0);
  r134_wrap(315,184,142,5,11,potion?game_potion_description(id):relic_defs[id].desc,WHITE,&r134_f9,0);
 }
}
static void custom_view(const Game*g){
 char b[128];int cards_page=g->screen==CUSTOM_CARDS||(g->screen==CUSTOM_QUANTITY&&g->prior_screen==CUSTOM_CARDS),tab=g->revision184.tab;
 ascent_heading(g->screen==CUSTOM_CONFIRM?"CUSTOM RUN / READY?":cards_page?"CUSTOM RUN / BUILD YOUR DECK":"CUSTOM RUN / STARTING ITEMS");
 if(g->screen==CUSTOM_CONFIRM){
  snprintf(b,sizeof(b),"%s  /  %d CARDS  /  %d RELICS  /  %d POTIONS",ascent_names[g->character],g->deck_count,g->relic_count,g->potion_count);r134_center(240,60,b,GOLD,&r134_f8);
  r134_center(240,83,"Custom runs do not award unlocks or character records.",WHITE,&r134_f8);
  button(100,112,280,31,"START RUN",g->selection==0);button(100,153,280,31,"BACK TO LOADOUT",g->selection==1);button(100,194,280,31,"DISCARD SETUP",g->selection==2);
  if(g->message[0])r134_center(240,235,g->message,RED,&r134_f8);footer("X CONFIRM   O BACK");return;
 }
 if(g->screen==CUSTOM_QUANTITY){
  int id=g->revision184.item,remove=cards_page?tab==3:tab>=2;
  custom_item_preview(id,cards_page,!cards_page&&(tab==1||tab==3));
  r134_center(163,75,remove?"REMOVE COPIES":"ADD COPIES",GOLD,&r138_title10);
  snprintf(b,sizeof(b),"%d",g->revision184.quantity);r134_center(163,118,b,WHITE,&r134_f18b);
  r134_center(92,118,"-",GOLD,&r134_f18b);r134_center(234,118,"+",GOLD,&r134_f18b);
  snprintf(b,sizeof(b),"CURRENTLY SELECTED: %d",custom_copies(g,tab,id));r134_center(163,161,b,MUTED,&r134_f8);
  if(cards_page&&tab<3)r134_center(163,188,CARD_UP(id)?"UPGRADED":"BASE CARD",GOLD,&r134_f9);
  if(!cards_page&&(tab==0||tab==2))r134_wrap(42,183,243,2,11,"Copies repeat pickup bonuses. Passive effects keep their normal rules.",MUTED,&r134_f8,0);
  if(g->message[0])r134_wrap(38,216,248,2,11,g->message,RED,&r134_f8,0);
  footer(cards_page&&tab<3?"X ADD   SQ UPGRADE   O CANCEL":remove?"X REMOVE   O CANCEL":"X ADD   O CANCEL");return;
 }
 const char*card_tabs[]={"WARDEN","VESPER","PROXY","YOUR DECK"};
 const char*item_tabs[]={"RELICS","POTIONS","YOUR RELICS","YOUR POTIONS"};
 for(int i=0;i<4;i++)button(23+i*109,45,106,23,cards_page?card_tabs[i]:item_tabs[i],tab==i);
 int count=custom_count(g,tab),first=g->selection/6*6,remove=cards_page?tab==3:tab>=2;
 for(int i=first;i<count&&i<first+6;i++){
  int id=custom_id(g,tab,i),selected=i==g->selection,x=24+(i-first)%3*92,y=85+(i-first)/3*74-(selected?5:0);
  gilded_panel(x,y,85,69,selected);
  if(cards_page)r134_card_art(x+8,y+5,69,32,id);
  else if(tab==1||tab==3)ascent_potion_icon(x+23,y+3,38,id);else r150b_relic_icon(x+23,y+3,38,id);
  const char*name=cards_page?cards[CARD_ID(id)].name:tab==1||tab==3?game_potion_name(id):relic_defs[id].name;
  r134_wrap(x+8,y+39,69,2,9,name,selected?WHITE:MUTED,&r134_f8,1);
  int copies=custom_copies(g,tab,id);if(copies){snprintf(b,sizeof(b),"x%d",copies);r135_shadow_text(x+57,y+6,b,GOLD,&r134_f8);}
  if(selected)r134_blit(x+23,y+1,40,23,r138_choice,42,28,255);else dark_rect(x+5,y+5,75,59,45);
 }
 if(count)custom_item_preview(custom_id(g,tab,g->selection),cards_page,!cards_page&&(tab==1||tab==3));
 else r134_center(163,138,"NOTHING SELECTED",MUTED,&r134_f9);
 snprintf(b,sizeof(b),cards_page?"DECK %d / 128":"RELICS %d / 96   POTIONS %d / 3",cards_page?g->deck_count:g->relic_count,g->potion_count);
 r134_text(28,235,g->message[0]?g->message:b,g->message[0]?RED:GOLD,&r134_f8);
 footer(cards_page?(remove?"L/R TABS  X REMOVE  START NEXT  O BACK":"L/R TABS  X ADD  START NEXT  O BACK"):(remove?"L/R TABS  X REMOVE  START READY  O BACK":"L/R TABS  X ADD  START READY  O BACK"));
}
